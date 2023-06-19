import {encode as cobsEncode, decode as cobsDecode} from './cobs'
import * as CRC32 from 'crc-32'

import {PB} from 'smartknobjs-proto'

const PROTOBUF_PROTOCOL_VERSION = 1

export type MessageCallback = (message: PB.FromSmartKnob) => void
export type SendPacket = (packet: Uint8Array) => void

type QueueEntry = {
    nonce: number
    encodedToSmartknobPayload: Uint8Array
}

export {cobsEncode, cobsDecode}

export class SmartKnobCore {
    private static readonly RETRY_MILLIS = 250
    public static readonly BAUD = 921600

    private onMessage: MessageCallback
    private sendPacket: SendPacket

    private outgoingQueue: QueueEntry[] = []

    private lastNonce = 1
    private retryTimeout: ReturnType<typeof setTimeout> | null = null
    protected portAvailable = false

    constructor(onMessage: MessageCallback, sendPacket: SendPacket) {
        this.lastNonce = Math.floor(Math.random() * (2 ^ (32 - 1)))
        this.onMessage = onMessage
        this.sendPacket = sendPacket
    }

    public sendConfig(config: PB.SmartKnobConfig): void {
        this.enqueueMessage(
            PB.ToSmartknob.create({
                smartknobConfig: config,
            }),
        )
    }

    protected handleMessage(message: PB.FromSmartKnob): void {
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

    private enqueueMessage(message: PB.ToSmartknob) {
        if (!this.portAvailable) {
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
        if (!this.portAvailable) {
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
        }, SmartKnobCore.RETRY_MILLIS)

        console.debug(
            `Sent ${payload.length} byte payload with CRC ${(crc >>> 0).toString(16)} (${
                cobsEncodedPacket.length
            } bytes encoded)`,
            encodedDelimitedPacket,
        )
        this.sendPacket(encodedDelimitedPacket)
    }
}
