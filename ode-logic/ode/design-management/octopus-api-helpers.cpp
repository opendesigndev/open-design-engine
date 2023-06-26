
#include "octopus-api-helpers.h"

#include <octopus/octopus.h>
#include <octopus-manifest/octopus-manifest.h>
#include <octopus/parser.h>
#include <octopus-manifest/parser.h>
#include <octopus-manifest/serializer.h>
#include <ode-essentials.h>
#include "../design-management/OctopusFile.h"

using namespace ode;

namespace {

// TODO: Unify with logic-api.cpp?
template <typename ErrorType>
static ODE_ParseError::Type ode_parseErrorType(ErrorType type) {
    switch (type) {
        case ErrorType::OK:
            return ODE_ParseError::OK;
        case ErrorType::JSON_SYNTAX_ERROR:
            return ODE_ParseError::JSON_SYNTAX_ERROR;
        case ErrorType::UNEXPECTED_END_OF_FILE:
            return ODE_ParseError::UNEXPECTED_END_OF_FILE;
        case ErrorType::TYPE_MISMATCH:
            return ODE_ParseError::TYPE_MISMATCH;
        case ErrorType::ARRAY_SIZE_MISMATCH:
            return ODE_ParseError::ARRAY_SIZE_MISMATCH;
        case ErrorType::UNKNOWN_KEY:
            return ODE_ParseError::UNKNOWN_KEY;
        case ErrorType::UNKNOWN_ENUM_VALUE:
            return ODE_ParseError::UNKNOWN_ENUM_VALUE;
        case ErrorType::VALUE_OUT_OF_RANGE:
            return ODE_ParseError::VALUE_OUT_OF_RANGE;
        case ErrorType::STRING_EXPECTED:
            return ODE_ParseError::STRING_EXPECTED;
        case ErrorType::UTF16_ENCODING_ERROR:
            return ODE_ParseError::UTF16_ENCODING_ERROR;
    }
    ODE_ASSERT(!"switch incomplete");
    return ODE_ParseError::JSON_SYNTAX_ERROR;
}

template <typename T>
void insertUnique(std::vector<T> &containingVector, const T &insertVal) {
    if (std::find_if(containingVector.begin(), containingVector.end(), [&insertVal](const auto &v)->bool { return insertVal.refId == v.refId; }) == containingVector.end()) {
        containingVector.emplace_back(insertVal);
    }
}

template <typename T>
void insertUnique(std::vector<T> &containingVector, const std::vector<T> &insertVector) {
    for (const auto &insertVal : insertVector) {
        insertUnique(containingVector, insertVal);
    }
}

octopus::Assets listAllAssets(const octopus::Layer &layer) {
    octopus::Assets assets = {};
    if (layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()) {
        for (const octopus::Fill &fill : layer.shape->fills) {
            if (fill.type == octopus::Fill::Type::IMAGE && fill.image.has_value()) {
                const octopus::Image &image = *fill.image;
                const std::string imageRef = ((FilePath)image.ref.value).filename();
                insertUnique(assets.images, octopus::AssetImage {
                    octopus::ResourceLocation {
                        image.ref.type == octopus::ImageRef::Type::PATH ? octopus::ResourceLocation::Type::RELATIVE : octopus::ResourceLocation::Type::EXTERNAL,
                        image.ref.value,
                        nonstd::nullopt,
                        nonstd::nullopt
                    },
                    imageRef, // asset ref Id
                    imageRef  // asset name
                });
            }
        }
    }
    if (layer.type == octopus::Layer::Type::TEXT && layer.text.has_value()) {
        if (layer.text->defaultStyle.font.has_value()) {
            const octopus::Font &font = *layer.text->defaultStyle.font;
            insertUnique(assets.fonts, octopus::AssetFont {
                nonstd::nullopt,
                nonstd::nullopt,
                font.postScriptName,
                font.postScriptName
            });
        }
        if (layer.text->styles.has_value()) {
            for (const octopus::StyleRange &styleRange : *layer.text->styles) {
                if (styleRange.style.font.has_value()) {
                    const octopus::Font &font = *styleRange.style.font;
                    insertUnique(assets.fonts, octopus::AssetFont {
                        nonstd::nullopt,
                        nonstd::nullopt,
                        font.postScriptName,
                        font.postScriptName
                    });
                }
            }
        }
    }
    if (layer.layers.has_value()) {
        for (const octopus::Layer &subLayer : *layer.layers) {
            const octopus::Assets sublayerAssets = listAllAssets(subLayer);
            insertUnique(assets.images, sublayerAssets.images);
            insertUnique(assets.fonts, sublayerAssets.fonts);
        }
    }
    return assets;
}

std::string fontFileExtension(const ODE_MemoryBuffer &fontBuffer) {
    if (strncmp(static_cast<const char *>(fontBuffer.data), "ttcf", 4) == 0) {
        return "ttc";
    } else if (strncmp(static_cast<const char *>(fontBuffer.data), "\x00\x01\x00\x00", 4) == 0) {
        return "ttf";
    } else if (strncmp(static_cast<const char *>(fontBuffer.data), "OTTO", 4) == 0) {
        return "otf";
    }
    return "";
}

}

ODE_Result ode::loadDesignFromOctopusFile(ODE_DesignHandle *design, const FilePath &path, const ImageFunction &imageLoader, ODE_ParseError *parseError) {
    ODE_ASSERT(design && design->ptr);

    // Read Octopus design file
    OctopusFile octopusFile;
    if (!octopusFile.load(path)) {
        // TODO: A better error type
        return ODE_RESULT_OCTOPUS_UNAVAILABLE;
    }
    // Read the manifest
    const std::optional<std::string> manifestFileData = octopusFile.getFileData("octopus-manifest.json");
    if (!manifestFileData.has_value()) {
        // TODO: A better error type
        return ODE_RESULT_OCTOPUS_UNAVAILABLE;
    }
    octopus::OctopusManifest manifest;
    if (octopus::ManifestParser::Error error = octopus::ManifestParser::parse(manifest, manifestFileData->c_str())) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR;
    }

    // Add the actual component files
    for (const FilePath &filePath : octopusFile.filePaths()) {
        const std::string filePathStr = (std::string)filePath;
        const bool isJson = (filePathStr.substr(filePathStr.find_last_of(".")+1) == "json");
        const bool isOctopus = (filePathStr.substr() == "json");;
        const bool isOctopusComponentFile = isJson && filePathStr != "octopus-manifest.json";

        if (isOctopusComponentFile) {
            const std::optional<std::string> fileData = octopusFile.getFileData(filePath);
            if (!fileData.has_value()) {
                continue;
            }
            const size_t prefixLength = strlen("octopus-");
            const size_t extensionLength = strlen(".json");
            const std::string componentId = filePathStr.substr(prefixLength, filePathStr.size()-prefixLength-extensionLength);
            const std::vector<octopus::Component>::const_iterator manifestComponent = std::find_if(manifest.components.begin(), manifest.components.end(), [&componentId](const octopus::Component &c) {
                return c.id == componentId;
            });
            if (manifestComponent == manifest.components.end()) {
                continue;
            }
            // Add the component
            ODE_ComponentHandle component = {};
            ODE_ComponentMetadata metadata = {};
            metadata.id = ode_stringRef(componentId);
            metadata.page = {}; // TODO: Metadata page?
            metadata.position = ODE_Vector2 { manifestComponent->bounds.x, manifestComponent->bounds.y };
            if (const ODE_Result result = ode_design_addComponentFromOctopusString(*design, &component, metadata, ode_stringRef(*fileData), parseError)) {
                return result;
            }
        }
    }

    // Load images if image loader function is supplied
    if (imageLoader) {
        for (const ode::FilePath &filePath : octopusFile.filePaths()) {
            const std::optional<std::string> fileData = octopusFile.getFileData(filePath);
            if (fileData.has_value()) {
                ODE_MemoryBuffer imageData = ode_makeMemoryBuffer(fileData->c_str(), fileData->size());
                imageLoader(ode_stringRef((std::string)filePath), imageData);
            }
        }
    }

    // Load fonts
    ODE_StringList missingFonts;
    ode_design_listMissingFonts(*design, &missingFonts);
    for (int i = 0; i < missingFonts.n; ++i) {
        const ODE_StringRef &missingFontName = missingFonts.entries[i];
        for (const ode::FilePath &filePath : octopusFile.filePaths()) {
            const char *fileName = filePath.filename();
            if (std::strncmp(fileName, missingFontName.data, std::strlen(missingFontName.data)) == 0) {
                const std::optional<std::string> fileData = octopusFile.getFileData(filePath);
                if (fileData.has_value()) {
                    ODE_MemoryBuffer fileBuffer = ode_makeMemoryBuffer(fileData->data(), fileData->size());
                    if (const ODE_Result result = ode_design_loadFontBytes(*design, missingFontName, &fileBuffer, ODE_StringRef())) {
                        return result;
                    }
                }
            }
        }
    }

    return ODE_RESULT_OK;
}

ODE_Result ode::saveDesignToOctopusFile(ODE_DesignHandle design, const FilePath &path, const ImageFunction &imageExporter) {
    const std::string fileName = path.filename();
    const size_t extPos = fileName.rfind('.', fileName.length());
    const std::string cleanFileName = (extPos == std::string::npos) ? fileName : fileName.substr(0, extPos);

    OctopusFile octopusFile;
    octopusFile.add("Octopus", " is universal design format. opendesign.dev.", MemoryFileSystem::CompressionMethod::NONE);

    octopus::OctopusManifest manifest;
    manifest.version = OCTOPUS_MANIFEST_VERSION;
    manifest.origin.name = "OD Internal Design Editor";
    manifest.origin.version = "1.0.0.";
    manifest.name = cleanFileName;

    ODE_StringList componentList;
    ode_design_listComponents(design, &componentList);

    for (int i = 0; i < componentList.n; ++i) {
        const ODE_StringRef &componentId = componentList.entries[i];
        ODE_ComponentHandle component;
        ODE_ComponentMetadata componentMetadata;
        if (ode_design_getComponent(design, &component, componentId) != ODE_RESULT_OK) {
            continue;
        }
        if (ode_design_getComponentMetadata(design, &componentMetadata, componentId)) {
            continue;
        }

        ODE_String octopusString;
        if (ode_component_getOctopus(component, &octopusString) == ODE_RESULT_OK) {
            const std::string componentFileName = std::string("octopus-") + ode_stringDeref(componentId) + ".json";
            if (!octopusFile.add(componentFileName, std::string(octopusString.data, octopusString.length), MemoryFileSystem::CompressionMethod::DEFLATE).has_value()) {
                continue;
            }

            octopus::Octopus octopus;
            octopus::Parser::parse(octopus, octopusString.data);

            octopus::Component &octopusComponent = manifest.components.emplace_back();
            octopusComponent.id = octopus.id;
            octopusComponent.name = octopus.content->name;
            octopusComponent.status = octopus::Status {
                octopus::Status::Value::READY,
                nonstd::nullopt,
                0.0 };
            octopusComponent.bounds = octopus::Bounds {
                componentMetadata.position.x,
                componentMetadata.position.y,
                octopus.dimensions->width,
                octopus.dimensions->height };
            octopusComponent.location = octopus::ResourceLocation {
                octopus::ResourceLocation::Type::RELATIVE,
                componentFileName,
                nonstd::nullopt,
                nonstd::nullopt };
            octopusComponent.assets = octopus.content.has_value() ? listAllAssets(*octopus.content) : octopus::Assets{};

            // Save images if image exporter function is supplied
            if (imageExporter) {
                for (const octopus::AssetImage &assetImage : octopusComponent.assets->images) {
                    if (assetImage.location.path.has_value()) {
                        ODE_MemoryBuffer imageData;
                        if (imageExporter(ode_stringRef(assetImage.location.path.value()), imageData) == ODE_RESULT_OK) {
                            if (!octopusFile.add(assetImage.location.path.value(), std::string(static_cast<const char *>(imageData.data), imageData.length), MemoryFileSystem::CompressionMethod::NONE)) {
                                // TODO: Error saving PNG to octopus file
                                continue;
                            }
                        }
                    }
                }
            }

            for (octopus::AssetFont &assetFont : octopusComponent.assets->fonts) {
                ODE_MemoryBuffer fontData;
                if (ode_pr1_design_exportFontBytes(design, ODE_StringRef {assetFont.name.data(), (int)assetFont.name.size()}, &fontData) == ODE_RESULT_OK) {
                    if (fontData.data) {
                        const std::string fileExtension = fontFileExtension(fontData);
                        if (!fileExtension.empty()) {
                            assetFont.location = octopus::ResourceLocation {
                                octopus::ResourceLocation::Type::RELATIVE,
                                "fonts/"+assetFont.name+"."+fileExtension,
                                nonstd::nullopt,
                                nonstd::nullopt };
                            if (!octopusFile.exists(assetFont.location->path.value())) {
                                if (!octopusFile.add(assetFont.location->path.value(), std::string(static_cast<const char *>(fontData.data), fontData.length), MemoryFileSystem::CompressionMethod::NONE)) {
                                    // TODO: Error saving Font to octopus file
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::string manifestJson;
    if (octopus::ManifestSerializer::serialize(manifestJson, manifest) == octopus::ManifestSerializer::Error::OK) {
        if (octopusFile.add("octopus-manifest.json", manifestJson, MemoryFileSystem::CompressionMethod::DEFLATE).has_value()) {
            const bool isSaved = octopusFile.save(path);
            if (!isSaved) {
                fprintf(stderr, "Internal error (saving Octopus json to filesystem)\n");
            }
        }
    }

    return ODE_RESULT_OK;
}
