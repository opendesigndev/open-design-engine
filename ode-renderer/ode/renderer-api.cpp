
#include "renderer-api.h"

#include <memory>
#include <octopus/octopus.h>
#include <ode-logic.h>
#include "image/Image.h"
#include "image/ImageBase.h"
#include "optimized-renderer/Renderer.h"
#include "optimized-renderer/render.h"

using namespace ode;

const int ODE_PIXEL_FORMAT_RGBA = int(PixelFormat::RGBA);
const int ODE_PIXEL_FORMAT_PREMULTIPLIED_RGBA = int(PixelFormat::PREMULTIPLIED_RGBA);

struct ODE_internal_RendererContext {
    GraphicsContext gc;
    std::unique_ptr<Renderer> renderer;

    inline ODE_internal_RendererContext(const char *label, const Vector2i &dimensions) : gc(label, dimensions) { }
    inline ODE_internal_RendererContext(GraphicsContext::Offscreen offscreen, const Vector2i &dimensions) : gc(offscreen, dimensions) { }
};

struct ODE_internal_DesignImageBase {
    ImageBase imageBase;

    inline explicit ODE_internal_DesignImageBase(GraphicsContext &gc) : imageBase(gc) { }
};

struct ODE_internal_AnimationRenderer {
    Renderer *renderer;
    ImageBase *imageBase;
    Design::ComponentAccessor component;
    Rendexptr renderExpression;
    int renderRevision;
};

// TODO !!!!!! move to common file (duplicate of logic-api)
struct ODE_internal_Component {
    Design::ComponentAccessor accessor;

    inline ODE_internal_Component() : accessor() { }
    inline explicit ODE_internal_Component(const Design::ComponentAccessor &accessor) : accessor(accessor) { }
};

// TODO move to some header
ODE_Result ode_result(DesignError::Error error);

ODE_Result ODE_API ode_destroyBitmap(ODE_Bitmap bitmap) {
    free(reinterpret_cast<void *>(bitmap.pixels));
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_createRendererContext(ODE_EngineHandle engine, ODE_RendererContextHandle *rendererContext, ODE_StringRef target) {
    ODE_ASSERT(engine.ptr && rendererContext);
    if (target.data)
        rendererContext->ptr = new ODE_internal_RendererContext(reinterpret_cast<const char *>(target.data), Vector2i(640, 480));
    else
        rendererContext->ptr = new ODE_internal_RendererContext(GraphicsContext::OFFSCREEN, Vector2i(640, 480));
    if (!rendererContext->ptr->gc) {
        delete rendererContext->ptr;
        rendererContext->ptr = nullptr;
        return ODE_RESULT_GRAPHICS_CONTEXT_ERROR;
    }
    rendererContext->ptr->renderer.reset(new Renderer(rendererContext->ptr->gc));
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_destroyRendererContext(ODE_RendererContextHandle rendererContext) {
    delete rendererContext.ptr;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_createDesignImageBase(ODE_RendererContextHandle rendererContext, ODE_DesignHandle design, ODE_DesignImageBaseHandle *designImageBase) {
    ODE_ASSERT(design.ptr && designImageBase);
    if (!rendererContext.ptr)
        return ODE_RESULT_INVALID_RENDERER_CONTEXT;
    designImageBase->ptr = new ODE_internal_DesignImageBase(rendererContext.ptr->gc);
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_destroyDesignImageBase(ODE_DesignImageBaseHandle designImageBase) {
    delete designImageBase.ptr;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_design_loadImagePixels(ODE_DesignImageBaseHandle designImageBase, ODE_StringRef key, ODE_BitmapRef bitmap) {
    ODE_ASSERT(key.data && bitmap.pixels.data && bitmap.pixels.length == 4*bitmap.width*bitmap.height);
    if (!designImageBase.ptr)
        return ODE_RESULT_INVALID_IMAGE_BASE;
    if (!(bitmap.width > 0 && bitmap.height > 0))
        return ODE_RESULT_INVALID_BITMAP_DIMENSIONS;
    switch (bitmap.format) {
        case ODE_PIXEL_FORMAT_RGBA:
        case ODE_PIXEL_FORMAT_PREMULTIPLIED_RGBA:
            break;
        default:
            return ODE_RESULT_INVALID_PIXEL_FORMAT;
    }
    BitmapConstRef bitmapRef(PixelFormat(bitmap.format), reinterpret_cast<const void *>(bitmap.pixels.data), bitmap.width, bitmap.height);
    octopus::Image imageRef = { };
    imageRef.ref.type = octopus::ImageRef::Type::PATH;
    imageRef.ref.value = ode_stringDeref(key);
    designImageBase.ptr->imageBase.add(imageRef, bitmapRef);
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_pr1_drawComponent(ODE_RendererContextHandle rendererContext, ODE_ComponentHandle component, ODE_DesignImageBaseHandle designImageBase, ODE_Bitmap *outputBitmap, ODE_PR1_FrameView frameView) {
    ODE_ASSERT(rendererContext.ptr && component.ptr && designImageBase.ptr && outputBitmap);
    if (Result<Rendexptr, DesignError> renderTree = component.ptr->accessor.assemble()) {
        PixelBounds pixelBounds = outerPixelBounds(ScaledBounds(0, 0, frameView.width, frameView.height)+frameView.scale*Vector2d(frameView.offset.x, frameView.offset.y));
        if (PlacedImagePtr image = render(*rendererContext.ptr->renderer, designImageBase.ptr->imageBase, *component.ptr->accessor.TEMP_GET_COMPONENT_DELETE_ME_ASAP(), renderTree.value(), frameView.scale, pixelBounds, 0)) {
            if (BitmapPtr bitmap = image->asBitmap()) {
                outputBitmap->format = ODE_PIXEL_FORMAT_RGBA;
                ODE_ASSERT(bitmap->format() == PixelFormat::RGBA);
                outputBitmap->width = bitmap->width();
                outputBitmap->height = bitmap->height();
                outputBitmap->pixels = reinterpret_cast<ODE_VarDataPtr>(bitmap->eject());
                return ODE_RESULT_OK;
            }
        }
        return ODE_RESULT_UNKNOWN_ERROR;
    } else
        return ode_result(renderTree.error().type());
}

ODE_Result ODE_API ode_pr1_createAnimationRenderer(ODE_RendererContextHandle rendererContext, ODE_ComponentHandle component, ODE_PR1_AnimationRendererHandle *animationRenderer, ODE_DesignImageBaseHandle imageBase) {
    ODE_ASSERT(animationRenderer);
    if (!rendererContext.ptr)
        return ODE_RESULT_INVALID_RENDERER_CONTEXT;
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    if (!imageBase.ptr)
        return ODE_RESULT_INVALID_IMAGE_BASE;
    animationRenderer->ptr = new ODE_internal_AnimationRenderer;
    animationRenderer->ptr->renderer = rendererContext.ptr->renderer.get();
    animationRenderer->ptr->imageBase = &imageBase.ptr->imageBase;
    animationRenderer->ptr->component = component.ptr->accessor;
    animationRenderer->ptr->renderRevision = -1;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_pr1_destroyAnimationRenderer(ODE_PR1_AnimationRendererHandle animationRenderer) {
    delete animationRenderer.ptr;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_pr1_animation_drawFrame(ODE_PR1_AnimationRendererHandle renderer, ODE_PR1_FrameView frameView, ODE_Scalar time) {
    ODE_ASSERT(renderer.ptr && renderer.ptr->renderer && renderer.ptr->imageBase);
    if (!renderer.ptr->renderExpression || renderer.ptr->renderRevision != renderer.ptr->component.revision()) {
        if (Result<Rendexptr, DesignError> renderExpr = renderer.ptr->component.assemble())
            renderer.ptr->renderExpression = renderExpr.value();
        else
            return ode_result(renderExpr.error().type());
        renderer.ptr->renderRevision = renderer.ptr->component.revision();
    }
    PixelBounds pixelBounds = outerPixelBounds(ScaledBounds(0, 0, frameView.width, frameView.height)+frameView.scale*Vector2d(frameView.offset.x, frameView.offset.y));
    if (PlacedImagePtr frame = render(*renderer.ptr->renderer, *renderer.ptr->imageBase, *renderer.ptr->component.TEMP_GET_COMPONENT_DELETE_ME_ASAP(), renderer.ptr->renderExpression, frameView.scale, pixelBounds, time)) {
        renderer.ptr->renderer->screenDraw(
            PixelBounds(0, 0, frameView.width, frameView.height),
            PlacedImagePtr(frame, frame.bounds()-frameView.scale*Vector2d(frameView.offset.x, frameView.offset.y)),
            Color(1, 1, 1, 1)
        );
    } else
        return ODE_RESULT_UNKNOWN_ERROR;
    return ODE_RESULT_OK;
}
