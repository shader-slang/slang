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

void GLSLExtensionTracker::requireHalfExtension()
{
    if (!m_hasHalfExtension)
    {
        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_16bit_storage.txt
        requireExtension("GL_EXT_shader_16bit_storage");

        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt
        requireExtension("GL_EXT_shader_explicit_arithmetic_types");

        m_hasHalfExtension = true;
    }
}


} // namespace Slang
