
// FILE GENERATED BY generate-api-bindings.py

import { EngineSymbol, Enum, EnumValue } from "./internal.js";
import * as ode from "./exports.js";

/** Layer has visible attribute set to true */
export const LAYER_FLAG_VISIBLE: ode.Int;
/** Layer is the mask member of its parent layer */
export const LAYER_FLAG_MASK: ode.Int;

/** Layer types */
export const LayerType: Enum<LayerType_Map>;
export type LayerType = EnumValue<LayerType_Index>;
export type LayerType_Index = LayerType_Map[keyof LayerType_Map];
export type LayerType_Map = {
    UNSPECIFIED: 0;
    SHAPE: 1;
    TEXT: 2;
    GROUP: 3;
    MASK_GROUP: 4;
    COMPONENT_REFERENCE: 5;
    COMPONENT_INSTANCE: 6;
};

/** An array of 6 ODE_Scalars */
export type Scalar_array_6 = readonly [
    ode.Scalar,
    ode.Scalar,
    ode.Scalar,
    ode.Scalar,
    ode.Scalar,
    ode.Scalar,
];

/** A 3x2 affine transformation matrix in column-major order */
export type Transformation = {
    matrix: ode.Scalar_array_6;
};

/** Attributes of newly created engine instance */
export const EngineAttributes: { new (): ode.EngineAttributes };
export type EngineAttributes = {
    [EngineSymbol]: "EngineAttributes";
    padding: ode.Int;
    delete(): void;
};

/** Metadata of an Octopus component */
export type ComponentMetadata = {
    /** Octopus ID (must match the one in the Octopus structure) */
    id: ode.StringRef;
    /** Optional name of page which the component is a part of */
    page: ode.StringRef;
    /** Coordinates of the component within the design */
    position: ode.Vector2;
};

/** A single entry in the layer list */
export type LayerList_Entry = {
    /** ID of parent layer (empty for root layer) */
    parentId: ode.StringRef;
    /** Layer ID */
    id: ode.StringRef;
    /** Type of layer */
    type: ode.LayerType;
    /** Bit field of layer attributes, see ODE_LAYER_FLAG_... values */
    flags: ode.Int;
    /** User-defined layer name */
    name: ode.StringRef;
};

/** List of layer ID's and metadata - should be destroyed by ode_destroyLayerList */
export const LayerList: { new (): ode.LayerList };
export type LayerList = {
    [EngineSymbol]: "LayerList";
    /** The number of list entries */
    n: ode.Int;
    /** Get single entry */
    getEntry(i: ode.Int): ode.LayerList_Entry;
    delete(): void;
};

/** Layer metrics - transformation and bounds */
export const LayerMetrics: { new (): ode.LayerMetrics };
export type LayerMetrics = {
    [EngineSymbol]: "LayerMetrics";
    /** Layer's full transformation matrix - includes transformations of parent layers */
    transformation: ode.Transformation;
    /**
     * Layer's untransformed logical bounding box
     * - some parts of the layer may exceed these bounds, but these are the bounds that make sense from a user's perspective
     * - apply transformation in order to draw on top of component
     */
    logicalBounds: ode.Rectangle;
    /**
     * Layer's untransformed graphical bounding box
     * - these bounds are guaranteed to contain all pixels of the layer and its effects
     */
    graphicalBounds: ode.Rectangle;
    /**
     * Layer's graphical bounding box in the component's coordinate space (after the layer is transformed)
     * - may be smaller than applying transformation to graphicalBounds and wrapping those in an axis-aligned rectangle
     */
    transformedGraphicalBounds: ode.Rectangle;
    delete(): void;
};

/** Type of parse error */
export const ParseError_Type: Enum<ParseError_Type_Map>;
export type ParseError_Type = EnumValue<ParseError_Type_Index>;
export type ParseError_Type_Index = ParseError_Type_Map[keyof ParseError_Type_Map];
export type ParseError_Type_Map = {
    /** No error - parse was successful */
    OK: 0;
    /** Invalid JSON syntax encountered */
    JSON_SYNTAX_ERROR: 1;
    /** End of input reached unexpectedly (e.g. some brackets not yet closed) */
    UNEXPECTED_END_OF_FILE: 2;
    /** Data type of value in JSON does not match the expected type */
    TYPE_MISMATCH: 3;
    /** Fixed-length array expected a different length than in the JSON */
    ARRAY_SIZE_MISMATCH: 4;
    /** An unknown key in object */
    UNKNOWN_KEY: 5;
    /** Value of an enum field does not match any known enumeration value */
    UNKNOWN_ENUM_VALUE: 6;
    /** A numerical value written in JSON cannot be represented by the target variable type (e.g. too large value for int) */
    VALUE_OUT_OF_RANGE: 7;
    /** The beginning of a string was expected but not encountered */
    STRING_EXPECTED: 8;
    /** Invalid combination of UTF-16 escape sequences (likely to do with UTF-16 surrogate pairs) */
    UTF16_ENCODING_ERROR: 9;
};

/** Details of reported JSON parser error */
export const ParseError: { new (): ode.ParseError };
export type ParseError = {
    [EngineSymbol]: "ParseError";
    /** Type of parse error */
    type: ode.ParseError_Type;
    /** Approximate position of the error within the JSON string (in bytes) */
    position: ode.Int;
    delete(): void;
};

/** Represents the instance of Open Design Engine */
export const EngineHandle: { new (): ode.EngineHandle };
export type EngineHandle = {
    [EngineSymbol]: "EngineHandle";
    constructor();
    delete(): void;
};

/** Represents a design */
export const DesignHandle: { new (): ode.DesignHandle };
export type DesignHandle = {
    [EngineSymbol]: "DesignHandle";
    constructor();
    delete(): void;
};

/** Represents a component within a design */
export const ComponentHandle: { new (): ode.ComponentHandle };
export type ComponentHandle = {
    [EngineSymbol]: "ComponentHandle";
    constructor();
    delete(): void;
};

/** Destroys an ODE_LayerList object */
export function destroyLayerList(
    layerList: ode.LayerList,
): ode.Result;

/** Destroys an ODE_StringList constructed by ode_design_listMissingFonts or ode_component_listMissingFonts */
export function destroyMissingFontList(
    fontList: ode.StringList,
): ode.Result;

/** Fills the ODE_EngineAttributes structure with default values of attributes */
export function initializeEngineAttributes(
    engineAttributes: ode.EngineAttributes,
): ode.Result;

/** Creates an instance of Open Design Engine with the given attributes and saves its handle to the engine argument. Use ode_destroyEngine to destroy it */
export function createEngine(
    engine: ode.EngineHandle,
    engineAttributes: ode.EngineAttributes,
): ode.Result;

/** Destroys an instance of Open Design Engine */
export function destroyEngine(
    engine: ode.EngineHandle,
): ode.Result;

/**
 * Creates a new empty design - deallocate with ode_destroyDesign
 * @param engine instance of engine
 * @param design output argument for the new design handle
 */
export function createDesign(
    engine: ode.EngineHandle,
    design: ode.DesignHandle,
): ode.Result;

/**
 * Loads a design from an Octopus Manifest string - deallocate with ode_destroyDesign
 * @param engine instance of engine
 * @param design output argument for the new design handle
 * @param manifestString Octopus manifest JSON string reference
 * @param parseError output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function loadDesignFromManifestString(
    engine: ode.EngineHandle,
    design: ode.DesignHandle,
    manifestString: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/** Destroys a design and its components */
export function destroyDesign(
    design: ode.DesignHandle,
): ode.Result;

/**
 * Loads an Octopus Manifest from JSON string into an existing design. Caller must ensure no conflicts with preexisting content
 * @param design target design
 * @param manifestString Octopus manifest JSON string reference
 * @param parseError output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function design_loadManifestString(
    design: ode.DesignHandle,
    manifestString: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/**
 * Loads a component from Octopus JSON string and adds the component to design
 * @param design target design
 * @param component output argument for the new component handle
 * @param metadata component metadata that are not present in Octopus but are required to properly integrate it in the design
 * @param octopusString Octopus JSON string reference
 * @param parseError output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function design_addComponentFromOctopusString(
    design: ode.DesignHandle,
    component: ode.ComponentHandle,
    metadata: ode.ComponentMetadata,
    octopusString: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/** Removes component from design. It is not necessary to call this before ode_destroyDesign */
export function design_removeComponent(
    design: ode.DesignHandle,
    component: ode.ComponentHandle,
): ode.Result;

/**
 * Outputs a list of fonts (post-script names) required by a design which haven't been provided yet
 * @param fontList the list is stored in this output argument. Deallocate with ode_destroyMissingFontList
 */
export function design_listMissingFonts(
    design: ode.DesignHandle,
    fontList: ode.StringList,
): ode.Result;

/**
 * Loads a font for a design from bytes in memory
 * @param design target design
 * @param name font name identifier (post-script name)
 * @param data memory buffer holding the raw font data. Ownership of the memory buffer will be transferred to ODE, so it needn't be destroyed on success
 * @param faceName for files containing multiple faces, identifies the specific face within the file, otherwise should be left empty
 */
export function design_loadFontBytes(
    design: ode.DesignHandle,
    name: ode.StringRef,
    data: ode.MemoryBuffer,
    faceName: ode.StringRef,
): ode.Result;

/**
 * Finds component within a design by ID and outputs its handle
 * @param component output argument for the component handle
 * @param componentId component ID string reference
 */
export function design_getComponent(
    design: ode.DesignHandle,
    component: ode.ComponentHandle,
    componentId: ode.StringRef,
): ode.Result;

/**
 * Adds a layer to component
 * @param component target component
 * @param parentLayerId ID of group layer in which the new layer should be inserted (must be of ODE_LAYER_TYPE_GROUP or ODE_LAYER_TYPE_MASK_GROUP type)
 * @param beforeLayerId ID of layer within the parent layer before which the new layer should be inserted. If empty, layer will be inserted at the end of the group
 * @param layerOctopusString the Octopus representation of the single layer being inserted as a JSON string reference
 * @param parseError output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function component_addLayer(
    component: ode.ComponentHandle,
    parentLayerId: ode.StringRef,
    beforeLayerId: ode.StringRef,
    layerOctopusString: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/**
 * Applies a set of permanent changes to a single layer within a component
 * @param component target component
 * @param layerId ID of layer to be subject to modifications
 * @param layerChangeOctopusString string representation of the requested changes encoded as an Octopus "Change" object in JSON format
 * NOTE - CURRENTLY NOT ALL MODIFICATIONS ARE IMPLEMENTED. In some cases, you may get ODE_RESULT_NOT_IMPLEMENTED.
 * THE ONLY IMPLEMENTED "subject" IS CURRENTLY "LAYER"
 * @param parseError output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function component_modifyLayer(
    component: ode.ComponentHandle,
    layerId: ode.StringRef,
    layerChangeOctopusString: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/**
 * PROTOTYPE - Loads specification of animations for a given component
 * @param component target component
 * @param animationDefinition the animation definition encoded as a JSON string
 * @param parseError output argument to store details of parse error if ODE_RESULT_ANIMATION_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
export function pr1_component_loadAnimation(
    component: ode.ComponentHandle,
    animationDefinition: ode.StringRef,
    parseError: ode.ParseError,
): ode.Result;

/**
 * PROTOTYPE - Evaluates the value of a layer's animation at a specific point in time
 * @param index the index of the animation in the submitted animation definition
 * @param time animation time in seconds
 * @param value pointer to where the output value will be stored. The type of stored data depends on the animation type:
 * TRANSFORM - 6x ODE_Scalar
 * ROTATION - 1x ODE_Scalar
 * OPACITY - 1x ODE_Scalar
 * FILL_COLOR - 4x ODE_Scalar (RGBA)
 */
export function pr1_component_getAnimationValueAtTime(
    component: ode.ComponentHandle,
    index: ode.Int,
    time: ode.Scalar,
    value: ode.VarDataPtr,
): ode.Result;

/**
 * Outputs a list of all layers within a component and their metadata
 * @param layerList output argument where the layer list is stored. Deallocate with ode_destroyLayerList
 */
export function component_listLayers(
    component: ode.ComponentHandle,
    layerList: ode.LayerList,
): ode.Result;

/**
 * Finds the component's topmost layer at a given position
 * @param layerId output argument where the found layer's ID is stored, empty if no layer found. Deallocate with ode_destroyString
 * @param position position in component's coordinate system
 * @param radius tolerance radius around position (a layer not intersecting position but within this radius may be outputted)
 */
export function component_identifyLayer(
    component: ode.ComponentHandle,
    layerId: ode.String,
    position: ode.Vector2,
    radius: ode.Scalar,
): ode.Result;

/**
 * Outputs layer metrics for a given layer of a component
 * @param component handle of component that contains the layer
 * @param layerId layer's ID
 * @param layerMetrics output argument where the layer's metrics will be stored
 */
export function component_getLayerMetrics(
    component: ode.ComponentHandle,
    layerId: ode.StringRef,
    layerMetrics: ode.LayerMetrics,
): ode.Result;

/**
 * Outputs a list of fonts (post-script names) required by a design component which haven't been provided yet
 * @param fontList the list is stored in this output argument. Deallocate with ode_destroyMissingFontList
 */
export function component_listMissingFonts(
    component: ode.ComponentHandle,
    fontList: ode.StringList,
): ode.Result;

/**
 * Outputs the Octopus string representing a given design component
 * @param octopusString output argument where the Octopus JSON will be stored. Deallocate with ode_destroyString
 */
export function component_getOctopus(
    component: ode.ComponentHandle,
    octopusString: ode.String,
): ode.Result;
