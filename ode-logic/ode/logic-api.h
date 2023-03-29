
#ifndef ODE_LOGIC_API_H
#define ODE_LOGIC_API_H

// THIS API IS NOT FINAL

#include <ode/api-base.h>

#ifdef __cplusplus
extern "C" {
#endif

// Layer flags for ODE_LayerList::Entry::flags
/// Layer has visible attribute set to true
#define ODE_LAYER_FLAG_VISIBLE 0x01
/// Layer is the mask member of its parent layer
#define ODE_LAYER_FLAG_MASK 0x02

/// Layer types
typedef enum {
    ODE_LAYER_TYPE_UNSPECIFIED = 0,
    ODE_LAYER_TYPE_SHAPE = 1,
    ODE_LAYER_TYPE_TEXT = 2,
    ODE_LAYER_TYPE_GROUP = 3,
    ODE_LAYER_TYPE_MASK_GROUP = 4,
    ODE_LAYER_TYPE_COMPONENT_REFERENCE = 5,
    ODE_LAYER_TYPE_COMPONENT_INSTANCE = 6,
} ODE_LayerType;

/// The coordinate space a transformation matrix is defined in
typedef enum {
    ODE_TRANSFORMATION_BASIS_LAYER = 0,
    ODE_TRANSFORMATION_BASIS_PARENT_LAYER = 1,
    ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT = 2,
} ODE_TransformationBasis;

// Data structures

/// A 3x2 affine transformation matrix in column-major order
typedef struct {
    ODE_Scalar matrix[6];
} ODE_Transformation;

/// Attributes of newly created engine instance
typedef struct {
    // future
    int padding; // to avoid warnings
} ODE_EngineAttributes;

/// Metadata of an Octopus component
typedef struct {
    /// Octopus ID (must match the one in the Octopus structure)
    ODE_StringRef id;
    /// Optional name of page which the component is a part of
    ODE_StringRef page;
    /// Coordinates of the component within the design
    ODE_Vector2 position;
} ODE_ComponentMetadata;

/// List of layer ID's and metadata - should be destroyed by ode_destroyLayerList
typedef struct {
    /// A single entry in the layer list
    struct Entry {
        /// ID of parent layer (empty for root layer)
        ODE_StringRef parentId;
        /// Layer ID
        ODE_StringRef id;
        /// Type of layer
        ODE_LayerType type;
        /// Bit field of layer attributes, see ODE_LAYER_FLAG_... values
        int flags;
        /// User-defined layer name
        ODE_StringRef name;
    } *entries;
    /// The number of list entries
    int n;
    /// Get single entry
    ODE_BIND_ARRAY_GETTER(getEntry, entries, n);
} ODE_LayerList;

/// Layer metrics - transformation and bounds
typedef struct {
    /// Layer's full transformation matrix - includes transformations of parent layers
    ODE_Transformation transformation;
    /** Layer's untransformed logical bounding box
     *   - some parts of the layer may exceed these bounds, but these are the bounds that make sense from a user's perspective
     *   - apply transformation in order to draw on top of component */
    ODE_Rectangle logicalBounds;
    /** Layer's untransformed graphical bounding box
     *   - these bounds are guaranteed to contain all pixels of the layer and its effects */
    ODE_Rectangle graphicalBounds;
    /** Layer's graphical bounding box in the component's coordinate space (after the layer is transformed)
     *   - may be smaller than applying transformation to graphicalBounds and wrapping those in an axis-aligned rectangle */
    ODE_Rectangle transformedGraphicalBounds;
} ODE_LayerMetrics;

/// Details of reported JSON parser error
typedef struct {
    /// Type of parse error
    enum Type {
        /// No error - parse was successful
        OK = 0,
        /// Invalid JSON syntax encountered
        JSON_SYNTAX_ERROR = 1,
        /// End of input reached unexpectedly (e.g. some brackets not yet closed)
        UNEXPECTED_END_OF_FILE = 2,
        /// Data type of value in JSON does not match the expected type
        TYPE_MISMATCH = 3,
        /// Fixed-length array expected a different length than in the JSON
        ARRAY_SIZE_MISMATCH = 4,
        /// An unknown key in object
        UNKNOWN_KEY = 5,
        /// Value of an enum field does not match any known enumeration value
        UNKNOWN_ENUM_VALUE = 6,
        /// A numerical value written in JSON cannot be represented by the target variable type (e.g. too large value for int)
        VALUE_OUT_OF_RANGE = 7,
        /// The beginning of a string was expected but not encountered
        STRING_EXPECTED = 8,
        /// Invalid combination of UTF-16 escape sequences (likely to do with UTF-16 surrogate pairs)
        UTF16_ENCODING_ERROR = 9,
    } type;
    /// Approximate position of the error within the JSON string (in bytes)
    int position;
} ODE_ParseError;

// Object handles (wraps pointer to opaque internal representation)
/// Represents the instance of Open Design Engine
ODE_HANDLE_DECL(ODE_internal_Engine) ODE_EngineHandle;
/// Represents a design
ODE_HANDLE_DECL(ODE_internal_Design) ODE_DesignHandle;
/// Represents a component within a design
ODE_HANDLE_DECL(ODE_internal_Component) ODE_ComponentHandle;

// General

/// Destroys an ODE_LayerList object
ODE_Result ODE_API ode_destroyLayerList(ODE_LayerList layerList);
/// Destroys an ODE_StringList constructed by ode_design_listMissingFonts or ode_component_listMissingFonts
ODE_Result ODE_API ode_destroyMissingFontList(ODE_StringList fontList);

// Engine

/// Fills the ODE_EngineAttributes structure with default values of attributes
ODE_Result ODE_API ode_initializeEngineAttributes(ODE_OUT_RETURN ODE_EngineAttributes *engineAttributes);
/// Creates an instance of Open Design Engine with the given attributes and saves its handle to the engine argument. Use ode_destroyEngine to destroy it
ODE_Result ODE_API ode_createEngine(ODE_OUT_RETURN ODE_EngineHandle *engine, const ODE_EngineAttributes *engineAttributes);
/// Destroys an instance of Open Design Engine
ODE_Result ODE_API ode_destroyEngine(ODE_EngineHandle engine);

// Design

/**
 * Creates a new empty design - deallocate with ode_destroyDesign
 * @param engine - instance of engine
 * @param design - output argument for the new design handle
 */
ODE_Result ODE_API ode_createDesign(ODE_EngineHandle engine, ODE_OUT_RETURN ODE_DesignHandle *design);

/**
 * Loads a design from a comprehensive binary file representation - deallocate with ode_destroyDesign
 * @param engine - instance of engine
 * @param design - output argument for the new design handle
 * @param path - path to design file
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR or ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_FUTURE_API ode_loadDesignFromFile(ODE_EngineHandle engine, ODE_OUT_RETURN ODE_DesignHandle *design, ODE_StringRef path, ODE_OUT ODE_ParseError *parseError);

/**
 * Loads a design from an Octopus Manifest file - deallocate with ode_destroyDesign
 * @param engine - instance of engine
 * @param design - output argument for the new design handle
 * @param path - path to Octopus manifest file
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_NATIVE_API ode_loadDesignFromManifestFile(ODE_EngineHandle engine, ODE_OUT_RETURN ODE_DesignHandle *design, ODE_StringRef path, ODE_OUT ODE_ParseError *parseError);

/**
 * Loads a design from an Octopus Manifest string - deallocate with ode_destroyDesign
 * @param engine - instance of engine
 * @param design - output argument for the new design handle
 * @param manifestString - Octopus manifest JSON string reference
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_loadDesignFromManifestString(ODE_EngineHandle engine, ODE_OUT_RETURN ODE_DesignHandle *design, ODE_StringRef manifestString, ODE_OUT ODE_ParseError *parseError);

/// Destroys a design and its components
ODE_Result ODE_API ode_destroyDesign(ODE_DesignHandle design);

/**
 * Loads an Octopus Manifest file into an existing design. Caller must ensure no conflicts with preexisting content
 * @param design - target design
 * @param path - path to Octopus manifest file
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_NATIVE_API ode_design_loadManifestFile(ODE_DesignHandle design, ODE_StringRef path, ODE_OUT ODE_ParseError *parseError);

/**
 * Loads an Octopus Manifest from JSON string into an existing design. Caller must ensure no conflicts with preexisting content
 * @param design - target design
 * @param manifestString - Octopus manifest JSON string reference
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_design_loadManifestString(ODE_DesignHandle design, ODE_StringRef manifestString, ODE_OUT ODE_ParseError *parseError);

/**
 * Loads a component manifest from JSON string and adds the component to design
 * @param design - target design
 * @param component - output argument for the new component handle
 * @param componentManifestString - component manifest JSON string reference
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR or ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_FUTURE_API ode_design_addComponentFromManifestString(ODE_DesignHandle design, ODE_OUT_RETURN ODE_ComponentHandle *component, ODE_StringRef componentManifestString, ODE_OUT ODE_ParseError *parseError);

/**
 * Loads a component from Octopus JSON string and adds the component to design
 * @param design - target design
 * @param component - output argument for the new component handle
 * @param metadata - component metadata that are not present in Octopus but are required to properly integrate it in the design
 * @param octopusString - Octopus JSON string reference
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_design_addComponentFromOctopusString(ODE_DesignHandle design, ODE_OUT_RETURN ODE_ComponentHandle *component, ODE_ComponentMetadata metadata, ODE_StringRef octopusString, ODE_OUT ODE_ParseError *parseError);

/// Removes component from design. It is not necessary to call this before ode_destroyDesign
ODE_Result ODE_API ode_design_removeComponent(ODE_DesignHandle design, ODE_ComponentHandle component);

/**
 * Outputs a list of fonts (post-script names) required by a design which haven't been provided yet
 * @param fontList - the list is stored in this output argument. Deallocate with ode_destroyMissingFontList
 */
ODE_Result ODE_API ode_design_listMissingFonts(ODE_DesignHandle design, ODE_OUT_RETURN ODE_StringList *fontList);

/**
 * Loads a font for a design from a font file
 * @param design - target design
 * @param name - font name identifier (post-script name)
 * @param path - path to font file
 * @param faceName - for files containing multiple faces, identifies the specific face within the file, otherwise should be left empty
 */
ODE_Result ODE_NATIVE_API ode_design_loadFontFile(ODE_DesignHandle design, ODE_StringRef name, ODE_StringRef path, ODE_StringRef faceName);

/**
 * Loads a font for a design from bytes in memory
 * @param design - target design
 * @param name - font name identifier (post-script name)
 * @param data - memory buffer holding the raw font data. Ownership of the memory buffer will be transferred to ODE, so it must not be destroyed on success
 * @param faceName - for files containing multiple faces, identifies the specific face within the file, otherwise should be left empty
 */
ODE_Result ODE_API ode_design_loadFontBytes(ODE_DesignHandle design, ODE_StringRef name, ODE_INOUT ODE_MemoryBuffer *data, ODE_StringRef faceName);

/**
 * Finds component within a design by ID and outputs its handle
 * @param component - output argument for the component handle
 * @param componentId - component ID string reference
 */
ODE_Result ODE_API ode_design_getComponent(ODE_DesignHandle design, ODE_OUT_RETURN ODE_ComponentHandle *component, ODE_StringRef componentId);

// Component

ODE_Result ODE_FUTURE_API ode_component_loadOctopusFile(ODE_ComponentHandle component, ODE_StringRef path, ODE_StringRef assetBasePath, ODE_OUT ODE_ParseError *parseError);
ODE_Result ODE_FUTURE_API ode_component_loadOctopusString(ODE_ComponentHandle component, ODE_StringRef octopusString, ODE_StringRef assetBasePath, ODE_OUT ODE_ParseError *parseError);
ODE_Result ODE_FUTURE_API ode_component_setRootLayer(ODE_ComponentHandle component, ODE_StringRef layerOctopusString, ODE_OUT ODE_ParseError *parseError);

/**
 * Adds a layer to component
 * @param component - target component
 * @param parentLayerId - ID of group layer in which the new layer should be inserted (must be of ODE_LAYER_TYPE_GROUP or ODE_LAYER_TYPE_MASK_GROUP type)
 * @param beforeLayerId - ID of layer within the parent layer before which the new layer should be inserted. If empty, layer will be inserted at the end of the group
 * @param layerOctopusString - the Octopus representation of the single layer being inserted as a JSON string reference
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_component_addLayer(ODE_ComponentHandle component, ODE_StringRef parentLayerId, ODE_StringRef beforeLayerId, ODE_StringRef layerOctopusString, ODE_OUT ODE_ParseError *parseError);

ODE_Result ODE_FUTURE_API ode_component_removeLayer(ODE_ComponentHandle component, ODE_StringRef layerId);

/**
 * Applies a set of permanent changes to a single layer within a component
 * @param component - target component
 * @param layerId - ID of layer to be subject to modifications
 * @param layerChangeOctopusString - string representation of the requested changes encoded as an Octopus "Change" object in JSON format
 *     NOTE - CURRENTLY NOT ALL MODIFICATIONS ARE IMPLEMENTED. In some cases, you may get ODE_RESULT_NOT_IMPLEMENTED.
 *     THE ONLY IMPLEMENTED "subject" IS CURRENTLY "LAYER"
 * @param parseError - output argument to store details of parse error if ODE_RESULT_OCTOPUS_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_component_modifyLayer(ODE_ComponentHandle component, ODE_StringRef layerId, ODE_StringRef layerChangeOctopusString, ODE_OUT ODE_ParseError *parseError);

/**
 * Permanently applies a transformation matrix to a single layer within a component
 * @param component - target component
 * @param layerId - ID of layer to be transformed
 * @param basis - specifies the coordinate space of the transformation matrix
 * @param transformation - the transformation matrix
 */
ODE_Result ODE_API ode_component_transformLayer(ODE_ComponentHandle component, ODE_StringRef layerId, ODE_TransformationBasis basis, ODE_Transformation transformation);

/**
 * PROTOTYPE - Loads specification of animations for a given component
 * @param component - target component
 * @param animationDefinition - the animation definition encoded as a JSON string
 * @param parseError - output argument to store details of parse error if ODE_RESULT_ANIMATION_PARSE_ERROR is returned. Can be null if this information is not needed.
 */
ODE_Result ODE_API ode_pr1_component_loadAnimation(ODE_ComponentHandle component, ODE_StringRef animationDefinition, ODE_OUT ODE_ParseError *parseError);

/**
 * PROTOTYPE - Evaluates the value of a layer's animation at a specific point in time
 * @param index - the index of the animation in the submitted animation definition
 * @param time - animation time in seconds
 * @param value - pointer to where the output value will be stored. The type of stored data depends on the animation type:
 *     TRANSFORM - 6x ODE_Scalar
 *     ROTATION - 1x ODE_Scalar
 *     OPACITY - 1x ODE_Scalar
 *     FILL_COLOR - 4x ODE_Scalar (RGBA)
 */
ODE_Result ODE_API ode_pr1_component_getAnimationValueAtTime(ODE_ComponentHandle component, int index, ODE_Scalar time, ODE_VarDataPtr value);

/**
 * Outputs a list of all layers within a component and their metadata
 * @param layerList - output argument where the layer list is stored. Deallocate with ode_destroyLayerList
 */
ODE_Result ODE_API ode_component_listLayers(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_LayerList *layerList);

/**
 * Finds the component's topmost layer at a given position
 * @param layerId - output argument where the found layer's ID is stored, empty if no layer found. Deallocate with ode_destroyString
 * @param position - position in component's coordinate system
 * @param radius - tolerance radius around position (a layer not intersecting position but within this radius may be outputted)
 */
ODE_Result ODE_API ode_component_identifyLayer(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_String *layerId, ODE_Vector2 position, ODE_Scalar radius);

/**
 * Outputs layer metrics for a given layer of a component
 * @param component - handle of component that contains the layer
 * @param layerId - layer's ID
 * @param layerMetrics - output argument where the layer's metrics will be stored
 */
ODE_Result ODE_API ode_component_getLayerMetrics(ODE_ComponentHandle component, ODE_StringRef layerId, ODE_OUT_RETURN ODE_LayerMetrics *layerMetrics);

/**
 * Outputs a list of fonts (post-script names) required by a design component which haven't been provided yet
 * @param fontList - the list is stored in this output argument. Deallocate with ode_destroyMissingFontList
 */
ODE_Result ODE_API ode_component_listMissingFonts(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_StringList *fontList);

/**
 * Outputs the Octopus string representing a given design component
 * @param octopusString - output argument where the Octopus JSON will be stored. Deallocate with ode_destroyString
 */
ODE_Result ODE_API ode_component_getOctopus(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_String *octopusString);

ODE_Result ODE_FUTURE_API ode_component_getInstancedOctopus(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_String *octopusString);
ODE_Result ODE_FUTURE_API ode_component_getLayerOctopus(ODE_ComponentHandle component, ODE_OUT_RETURN ODE_String *octopusString, ODE_StringRef layerId);

#ifdef __cplusplus
}
#endif

#endif // ODE_LOGIC_API_H
