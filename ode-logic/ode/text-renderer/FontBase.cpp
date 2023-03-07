
#include "FontBase.h"

#include "text-renderer-instance.h"

namespace ode {

void FontBase::setFontDirectory(const FilePath &path) {
    fontDirectory = path;
}

bool FontBase::loadFonts(const octopus::Text &text) {
    std::vector<std::string> missingFonts = odtr::listMissingFonts(TEXT_RENDERER_CONTEXT, text);
    for (const std::string &missingFont : missingFonts) {
        odtr::addFontFile(TEXT_RENDERER_CONTEXT, missingFont, std::string(), (std::string) (fontDirectory+(missingFont+".ttf")), false) ||
        odtr::addFontFile(TEXT_RENDERER_CONTEXT, missingFont, std::string(), (std::string) (fontDirectory+(missingFont+".otf")), false);
    }
    return true; // TODO
}

}
