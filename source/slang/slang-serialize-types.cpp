// slang-serialize-types.cpp
#include "slang-serialize-types.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "../core/slang-math.h"

namespace Slang {

// Needed for linkage with some compilers
/* static */ const SerialStringData::StringIndex SerialStringData::kNullStringIndex;
/* static */ const SerialStringData::StringIndex SerialStringData::kEmptyStringIndex;

namespace { // anonymous

struct ByteReader
{
    Byte operator()() const { return Byte(*m_pos++); }
    ByteReader(const char* pos) :m_pos(pos) {}
    mutable const char* m_pos;
};

} // anonymous


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialStringTableUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void SerialStringTableUtil::encodeStringTable(const StringSlicePool& pool, List<char>& stringTable)
{
    // Skip the default handles -> nothing is encoded via them
    return encodeStringTable(pool.getAdded(), stringTable);
}
    
/* static */void SerialStringTableUtil::encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable)
{
    stringTable.clear();
    for (const auto& slice : slices)
    {
        // TODO(JS):
        // This is a bit of a hack. We need to store the string length, along with the string contents. We don't want to write
        // the size as (say) uint32, because most strings are short. So we just save off the length as a utf8 encoding.
        // As it stands this *does* have an arguable problem because encoding isn't of the full 32 bits. 
        const int len = int(slice.getLength());
        
        // We need to write into the the string array
        char prefixBytes[6];
        const int numPrefixBytes = encodeUnicodePointToUTF8(len, prefixBytes);
        const Index baseIndex = stringTable.getCount();

        auto newCount = baseIndex + numPrefixBytes + len;
        stringTable.growToCount(newCount);

        char* dst = stringTable.begin() + baseIndex;

        memcpy(dst, prefixBytes, numPrefixBytes);
        memcpy(dst + numPrefixBytes, slice.begin(), len);   
    }
}

/* static */void SerialStringTableUtil::appendDecodedStringTable(const char* table, size_t tableSize, List<UnownedStringSlice>& slicesOut)
{
    const char* start = table;
    const char* cur = start;
    const char* end = table + tableSize;

    while (cur < end)
    {
        ByteReader reader(cur);
        const int len = getUnicodePointFromUTF8(reader);
        slicesOut.add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

/* static */void SerialStringTableUtil::decodeStringTable(const char* table, size_t tableSize, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.setCount(2);
    slicesOut[0] = UnownedStringSlice(nullptr, size_t(0));
    slicesOut[1] = UnownedStringSlice("", size_t(0));

    appendDecodedStringTable(table, tableSize, slicesOut);
}

/* static */void SerialStringTableUtil::decodeStringTable(const char* table, size_t tableSize, StringSlicePool& outPool)
{
    outPool.clear();

    const char* start = table;
    const char* cur = start;
    const char* end = table + tableSize;

    while (cur < end)
    {
        ByteReader reader(cur);
        const int len = getUnicodePointFromUTF8(reader);
        outPool.add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

/* static */void SerialStringTableUtil::calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMapOut)
{
    SLANG_ASSERT(slices.getCount() >= StringSlicePool::kDefaultHandlesCount);
    SLANG_ASSERT(slices[int(StringSlicePool::kNullHandle)] == "" && slices[int(StringSlicePool::kNullHandle)].begin() == nullptr);
    SLANG_ASSERT(slices[int(StringSlicePool::kEmptyHandle)] == "");

    indexMapOut.setCount(slices.getCount());
    // Set up all of the defaults
    for (int i = 0; i < StringSlicePool::kDefaultHandlesCount; ++i)
    {
        indexMapOut[i] = StringSlicePool::Handle(i);
    }

    const Index numSlices = slices.getCount();
    for (Index i = StringSlicePool::kDefaultHandlesCount; i < numSlices ; ++i)
    {
        indexMapOut[i] = pool.add(slices[i]);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialRiffUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */ Result SerialRiffUtil::writeArrayChunk(SerialCompressionType compressionType, FourCC chunkId, const void* data, size_t numEntries, size_t typeSize, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    if (numEntries == 0)
    {
        return SLANG_OK;
    }

    // Make compressed fourCC
    chunkId = (compressionType != SerialCompressionType::None) ? SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId) : chunkId;

    ScopeChunk scope(container, Chunk::Kind::Data, chunkId);

    switch (compressionType)
    {
        case SerialCompressionType::None:
        {
            SerialBinary::ArrayHeader header;
            header.numEntries = uint32_t(numEntries);

            container->write(&header, sizeof(header));
            container->write(data, typeSize * numEntries);
            break;
        }
        case SerialCompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;

            size_t numCompressedEntries = (numEntries * typeSize) / sizeof(uint32_t);
            ByteEncodeUtil::encodeLiteUInt32((const uint32_t*)data, numCompressedEntries, compressedPayload);

            SerialBinary::CompressedArrayHeader header;
            header.numEntries = uint32_t(numEntries);
            header.numCompressedEntries = uint32_t(numCompressedEntries);

            container->write(&header, sizeof(header));
            container->write(compressedPayload.getBuffer(), compressedPayload.getCount());
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

/* static */Result SerialRiffUtil::readArrayChunk(SerialCompressionType compressionType, RiffContainer::DataChunk* dataChunk, ListResizer& listOut)
{
    typedef SerialBinary Bin;

    RiffReadHelper read = dataChunk->asReadHelper();
    const size_t typeSize = listOut.getTypeSize();

    switch (compressionType)
    {
        case SerialCompressionType::VariableByteLite:
        {
            Bin::CompressedArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));

            void* dst = listOut.setSize(header.numEntries);
            SLANG_ASSERT(header.numCompressedEntries == uint32_t((header.numEntries * typeSize) / sizeof(uint32_t)));

            // Decode..
            ByteEncodeUtil::decodeLiteUInt32(read.getData(), header.numCompressedEntries, (uint32_t*)dst);
            break;
        }
        case SerialCompressionType::None:
        {
            // Read uncompressed
            Bin::ArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));
            const size_t payloadSize = header.numEntries * typeSize;
            SLANG_ASSERT(payloadSize == read.getRemainingSize());
            void* dst = listOut.setSize(header.numEntries);
            ::memcpy(dst, read.getData(), payloadSize);
            break;
        }
    }
    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialParseUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define SLANG_SERIAL_BINARY_COMPRESSION_TYPE(x) \
    x(None, none) \
    x(VariableByteLite, lite)

/* static */SlangResult SerialParseUtil::parseCompressionType(const UnownedStringSlice& text, SerialCompressionType& outType)
{
    struct Pair
    {
        UnownedStringSlice name;
        SerialCompressionType type;
    };

#define SLANG_SERIAL_BINARY_PAIR(type, name) { UnownedStringSlice::fromLiteral(#name), SerialCompressionType::type},

    static const Pair s_pairs[] =
    {
        SLANG_SERIAL_BINARY_COMPRESSION_TYPE(SLANG_SERIAL_BINARY_PAIR)
    };

    for (const auto& pair : s_pairs)
    {
        if (pair.name == text)
        {
            outType = pair.type;
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */UnownedStringSlice SerialParseUtil::getText(SerialCompressionType type)
{
#define SLANG_SERIAL_BINARY_CASE(type, name) case SerialCompressionType::type: return UnownedStringSlice::fromLiteral(#name);
    switch (type)
    {
        SLANG_SERIAL_BINARY_COMPRESSION_TYPE(SLANG_SERIAL_BINARY_CASE)
        default: break;
    }
    SLANG_ASSERT(!"Unknown compression type");
    return UnownedStringSlice::fromLiteral("unknown");
}


} // namespace Slang
