#ifndef SLANG_COMPILER_CORE_JSON_RTTI_H
#define SLANG_COMPILER_CORE_JSON_RTTI_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-json-value.h"

namespace Slang {

struct JSONRttiUtil
{
    static SlangResult nativeToJSON(JSONContainer* container, const RttiInfo* rttiInfo, const void* in, JSONValue& out);
    static SlangResult jsonToNative(JSONContainer* container, const JSONValue& in, const RttiInfo* rttiInfo, void* out);
};


} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_RTTI_H
