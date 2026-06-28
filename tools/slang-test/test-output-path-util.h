#ifndef TEST_OUTPUT_PATH_UTIL_H
#define TEST_OUTPUT_PATH_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{

void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args);

}

#endif
