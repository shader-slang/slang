// slang-spirv-target-info.h
#ifndef SLANG_SPIRV_TARGET_INFO_H
#define SLANG_SPIRV_TARGET_INFO_H

#include "../core/slang-basic.h"

namespace Slang
{

struct SPIRVVersion
{
    SPIRVVersion():major(0), minor(0), patch(0) {}
    SPIRVVersion(int inMajor, int inMinor = 0, int inPatch = 0):
        major(uint8_t(inMajor)),
        minor(uint8_t(inMinor)),
        patch(uint8_t(inPatch))
    {}

    Index toInteger() const { return (Index(major) << 16) | (Index(minor) << 8) | patch; }
    SPIRVVersion setFromInteger(Index i)
    {
        major = uint8_t((i >> 16) & 0xff);
        minor = uint8_t((i >> 8) & 0xff);
        patch = uint8_t((i >> 0) & 0xff);
    }

    uint8_t major, minor, patch;
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
