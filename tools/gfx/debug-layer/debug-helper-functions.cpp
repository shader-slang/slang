// debug-helper-functions.cpp
#include "debug-helper-functions.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

String _gfxGetFuncName(const char* input)
{
    UnownedStringSlice str(input);
    auto prefixIndex = str.indexOf(UnownedStringSlice("Debug"));
    if (prefixIndex == -1)
        return input;
    auto endIndex = str.lastIndexOf('(');
    if (endIndex == -1)
        endIndex = str.getLength();
    auto startIndex = prefixIndex + 5;
    StringBuilder sb;
    sb.appendChar('I');
    sb.append(str.subString(startIndex, endIndex - startIndex));
    return sb.ProduceString();
}

template <typename... TArgs>
char* _gfxDiagnoseFormat(
    char* buffer, // Initial buffer to output formatted string.
    size_t shortBufferSize, // Size of the initial buffer.
    List<char>& bufferArray, // A list for allocating a large buffer if needed.
    const char* format, // The format string.
    TArgs... args)
{
    int length = sprintf_s(buffer, shortBufferSize, format, args...);
    if (length < 0)
        return buffer;
    if (length > 255)
    {
        bufferArray.setCount(length + 1);
        buffer = bufferArray.getBuffer();
        sprintf_s(buffer, bufferArray.getCount(), format, args...);
    }
    return buffer;
}

template <typename... TArgs>
void _gfxDiagnoseImpl(DebugMessageType type, const char* format, TArgs... args)
{
    char shortBuffer[256];
    List<char> bufferArray;
    auto buffer =
        _gfxDiagnoseFormat(shortBuffer, sizeof(shortBuffer), bufferArray, format, args...);
    getDebugCallback()->handleMessage(type, DebugMessageSource::Layer, buffer);
}

void validateAccelerationStructureBuildInputs(
    const IAccelerationStructure::BuildInputs& buildInputs)
{
    switch (buildInputs.kind)
    {
    case IAccelerationStructure::Kind::TopLevel:
        if (!buildInputs.instanceDescs)
        {
            GFX_DIAGNOSE_ERROR("IAccelerationStructure::BuildInputs::instanceDescs cannot be null "
                "when creating a top-level acceleration structure.");
        }
        break;
    case IAccelerationStructure::Kind::BottomLevel:
        if (!buildInputs.geometryDescs)
        {
            GFX_DIAGNOSE_ERROR("IAccelerationStructure::BuildInputs::geometryDescs cannot be null "
                "when creating a bottom-level acceleration structure.");
        }
        for (int i = 0; i < buildInputs.descCount; i++)
        {
            switch (buildInputs.geometryDescs[i].type)
            {
            case IAccelerationStructure::GeometryType::Triangles:
                switch (buildInputs.geometryDescs[i].content.triangles.vertexFormat)
                {
                case Format::R32G32B32_FLOAT:
                case Format::R32G32_FLOAT:
                case Format::R16G16B16A16_FLOAT:
                case Format::R16G16_FLOAT:
                case Format::R16G16B16A16_SNORM:
                case Format::R16G16_SNORM:
                    break;
                default:
                    GFX_DIAGNOSE_ERROR(
                        "Unsupported IAccelerationStructure::TriangleDesc::vertexFormat. Valid "
                        "values are R32G32B32_FLOAT, R32G32_FLOAT, R16G16B16A16_FLOAT, R16G16_FLOAT, "
                        "R16G16B16A16_SNORM or R16G16_SNORM.");
                }
                if (buildInputs.geometryDescs[i].content.triangles.indexCount)
                {
                    switch (buildInputs.geometryDescs[i].content.triangles.indexFormat)
                    {
                    case Format::R32_UINT:
                    case Format::R16_UINT:
                        break;
                    default:
                        GFX_DIAGNOSE_ERROR(
                            "Unsupported IAccelerationStructure::TriangleDesc::indexFormat. Valid "
                            "values are Unknown, R32_UINT or R16_UINT.");
                    }
                    if (!buildInputs.geometryDescs[i].content.triangles.indexData)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData cannot be null if "
                            "IAccelerationStructure::TriangleDesc::indexCount is not 0");
                    }
                }
                if (buildInputs.geometryDescs[i].content.triangles.indexFormat != Format::Unknown)
                {
                    if (buildInputs.geometryDescs[i].content.triangles.indexCount == 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexCount cannot be 0 if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is not Format::Unknown");
                    }
                    if (buildInputs.geometryDescs[i].content.triangles.indexData == 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData cannot be null if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is not "
                            "Format::Unknown");
                    }
                }
                else
                {
                    if (buildInputs.geometryDescs[i].content.triangles.indexCount != 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexCount must be 0 if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is "
                            "Format::Unknown");
                    }
                    if (buildInputs.geometryDescs[i].content.triangles.indexData != 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData must be null if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is "
                            "Format::Unknown");
                    }
                }
                if (!buildInputs.geometryDescs[i].content.triangles.vertexData)
                {
                    GFX_DIAGNOSE_ERROR(
                        "IAccelerationStructure::TriangleDesc::vertexData cannot be null.");
                }
                break;
            }
        }
        break;
    default:
        GFX_DIAGNOSE_ERROR("Invalid value of IAccelerationStructure::Kind.");
        break;
    }
}

} // namespace debug
} // namespace gfx
