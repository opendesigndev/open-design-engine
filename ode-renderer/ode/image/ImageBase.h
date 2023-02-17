
#pragma once

#include <string>
#include <map>
#include <octopus/octopus.h>
#include <ode-essentials.h>
#include "../image/Image.h"

namespace ode {

/// Image asset database
class ImageBase {

public:
    explicit ImageBase(GraphicsContext &gc);
    /// Sets the directory where images are searched if not added manually
    void setImageDirectory(const FilePath &path);
    /// Adds an image to the database
    void add(const octopus::Image &ref, const ImagePtr &image);
    /// Retrieves an image from the database
    ImagePtr get(const octopus::Image &ref);

private:
    std::map<std::string, ImagePtr> images;
    FilePath directory;

};

}
