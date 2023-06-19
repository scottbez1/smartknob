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

export function findNClosest(numbers: number[], target: number, n: number): number[] {
    // First, we sort the numbers in ascending order based on their absolute difference
    // from the target number. This means that the numbers that are closest to the target
    // will come first in the sorted array.
    const sortedNumbers = numbers.sort((a, b) => Math.abs(a - target) - Math.abs(b - target))

    // Next, we return the first N numbers from the sorted array as the N closest numbers.
    return sortedNumbers.slice(0, n)
}
