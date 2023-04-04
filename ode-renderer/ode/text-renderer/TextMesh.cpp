
#include "TextMesh.h"

#include <open-design-text-renderer/PlacedTextData.h>

#include "FontAtlas.h"
#include "TextRenderer.h"

namespace ode {

static const int TEXT_VERTEX_ATTRIBUTES[] = {
    2, // coord
    2, // texCoord
    4, // color
    1, // outputRange
};

static void generateVertex(std::vector<float> &meshData, const Vector2d &coord, const Vector2d &texCoord, const float color[4], float outputRange) {
    meshData.push_back(float(coord.x));
    meshData.push_back(float(coord.y));
    meshData.push_back(float(texCoord.x));
    meshData.push_back(float(texCoord.y));
    meshData.push_back(color[0]);
    meshData.push_back(color[1]);
    meshData.push_back(color[2]);
    meshData.push_back(color[3]);
    meshData.push_back(outputRange);
}

static void generateQuadVertices(std::vector<float> &meshData, const FontAtlas::GlyphQuad &quad, uint32_t color, float outputRange) {
    float fColor[4] = {
        channelByteToFloat(byte(color)),
        channelByteToFloat(byte(color>>8)),
        channelByteToFloat(byte(color>>16)),
        channelByteToFloat(byte(color>>24)),
    };
    generateVertex(meshData, quad.planeBounds.a, quad.atlasBounds.a, fColor, outputRange);
    generateVertex(meshData, Vector2d(quad.planeBounds.b.x, quad.planeBounds.a.y), Vector2d(quad.atlasBounds.b.x, quad.atlasBounds.a.y), fColor, outputRange);
    generateVertex(meshData, quad.planeBounds.b, quad.atlasBounds.b, fColor, outputRange);
    generateVertex(meshData, quad.planeBounds.b, quad.atlasBounds.b, fColor, outputRange);
    generateVertex(meshData, Vector2d(quad.planeBounds.a.x, quad.planeBounds.b.y), Vector2d(quad.atlasBounds.a.x, quad.atlasBounds.b.y), fColor, outputRange);
    generateVertex(meshData, quad.planeBounds.a, quad.atlasBounds.a, fColor, outputRange);
}

std::unique_ptr<TextMesh> TextMesh::build(TextRenderer &parent, odtr::TextShapeHandle handle) {
    const odtr::PlacedTextData *placedText = odtr::getShapedText(TEXT_RENDERER_CONTEXT, handle);
    if (!placedText)
        return nullptr; // error
    std::vector<float> meshData;
    std::unique_ptr<TextMesh> mesh(new TextMesh);
    int quadCount = 0;
    for (const odtr::PlacedGlyphsPerFont::value_type &fontGlyphs : placedText->glyphs) {
        int segmentStart = 6*quadCount;
        FontAtlas &fontAtlas = parent.fontAtlas(fontGlyphs.first);
        if (!fontAtlas.initialize(fontGlyphs.first))
            continue; // error
        for (const odtr::PlacedGlyph &glyph : fontGlyphs.second) {
            FontAtlas::GlyphQuad quad;
            double emRange = fontAtlas.getGlyphQuad(quad, glyph.codepoint);
            if (!emRange)
                continue; // error
            float outputRange = glyph.fontSize*float(emRange);
            Vector2d glyphPos(glyph.originPosition.x, glyph.originPosition.y);
            quad.planeBounds *= glyph.fontSize;
            quad.planeBounds += glyphPos;
            generateQuadVertices(meshData, quad, glyph.color, outputRange);
            ++quadCount;
        }
        if (const Texture2D *texture = fontAtlas.getTexture()) {
            FontSegment &segment = mesh->segments.emplace_back();
            segment.start = segmentStart;
            segment.length = 6*quadCount-segmentStart;
            segment.texture = texture;
        }
    }
    if (!mesh->mesh.initialize(meshData.data(), TEXT_VERTEX_ATTRIBUTES, sizeof(TEXT_VERTEX_ATTRIBUTES)/sizeof(*TEXT_VERTEX_ATTRIBUTES), GL_TRIANGLES, 6*quadCount))
        return nullptr; // error;
    return mesh;
}

void TextMesh::draw(Uniform &vec2TexCoordFactor, int textureUnit) const {
    for (const FontSegment &segment : segments) {
        float texCoordFactor[2] = {
            float(1./segment.texture->dimensions().x),
            float(1./segment.texture->dimensions().y)
        };
        segment.texture->bind(textureUnit);
        vec2TexCoordFactor.setVec2(texCoordFactor);
        mesh.drawPart(segment.start, segment.length);
    }
}

}
