// unit-test-riff.cpp

#include "../../source/core/slang-riff.h"

#include "test-context.h"

using namespace Slang;

static void riffUnitTest()
{
    const FourCC markThings = SLANG_FOUR_CC('T', 'H', 'I', 'N');
    const FourCC markData = SLANG_FOUR_CC('D', 'A', 'T', 'A');

    {
        typedef RiffContainer::ScopeChunk ScopeChunk;
        typedef RiffContainer::Chunk::Kind Kind;

        RiffContainer container;

        {
            ScopeChunk scopeContainer(&container, Kind::Container, markThings);

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
                ScopeChunk innerScopeContainer(&container, Kind::Container, markThings);

                {
                    ScopeChunk scopeChunk(&container, Kind::Data, markData);

                    const char another[] = "Another?";
                    container.write(another, sizeof(another));
                }
            }
        }

        SLANG_CHECK(container.isFullyConstructed());
        SLANG_CHECK(RiffContainer::isContainerOk(container.getRoot()));

    }
}

SLANG_UNIT_TEST("Riff", riffUnitTest);
