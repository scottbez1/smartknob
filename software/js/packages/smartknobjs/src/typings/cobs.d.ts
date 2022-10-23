declare module 'cobs' {
    export function decode(buf: Buffer): Buffer
    export function encode(buf: Buffer, zeroFrame?: boolean): Buffer
}
