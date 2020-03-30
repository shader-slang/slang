// slang-semantic-version.h
#ifndef SLANG_SEMANTIC_VERSION_H
#define SLANG_SEMANTIC_VERSION_H

#include "../core/slang-basic.h"

namespace Slang
{

struct SemanticVersion
{
    typedef SemanticVersion ThisType;

    typedef uint64_t IntegerType;

    SemanticVersion():m_major(0), m_minor(0), m_patch(0) {}
    SemanticVersion(int inMajor, int inMinor = 0, int inPatch = 0):
        m_major(uint32_t(inMajor)),
        m_minor(uint16_t(inMinor)),
        m_patch(uint16_t(inPatch))
    {}

    void reset()
    {
        m_major = 0;
        m_minor = 0;
        m_patch = 0;
    }

        /// All zeros means nothing is set
    bool isSet() const { return m_major || m_minor || m_patch; }

    IntegerType toInteger() const { return (IntegerType(m_major) << 32) | (uint32_t(m_minor) << 16) | m_patch; }
    void setFromInteger(IntegerType v)
    {
        set(int(v >> 32), int((v >> 16) & 0xffff), int(v & 0xffff));
    }
    void set(int major, int minor, int patch = 0)
    {
        SLANG_ASSERT(major >= 0 && minor >=0 && patch >= 0);

        m_major = uint32_t(major);
        m_minor = uint16_t(minor);
        m_patch = uint16_t(patch);
    }

    static SlangResult parse(const UnownedStringSlice& value, SemanticVersion& outVersion);
    static SlangResult parse(const UnownedStringSlice& value, char separatorChar, SemanticVersion& outVersion);

    void append(StringBuilder& buf) const;

    bool operator>(const ThisType& rhs) const { return toInteger() > rhs.toInteger(); }
    bool operator>=(const ThisType& rhs) const { return toInteger() >= rhs.toInteger(); }

    bool operator<(const ThisType& rhs) const { return toInteger() < rhs.toInteger(); }
    bool operator<=(const ThisType& rhs) const { return toInteger() <= rhs.toInteger(); }

    bool operator==(const ThisType& rhs) const { return toInteger() == rhs.toInteger(); }
    bool operator!=(const ThisType& rhs) const { return toInteger() != rhs.toInteger(); }

    uint32_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
};

}
#endif
