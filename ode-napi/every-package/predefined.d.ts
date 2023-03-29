import { StringRef, String, MemoryBuffer } from './exports.js'

export function makeString(value: string): String
export function getString(value: StringRef): string
export function makeMemoryBuffer(value: ArrayBuffer): MemoryBuffer
