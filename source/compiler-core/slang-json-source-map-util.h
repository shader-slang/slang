#ifndef SLANG_COMPILER_CORE_JSON_SOURCE_MAP_UTIL_H
#define SLANG_COMPILER_CORE_JSON_SOURCE_MAP_UTIL_H

#include "slang-source-map.h"

#include "slang-json-value.h"

namespace Slang {

struct  JSONSourceMapUtil
{
        /// Decode from root into the source map
    static SlangResult decode(JSONContainer* container, JSONValue root, DiagnosticSink* sink, RefPtr<SourceMap>& out);

        /// Converts the source map contents into JSON
    static SlangResult encode(SourceMap* sourceMap, JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);

        /// Read the blob (encoded as JSON) as a source map. 
        /// Sink is optional, and can be passed as nullptr
    static SlangResult read(ISlangBlob* blob, DiagnosticSink* sink, RefPtr<SourceMap>& outSourceMap);
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_JSON_SOURCE_MAP_UTIL_H
