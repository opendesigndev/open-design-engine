import { StringRef } from './exports.js'

export type MemoryBuffer = ArrayBuffer;
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
export type String = string
