import {MessageCallback, SmartKnobCore} from 'smartknobjs-core'

export class SmartKnobWebSerial extends SmartKnobCore {
    private port: SerialPort | null
    private writer: WritableStreamDefaultWriter<Uint8Array> | undefined = undefined

    constructor(port: SerialPort, onMessage: MessageCallback) {
        super(onMessage, (packet: Uint8Array) => {
            this.writer?.write(packet).catch((e) => {
                console.error('Error writing serial', e)
                this.port?.close()
                this.port = null
                this.portAvailable = false
            })
        })
        this.port = port
        this.portAvailable = true
        this.port.addEventListener('disconnect', () => {
            console.log('shutting down on disconnect')
            this.port = null
            this.portAvailable = false
        })
    }

    public async openAndLoop() {
        if (this.port === null) {
            return
        }
        await this.port.open({baudRate: SmartKnobCore.BAUD})
        if (this.port.readable === null || this.port.writable === null) {
            throw new Error('Port missing readable or writable!')
        }

        const reader = this.port.readable.getReader()
        try {
            this.writer = this.port.writable.getWriter()
            try {
                // eslint-disable-next-line no-constant-condition
                while (true) {
                    const {value, done} = await reader.read()
                    if (done) {
                        break
                    }
                    if (value !== undefined) {
                        this.onReceivedData(value)
                    }
                }
            } finally {
                console.log('Releasing writer')
                this.writer?.releaseLock()
            }
        } finally {
            console.log('Releasing reader')
            reader.releaseLock()
        }
    }
}
