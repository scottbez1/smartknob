import SerialPort = require('serialport')
import {MessageCallback, SmartKnobCore, cobsDecode} from 'smartknobjs-core'
import * as CRC32 from 'crc-32'
import {PB} from 'smartknobjs-proto'

export class SmartKnobNode extends SmartKnobCore {
    private port: SerialPort | null
    private buffer: Buffer

    constructor(serialPath: string, onMessage: MessageCallback) {
        super(onMessage, (packet: Uint8Array) => {
            this.port?.write(Buffer.from(packet))
        })
        this.port = new SerialPort(serialPath, {
            baudRate: SmartKnobCore.BAUD,
        })
        this.port.on('data', (data: Buffer) => {
            this.buffer = Buffer.concat([this.buffer, data])
            this.processBuffer()
        })
        this.portAvailable = true
        this.buffer = Buffer.alloc(0)
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
            this.handleMessage(message)
        }
    }
}
