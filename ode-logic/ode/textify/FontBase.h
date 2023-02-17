
#pragma once

#include <memory>
#include <octopus/octopus.h>
#include <textify/textify-api.h>
#include <ode-essentials.h>

namespace ode {

class FontBase;

typedef std::shared_ptr<FontBase> FontBasePtr;

class FontBase {

public:
    void setFontDirectory(const FilePath &path);
    bool loadFonts(const octopus::Text &text);

private:
    FilePath fontDirectory;

};

}
