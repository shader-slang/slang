// slang-extension-usage-tracker.cpp
#include "slang-extension-usage-tracker.h"

namespace Slang {

void ExtensionUsageTracker::requireGLSLExtension(const String& name)
{
    if (m_glslExtensionsRequired.Contains(name))
        return;

    StringBuilder& sb = m_glslExtensionRequireLines;

    sb.append("#extension ");
    sb.append(name);
    sb.append(" : require\n");

    m_glslExtensionsRequired.Add(name);
}

void ExtensionUsageTracker::requireGLSLVersion(ProfileVersion version)
{
    // Check if this profile is newer
    if ((UInt)version > (UInt)m_glslProfileVersion)
    {
        m_glslProfileVersion = version;
    }
}

void ExtensionUsageTracker::requireGLSLHalfExtension()
{
    if (!m_hasGLSLHalfExtension)
    {
        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_16bit_storage.txt
        requireGLSLExtension("GL_EXT_shader_16bit_storage");

        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt
        requireGLSLExtension("GL_EXT_shader_explicit_arithmetic_types");

        m_hasGLSLHalfExtension = true;
    }
}


} // namespace Slang
