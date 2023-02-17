
#include "FontBase.h"

#include "textify-instance.h"

namespace ode {

void FontBase::setFontDirectory(const FilePath &path) {
    fontDirectory = path;
}

bool FontBase::loadFonts(const octopus::Text &text) {
    std::vector<std::string> missingFonts = textify::listMissingFonts(TEXTIFY_CONTEXT, text);
    for (const std::string &missingFont : missingFonts) {
        textify::addFontFile(TEXTIFY_CONTEXT, missingFont, std::string(), (std::string) (fontDirectory+(missingFont+".ttf")), false) ||
        textify::addFontFile(TEXTIFY_CONTEXT, missingFont, std::string(), (std::string) (fontDirectory+(missingFont+".otf")), false);
    }
    return true; // TODO
}

}
