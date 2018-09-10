
#include "slang-random-generator.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! RandomGenerator !!!!!!!!!!!!!!!!!!!!!!!! */

float RandomGenerator::nextUnitFloat32()
{
    int32_t intValue = nextInt32();
    return (intValue & 0x7fffffff) * (1.0f / 0x7fffffff);
}

bool RandomGenerator::nextBool()
{
    uint32_t bits = uint32_t(nextInt32());

    // Xor together all bits in each byte
    bits = ((bits & 0xaaaaaaaa) >> 1) ^ (bits & 0x55555555);
    bits = ((bits & 0x44444444) >> 2) ^ (bits & 0x11111111);
    bits = ((bits & 0x10101010) >> 4) ^ (bits & 0x01010101);

    // In effect is the xor of all the bits of the original last byte 
    return ( bits & 1) != 0;
}

int64_t RandomGenerator::nextInt64()
{
    const int32_t high = nextInt32();
    const int32_t low = nextInt32();

    return (int64_t(high) << 32) | low;
}

/* static */RandomGenerator* RandomGenerator::create(int32_t seed)
{
    return new DefaultRandomGenerator(seed);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! Mt19937RandomGenerator !!!!!!!!!!!!!!!!!!!!!!!! */

Mt19937RandomGenerator::Mt19937RandomGenerator()
{
    reset(21452);
}

Mt19937RandomGenerator::Mt19937RandomGenerator(const ThisType& rhs)
{
    *this = rhs;
}

Mt19937RandomGenerator::Mt19937RandomGenerator(int32_t seed)
{
    reset(seed);
}

void Mt19937RandomGenerator::_generate()
{
    const uint32_t xorValue = 2567483615u;
    for (int i = 0; i < kNumEntries - 1; ++i)
    {
        const uint32_t y = (m_mt[i] & 0x80000000) + (m_mt[i + 1] & 0x7fffffff);    

        // o = (i + 397) % kNumEntries
        int32_t o = i + 397;
        o = (o >= kNumEntries) ? (o - kNumEntries) : o;

        m_mt[i] = m_mt[o] ^ (y >> 1);
        // If y is odd
        if (y & 1)
        {
            m_mt[i] = m_mt[i] ^ xorValue;
        }
    }

    // Last
    {
        const int i = kNumEntries - 1;
        const uint32_t y = (m_mt[i] & 0x80000000) + (m_mt[0] & 0x7fffffff);
        const int32_t o = ((i + 397) - kNumEntries);

        m_mt[i] = m_mt[o] ^ (y >> 1);
        // If y is odd
        if (y & 1)
        {
            m_mt[i] = m_mt[i] ^ xorValue;
        }
    }
}

void Mt19937RandomGenerator::reset(int32_t seedIn)
{
    m_index = 0;
    m_mt[0] = uint32_t(seedIn);
    for (int i = 1; i < kNumEntries; ++i)
    {
        m_mt[i] = (1812433253 * (m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) + i);
    }
}

int32_t Mt19937RandomGenerator::nextInt32()
{
    if (m_index >= kNumEntries)
    {
        _generate();
    }

    uint32_t y = m_mt[m_index++];
    y = y ^ (y >> 11);
    y = y ^ ((y << 7) & uint32_t(0x9d2c5680u));
    y = y ^ ((y << 15) & uint32_t(0xefc6000u)); 
    y = y ^ (y >> 18);

    return int32_t(y);
}

} // namespace Slang
