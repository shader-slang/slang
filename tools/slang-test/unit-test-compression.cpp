// unit-compression.cpp

#include "test-context.h"

#include "../../source/core/slang-zip-file-system.h"

using namespace Slang;

static void compressionUnitTest()
{
    ZipCompressionUtil::unitTest();
}

SLANG_UNIT_TEST("Compression", compressionUnitTest);
