
#include "DesignEditorComponent.h"

DesignEditorComponent::~DesignEditorComponent() {
    if (bitmap.pixels != nullptr) {
        ode_destroyBitmap(bitmap);
    }
    if (layerList.entries != nullptr) {
        ode_destroyLayerList(layerList);
    }
}
