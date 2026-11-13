// unit-test-file-write-failure.cpp

#include "core/slang-io.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// On Linux, /dev/full can be opened for writing but every write fails with ENOSPC. A small write
// is accepted into the stdio buffer, so the failure is only observable at flush/close time.
SLANG_UNIT_TEST(fileWriteFailureAtFlush)
{
#if SLANG_LINUX_FAMILY
    const char data[] = "small";

    SLANG_CHECK(SLANG_FAILED(File::writeAllBytes("/dev/full", data, sizeof(data))));
    SLANG_CHECK(SLANG_FAILED(File::writeNativeText("/dev/full", data, sizeof(data))));
#endif
}
