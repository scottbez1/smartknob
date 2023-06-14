import SerialPort = require('serialport')
import {decode as cobsDecode, encode as cobsEncode} from 'cobs'
import * as CRC32 from 'crc-32'

import {PB} from 'smartknobjs-proto'

const PROTOBUF_PROTOCOL_VERSION = 1

export type MessageCallback = (message: PB.FromSmartKnob) => void

type QueueEntry = {
    nonce: number
    encodedToSmartknobPayload: Uint8Array
}

const sleep = (millis: number) => {
    return new Promise((resolve) => {
        setTimeout(resolve, millis)
    })
}

export class SmartKnob {
    private static readonly RETRY_MILLIS = 250
    private static readonly BAUD = 921600

    private port: SerialPort | null
    private onMessage: MessageCallback
    private buffer: Buffer

    private outgoingQueue: QueueEntry[] = []

    private lastNonce = 1
    private retryTimeout: NodeJS.Timeout | null = null

    private currentConfig: PB.SmartKnobConfig

    constructor(serialPath: string | null, onMessage: MessageCallback) {
        this.onMessage = onMessage

        this.buffer = Buffer.alloc(0)

        if (serialPath !== null) {
            this.port = new SerialPort(serialPath, {
                baudRate: SmartKnob.BAUD,
            })
            this.port.on('data', (data: Buffer) => {
                this.buffer = Buffer.concat([this.buffer, data])
                this.processBuffer()
            })
        } else {
            this.port = null
        }

        this.currentConfig = PB.SmartKnobConfig.create({})

        this.lastNonce = Math.floor(Math.random() * (2 ^ (32 - 1)))
    }

    /**
     * Perform a hard reset of the MCU. Takes a few seconds.
     */
    public async hardReset(): Promise<void> {
        if (this.port === null) {
            console.warn("Not connected to SmartKnob, so hard reset isn't possible")
            return
        }
        this.outgoingQueue = []
        this.port.set({rts: true, dtr: false})
        await sleep(200)
        this.port.set({rts: true, dtr: true})
        await sleep(200)
        return
    }

    public sendConfig(config: PB.SmartKnobConfig): void {
        this.sendMessage(
            PB.ToSmartknob.create({
                smartknobConfig: config,
            }),
        )
    }

    private processBuffer(): void {
        let i: number
        // Iterate 0-delimited packets
        while ((i = this.buffer.indexOf(0)) != -1) {
            const raw_buffer = this.buffer.slice(0, i)
            const packet = cobsDecode(raw_buffer) as Buffer
            this.buffer = this.buffer.slice(i + 1)
            if (packet.length <= 4) {
                console.debug(`Received short packet ${this.buffer.slice(0, i)}`)
                continue
            }
            const payload = packet.slice(0, packet.length - 4)

            // Validate CRC32
            const crc_buf = packet.slice(packet.length - 4, packet.length)
            const provided_crc = crc_buf[0] | (crc_buf[1] << 8) | (crc_buf[2] << 16) | (crc_buf[3] << 24)
            const crc = CRC32.buf(payload)
            if (crc !== provided_crc) {
                console.debug(`Bad CRC. Expected ${crc} but received ${provided_crc}`)
                console.debug(raw_buffer.toString())
                continue
            }

            let message: PB.FromSmartKnob
            try {
                message = PB.FromSmartKnob.decode(payload)
            } catch (err) {
                console.warn(`Invalid protobuf message ${payload}`)
                return
            }

            if (message.protocolVersion !== PROTOBUF_PROTOCOL_VERSION) {
                console.warn(
                    `Invalid protocol version. Expected ${PROTOBUF_PROTOCOL_VERSION}, received ${message.protocolVersion}`,
                )
                return
            }

            if (message.payload === 'ack') {
                const nonce = message.ack?.nonce ?? undefined
                if (nonce === undefined) {
                    console.warn('Received ack without nonce')
                } else {
                    this.handleAck(nonce)
                }
            }

            this.onMessage(message)
        }
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
            console.debug(`Ignoring unexpected ack for nonce ${nonce}`)
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
        const crcBuffer = Buffer.from([crc & 0xff, (crc >>> 8) & 0xff, (crc >>> 16) & 0xff, (crc >>> 24) & 0xff])
        const packet = Buffer.concat([payload, crcBuffer])

        const encodedDelimitedPacket = Buffer.concat([cobsEncode(packet), Buffer.from([0])])

        this.retryTimeout = setTimeout(() => {
            this.retryTimeout = null
            console.log(`Retrying ToSmartknob...`)
            this.serviceQueue()
        }, SmartKnob.RETRY_MILLIS)

        console.debug(`Sent ${payload.length} byte packet with CRC ${(crc >>> 0).toString(16)}`)
        this.port.write(encodedDelimitedPacket)
    }
}
