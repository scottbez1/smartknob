class DelimiterChunker implements Transformer<Uint8Array, Uint8Array> {
    private delimiter: number
    private buffer: Uint8Array

    constructor(delimiter: number) {
        this.delimiter = delimiter
        this.buffer = new Uint8Array(0)
    }

    transform(chunk: Uint8Array, controller: TransformStreamDefaultController<Uint8Array>) {
        let combinedBuffer = new Uint8Array(this.buffer.length + chunk.length)
        combinedBuffer.set(this.buffer, 0)
        combinedBuffer.set(chunk, this.buffer.length)

        let delimiterIndex = combinedBuffer.indexOf(this.delimiter)
        while (delimiterIndex !== -1) {
            const chunkToSend = combinedBuffer.subarray(0, delimiterIndex)
            controller.enqueue(chunkToSend)

            // unnecessary copy...
            combinedBuffer = combinedBuffer.subarray(delimiterIndex + 1)

            delimiterIndex = combinedBuffer.indexOf(this.delimiter)
        }

        this.buffer = combinedBuffer
    }
}

export class DelimiterChunkedStream extends TransformStream<Uint8Array, Uint8Array> {
    constructor(delimiter: number) {
        super(new DelimiterChunker(delimiter))
    }
}
