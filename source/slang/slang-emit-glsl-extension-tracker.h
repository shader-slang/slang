// slang-emit-glsl-extension-tracker.h
#ifndef SLANG_EMIT_GLSL_EXTENSION_TRACKER_H
#define SLANG_EMIT_GLSL_EXTENSION_TRACKER_H

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{

class GLSLExtensionTracker : public RefObject
{
public:

    void requireExtension(const String& name);
    void requireVersion(ProfileVersion version);
    void requireBaseTypeExtension(BaseType baseType);
    void requireSPIRVVersion(SPIRVVersion version);

    ProfileVersion getRequiredProfileVersion() const { return m_profileVersion; }
    const StringBuilder& getExtensionRequireLines() const { return m_extensionRequireLines; }

    SPIRVVersion getSPIRVVersion() const { return m_spirvVersion; }

protected:
    // Record the GLSL extensions we have already emitted a `#extension` for
    HashSet<String> m_extensionsRequired;
    StringBuilder m_extensionRequireLines;

    SPIRVVersion m_spirvVersion = makeSPIRVVersion(1, 2);

    ProfileVersion m_profileVersion = ProfileVersion::GLSL_110;

    static uint32_t _getFlag(BaseType baseType) { return uint32_t(1) << int(baseType); }

    uint32_t m_hasBaseTypeFlags = 0xffffffff & ~(_getFlag(BaseType::UInt64) + _getFlag(BaseType::Int64) + _getFlag(BaseType::Half)); 
};

}
#endif
