// unit-test-riff.cpp

#include "../../source/core/slang-riff.h"

#include "../../source/core/slang-random-generator.h"

#include "test-context.h"

using namespace Slang;

static void riffUnitTest()
{
    typedef RiffContainer::ScopeChunk ScopeChunk;
    typedef RiffContainer::Chunk::Kind Kind;

    const FourCC markThings = SLANG_FOUR_CC('T', 'H', 'I', 'N');
    const FourCC markData = SLANG_FOUR_CC('D', 'A', 'T', 'A');

    {
        RiffContainer container;

        {
            ScopeChunk scopeContainer(&container, Kind::List, markThings);        
            {
                ScopeChunk scopeChunk(&container, Kind::Data, markData);

                const char hello[] = "Hello ";
                const char world[] = "World!";

                container.write(hello, sizeof(hello));
                container.write(world, sizeof(world));
            }

            {
                ScopeChunk scopeChunk(&container, Kind::Data, markData);

                const char test0[] = "Testing... ";
                const char test1[] = "Testing!";

                container.write(test0, sizeof(test0));
                container.write(test1, sizeof(test1));
            }

            {
                ScopeChunk innerScopeContainer(&container, Kind::List, markThings);

                {
                    ScopeChunk scopeChunk(&container, Kind::Data, markData);

                    const char another[] = "Another?";
                    container.write(another, sizeof(another));
                }
            }
        }

        SLANG_CHECK(container.isFullyConstructed());
        SLANG_CHECK(RiffContainer::isChunkOk(container.getRoot()));

        {
            StringBuilder builder;
            {
                StringWriter writer(&builder, 0);
                RiffUtil::dump(container.getRoot(), &writer);
            }

            {
                OwnedMemoryStream stream(FileAccess::ReadWrite); 
                SLANG_CHECK(SLANG_SUCCEEDED(RiffUtil::write(container.getRoot(), true, &stream)));

                stream.seek(SeekOrigin::Start, 0);

                RiffContainer readContainer;
                SLANG_CHECK(SLANG_SUCCEEDED(RiffUtil::read(&stream, readContainer)));

                // Dump the read contents
                StringBuilder readBuilder;
                {
                    StringWriter writer(&readBuilder, 0);
                    RiffUtil::dump(readContainer.getRoot(), &writer);
                }

                // They should be the same
                SLANG_CHECK(readBuilder == builder);
            }
        }

    }

    {
        RiffContainer container;

        ScopeChunk scopeChunk(&container, Kind::List, markData);
        {
            ScopeChunk scopeChunk(&container, Kind::Data, markData);

            size_t maxUse =  container.getMemoryArena().getBlockPayloadSize() / 2;

            RefPtr<RandomGenerator> rand = RandomGenerator::create(0x345234);

            List<uint8_t> data;

            while (true)
            {
                const Index oldCount = data.getCount();

                const size_t allocSize = size_t(rand->nextInt32InRange(1, 50));

                if (allocSize + oldCount > maxUse)
                {
                    break;
                }

                data.setCount(oldCount + Index(allocSize));
                rand->nextData(data.getBuffer() + oldCount, allocSize);

                // Write
                container.write(data.getBuffer() + oldCount, allocSize);
            }

            // Should be a single block with same data as the List
            RiffContainer::DataChunk* dataChunk = as<RiffContainer::DataChunk>(container.getCurrentChunk());
            SLANG_ASSERT(dataChunk);

            RiffContainer::Data* chunkData = dataChunk->getSingleData();

            SLANG_CHECK(chunkData != nullptr);
            if (chunkData)
            {
                SLANG_CHECK(chunkData->getSize() == data.getCount());
                SLANG_CHECK(::memcmp(chunkData->getPayload(), data.getBuffer(), chunkData->getSize()) == 0);
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

SLANG_UNIT_TEST("Riff", riffUnitTest);
