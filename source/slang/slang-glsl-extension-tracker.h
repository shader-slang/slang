// slang-glsl-extension-tracker.h
#ifndef SLANG_GLSL_EXTENSION_TRACKER_H
#define SLANG_GLSL_EXTENSION_TRACKER_H

#include "../core/slang-basic.h"

#include "../core/slang-string-slice-pool.h"
#include "../core/slang-semantic-version.h"

#include "slang-compiler.h"

namespace Slang
{

class GLSLExtensionTracker : public ExtensionTracker
{
public:
    
        /// Return the list of extensionsspecified. NOTE that they are specified in the order requested, and they *do* have terminating zeros
    const List<UnownedStringSlice>& getExtensions() const { return m_extensionPool.getSlices(); }
    
    void requireExtension(const UnownedStringSlice& name) { m_extensionPool.add(name); }
    void requireVersion(ProfileVersion version);
    void requireBaseTypeExtension(BaseType baseType);
    void requireSPIRVVersion(const SemanticVersion& version);
    
    ProfileVersion getRequiredProfileVersion() const { return m_profileVersion; }
    void appendExtensionRequireLines(StringBuilder& builder) const;

    const SemanticVersion& getSPIRVVersion() const { return m_spirvVersion; }

    GLSLExtensionTracker():
        m_extensionPool(StringSlicePool::Style::Empty)
    {
    }

protected:
    static uint32_t _getFlag(BaseType baseType) { return uint32_t(1) << int(baseType); }

    uint32_t m_hasBaseTypeFlags = _getFlag(BaseType::Float) | _getFlag(BaseType::Int) | _getFlag(BaseType::UInt) | _getFlag(BaseType::Void) | _getFlag(BaseType::Bool);

    ProfileVersion m_profileVersion = ProfileVersion::GLSL_110;

    StringSlicePool m_extensionPool;

    SemanticVersion m_spirvVersion;
};

}
#endif
