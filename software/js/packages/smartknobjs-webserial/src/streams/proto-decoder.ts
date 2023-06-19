import {cobsDecode} from 'smartknobjs-core'
import * as CRC32 from 'crc-32'
import {PB} from 'smartknobjs-proto'

class ProtoDecoder implements Transformer<Uint8Array, PB.FromSmartKnob> {
    transform(chunk: Uint8Array, controller: TransformStreamDefaultController<PB.FromSmartKnob>) {
        const packet = cobsDecode(chunk)
        if (packet.length <= 4) {
            console.debug(`Received short packet ${packet}`)
            return
        }

        const payload = packet.slice(0, packet.length - 4)

        // Validate CRC32
        const crc_buf = packet.slice(packet.length - 4, packet.length)
        const provided_crc = crc_buf[0] | (crc_buf[1] << 8) | (crc_buf[2] << 16) | (crc_buf[3] << 24)
        const crc = CRC32.buf(payload)
        if (crc !== provided_crc) {
            console.debug(`Bad CRC. Expected ${crc} but received ${provided_crc}`)
            console.debug(packet.toString())
            return
        }

        let message: PB.FromSmartKnob
        try {
            message = PB.FromSmartKnob.decode(payload)
        } catch (err) {
            console.warn(`Invalid protobuf message ${payload}`)
            return
        }
        controller.enqueue(message)
    }
}

export class ProtoDecoderStream extends TransformStream<Uint8Array, PB.FromSmartKnob> {
    constructor() {
        super(new ProtoDecoder())
    }
}
