
// FILE GENERATED BY generate-api-bindings.py

import { EngineSymbol, Enum, EnumValue } from "./internal.js";
import * as ode from "./exports.js";

/** Function call result type */
export const Result: Enum<Result_Map>;
export type Result = EnumValue<Result_Index>;
export type Result_Index = Result_Map[keyof Result_Map];
export type Result_Map = {
    OK: 0;
    UNKNOWN_ERROR: 1;
    NOT_IMPLEMENTED: 2;
    MEMORY_ALLOCATION_ERROR: 3;
    FILE_READ_ERROR: 4;
    FILE_WRITE_ERROR: 5;
    OCTOPUS_PARSE_ERROR: 8;
    OCTOPUS_MANIFEST_PARSE_ERROR: 9;
    ANIMATION_PARSE_ERROR: 10;
    INVALID_ENUM_VALUE: 15;
    ITEM_NOT_FOUND: 16;
    LAYER_NOT_FOUND: 17;
    COMPONENT_NOT_FOUND: 18;
    DUPLICATE_COMPONENT_ID: 20;
    DUPLICATE_LAYER_ID: 21;
    OCTOPUS_UNAVAILABLE: 22;
    COMPONENT_IN_USE: 24;
    ALREADY_INITIALIZED: 25;
    SHAPE_LAYER_ERROR: 32;
    TEXT_LAYER_ERROR: 33;
    WRONG_LAYER_TYPE: 48;
    SINGULAR_TRANSFORMATION: 49;
    INVALID_DESIGN: 52;
    INVALID_COMPONENT: 53;
    INVALID_PIXEL_FORMAT: 96;
    INVALID_BITMAP_DIMENSIONS: 97;
    INVALID_RENDERER_CONTEXT: 108;
    INVALID_IMAGE_BASE: 109;
    FONT_ERROR: 119;
    GRAPHICS_CONTEXT_ERROR: 128;
};

/** A mathematical 2-dimensional vector */
export type Vector2 = readonly [
    x: ode.Scalar,
    y: ode.Scalar
];

/** An axis-aligned rectangle specified by its two opposite corners */
export type Rectangle = readonly [
    a: ode.Vector2,
    b: ode.Vector2
];

/** A reference to an immutable null-terminated string in contiguous memory (does not hold or change ownership) */
export type StringRef = {
    /** Pointer to the beginning of UTF-8 encoded string */
    data: ode.ConstCharPtr;
    /** Length of the string in bytes excluding the terminating null character */
    length: ode.Int;
};

/** A standalone string in its own memory block (must be manually destroyed with ode_destroyString) */
export const String: { new (str: ode.Std_string): ode.String };
export type String = {
    [EngineSymbol]: "String";
    constructor();
    /** Length of the string in bytes excluding the terminating null character */
    length: ode.Int;
    /** Convert to ODE_StringRef */
    ref(): ode.StringRef;
    /** Get pointer to the beginning of string */
    getData(): ode.VarDataPtr;
    delete(): void;
};

/** A buffer of raw data bytes in physical memory - deallocate with ode_destroyMemoryBuffer */
export const MemoryBuffer: { new (): ode.MemoryBuffer };
export type MemoryBuffer = {
    [EngineSymbol]: "MemoryBuffer";
    /** Pointer to the beginning of the memory block */
    data: ode.VarDataPtr;
    /** Length of the buffer in bytes */
    length: ode.Size_t;
    delete(): void;
};

/** A list of immutable string references */
export const StringList: { new (): ode.StringList };
export type StringList = {
    [EngineSymbol]: "StringList";
    /** Number of entries; */
    n: ode.Int;
    /** Get single entry */
    getEntry(i: ode.Int): ode.StringRef;
    delete(): void;
};

/** Destroys the ODE_String object, freeing its allocated memory */
export function destroyString(
    string: ode.String,
): ode.Result;

/**
 * Allocates a new memory buffer of a given size
 * @param buffer the resulting memory buffer will be stored in this output argument
 * @param length the desired length of the buffer in bytes
 */
export function allocateMemoryBuffer(
    buffer: ode.MemoryBuffer,
    length: ode.Size_t,
): ode.Result;

/**
 * Resizes an existing memory buffer to a given size, or allocates a new memory buffer if buffer's data and length are zero.
 * Pre-existing data in the buffer (up to the new length) will be preserved
 * @param buffer the memory buffer to be resized
 * @param length the desired new length of the buffer in bytes
 */
export function reallocateMemoryBuffer(
    buffer: ode.MemoryBuffer,
    length: ode.Size_t,
): ode.Result;

/** Destroys the memory buffer, freeing its allocated memory */
export function destroyMemoryBuffer(
    buffer: ode.MemoryBuffer,
): ode.Result;
