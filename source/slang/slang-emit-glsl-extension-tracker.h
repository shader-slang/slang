// slang-emit-glsl-extension-tracker.h
#ifndef SLANG_EMIT_GLSL_EXTENSION_TRACKER_H
#define SLANG_EMIT_GLSL_EXTENSION_TRACKER_H

#include "../core/slang-basic.h"

#include "../core/slang-string-slice-pool.h"
#include "slang-compiler.h"

#include "slang-spirv-target-info.h"

namespace Slang
{

class GLSLExtensionTracker : public RefObject
{
public:
    
        /// Return the list of extensionsspecified. NOTE that they are specified in the order requested, and they *do* have terminating zeros
    const List<UnownedStringSlice>& getExtensions() const { return m_extensionPool.getSlices(); }
        /// Return the list of SPIRV versions specified. NOTE that they are specified in the order requested, and they *do* have terminating zeros
    const List<UnownedStringSlice>& getSPIRVVersions() const { return m_spirvVersionPool.getSlices(); }

    void requireExtension(const UnownedStringSlice& name) { m_extensionPool.add(name); }
    void requireVersion(ProfileVersion version);
    void requireBaseTypeExtension(BaseType baseType);
    void requireSPIRVVersion(const UnownedStringSlice& version);
    
    ProfileVersion getRequiredProfileVersion() const { return m_profileVersion; }
    void appendExtensionRequireLines(StringBuilder& builder) const;

    const char* getSPIRVTargetName() const { return m_spirvTarget; }
    SPIRVVersion getSPIRVVersion() const { return m_spirvVersion; }

    GLSLExtensionTracker():
        m_spirvVersionPool(StringSlicePool::Style::Empty),
        m_extensionPool(StringSlicePool::Style::Empty)
    {
    }

protected:
    static uint32_t _getFlag(BaseType baseType) { return uint32_t(1) << int(baseType); }

    uint32_t m_hasBaseTypeFlags = 0xffffffff & ~(_getFlag(BaseType::UInt64) + _getFlag(BaseType::Int64) + _getFlag(BaseType::Half));

    ProfileVersion m_profileVersion = ProfileVersion::GLSL_110;

    StringSlicePool m_extensionPool;
    StringSlicePool m_spirvVersionPool;

    SPIRVVersion m_spirvVersion;
    const char* m_spirvTarget = nullptr;
};

}
#endif
