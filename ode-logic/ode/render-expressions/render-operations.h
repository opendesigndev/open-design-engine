
#pragma once

#include <octopus/octopus.h>
#include <ode-essentials.h>
#include "../core/ChannelMatrix.h"
#include "../core/LayerInstanceSpecifier.h"
#include "RenderExpression.h"

namespace ode {

Rendexptr blend(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode);
Rendexptr blend(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode, Rendexptr *&inlayPoint);
Rendexptr blendIgnoreAlpha(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode);
Rendexptr mask(const Rendexptr &image, const Rendexptr &mask, const ChannelMatrix &channelMatrix);
Rendexptr mixMask(const Rendexptr &dst, const Rendexptr &src, const Rendexptr &mask, const ChannelMatrix &channelMatrix);
Rendexptr mix(const Rendexptr &a, const Rendexptr &b, double ratio);
Rendexptr multiplyAlpha(const Rendexptr &image, double multiplier);

Rendexptr drawLayerBody(const LayerInstanceSpecifier &layer);
Rendexptr drawLayerStroke(const LayerInstanceSpecifier &layer, int index);
Rendexptr drawLayerFill(const LayerInstanceSpecifier &layer, int index);
Rendexptr drawLayerStrokeFill(const LayerInstanceSpecifier &layer, int index);
Rendexptr drawLayerText(const LayerInstanceSpecifier &layer);
Rendexptr drawLayerEffect(const Rendexptr &basis, const LayerInstanceSpecifier &layer, int index);
Rendexptr applyFilter(const Rendexptr &basis, const octopus::Filter &filter);

Rendexptr makeInlayPoint(Rendexptr content, Rendexptr *&inlayPoint);
Rendexptr makeBackground();
Rendexptr setBackground(const Rendexptr &content, const Rendexptr &background);
Rendexptr unsetBackground(const Rendexptr &content);

// FOR OPACITY ANIMATION
Rendexptr mixLayerOpacity(const LayerInstanceSpecifier &layer, const Rendexptr &a, const Rendexptr &b);

}
