
#include <cstring>
#include <octopus/octopus.h>
#include <octopus/serializer.h>
#include <octopus/validator.h>
#include <ode-essentials.h>
#include <ode-media.h>
#include <ode-logic.h>
#include <ode-renderer.h>
#include <ode-diagnostics.h>
#include "image-asset-generator.h"

using namespace ode;
using namespace octopus_builder;

static Bitmap unpremultipliedCopy(const SparseBitmapConstRef &bitmap) {
    Bitmap copy(bitmap);
    bitmapUnpremultiply(copy);
    return copy;
}

class TestRenderer {

public:
    inline explicit TestRenderer(GraphicsContext &gc) : gc(gc), renderer(gc), imageBase(gc) {
        octopus::Image imgDef;
        imgDef.ref.type = octopus::ImageRef::Type::PATH;
        imgDef.ref.value = "IMAGE00";
        Bitmap bitmap00(PixelFormat::PREMULTIPLIED_RGBA, 360, 240);
        generateImageAsset(bitmap00, true);
        savePng(imgDef.ref.value, unpremultipliedCopy(bitmap00));
        imageBase.add(imgDef, Image::fromTexture(Image::fromBitmap((Bitmap &&) bitmap00, Image::PREMULTIPLIED)->asTexture(), Image::PREMULTIPLIED));
        imgDef.ref.value = "IMAGE01";
        Bitmap bitmap01(PixelFormat::PREMULTIPLIED_RGBA, 200, 400);
        generateImageAsset(bitmap01, false);
        savePng(imgDef.ref.value, unpremultipliedCopy(bitmap01));
        imageBase.add(imgDef, Image::fromTexture(Image::fromBitmap((Bitmap &&) bitmap01, Image::PREMULTIPLIED)->asTexture(), Image::PREMULTIPLIED));
    }

    inline bool renderOctopusIntoFile(const octopus::Octopus &octopus) {
        std::set<std::string> ids;
        std::string validationError;
        if (!octopus::validate(octopus, ids, &validationError))
            return false;

        std::string json;
        octopus::Serializer::serialize(json, octopus);
        writeFile(octopus.id+".json", json);

        Component component;
        if (DesignError error = component.initialize(octopus))
            return false;

        UnscaledBounds componentBounds;
        if (component.getOctopus().dimensions.has_value()) {
            componentBounds.b.x = component.getOctopus().dimensions.value().width;
            componentBounds.b.y = component.getOctopus().dimensions.value().height;
        } else if (component.getOctopus().content.has_value()) {
            if (Result<LayerBounds, DesignError> contentBounds = component.getLayerBounds(component.getOctopus().content->id))
                componentBounds = contentBounds.value().bounds;
        } else {
            return false;
        }

        Result<Rendexptr, DesignError> renderGraph = component.assemble();
        if (!renderGraph)
            return false;

        PlacedImagePtr image = render(renderer, imageBase, component, renderGraph.value(), 1, outerPixelBounds(scaleBounds(componentBounds, 1)), 0);
        if (!image)
            return false;

        BitmapPtr bitmap = image->asBitmap();
        if (!bitmap)
            return false;

        if (isPixelPremultiplied(bitmap->format()))
            bitmapUnpremultiply(*bitmap);
        return savePng(octopus.id+".png", *bitmap);
    }

private:
    GraphicsContext &gc;
    Renderer renderer;
    ImageBase imageBase;

};

static octopus::Gradient::ColorStop makeColorStop(double position, const Color &color) {
    octopus::Gradient::ColorStop stop;
    stop.position = position;
    stop.color.r = color.r;
    stop.color.g = color.g;
    stop.color.b = color.b;
    stop.color.a = color.a;
    return stop;
}

int basicRenderingOutput(GraphicsContext &gc) {
    TestRenderer renderer(gc);

    // Simplest possible design - one rectangle
    renderer.renderOctopusIntoFile(buildOctopus("TEST00", ShapeLayer(160, 120, 320, 240)));

    // Gradient fill tests
    ShapeLayer gradientShape(160, 120, 320, 240);
    gradientShape.shape->fills.front() = octopus::Fill();
    gradientShape.shape->fills.front().type = octopus::Fill::Type::GRADIENT;
    octopus::Fill &gradientFill = gradientShape.shape->fills.front();
    gradientFill.gradient = octopus::Gradient();
    gradientFill.gradient->type = octopus::Gradient::Type::LINEAR;
    gradientFill.gradient->stops.push_back(makeColorStop(0.0, Color(1, 0, 0)));
    gradientFill.gradient->stops.push_back(makeColorStop(0.5, Color(1, 1, 0)));
    gradientFill.gradient->stops.push_back(makeColorStop(1.0, Color(0, 1, 0)));
    gradientFill.positioning = octopus::Fill::Positioning();
    // basic horizontal gradient
    gradientFill.positioning->layout = octopus::Fill::Positioning::Layout::FILL;
    gradientFill.positioning->transform[0] = 320;
    gradientFill.positioning->transform[3] = 240;
    gradientFill.positioning->transform[4] = 160;
    gradientFill.positioning->transform[5] = 120;
    renderer.renderOctopusIntoFile(buildOctopus("TEST01", gradientShape));
    // tilted linear gradient - layout tests
    gradientFill.gradient->stops[1].color.a = .5;
    TransformationMatrix gradientMat;
    gradientMat = TransformationMatrix::translate(Vector2d(-.5))*gradientMat;
    gradientMat = TransformationMatrix::rotate(1)*gradientMat;
    gradientMat = TransformationMatrix::scale(120)*gradientMat;
    gradientMat = TransformationMatrix::translate(Vector2d(320, 240))*gradientMat;
    memcpy(gradientFill.positioning->transform, &gradientMat, sizeof(gradientFill.positioning->transform));
    gradientFill.positioning->layout = octopus::Fill::Positioning::Layout::STRETCH;
    renderer.renderOctopusIntoFile(buildOctopus("TEST02", gradientShape));
    octopus::Fill gradientFillCopy = gradientFill;
    gradientFill.positioning->layout = octopus::Fill::Positioning::Layout::FIT;
    renderer.renderOctopusIntoFile(buildOctopus("TEST03", gradientShape));
    gradientFill.positioning->layout = octopus::Fill::Positioning::Layout::TILE;
    renderer.renderOctopusIntoFile(buildOctopus("TEST04", gradientShape));
    gradientFill.positioning->layout = octopus::Fill::Positioning::Layout::FILL;
    // gradient shape tests
    gradientFill.positioning->transform[0] = 120;
    gradientFill.positioning->transform[1] = 0;
    gradientFill.positioning->transform[2] = 0;
    gradientFill.positioning->transform[3] = 120;
    gradientFill.positioning->transform[4] = 320;
    gradientFill.positioning->transform[5] = 240;
    gradientFill.gradient->type = octopus::Gradient::Type::RADIAL;
    renderer.renderOctopusIntoFile(buildOctopus("TEST05", gradientShape));
    gradientFill.gradient->type = octopus::Gradient::Type::ANGULAR;
    renderer.renderOctopusIntoFile(buildOctopus("TEST06", gradientShape));
    gradientFill.gradient->type = octopus::Gradient::Type::DIAMOND;
    renderer.renderOctopusIntoFile(buildOctopus("TEST07", gradientShape));
    gradientFill.gradient->type = octopus::Gradient::Type::LINEAR;

    // stroke gradient test
    ShapeLayer gradientStrokeShape(160, 120, 320, 240);
    gradientStrokeShape.setColor(Color(1, .75, 0, .75)).addStroke(CENTER, 24, Color());
    gradientStrokeShape.shape->strokes[0].fill = gradientFill;
    renderer.renderOctopusIntoFile(buildOctopus("TEST08", gradientStrokeShape));

    // Test unpremultiplication of output file (transparent rectangle should be white)
    renderer.renderOctopusIntoFile(buildOctopus("TEST09", ShapeLayer(160, 120, 320, 240).setColor(Color(1, .25))));

    // Image fill test
    ShapeLayer imageShape(160, 120, 320, 240);
    imageShape.shape->fills.front() = octopus::Fill();
    imageShape.shape->fills.front().type = octopus::Fill::Type::IMAGE;
    octopus::Fill &imageFill = imageShape.shape->fills.front();
    imageFill.image = octopus::Image();
    imageFill.image->ref.type = octopus::ImageRef::Type::PATH;
    imageFill.image->ref.value = "IMAGE00";
    imageFill.positioning = octopus::Fill::Positioning();
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::STRETCH;
    imageFill.positioning->transform[0] = 360;
    imageFill.positioning->transform[3] = 240;
    imageFill.positioning->transform[4] = 160;
    imageFill.positioning->transform[5] = 120;
    renderer.renderOctopusIntoFile(buildOctopus("TEST10", imageShape));
    // layout tests
    imageFill.positioning->transform[0] = 320;
    imageFill.image->ref.value = "IMAGE01";
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::STRETCH;
    renderer.renderOctopusIntoFile(buildOctopus("TEST11", imageShape));
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::FILL;
    renderer.renderOctopusIntoFile(buildOctopus("TEST12", imageShape));
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::FIT;
    renderer.renderOctopusIntoFile(buildOctopus("TEST13", imageShape));
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::TILE;
    imageFill.positioning->transform[0] = 120;
    imageFill.positioning->transform[3] = 120;
    renderer.renderOctopusIntoFile(buildOctopus("TEST14", imageShape));
    // transformation test
    imageFill.positioning->layout = octopus::Fill::Positioning::Layout::STRETCH;
    imageFill.image->ref.value = "IMAGE00";
    TransformationMatrix imageMat;
    imageMat = TransformationMatrix::scale(Vector2d(360, 240))*imageMat;
    imageMat = TransformationMatrix::rotate(.25)*imageMat;
    imageMat = TransformationMatrix::translate(Vector2d(160, 120))*imageMat;
    memcpy(imageFill.positioning->transform, &imageMat, sizeof(imageFill.positioning->transform));
    renderer.renderOctopusIntoFile(buildOctopus("TEST15", imageShape));

    // Overlay effect tests
    octopus::Effect overlayEffect;
    overlayEffect.type = octopus::Effect::Type::OVERLAY;
    overlayEffect.basis = octopus::EffectBasis::BODY;
    overlayEffect.overlay = octopus::Fill();
    // color overlay
    overlayEffect.overlay->type = octopus::Fill::Type::COLOR;
    overlayEffect.overlay->color = octopus::Color { 1, 0, 0, 1 };
    gradientShape.addEffect(overlayEffect);
    renderer.renderOctopusIntoFile(buildOctopus("TEST20", gradientShape));
    // gradient overlay
    overlayEffect.overlay = gradientFillCopy;
    renderer.renderOctopusIntoFile(buildOctopus("TEST21", ShapeLayer(160, 120, 320, 240).addEffect(overlayEffect)));

    // Shadow effect tests
    gradientShape.effects[0].overlay = nonstd::optional<octopus::Fill>();
    gradientShape.effects[0].type = octopus::Effect::Type::DROP_SHADOW;
    gradientShape.effects[0].shadow = octopus::Shadow();
    octopus::Shadow &shadow = gradientShape.effects[0].shadow.value();
    shadow.offset.x = 64;
    shadow.offset.y = 16;
    shadow.blur = 8;
    shadow.choke = 0;
    shadow.color = toOctopus(Color(0, 0, 1));
    renderer.renderOctopusIntoFile(buildOctopus("TEST24", gradientShape));
    shadow.blur = 4;
    shadow.choke = 16;
    renderer.renderOctopusIntoFile(buildOctopus("TEST25", gradientShape));
    shadow.blur = 0;
    shadow.choke = -4;
    renderer.renderOctopusIntoFile(buildOctopus("TEST26", gradientShape));
    gradientShape.effects[0].type = octopus::Effect::Type::INNER_SHADOW;
    shadow.blur = 48;
    renderer.renderOctopusIntoFile(buildOctopus("TEST27", gradientShape));
    shadow.offset.x = 8;
    shadow.choke = 8;
    renderer.renderOctopusIntoFile(buildOctopus("TEST28", gradientShape));
    shadow.offset.x = 64;
    shadow.blur = 0;
    //shadow.choke = -8;
    renderer.renderOctopusIntoFile(buildOctopus("TEST29", gradientShape));
    shadow.choke = 0;
    renderer.renderOctopusIntoFile(buildOctopus("TEST30", gradientShape));
    gradientShape.effects[0].type = octopus::Effect::Type::DROP_SHADOW;
    renderer.renderOctopusIntoFile(buildOctopus("TEST31", gradientShape));

    // Stroke effect tests
    gradientShape.effects[0].shadow = nonstd::optional<octopus::Shadow>();
    gradientShape.effects[0].type = octopus::Effect::Type::STROKE;
    gradientShape.effects[0].stroke = octopus::Stroke();
    octopus::Stroke &effectStroke = gradientShape.effects[0].stroke.value();
    effectStroke.fill.type = octopus::Fill::Type::COLOR;
    effectStroke.fill.color = toOctopus(Color(1, 1, 0));
    effectStroke.thickness = 20;
    effectStroke.position = octopus::Stroke::Position::OUTSIDE;
    renderer.renderOctopusIntoFile(buildOctopus("TEST32", gradientShape));
    effectStroke.position = octopus::Stroke::Position::CENTER;
    renderer.renderOctopusIntoFile(buildOctopus("TEST33", gradientShape));
    effectStroke.position = octopus::Stroke::Position::INSIDE;
    renderer.renderOctopusIntoFile(buildOctopus("TEST34", gradientShape));

    // Blur effect test
    octopus::Effect blurEffect;
    blurEffect.type = octopus::Effect::Type::GAUSSIAN_BLUR;
    blurEffect.basis = octopus::EffectBasis::FILL;
    blurEffect.blur = 4;
    renderer.renderOctopusIntoFile(buildOctopus("TEST36", imageShape.addEffect(blurEffect)));
    imageShape.effects[0].type = octopus::Effect::Type::BOUNDED_BLUR;
    renderer.renderOctopusIntoFile(buildOctopus("TEST37", imageShape));

    // Edge cases

    // Empty component
    renderer.renderOctopusIntoFile(buildOctopus("TEST64", GroupLayer()));

    // Shape mask without fill
    ShapeLayer filllessShape(160, 120, 320, 240);
    filllessShape.visible = false;
    filllessShape.shape->fills.clear();
    filllessShape.shape->strokes.clear();
    renderer.renderOctopusIntoFile(buildOctopus("TEST65", MaskGroupLayer(octopus::MaskBasis::BODY, filllessShape).add(ShapeLayer(280, 200, 320, 240))));

    return 0;
}
