#ifndef SLANG_RIFF_H
#define SLANG_RIFF_H

#include "../core/slang-basic.h"
#include "../core/slang-stream.h"

namespace Slang
{

// http://fileformats.archiveteam.org/wiki/RIFF
// http://www.fileformat.info/format/riff/egff.htm

#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 0) | (uint32_t(c1) << 8) | (uint32_t(c2) << 16) | (uint32_t(c3) << 24)) 

struct RiffFourCC
{
     static const uint32_t kRiff = SLANG_FOUR_CC('R', 'I', 'F', 'F');
};

// Follows semantic version rules
// https://semver.org/
// 
// major.minor.patch
// Patch versions indicate a change. 
// Minor means a change that is backwards compatible with previous minor versions. A step in minor and/or major zeros patch.
// Major means a non compatible change. A step in major, zeros minor and patch. 
struct RiffSemanticVersion
{
    typedef RiffSemanticVersion ThisType;
    typedef uint32_t RawType;

        /// ==
    bool operator==(const ThisType& rhs) const { return m_raw == rhs.m_raw; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }
    
        /// A patch change indices a different version but does not change the compatibility of the format
    int getPatch() const { return m_raw & 0xff; }
        /// A minor change implies a format change that is backwards compatible  
    int getMinor() const { return (m_raw >> 8) & 0xff; }
        /// A major change is binary incompatible by default
    int getMajor() const { return (m_raw >> 16); }

    static RawType makeRaw(int major, int minor, int patch)
    {
        SLANG_ASSERT((major | minor | patch) >= 0);
        SLANG_ASSERT(major < 0x10000 && minor < 0x100 && patch < 0x100);
        return (RawType(major) << 16) | (RawType(minor) << 8) | RawType(patch);
    }

    static RiffSemanticVersion makeFromRaw(RawType raw)
    {
        ThisType version;
        version.m_raw = raw;
        return version;
    }

    static RiffSemanticVersion make(int major, int minor, int patch) { return makeFromRaw(makeRaw(major, minor, patch)); }

        /// 
    static const bool areCompatible(const ThisType& currentVersion, const ThisType& readVersion)
    {
        const RawType currentRaw = currentVersion.m_raw;
        const RawType readRaw = readVersion.m_raw;

        // Must have same major version.
        // For minor version, the read version must be less than or equal. 
        return ((currentRaw & 0xffff0000) == (readRaw & 0xffff0000)) && ((currentRaw & 0xff00) >= (readRaw & 0xff00));
    }

    RawType m_raw;
};

struct RiffChunk
{
    uint32_t m_type;            ///< The FourCC code that identifies this chunk
    uint32_t m_size;            ///< Size does *NOT* include the riff chunk size. The size can be byte sized, but on storage it will always be treated as aligned up by 4.
};

struct RiffUtil
{
    static int64_t calcChunkTotalSize(const RiffChunk& chunk);

    static SlangResult skip(const RiffChunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    static SlangResult readChunk(Stream* stream, RiffChunk& outChunk);

    static SlangResult writeData(const RiffChunk* header, size_t headerSize, const void* payload, size_t payloadSize, Stream* out);
    static SlangResult readData(Stream* stream, RiffChunk* outHeader, size_t headerSize, List<uint8_t>& data);
};

}

#endif
