// slang-emit-glsl-extension-tracker.h
#ifndef SLANG_EMIT_GLSL_EXTENSION_TRACKER_H
#define SLANG_EMIT_GLSL_EXTENSION_TRACKER_H

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{

class GLSLExtensionTracker
{
public:

    void requireExtension(const String& name);
    void requireVersion(ProfileVersion version);
    void requireHalfExtension();

    ProfileVersion getRequiredProfileVersion() const { return m_profileVersion; }
    const StringBuilder& getExtensionRequireLines() const { return m_extensionRequireLines; }

protected:
    // Record the GLSL extensions we have already emitted a `#extension` for
    HashSet<String> m_extensionsRequired;
    StringBuilder m_extensionRequireLines;

    ProfileVersion m_profileVersion = ProfileVersion::GLSL_110;

    bool m_hasHalfExtension = false;
};

}
#endif
