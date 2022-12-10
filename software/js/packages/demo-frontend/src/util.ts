export const exhaustiveCheck = (x: never): never => {
    throw new Error("Didn't expect to get here", x)
}

export const isSome = <T>(v: T | null | undefined): v is T => {
    return v !== null && v !== undefined
}

export const lerp = (value: number, inMin: number, inMax: number, min: number, max: number): number => {
    // Map the input value from the input range to the output range
    value = ((value - inMin) / (inMax - inMin)) * (max - min) + min

    // Clamp the mapped value between the minimum and maximum range
    return Math.min(Math.max(value, min), max)
}

export type NoUndefinedField<T> = {
    [P in keyof T]-?: NoUndefinedField<NonNullable<T[P]>>
}
