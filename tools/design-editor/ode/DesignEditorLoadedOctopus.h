
#pragma once

#include <optional>

#include <ode-essentials.h>
#include <ode-logic.h>

// Forward declarations
namespace ode {
class Bitmap;
using BitmapPtr = std::shared_ptr<Bitmap>;
}

using namespace ode;

struct DesignEditorLoadedOctopus {
    /// The file path of the Octopus file loaded.
    FilePath filePath;
    /// The artboard, root layer of the design.
    ComponentPtr artboard;

    // TODO: DE render graph root
    Rendexptr renderGraphRoot;
    /// Rendererd image.
    BitmapPtr bitmap;
    /// Image placement.
    ScaledBounds placement;

    //! Pointers to all unique layers.
    std::vector<const octopus::Layer *> layers;

    void clear();
    bool isLoaded() const;
    /// Get  result node bitmap.
    BitmapPtr resultBitmap() const;
    /// Get  result output image placement.
    std::optional<ScaledBounds> resultPlacement() const;
    /// Reset the vector of layers for this Octopus file.
    void resetLayers();
};
