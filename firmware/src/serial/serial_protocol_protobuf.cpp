#include <PacketSerial.h>

#include "../proto_gen/smartknob.pb.h"

#include "proto_helpers.h"

#include "crc32.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "serial_protocol_protobuf.h"

static SerialProtocolProtobuf* singleton_for_packet_serial = 0;

static const uint16_t MIN_STATE_INTERVAL_MILLIS = 5;
static const uint16_t PERIODIC_STATE_INTERVAL_MILLIS = 5000;

SerialProtocolProtobuf::SerialProtocolProtobuf(Stream& stream, ConfigCallback config_callback) :
        SerialProtocol(),
        stream_(stream),
        config_callback_(config_callback),
        packet_serial_() {
    packet_serial_.setStream(&stream);

    // Note: not threadsafe or instance safe!! but PacketSerial requires a legacy function pointer, so we can't
    // use a member, std::function, or lambda with captures
    assert(singleton_for_packet_serial == 0);
    singleton_for_packet_serial = this;

    packet_serial_.setPacketHandler([](const uint8_t* buffer, size_t size) {
        singleton_for_packet_serial->handlePacket(buffer, size);
    });
}

void SerialProtocolProtobuf::handleState(const PB_SmartKnobState& state) {
    latest_state_ = state;
}

void SerialProtocolProtobuf::ack(uint32_t nonce) {
    pb_tx_buffer_ = {};
    pb_tx_buffer_.which_payload = PB_FromSmartKnob_ack_tag;
    pb_tx_buffer_.payload.ack.nonce = nonce;
    sendPbTxBuffer();
}

void SerialProtocolProtobuf::log(const char* msg) {
    pb_tx_buffer_ = {};
    pb_tx_buffer_.which_payload = PB_FromSmartKnob_log_tag;

    strlcpy(pb_tx_buffer_.payload.log.msg, msg, sizeof(pb_tx_buffer_.payload.log.msg));

    sendPbTxBuffer();
}

void SerialProtocolProtobuf::loop() {
    do {
        packet_serial_.update();
    } while (stream_.available());

    // Rate limit state change transmissions
    bool state_changed = !state_eq(latest_state_, last_sent_state_) && millis() - last_sent_state_millis_ >= MIN_STATE_INTERVAL_MILLIS;

    // Send state periodically or when forced, regardless of rate limit for state changes
    bool force_send_state = state_requested_ || millis() - last_sent_state_millis_ > PERIODIC_STATE_INTERVAL_MILLIS;
    if (state_changed || force_send_state) {
        state_requested_ = false;
        pb_tx_buffer_ = {};
        pb_tx_buffer_.which_payload = PB_FromSmartKnob_smartknob_state_tag;
        pb_tx_buffer_.payload.smartknob_state = latest_state_;

        sendPbTxBuffer();

        last_sent_state_ = latest_state_;
        last_sent_state_millis_ = millis();
    }
}

void SerialProtocolProtobuf::handlePacket(const uint8_t* buffer, size_t size) {
    if (size <= 4) {
        // Too small, ignore bad packet
        log("Small packet");
        return;
    }

    // Compute and append little-endian CRC32
    uint32_t expected_crc = 0;
    crc32(buffer, size - 4, &expected_crc);

    uint32_t provided_crc = buffer[size - 4]
                         | (buffer[size - 3] << 8)
                         | (buffer[size - 2] << 16)
                         | (buffer[size - 1] << 24);

    if (expected_crc != provided_crc) {
        char buf[200];
        snprintf(buf, sizeof(buf), "Bad CRC (%u byte packet). Expected %08x but got %08x.", size - 4, expected_crc, provided_crc);
        log(buf);
        return;
    }

    pb_istream_t stream = pb_istream_from_buffer(buffer, size - 4);
    if (!pb_decode(&stream, PB_ToSmartknob_fields, &pb_rx_buffer_)) {
        char buf[200];
        snprintf(buf, sizeof(buf), "Decoding failed: %s", PB_GET_ERROR(&stream));
        log(buf);
        return;
    }

    if (pb_rx_buffer_.protocol_version != PROTOBUF_PROTOCOL_VERSION) {
        char buf[200];
        snprintf(buf, sizeof(buf), "Invalid protocol version. Expected %u, received %u", PROTOBUF_PROTOCOL_VERSION, pb_rx_buffer_.protocol_version);
        log(buf);
        return;
    }

    // Always ACK immediately
    ack(pb_rx_buffer_.nonce);
    if (pb_rx_buffer_.nonce == last_nonce_) {
        // Ignore any extraneous retries
        char buf[200];
        snprintf(buf, sizeof(buf), "Already handled nonce %u", pb_rx_buffer_.nonce);
        log(buf);
        return;
    }
    last_nonce_ = pb_rx_buffer_.nonce;
    
    switch (pb_rx_buffer_.which_payload) {
        case PB_ToSmartknob_smartknob_config_tag: {
            config_callback_(pb_rx_buffer_.payload.smartknob_config);
            break;
        }
        case PB_ToSmartknob_request_state_tag:
            state_requested_ = true;
            break;
        default: {
            char buf[200];
            snprintf(buf, sizeof(buf), "Unknown payload type: %d", pb_rx_buffer_.which_payload);
            log(buf);
            return;
        }
    }
}

void SerialProtocolProtobuf::sendPbTxBuffer() {
    // Encode protobuf message to byte buffer
    pb_ostream_t stream = pb_ostream_from_buffer(tx_buffer_, sizeof(tx_buffer_));
    pb_tx_buffer_.protocol_version = PROTOBUF_PROTOCOL_VERSION;
    if (!pb_encode(&stream, PB_FromSmartKnob_fields, &pb_tx_buffer_)) {
        stream_.println(stream.errmsg);
        stream_.flush();
        assert(false);
    }

    // Compute and append little-endian CRC32
    uint32_t crc = 0;
    crc32(tx_buffer_, stream.bytes_written, &crc);
    tx_buffer_[stream.bytes_written + 0] = (crc >> 0)  & 0xFF;
    tx_buffer_[stream.bytes_written + 1] = (crc >> 8)  & 0xFF;
    tx_buffer_[stream.bytes_written + 2] = (crc >> 16) & 0xFF;
    tx_buffer_[stream.bytes_written + 3] = (crc >> 24) & 0xFF;

    // Encode and send proto+CRC as a COBS packet
    packet_serial_.send(tx_buffer_, stream.bytes_written + 4);
}
