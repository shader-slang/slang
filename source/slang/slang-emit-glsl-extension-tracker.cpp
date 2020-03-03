// slang-emit-glsl-extension-tracker.cpp
#include "slang-emit-glsl-extension-tracker.h"

namespace Slang {

void GLSLExtensionTracker::requireExtension(const String& name)
{
    if (m_extensionsRequired.Contains(name))
        return;

    StringBuilder& sb = m_extensionRequireLines;

    sb.append("#extension ");
    sb.append(name);
    sb.append(" : require\n");

    m_extensionsRequired.Add(name);
}

void GLSLExtensionTracker::requireVersion(ProfileVersion version)
{
    // Check if this profile is newer
    if ((UInt)version > (UInt)m_profileVersion)
    {
        m_profileVersion = version;
    }
}

void GLSLExtensionTracker::requireSPIRVVersion(SPIRVVersion version)
{
    if (asInteger(version) > asInteger(m_spirvVersion))
    {
        m_spirvVersion = version;
    }
}

void GLSLExtensionTracker::requireBaseTypeExtension(BaseType baseType)
{
    uint32_t bit = 1 << int(baseType);
    if (m_hasBaseTypeFlags & bit)
    {
        return;
    }

    switch (baseType)
    {
        case BaseType::Half:
        {
            // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_16bit_storage.txt
            requireExtension("GL_EXT_shader_16bit_storage");

            // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt
            requireExtension("GL_EXT_shader_explicit_arithmetic_types");
            break;
        }
        case BaseType::UInt64:
        case BaseType::Int64:
        {
            requireExtension("GL_EXT_shader_explicit_arithmetic_types_int64");
            m_hasBaseTypeFlags |= _getFlag(BaseType::UInt64) | _getFlag(BaseType::Int64);
            break;
        }
    }

    m_hasBaseTypeFlags |= bit;
}


} // namespace Slang
