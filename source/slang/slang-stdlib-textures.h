#pragma once

#include "slang-ir.h"
#include "slang-type-system-shared.h"
#include "../core/slang-string.h"

namespace Slang
{

static const struct BaseTextureShapeInfo {
    char const*			    shapeName;
    TextureFlavor::Shape	baseShape;
    int					    coordCount;
} kBaseTextureShapes[] = {
    { "1D",		TextureFlavor::Shape::Shape1D,	1 },
    { "2D",		TextureFlavor::Shape::Shape2D,	2 },
    { "3D",		TextureFlavor::Shape::Shape3D,	3 },
    { "Cube",	TextureFlavor::Shape::ShapeCube,3 },
};

static const struct BaseTextureAccessInfo {
    char const*         name;
    SlangResourceAccess access;
} kBaseTextureAccessLevels[] = {
    { "",                   SLANG_RESOURCE_ACCESS_READ },
    { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
    { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
};

static const struct TextureTypePrefixInfo
{
    char const* name;
    bool        combined;
} kTexturePrefixes[] =
{
    { "Texture", false },
    { "Sampler", true },
};

struct TextureTypeInfo
{
    TextureTypeInfo(
        TextureTypePrefixInfo const& prefixInfo,
        BaseTextureShapeInfo const& base,
        bool isArray,
        bool isMultisample,
        bool isShadow,
        BaseTextureAccessInfo const& accessInfo,
        StringBuilder& inSB,
        String const& inPath);

    TextureTypePrefixInfo const& prefixInfo;
    BaseTextureShapeInfo const& base;
    bool isArray;
    bool isMultisample;
    bool isShadow;
    BaseTextureAccessInfo const& accessInfo;
    StringBuilder& sb;
    String path;

    void emitTypeDecl();

private:
    //
    // Functions for writing specific parts of a definition
    //
    void writeQueryFunctions();
    void writeSubscriptFunctions();
    void writeSampleFunctions();
    void writeGatherExtensions();

    //
    // More general utilities
    //
    enum class ReadNoneMode
    {
        Never,
        IfReadOnly,
        Always
    };

    void writeFuncBody(
        const char* funcName,
        const String& glsl,
        const String& cuda,
        const String& spirv
    );
    void writeFuncDecorations(
        const String& glsl,
        const String& cuda
    );
    void writeFuncWithSig(
        const char* funcName,
        const String& sig,
        const String& glsl = String{},
        const String& spirv = String{},
        const String& cuda = String{},
        const ReadNoneMode readNoneMode = ReadNoneMode::IfReadOnly
    );
    void writeFunc(
        const char* returnType,
        const char* funcName,
        const String& params,
        const String& glsl = String{},
        const String& spirv = String{},
        const String& cuda = String{},
        const ReadNoneMode readNoneMode = ReadNoneMode::IfReadOnly
    );

    // A pointer to a string representing the current level of indentation
    const char* i;
};

}
