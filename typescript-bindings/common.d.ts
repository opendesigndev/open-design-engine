
export const HEAP8: Int8Array;
export const HEAP16: Int16Array;
export const HEAP32: Int32Array;
export const HEAPU8: Uint8Array;
export const HEAPU16: Uint16Array;
export const HEAPU32: Uint32Array;
export const HEAPF32: Float32Array;
export const HEAPF64: Float64Array;
export function _malloc(size: Size_t): VarDataPtr;
export function _free(ptr: VarDataPtr): void;

export type Char = number;
export type Int = number;
export type Unsigned = number;
export type Float = number;
export type Double = number;
export type Size_t = number;
export type VarDataPtr = number;
export type ConstDataPtr = number;
export type ConstCharPtr = number;
export type Scalar = number;
export type Std_string = string;
