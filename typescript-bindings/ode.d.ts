
// FILE GENERATED BY generate-api-bindings.py

import * as ODE from "./exports.js";
export type ODE = typeof ODE;

export type LoadODEOptions = {
    locateFile?: () => string;
};

export default function loadODE(options?: LoadODEOptions): Promise<ODE>;
export const version: string;

export {
    type Result,
    type Vector2,
    type Rectangle,
    type StringRef,
    type String,
    type MemoryBuffer,
    type StringList,
    type LayerType,
    type Scalar_array_6,
    type Transformation,
    type EngineAttributes,
    type ComponentMetadata,
    type LayerList_Entry,
    type LayerList,
    type LayerMetrics,
    type ParseError_Type,
    type ParseError,
    type EngineHandle,
    type DesignHandle,
    type ComponentHandle,
    type Bitmap,
    type BitmapRef,
    type PR1_FrameView,
    type RendererContextHandle,
    type DesignImageBaseHandle,
    type PR1_AnimationRendererHandle,
} from "./exports.js";
