import {encode as cobsEncode} from './streams/cobs'
import * as CRC32 from 'crc-32'

import {PB} from 'smartknobjs-proto'
import {ProtoDecoderStream} from './streams/proto-decoder'
import {DelimiterChunkedStream} from './streams/delimiter-transform'

const PROTOBUF_PROTOCOL_VERSION = 1

export type MessageCallback = (message: PB.FromSmartKnob) => void

type QueueEntry = {
    nonce: number
    encodedToSmartknobPayload: Uint8Array
}

export class SmartKnob {
    private static readonly RETRY_MILLIS = 250
    private static readonly BAUD = 921600

    private port: SerialPort | null
    private onMessage: MessageCallback

    private outgoingQueue: QueueEntry[] = []

    private lastNonce = 1
    private retryTimeout: NodeJS.Timeout | null = null
    private writer: WritableStreamDefaultWriter<Uint8Array> | undefined = undefined

    constructor(port: SerialPort, onMessage: MessageCallback) {
        this.onMessage = onMessage
        this.port = port
        this.lastNonce = Math.floor(Math.random() * (2 ^ (32 - 1)))
        port.addEventListener('disconnect', () => {
            console.log('shutting down on disconnect')
            this.port = null
            if (this.retryTimeout !== null) {
                clearTimeout(this.retryTimeout)
            }
        })
    }

    public async openAndLoop() {
        if (this.port === null) {
            return
        }
        await this.port.open({baudRate: 921600})
        if (this.port.readable === null || this.port.writable === null) {
            throw new Error('Port missing readable or writable!')
        }

        const pbDecoder = new ProtoDecoderStream()
        this.port.readable.pipeThrough(new DelimiterChunkedStream(0)).pipeTo(pbDecoder.writable)
        const reader = pbDecoder.readable.getReader()
        try {
            this.writer = this.port.writable.getWriter()
            try {
                // eslint-disable-next-line no-constant-condition
                while (true) {
                    const {value, done} = await reader.read()
                    if (done) {
                        break
                    }
                    if (value.payload === 'ack') {
                        const nonce = value.ack?.nonce ?? undefined
                        if (nonce === undefined) {
                            console.warn('Received ack without nonce')
                        } else {
                            this.handleAck(nonce)
                        }
                    }
                    this.onMessage(value)
                }
            } finally {
                console.log('Releasing writer')
                this.writer.releaseLock()
            }
        } finally {
            console.log('Releasing reader')
            reader.releaseLock()
        }
    }

    public sendConfig(config: PB.SmartKnobConfig): void {
        this.sendMessage(
            PB.ToSmartknob.create({
                smartknobConfig: config,
            }),
        )
    }

    private sendMessage(message: PB.ToSmartknob) {
        if (this.port === null) {
            return
        }
        message.protocolVersion = PROTOBUF_PROTOCOL_VERSION
        message.nonce = this.lastNonce++

        // Encode before enqueueing to ensure messages don't change once they're queued
        const payload = PB.ToSmartknob.encode(message).finish()

        if (this.outgoingQueue.length > 10) {
            console.warn(`SmartKnob outgoing queue overflowed! Dropping ${this.outgoingQueue.length} pending messages!`)
            this.outgoingQueue.length = 0
        }
        this.outgoingQueue.push({
            nonce: message.nonce,
            encodedToSmartknobPayload: payload,
        })
        this.serviceQueue()
    }

    private handleAck(nonce: number): void {
        if (this.outgoingQueue.length > 0 && nonce === this.outgoingQueue[0].nonce) {
            if (this.retryTimeout !== null) {
                clearTimeout(this.retryTimeout)
                this.retryTimeout = null
            }
            this.outgoingQueue.shift()
            this.serviceQueue()
        } else {
            console.log(`Ignoring unexpected ack for nonce ${nonce}`)
        }
    }

    private serviceQueue(): void {
        if (this.port === null) {
            return
        }
        if (this.retryTimeout !== null) {
            // Retry is pending; let the pending timeout handle the next step
            return
        }
        if (this.outgoingQueue.length === 0) {
            return
        }
        const {encodedToSmartknobPayload: payload} = this.outgoingQueue[0]

        const crc = CRC32.buf(payload)
        const crcArray = [crc & 0xff, (crc >>> 8) & 0xff, (crc >>> 16) & 0xff, (crc >>> 24) & 0xff]

        const packet = new Uint8Array(payload.length + 4)
        packet.set(payload, 0)
        packet.set(crcArray, payload.length)

        const cobsEncodedPacket = cobsEncode(packet)

        const encodedDelimitedPacket = new Uint8Array(cobsEncodedPacket.length + 1)
        encodedDelimitedPacket.set(cobsEncodedPacket, 0)
        encodedDelimitedPacket.set([0], cobsEncodedPacket.length)

        this.retryTimeout = setTimeout(() => {
            this.retryTimeout = null
            console.log(`Retrying ToSmartknob...`)
            this.serviceQueue()
        }, SmartKnob.RETRY_MILLIS)

        console.debug(
            `Sent ${payload.length} byte payload with CRC ${(crc >>> 0).toString(16)} (${
                cobsEncodedPacket.length
            } bytes encoded)`,
            encodedDelimitedPacket,
        )
        this.writer?.write(encodedDelimitedPacket).catch((e) => {
            console.error('Error writing serial', e)
            if (this.retryTimeout !== null) {
                clearTimeout(this.retryTimeout)
                this.retryTimeout = null
            }
        })
    }
}
