#include "slang-perfect-hash.h"

#include "../core/slang-string-util.h"
#include "../core/slang-writer.h"

namespace Slang
{

// Implemented according to "Hash, displace, and compress"
// https://cmph.sourceforge.net/papers/esa09.pdf
HashFindResult minimalPerfectHash(const List<String>& ss, HashParams& hashParams)
{
    // Check for uniqueness
    for (Index i = 0; i < ss.getCount(); ++i)
    {
        for (Index j = i + 1; j < ss.getCount(); ++j)
        {
            if (ss[i] == ss[j])
            {
                return HashFindResult::NonUniqueKeys;
            }
        }
    }

    SLANG_ASSERT(UIndex(ss.getCount()) < std::numeric_limits<UInt32>::max());
    const UInt32       nBuckets = UInt32(ss.getCount());
    List<List<String>> initialBuckets;
    initialBuckets.setCount(nBuckets);

    const auto hash = [&](const String& s, const HashCode64 salt = 0) -> UInt32
    {
        //
        // The current getStableHashCode is susceptible to patterns of
        // collisions causing the search to fail for the SPIR-V opnames; it
        // performs poorly on short strings, taking over 300000 iterations to
        // diverge on "Ceil" and "FMix" (and place them in already unoccupied
        // slots)!
        //
        // Use FNV Hash here which seem perform much better on these short inputs
        // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
        //
        // If you change this, don't forget to also sync the version below in
        // the printing code.
        UInt64 h = salt;
        for (const char c : s) h = ((h * 0x00000100000001B3) ^ c);
        return h % nBuckets;
    };

    // Assign the inputs into their buckets according to the hash without salt.
    // Sort the buckets according to size, so that later we can make these have
    // unique destinations starting with the largest ones first as they are at
    // most risk of collision.
    for (const auto& s : ss)
    {
        initialBuckets[hash(s)].add(s);
    }
    initialBuckets.stableSort([](const List<String>& a, const List<String>& b) { return a.getCount() > b.getCount(); });

    // These are our outputs, the salts are calculated such that for all input
    // word, x, hash(x, salt[hash(x, 0)]) is unique
    //
    // We keep the final table as we need to detect when we've been given a
    // word not in our language.
    hashParams.saltTable.setCount(nBuckets);
    for (auto& s : hashParams.saltTable)
    {
        s = 0;
    }
    hashParams.destTable.setCount(nBuckets);
    for (auto& s : hashParams.destTable)
    {
        s.reduceLength(0);
    }

    // This mask will, in each salt tryout, be used to prevent collisions
    // within a single bucket.
    List<bool> bucketDestinations = List<bool>::makeRepeated(false, nBuckets);

    for (const auto& b : initialBuckets)
    {
        // Break if we've reached the empty buckets
        if (!b.getCount())
        {
            break;
        }

        // Try out all the salts until we get one which has no internal
        // collisions for this bucket and also no collisions with the buckets
        // we've processed so far.
        UInt32 salt = 1;
        while (true)
        {
            bool collision = false;
            for (auto& d : bucketDestinations)
            {
                d = false;
            }

            for (const auto& s : b)
            {
                const auto i = hash(s, salt);
                if (hashParams.destTable[i].getLength() || bucketDestinations[i])
                {
                    collision = true;
                    break;
                }
                bucketDestinations[i] = true;
            }
            if (!collision)
            {
                break;
            }
            salt++;

            // If we fail to find a solution after some massive amount of tries
            // it's almost certainly because of some property of the hash
            // function and language causing an irresolvable collision.
            if (salt > 10000 * nBuckets)
            {
                return HashFindResult::UnavoidableHashCollision;
            }
        }
        for (const auto& s : b)
        {
            hashParams.saltTable[hash(s)] = salt;
            hashParams.destTable[hash(s, salt)] = s;
        }
    }
    return HashFindResult::Success;
}

String perfectHashToEmbeddableCpp(
    const HashParams& hashParams,
    const UnownedStringSlice& valueType,
    const UnownedStringSlice& valuePrefix)
{
    StringBuilder sb;
    StringWriter writer(&sb, WriterFlags(0));
    WriterHelper w(&writer);

    w.print("static const unsigned tableSalt[%ld] =", hashParams.saltTable.getCount());
    w.print("{\n   ");
    for (Index i = 0; i < hashParams.saltTable.getCount(); ++i)
    {
        const auto salt = hashParams.saltTable[i];
        if (i != hashParams.saltTable.getCount() - 1)
        {
            w.print(" %d,", salt);
            if (i % 16 == 15)
            {
                w.print("\n   ");
            }
        }
        else
        {
            w.print(" %d", salt);
        }
    }
    w.print("\n};\n");
    w.print("\n");

    w.print("struct KV\n");
    w.print("{\n");
    w.print("    const char* name;\n");
    w.print("    %s value;\n", String(valueType).getBuffer());
    w.print("};\n");
    w.print("\n");

    w.print("static const KV words[%ld] =\n", hashParams.destTable.getCount());
    w.print("{\n");
    for (const auto& s : hashParams.destTable)
    {
        w.print("    {\"%s\", %s%s},\n", s.getBuffer(), String(valuePrefix).getBuffer(), s.getBuffer());
    }
    w.print("};\n");
    w.print("\n");

    // Make sure to update the hash function in the search function above if
    // you change this.
    w.print("static UInt32 hash(const UnownedStringSlice& str, UInt32 salt)\n");
    w.print("{\n");
    w.print("    UInt64 h = salt;\n");
    w.print("    for(const char c : str)\n");
    w.print("        h = ((h * 0x00000100000001B3) ^ c);\n");
    w.print("    return h %% (sizeof(tableSalt)/sizeof(tableSalt[0]));\n");
    w.print("}\n");
    w.print("\n");

    w.print("bool lookup%s(const UnownedStringSlice& str, %s& value)\n", String(valueType).getBuffer(), String(valueType).getBuffer());
    w.print("{\n");
    w.print("    const auto i = hash(str, tableSalt[hash(str, 0)]);\n");
    w.print("    if(str == words[i].name)\n");
    w.print("    {\n");
    w.print("        value = words[i].value;\n");
    w.print("        return true;\n");
    w.print("    }\n");
    w.print("    else\n");
    w.print("    {\n");
    w.print("        return false;\n");
    w.print("    }\n");
    w.print("}\n");
    w.print("\n");

    return sb;
}

}
