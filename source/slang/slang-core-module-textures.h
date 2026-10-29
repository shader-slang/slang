#pragma once

#include "core/slang-string.h"
#include "slang-ir.h"
#include "slang-type-system-shared.h"

namespace Slang
{

static const struct BaseTextureShapeInfo
{
    char const* shapeName;
    SlangResourceShape baseShape;
    int coordCount;
} kBaseTextureShapes[] = {
    {"1D", SLANG_TEXTURE_1D, 1},
    {"2D", SLANG_TEXTURE_2D, 2},
    {"3D", SLANG_TEXTURE_3D, 3},
    {"Cube", SLANG_TEXTURE_CUBE, 3},
};

static const struct BaseTextureAccessInfo
{
    char const* name;
    SlangResourceAccess access;
} kBaseTextureAccessLevels[] = {
    {"", SLANG_RESOURCE_ACCESS_READ},
    {"RW", SLANG_RESOURCE_ACCESS_READ_WRITE},
    {"RasterizerOrdered", SLANG_RESOURCE_ACCESS_RASTER_ORDERED},
    {"Feedback", SLANG_RESOURCE_ACCESS_FEEDBACK},
};

struct TextureTypeInfo
{
    TextureTypeInfo(
        BaseTextureShapeInfo const& base,
        bool isArray,
        bool isMultisample,
        bool isShadow,
        StringBuilder& inSB,
        String const& inPath);

    BaseTextureShapeInfo const& base;
    bool isArray;
    bool isMultisample;
    bool isShadow;
    StringBuilder& sb;
    String path;

    void emitTypeDecl();

public:
    //
    // Functions for writing specific parts of a definition
    //
    // Write the `GetDimensions` members of this texture shape. The file-scope functions that the
    // members' combined case forwards to are appended to `outCombinedForwarders`, which the caller
    // emits after the enclosing extension.
    void writeGetDimensionFunctions(StringBuilder& outCombinedForwarders);

    //
    // More general utilities
    //
    enum class ReadNoneMode
    {
        Never,
        Always
    };

    // `combinedForward` holds statements emitted verbatim (already indented) at the start of the
    // shared cpp/hlsl case and of the metal case, before their intrinsics. A path that handles the
    // call must `return`; every other path falls through to the intrinsic. The only use is
    // forwarding a combined receiver; an empty string emits only the intrinsics.
    void writeFuncBody(
        const char* funcName,
        const String& combinedForward,
        const String& glsl,
        const String& cuda,
        const String& spirvDefault,
        const String& spirvRWDefault,
        const String& spirvCombined,
        const String& metal,
        const String& wgsl);
    void writeFuncWithSig(
        const char* funcName,
        const String& combinedForward,
        const String& sig,
        const String& glsl = String{},
        const String& spirvDefault = String{},
        const String& spirvRWDefault = String{},
        const String& spirvCombined = String{},
        const String& cuda = String{},
        const String& metal = String{},
        const String& wgsl = String{},
        const ReadNoneMode readNoneMode = ReadNoneMode::Never);
    void writeFunc(
        const char* returnType,
        const char* funcName,
        const String& combinedForward,
        const String& params,
        const String& glsl = String{},
        const String& spirvDefault = String{},
        const String& spirvRWDefault = String{},
        const String& spirvCombined = String{},
        const String& cuda = String{},
        const String& metal = String{},
        const String& wgsl = String{},
        const ReadNoneMode readNoneMode = ReadNoneMode::Never);

    // A pointer to a string representing the current level of indentation
    const char* i;
};

} // namespace Slang
