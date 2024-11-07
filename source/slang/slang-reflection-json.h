#ifndef SLANG_REFLECTION_JSON_H
#define SLANG_REFLECTION_JSON_H

#include "slang.h"
#include "../compiler-core/slang-pretty-writer.h"

namespace Slang
{

void emitReflectionJSON(SlangCompileRequest* request, SlangReflection* reflection, PrettyWriter& writer);

}

#endif
