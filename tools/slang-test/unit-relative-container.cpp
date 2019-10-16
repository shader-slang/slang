// unit-test-path.cpp

#include "../../source/core/slang-relative-container.h"

#include "test-context.h"

using namespace Slang;

static void _checkEncodeDecode(uint32_t size)
{
    uint8_t encode[RelativeString::kMaxSizeEncodeSize];

    size_t encodeSize = RelativeString::calcEncodedSize(size, encode);

    size_t decodedSize;
    const char* chars = RelativeString::decodeSize((const char*)encode, decodedSize);

    SLANG_CHECK(decodedSize == size);
    SLANG_CHECK(chars - (const char*)encode == encodeSize);
}

namespace { // anonymous

struct Root
{
    Relative32Array<Relative32Ptr<RelativeString> > dirs;
    Relative32Ptr<RelativeString> name;
    float value;
};

} // anonymous

static void relativeContainerUnitTest()
{
    _checkEncodeDecode(253);

    for (int64_t i = 0; i < 0x100000000; i += (i / 2) + 1)
    {
        _checkEncodeDecode(uint32_t(i));
    }

    {
        RelativeContainer container;

        char* strings[] =
        {
            "Hello",
            "World",
            nullptr,
        };


        {
            Safe32Ptr<Root> root = container.allocate<Root>();

            auto array = container.allocateArray<Relative32Ptr<RelativeString>>(SLANG_COUNT_OF(strings));
            for (Int i =0 ; i < SLANG_COUNT_OF(strings); ++i)
            {
                array[i] = container.newString(strings[i]);
            }

            root->dirs = array;
        }

        {
            RelativeContainer copy;
            copy.set(container.getData(), container.getDataCount()); 

            Root* root = (Root*)copy.getData();

            SLANG_CHECK(root->dirs.getCount() == SLANG_COUNT_OF(strings));

            Int count = root->dirs.getCount();
            for (Int i = 0; i < count; ++i)
            {
                RelativeString* str = root->dirs[i];

                char* check = strings[i];

                if (check)
                {
                    SLANG_CHECK(str && strcmp(str->getCstr(), check) == 0);
                }
                else
                {
                    SLANG_CHECK(str == nullptr);
                }
            }
        }
    }
}

SLANG_UNIT_TEST("RelativeContainer", relativeContainerUnitTest);
