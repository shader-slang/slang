// slang-extension-usage-tracker.h
#ifndef SLANG_EXTENSION_USAGE_TRACKER_H_INCLUDED
#define SLANG_EXTENSION_USAGE_TRACKER_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{

class ExtensionUsageTracker
{
public:

    void requireGLSLExtension(const String& name);
    void requireGLSLVersion(ProfileVersion version);
    void requireGLSLHalfExtension();

    ProfileVersion getRequiredGLSLProfileVersion() const { return m_glslProfileVersion; }
    const StringBuilder& getGLSLExtensionRequireLines() const { return m_glslExtensionRequireLines; }

protected:
    // Record the GLSL extensions we have already emitted a `#extension` for
    HashSet<String> m_glslExtensionsRequired;
    StringBuilder m_glslExtensionRequireLines;

    ProfileVersion m_glslProfileVersion = ProfileVersion::GLSL_110;

    bool m_hasGLSLHalfExtension = false;
};

}
#endif
