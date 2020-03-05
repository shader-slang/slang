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
        m_major(uint8_t(inMajor)),
        m_minor(uint8_t(inMinor)),
        m_patch(uint8_t(inPatch))
    {}

    void reset()
    {
        m_major = 0;
        m_minor = 0;
        m_patch = 0;
    }

    IntegerType toInteger() const { return (IntegerType(m_major) << 32) | (uint32_t(m_minor) << 16) | m_patch; }
    void setFromInteger(IntegerType v)
    {
        m_major = (v >> 32);
        m_minor = uint16_t(v >> 16);
        m_patch = uint16_t(v);
    }

    static SlangResult parse(const UnownedStringSlice& value, SemanticVersion& outVersion);
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
