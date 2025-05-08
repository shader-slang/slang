// unit-test-riff.cpp

#include "../../source/core/slang-random-generator.h"
#include "../../source/core/slang-riff.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static void _writeRandom(
    RandomGenerator* rand,
    size_t maxSize,
    RIFF::BuildCursor& cursor,
    List<uint8_t>& ioData)
{
    while (true)
    {
        const Index oldCount = ioData.getCount();

        const size_t allocSize = size_t(rand->nextInt32InRange(1, 50));

        if (allocSize + oldCount > maxSize)
        {
            break;
        }

        ioData.setCount(oldCount + Index(allocSize));
        rand->nextData(ioData.getBuffer() + oldCount, allocSize);

        // Write
        cursor.addData(ioData.getBuffer() + oldCount, allocSize);
    }

    // Should be a single block with same data as the List
    auto dataChunk =
        as<RIFF::DataChunkBuilder>(cursor.getCurrentChunk());
    SLANG_ASSERT(dataChunk);
}

namespace
{
    struct DumpContext
    {
    private:
        WriterHelper _writer;
        Count _indent = 0;
        Count _hexByteCount = 0;
        bool _isRoot = true;

    public:
        DumpContext(ISlangWriter* writer)
            : _writer(writer)
        {}

        void beginListChunk(RIFF::Chunk::Type type)
        {
            _dumpIndent();
            // If it's the root it's 'riff'
            _dumpRiffType(_isRoot ? RIFF::RootChunk::kTag : RIFF::ListChunk::kTag);
            _writer.put(" ");
            _dumpRiffType(type);
            _writer.put("\n");
            _indent++;
        }

        void endListChunk()
        {
            _indent--;
        }

        void beginDataChunk(RIFF::Chunk::Type type)
        {
            _dumpIndent();
            // Write out the name
            _dumpRiffType(type);
            _writer.put("\n");
            _indent++;

            _hexByteCount = 0;
        }

        void endDataChunk()
        {
            _indent--;
        }

        void handleData(void const* data, Size size)
        {
            auto cursor = static_cast<Byte const*>(data);
            auto remainingSize = size;
            while (remainingSize--)
            {
                auto byte = *cursor++;

                static const Count kBytesPerLine = 32;
                static const Count kBytesPerCluster = 4;
                if (_hexByteCount % kBytesPerLine == 0)
                {
                    _writer.put("\n");
                    _dumpIndent();
                }
                else if (_hexByteCount % kBytesPerCluster == 0)
                {
                    _writer.put(" ");
                }
                _hexByteCount++;

                char text[4] = { 0, 0, ' ', 0};

                char const* hexDigits = "0123456789abcdef";
                text[0] = hexDigits[(byte >> 4) & 0xF];
                text[1] = hexDigits[(byte >> 0) & 0xF];

                _writer.put(text);
            }
        }

        void _dumpIndent()
        {
            for (int i = 0; i < _indent; ++i)
            {
                _writer.put("  ");
            }
        }
        void _dumpRiffType(FourCC fourCC)
        {
            auto rawValue = FourCC::RawValue(fourCC);

            char text[5];
            for (int i = 0; i < 4; ++i)
            {
                text[i] = char(rawValue & 0xFF);
                rawValue >>= 8;
            }
            text[4] = 0;
            _writer.put(text);
        }
    };

} // namespace

static void _dump(RIFF::Chunk const* chunk, DumpContext context)
{
    if (auto listChunk = as<RIFF::ListChunk>(chunk))
    {
        context.beginListChunk(listChunk->getType());
        for (auto child : listChunk->getChildren())
            _dump(child, context);
        context.endListChunk();
    }
    else if (auto dataChunk = as<RIFF::DataChunk>(chunk))
    {
        context.beginDataChunk(dataChunk->getType());
        context.handleData(dataChunk->getPayload(), dataChunk->getPayloadSize());
        context.endDataChunk();
    }
}

static void _dump(RIFF::ChunkBuilder* chunk, DumpContext context)
{
    if (auto listChunk = as<RIFF::ListChunkBuilder>(chunk))
    {
        context.beginListChunk(listChunk->getType());
        for (auto child : listChunk->getChildren())
            _dump(child, context);
        context.endListChunk();
    }
    else if (auto dataChunk = as<RIFF::DataChunkBuilder>(chunk))
    {
        context.beginDataChunk(dataChunk->getType());
        for (auto shard : dataChunk->getShards())
            context.handleData(shard->getPayload(), shard->getPayloadSize());
        context.endDataChunk();
    }
}

static bool _isSingleShard(RIFF::DataChunkBuilder* chunk)
{
    Count count = 0;
    for (auto shard : chunk->getShards())
    {
        count++;
        if (count > 1)
            break;
    }
    return count == 1;
}

static bool _isEqual(RIFF::DataChunkBuilder* chunk, void const* data, Size size)
{
    auto remainingData = static_cast<Byte const*>(data);
    auto remainingSize = size;

    for (auto shard : chunk->getShards())
    {
        // If there is more content in the chunk than remains
        // to compare against, then there is no chance of a match.
        //
        auto shardSize = shard->getPayloadSize();
        if (shard->getPayloadSize() > remainingSize)
        {
            return false;
        }

        // Contents must match, byte-for-byte.
        //
        if (::memcmp(remainingData, shard->getPayload(), shardSize) != 0)
        {
            return false;
        }

        remainingData += shardSize;
        remainingSize -= shardSize;
    }

    // If we reach the end of the chunk, then we have
    // a match if there is no data remaining to
    // compare against.
    //
    return remainingSize == 0;
}

SLANG_UNIT_TEST(riff)
{
    const FourCC markThings = SLANG_FOUR_CC('T', 'H', 'I', 'N');
    const FourCC markData = SLANG_FOUR_CC('D', 'A', 'T', 'A');

    {
        RIFF::Builder riffBuilder;
        RIFF::BuildCursor cursor(riffBuilder);

        {
            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, markThings);
            {
                SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, markData);

                const char hello[] = "Hello ";
                const char world[] = "World!";

                cursor.addData(hello, sizeof(hello));
                cursor.addData(world, sizeof(world));
            }

            {
                SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, markData);

                const char test0[] = "Testing... ";
                const char test1[] = "Testing!";

                cursor.addData(test0, sizeof(test0));
                cursor.addData(test1, sizeof(test1));
            }

            {
                SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, markThings);

                {
                    SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, markData);

                    const char another[] = "Another?";
                    cursor.addData(another, sizeof(another));
                }
            }
        }

        SLANG_CHECK(cursor.getCurrentChunk() == nullptr);
        SLANG_CHECK(riffBuilder.getRootChunk() != nullptr);

        {
            StringBuilder builder;
            {
                StringWriter writer(&builder);
                _dump(riffBuilder.getRootChunk(), &writer);
            }

            {
                ComPtr<ISlangBlob> blob;
                SLANG_CHECK(SLANG_SUCCEEDED(riffBuilder.writeToBlob(blob.writeRef())));

                auto rootChunk = RIFF::RootChunk::getFromBlob(blob);
                SLANG_CHECK(rootChunk != nullptr);

                // Dump the read contents
                StringBuilder readBuilder;
                {
                    StringWriter writer(&readBuilder, 0);
                    _dump(rootChunk, &writer);
                }

                // They should be the same
                SLANG_CHECK(readBuilder == builder);
            }
        }
    }

    // Test writing as a stream only allocates a single data block (as long as there is enough
    // space).
    {
        RIFF::Builder builder;
        RIFF::BuildCursor cursor(builder);

        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, markThings);
        {
            SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, markData);

            RefPtr<RandomGenerator> rand = RandomGenerator::create(0x345234);

            List<uint8_t> data;
            _writeRandom(
                rand,
                builder._getMemoryArena().getBlockPayloadSize() / 2,
                cursor,
                data);

            // Should be a single block with same data as the List
            RIFF::DataChunkBuilder* dataChunk =
                as<RIFF::DataChunkBuilder>(cursor.getCurrentChunk());
            SLANG_ASSERT(dataChunk);

            // It should be a single shard
            SLANG_CHECK(_isSingleShard(dataChunk));

            SLANG_CHECK(_isEqual(dataChunk, data.getBuffer(), data.getCount()));
        }
    }

    // Test writing across multiple data blocks
    {
        RefPtr<RandomGenerator> rand = RandomGenerator::create(0x345234);

        for (Int i = 0; i < 100; ++i)
        {
            RIFF::Builder builder;
            RIFF::BuildCursor cursor(builder);

            const size_t maxSize = rand->nextInt32InRange(
                1,
                int32_t(builder._getMemoryArena().getBlockPayloadSize() * 3));

            SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, markThings);
            {
                SLANG_SCOPED_RIFF_BUILDER_DATA_CHUNK(cursor, markData);

                List<uint8_t> data;
                _writeRandom(rand, maxSize, cursor, data);

                // Should be a single block with same data as the List
                RIFF::DataChunkBuilder* dataChunk =
                    as<RIFF::DataChunkBuilder>(cursor.getCurrentChunk());
                SLANG_CHECK(dataChunk && _isEqual(dataChunk, data.getBuffer(), data.getCount()));
            }
        }
    }

#if 0
    {
        RiffContainer container;
        {
            FileStream readStream("ambient-drop.wav", FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
            SLANG_CHECK(SLANG_SUCCEEDED(RiffUtil::read(&readStream, container)));
            RiffUtil::dump(container.getRoot(), StdWriters::getOut());
        }
        // Write it
        {

            FileStream writeStream("check.wav", FileMode::Create, FileAccess::Write, FileShare::ReadWrite);
            SLANG_CHECK(SLANG_SUCCEEDED(RiffUtil::write(container.getRoot(), true, &writeStream)));
        }
    }
#endif
}
