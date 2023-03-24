
#include "../../source/compiler-core/slang-json-lexer.h"
#include "../../source/core/slang-string-escape-util.h"
#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-json-value.h"

#include "../../source/compiler-core/slang-json-native.h"

#include "../../source/compiler-core/slang-source-map.h"
#include "../../source/compiler-core/slang-json-source-map-util.h"

#include "../../source/core/slang-rtti-info.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

static SlangResult _check()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, nullptr);

    const char jsonSource[] = R"(
{
        "version" : 3,
        "file" : "out.js",
        "sourceRoot" : "",
        "sources" : ["foo.js", "bar.js"],
        "sourcesContent" : [null, null],
        "names" : ["src", "maps", "are", "fun"],
        "mappings" : "A,AAAB;;ABCEG;" 
}
)";

    RefPtr<JSONContainer> container = new JSONContainer(&sourceManager);

    JSONValue rootValue;
    {
        // Now need to parse as JSON
        String contents(jsonSource);
        SourceFile* sourceFile = sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
        SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

        JSONLexer lexer;
        lexer.init(sourceView, &sink);

        JSONBuilder builder(container);

        JSONParser parser;
        SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

        rootValue = builder.getRootValue();
    }

    RefPtr<SourceMap> sourceMap;
        
    SLANG_RETURN_ON_FAIL(JSONSourceMapUtil::decode(container, rootValue, &sink, sourceMap));
    
    // Write it out
    String json;
    {
        JSONValue jsonValue;

        SLANG_RETURN_ON_FAIL(JSONSourceMapUtil::encode(sourceMap, container, &sink, jsonValue));
        
        // Convert into a string
        JSONWriter writer(JSONWriter::IndentationStyle::Allman);
        container->traverseRecursively(jsonValue, &writer);

        json = writer.getBuilder();
    }

    return SLANG_OK;
}

SLANG_UNIT_TEST(sourceMap)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_check()));
}

