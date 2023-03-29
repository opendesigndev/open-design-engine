
#ifndef ODE_RENDERER_API_H
#define ODE_RENDERER_API_H

// THIS API IS NOT FINAL

#include <ode/api-base.h>
#include <ode/logic-api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Pixel format of 4 channels - red, green, blue, alpha, each channel represented by 8-bit unsigned integer (0 to 255 range)
extern ODE_API const int ODE_PIXEL_FORMAT_RGBA;
/// Pixel format of 4 channels - red, green, blue, alpha, color channels are alpha-premultiplied, each channel represented by 8-bit unsigned integer (0 to 255 range)
extern ODE_API const int ODE_PIXEL_FORMAT_PREMULTIPLIED_RGBA;

// Data structures

/// Representation of a bitmap with its own storage. Deallocate with ode_destroyBitmap
typedef struct {
    /// The pixel format (see ODE_PIXEL_FORMAT_... constants)
    int format;
    /// Pointer to the first (top-left) pixel. Pixels are stored contiguously in memory in row-major order
    ODE_VarDataPtr pixels;
    /// Dimensions of bitmap
    int width, height;
} ODE_Bitmap;

/// Reference to an immutable bitmap - does not hold or change ownership
typedef struct {
    /// The pixel format (see ODE_PIXEL_FORMAT_... constants)
    int format;
    /// Pointer to the first (top-left) pixel. Pixels are stored contiguously in memory in row-major order
    ODE_ConstDataPtr pixels;
    /// Dimensions of bitmap
    int width, height;
} ODE_BitmapRef;

/// PROTOTYPE - specification of frame view
typedef struct {
    /// Viewport dimensions
    int width, height;
    /// Offset of top-left corner (not scaled by scale)
    ODE_Vector2 offset;
    /// View scale (zoom)
    ODE_Scalar scale;
} ODE_PR1_FrameView;

// Object handles (wraps pointer to opaque internal representation)
/// Represents a renderer context. Renderer context manages a GL context
ODE_HANDLE_DECL(ODE_internal_RendererContext) ODE_RendererContextHandle;
/// Represents a design's image base. Image base manages storage of image assets of the design
ODE_HANDLE_DECL(ODE_internal_DesignImageBase) ODE_DesignImageBaseHandle;
/// PROTOTYPE - Represents an animation renderer. A renderer facilitates rendering of components or designs in a way specific to the renderer class
ODE_HANDLE_DECL(ODE_internal_AnimationRenderer) ODE_PR1_AnimationRendererHandle;

/// Deallocates the data held by an ODE_Bitmap
ODE_Result ODE_API ode_destroyBitmap(ODE_Bitmap bitmap);

/**
 * Creates a new renderer context - destroy with ode_destroyRendererContext
 * @param engine - existing engine handle
 * @param rendererContext - output argument for the new renderer context handle
 * @param target - identifies the target window or WebGL canvas (platform-specific)
 */
ODE_Result ODE_API ode_createRendererContext(ODE_EngineHandle engine, ODE_OUT_RETURN ODE_RendererContextHandle *rendererContext, ODE_StringRef target);
/// Destroys the renderer context
ODE_Result ODE_API ode_destroyRendererContext(ODE_RendererContextHandle rendererContext);

/**
 * Creates a new empty image base for a design - deallocate with ode_destroyDesignImageBase
 * @param rendererContext - handle to parent rendererContext
 * @param design - design whose image assets will be managed by the new image base
 * @param designImageBase - output argument for the new image base handle
 */
ODE_Result ODE_API ode_createDesignImageBase(ODE_RendererContextHandle rendererContext, ODE_DesignHandle design, ODE_OUT_RETURN ODE_DesignImageBaseHandle *designImageBase);

/// Destroys a design image base and deallocates its image data
ODE_Result ODE_API ode_destroyDesignImageBase(ODE_DesignImageBaseHandle designImageBase);

/**
 * Loads a design's image asset as pixels in physical memory
 * @param designImageBase - target design image base
 * @param key - identifying key of the image asset
 * @param bitmap - bitmap reference to the image data - does not have to remain in memory after this call
 */
ODE_Result ODE_API ode_design_loadImagePixels(ODE_DesignImageBaseHandle designImageBase, ODE_StringRef key, ODE_BitmapRef bitmap);

/**
 * PROTOTYPE - draws a component into a bitmap in physical memory
 * @param rendererContext - target renderer context
 * @param component - component to be rendered
 * @param designImageBase - image base of the component's parent design to be used to provide image assets
 * @param outputBitmap - output argument for the newly created bitmap - deallocate with ode_destroyBitmap
 * @param frameView - pointer to frame view object, which specifies the parameters of the render
 */
ODE_Result ODE_API ode_pr1_drawComponent(ODE_RendererContextHandle rendererContext, ODE_ComponentHandle component, ODE_DesignImageBaseHandle designImageBase, ODE_OUT_RETURN ODE_Bitmap *outputBitmap, ODE_PR1_FrameView frameView);

/**
 * PROTOTYPE - creates a new animation renderer for a given component - destroy with ode_pr1_destroyAnimationRenderer
 * @param rendererContext - handle to parent rendererContext
 * @param component - component to be rendered by the renderer
 * @param animationRenderer - output argument for the new animation renderer handle
 * @param imageBase - image base of the component's parent design to be used to provide image assets
 */
ODE_Result ODE_API ode_pr1_createAnimationRenderer(ODE_RendererContextHandle rendererContext, ODE_ComponentHandle component, ODE_OUT_RETURN ODE_PR1_AnimationRendererHandle *animationRenderer, ODE_DesignImageBaseHandle imageBase);

/// Destroys the animation renderer
ODE_Result ODE_API ode_pr1_destroyAnimationRenderer(ODE_PR1_AnimationRendererHandle animationRenderer);

/**
 * Draw a frame of an animation into the renderer context framebuffer using an animation renderer
 * @param renderer - the animation renderer to be used for this operation
 * @param frameView - pointer to frame view object, which specifies the parameters of the render
 * @param time - the timepoint of the animation in seconds
 */
ODE_Result ODE_API ode_pr1_animation_drawFrame(ODE_PR1_AnimationRendererHandle renderer, ODE_PR1_FrameView frameView, ODE_Scalar time);

#ifdef __cplusplus
}

#ifndef ODE_MINIMAL_API

/// Creates a reference (ODE_BitmapRef) to an ODE_Bitmap object
inline ODE_BitmapRef ode_bitmapRef(ODE_Bitmap bitmap) {
    ODE_BitmapRef ref = { };
    ref.format = bitmap.format;
    ref.pixels = bitmap.pixels;
    ref.width = bitmap.width;
    ref.height = bitmap.height;
    return ref;
}

#endif
#endif

#endif // ODE_RENDERER_API_H
