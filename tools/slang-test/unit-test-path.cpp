// unit-test-path.cpp

#include "../../source/core/slang-io.h"


#include "test-context.h"

using namespace Slang;

static void pathUnitTest()
{
    {
        String path;
        SlangResult res = Path::GetCanonical(".", path);
        SLANG_CHECK(SLANG_SUCCEEDED(res));

        String parentPath;
        res = Path::GetCanonical("..", parentPath);
        SLANG_CHECK(SLANG_SUCCEEDED(res));

        String parentPath2 = Path::GetDirectoryName(path);
        SLANG_CHECK(parentPath == parentPath2);
    }
}

SLANG_UNIT_TEST("Path", pathUnitTest);