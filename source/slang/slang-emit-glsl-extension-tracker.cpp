// slang-emit-glsl-extension-tracker.cpp
#include "slang-emit-glsl-extension-tracker.h"

namespace Slang {

void GLSLExtensionTracker::appendExtensionRequireLines(StringBuilder& ioBuilder) const
{
    for (const auto& extension : m_extensionPool.getSlices())
    {
        ioBuilder.append("#extension ");
        ioBuilder.append(extension);
        ioBuilder.append(" : require\n");
    }
}

void GLSLExtensionTracker::requireSPIRVVersion(const UnownedStringSlice& inVersion)
{
    StringSlicePool::Handle handle;
    if (m_spirvVersionPool.findOrAdd(inVersion, handle))
    {
        // If we already have it, then we are done
        return;
    }
    // We want the version that will stay in scope
    UnownedStringSlice version = m_spirvVersionPool.getSlice(handle);
    SPIRVTargetInfo info;
    if (SLANG_FAILED(SPIRVTargetInfo::find(version, info)))
    {
        // Couldn't determine how to use
        SLANG_ASSERT(!"Unknown SPIR-V target name");
        return;
    }

    // We want to test that it's contained, and that it ends at the end of version (so we have 0 termination)
    SLANG_ASSERT(version.isMemoryContained(info.targetName) && version.end() == info.targetName.end());

    // Looked up version. We know it's a substring, that's 0
    version = info.targetName;

    const Index intLanguageVersion = info.version.toInteger();
    const Index currentIntLanguageVersion = m_spirvVersion.toInteger();

    if (intLanguageVersion > currentIntLanguageVersion)
    {
        // Set the language version
        m_spirvVersion = info.version;

        // The universal target is higher, assume it subsumes the lower versioned more specific target.
        // If there is no target just use this one
        if ((info.flags & SPIRVTargetFlag::Universal) || m_spirvTarget == nullptr)
        {
            // NOTE! We know this is null terminated and won't go out of scope because is in the pool
            m_spirvTarget = version.begin();
        }
    }
    else if (intLanguageVersion == currentIntLanguageVersion)
    {
        if (m_spirvTarget)
        {
            // If target is set, see if we should replace it. If it's the same, we are done
            if (version != UnownedStringSlice(m_spirvTarget))
            {
                // If the new target is *not* universal (ie is a more specific target) 
                if ((info.flags & SPIRVTargetFlag::Universal) == 0)
                {
                    // And the current target is universal.. 
                    SPIRVTargetInfo currentInfo;
                    if (SLANG_SUCCEEDED(SPIRVTargetInfo::find(UnownedStringSlice(m_spirvTarget), currentInfo)) &&
                        (currentInfo.flags & SPIRVTargetFlag::Universal))
                    {
                        // Then we can use the new one, as it's the same version, but more specific.
                        m_spirvTarget = version.begin();
                    }
                }
            }
        }
        else
        {
            // If no target is set, we can just set it
            m_spirvTarget = version.begin();
        }
    }
}

void GLSLExtensionTracker::requireVersion(ProfileVersion version)
{
    // Check if this profile is newer
    if ((UInt)version > (UInt)m_profileVersion)
    {
        m_profileVersion = version;
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
            requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_shader_16bit_storage"));

            // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt
            requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_shader_explicit_arithmetic_types"));
            break;
        }
        case BaseType::UInt64:
        case BaseType::Int64:
        {
            requireExtension(UnownedStringSlice::fromLiteral("GL_EXT_shader_explicit_arithmetic_types_int64"));
            m_hasBaseTypeFlags |= _getFlag(BaseType::UInt64) | _getFlag(BaseType::Int64);
            break;
        }
    }

    m_hasBaseTypeFlags |= bit;
}

} // namespace Slang
