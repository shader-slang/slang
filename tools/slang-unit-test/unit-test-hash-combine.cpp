// unit-test-hash-combine.cpp

#include "core/slang-hash.h"
#include "core/slang-list.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

// A cheap deterministic generator, so the test does not depend on <random>
// giving the same sequence everywhere.
struct Xorshift
{
    uint64_t state = 0x0123456789abcdefULL;
    uint64_t operator()()
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }
};

// Counts how the low `log2(kBuckets)` bits of a set of hashes are distributed,
// which is what a power-of-two-sized hash map uses to pick a bucket.
struct LowBitHistogram
{
    static const Index kBuckets = 64;
    List<Index> counts;

    LowBitHistogram()
    {
        counts.setCount(kBuckets);
        for (Index i = 0; i < kBuckets; ++i)
            counts[i] = 0;
    }

    void add(HashCode64 hash) { counts[Index(hash & (kBuckets - 1))]++; }

    Index getUsedBucketCount() const
    {
        Index used = 0;
        for (auto c : counts)
            used += (c != 0);
        return used;
    }

    Index getLargestBucket() const
    {
        Index largest = 0;
        for (auto c : counts)
            largest = c > largest ? c : largest;
        return largest;
    }
};

} // namespace

// `combineHash` has to spread entropy into *every* bit of its result, not just
// the high ones, because a hash map is free to take its bucket index straight
// from the low bits without mixing the hash further -- and which maps do that
// is an implementation detail we do not control.
//
// The shape exercised here is the one the compiler's deduplication keys
// actually have: a node tag, followed by a run of pointer operands. Pointers
// are aligned, so their low bits are always zero. A combining step that only
// moves entropy upwards -- `h * prime ^ m`, which this used to be -- therefore
// leaves the low bits of the result determined by the tag and the operand
// count alone. Measured on that shape, the bottom three bits took exactly one
// value out of eight, so only an eighth of the buckets were reachable.
SLANG_UNIT_TEST(combineHashSpreadsIntoLowBits)
{
    const HashCode64 tag = getHashCode(12345);
    Xorshift rng;

    LowBitHistogram histogram;
    const Index sampleCount = LowBitHistogram::kBuckets * 200;
    for (Index i = 0; i < sampleCount; ++i)
    {
        HashCode64 hash = tag;
        for (Index operand = 0; operand < 4; ++operand)
        {
            // An eight-byte-aligned "pointer": no entropy in the low 3 bits.
            const uint64_t pointerLike = rng() & ~UINT64_C(7);
            hash = combineHash(hash, pointerLike);
        }
        histogram.add(hash);
    }

    SLANG_CHECK(histogram.getUsedBucketCount() == LowBitHistogram::kBuckets);

    // With a uniform hash each bucket expects `sampleCount / kBuckets` entries.
    // Allow a generous factor before calling it clustered; the failure this
    // guards against is order-of-magnitude, not marginal.
    const Index expected = sampleCount / LowBitHistogram::kBuckets;
    SLANG_CHECK(histogram.getLargestBucket() < expected * 3);
}

// The same property, for a key that ends with a boolean rather than a pointer.
// `ImplicitCastMethodKey` has this shape, and under the old combining step the
// bottom bit of its hash was simply the value of its last `bool` member.
SLANG_UNIT_TEST(combineHashSpreadsTrailingFlags)
{
    Xorshift rng;

    LowBitHistogram histogram;
    const Index sampleCount = LowBitHistogram::kBuckets * 200;
    for (Index i = 0; i < sampleCount; ++i)
    {
        const HashCode64 hash = combineHash(
            getHashCode(rng() & ~UINT64_C(7)),
            getHashCode(rng() & ~UINT64_C(7)),
            HashCode32(i & 1),
            HashCode32((i >> 1) & 1));
        histogram.add(hash);
    }

    SLANG_CHECK(histogram.getUsedBucketCount() == LowBitHistogram::kBuckets);
    const Index expected = sampleCount / LowBitHistogram::kBuckets;
    SLANG_CHECK(histogram.getLargestBucket() < expected * 3);
}

// Changing *any* single bit of an operand has to be able to change the bucket
// a key lands in. A step built only from a multiply fails this at the top of
// the word, because multiplying carries bits upwards and nothing brings them
// back down: an earlier version of `foldHashStep` moved just two bits of the
// result when the top bit of an operand was flipped, and never once changed
// the bottom six, so two keys differing only high up in an operand always
// shared a bucket.
SLANG_UNIT_TEST(combineHashEveryOperandBitReachesTheBucketIndex)
{
    const HashCode64 tag = getHashCode(999);
    Xorshift rng;

    const Index trials = 400;
    for (Index bit = 0; bit < 64; ++bit)
    {
        Index changedTheBucket = 0;
        for (Index i = 0; i < trials; ++i)
        {
            const uint64_t operand = rng();
            const HashCode64 before = combineHash(tag, operand);
            const HashCode64 after = combineHash(tag, operand ^ (UINT64_C(1) << bit));
            changedTheBucket += ((before ^ after) & (LowBitHistogram::kBuckets - 1)) != 0;
        }
        // A well-mixed step changes the bucket about half the time. Anything
        // far below that means this input bit is barely reaching the index.
        SLANG_CHECK(changedTheBucket > trials / 4);
    }
}

// Folding is order-sensitive and injective in each step, so two keys that
// differ only in the order of their operands must not collide.
SLANG_UNIT_TEST(combineHashIsOrderSensitive)
{
    const HashCode64 a = getHashCode(0x1111u);
    const HashCode64 b = getHashCode(0x2222u);
    const HashCode64 c = getHashCode(0x3333u);

    SLANG_CHECK(combineHash(a, b, c) != combineHash(a, c, b));
    SLANG_CHECK(combineHash(a, b, c) != combineHash(c, b, a));
    SLANG_CHECK(combineHash(a, b) != combineHash(b, a));

    // And a `Hasher` must agree with the equivalent `combineHash` fold, since
    // the two are used interchangeably to build keys.
    Hasher hasher;
    hasher.addHash(a);
    hasher.addHash(b);
    SLANG_CHECK(hasher.getResult() == combineHash(combineHash(HashCode64(0), a), b));
}
