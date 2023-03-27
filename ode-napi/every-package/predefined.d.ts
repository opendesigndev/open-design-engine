import { StringRef, String, MemoryBuffer } from './exports.js'

export type Transformation = [
    a: number,
    b: number,
    c: number,
    d: number,
    e: number,
    f: number,
]
export function makeString(value: string): String
export function getString(value: StringRef): string
export function makeMemoryBuffer(value: ArrayBuffer): MemoryBuffer
