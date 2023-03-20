import { StringRef } from './exports.js'

export type MemoryRef = ArrayBuffer;
export type Transformation = [
    a: number,
    b: number,
    c: number,
    d: number,
    e: number,
    f: number,
]
export function makeString(value: string): StringRef
export function readString(value: StringRef): string
