// Based on https://github.com/tcr/node-cobs/blob/master/index.js

export function encode(buf: Uint8Array) {
    const dest = [0]
    let code_ptr = 0
    let code = 0x01

    const finish = (incllast?: boolean) => {
        dest[code_ptr] = code
        code_ptr = dest.length
        incllast !== false && dest.push(0x00)
        code = 0x01
    }

    for (let i = 0; i < buf.length; i++) {
        if (buf[i] == 0) {
            finish()
        } else {
            dest.push(buf[i])
            code += 1
            if (code == 0xff) {
                finish()
            }
        }
    }
    finish(false)

    return Uint8Array.from(dest)
}

export function decode(buf: Uint8Array) {
    const dest: number[] = []
    for (let i = 0; i < buf.length; ) {
        const code = buf[i++]
        for (let j = 1; j < code; j++) {
            dest.push(buf[i++])
        }
        if (code < 0xff && i < buf.length) {
            dest.push(0)
        }
    }
    return Uint8Array.from(dest)
}
