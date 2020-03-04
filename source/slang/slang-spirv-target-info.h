// slang-spirv-target-info.h
#ifndef SLANG_SPIRV_TARGET_INFO_H
#define SLANG_SPIRV_TARGET_INFO_H

#include "../core/slang-basic.h"

namespace Slang
{

struct SPIRVVersion
{
    SPIRVVersion():m_major(0), m_minor(0), m_patch(0) {}
    SPIRVVersion(int inMajor, int inMinor = 0, int inPatch = 0):
        m_major(uint8_t(inMajor)),
        m_minor(uint8_t(inMinor)),
        m_patch(uint8_t(inPatch))
    {}

    Index toInteger() const { return (Index(m_major) << 16) | (Index(m_minor) << 8) | m_patch; }
    SPIRVVersion setFromInteger(Index i)
    {
        m_major = uint8_t((i >> 16) & 0xff);
        m_minor = uint8_t((i >> 8) & 0xff);
        m_patch = uint8_t((i >> 0) & 0xff);
    }

    uint8_t m_major, m_minor, m_patch;
};

struct SPIRVTargetFlag
{
    typedef uint32_t IntegerType;
    enum Enum : IntegerType
    {
        Universal = 0x1
    };
};
typedef SPIRVTargetFlag::IntegerType SPIRVTargetFlags;

struct SPIRVTargetInfo
{
        /// Find an info based on the name.
        /// NOTE! That the targetName extracted can only be a substring from name
    static SlangResult find(const UnownedStringSlice& name, SPIRVTargetInfo& outInfo);

    UnownedStringSlice targetName;
    SPIRVVersion version;
    SPIRVTargetFlags flags = 0;
};

}
#endif
