
#include "../../source/compiler-core/slang-json-lexer.h"
#include "../../source/core/slang-string-escape-util.h"
#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-json-value.h"

#include "../../source/compiler-core/slang-json-native.h"

#include "../../source/compiler-core/slang-source-map.h"

#include "../../source/core/slang-rtti-info.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

static SlangResult _check()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, nullptr);

    const char json[] = R"(
{
        "version" : 3,
        "file" : "out.js",
        "sourceRoot" : "",
        "sources" : ["foo.js", "bar.js"],
        "sourcesContent" : [null, null],
        "names" : ["src", "maps", "are", "fun"],
        "mappings" : "A,AAAB;;ABCDE;"
}
)";

    RefPtr<JSONContainer> container = new JSONContainer(&sourceManager);

    RttiTypeFuncsMap typeMap = JSONNativeUtil::getTypeFuncsMap();

    JSONValue readValue;
    {
        // Now need to parse as JSON
        String contents(json);
        SourceFile* sourceFile = sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
        SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

        JSONLexer lexer;
        lexer.init(sourceView, &sink);

        JSONBuilder builder(container);

        JSONParser parser;
        SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

        readValue = builder.getRootValue();
    }

    // Convert to native
    JSONSourceMap readS;
    {
        JSONToNativeConverter converter(container, &typeMap, &sink);

        // Read it back
        SLANG_RETURN_ON_FAIL(converter.convert(readValue, GetRttiInfo<JSONSourceMap>::get(), &readS));
    }

    // Write it out
    {
        String json;
        
        NativeToJSONConverter converter(container, &typeMap, &sink);

        JSONValue value;
        SLANG_RETURN_ON_FAIL(converter.convert(GetRttiInfo<JSONSourceMap>::get(), &readS, value));

        // Convert into a string
        JSONWriter writer(JSONWriter::IndentationStyle::Allman);
        container->traverseRecursively(value, &writer);

        json = writer.getBuilder();
    }

    return SLANG_OK;
}

SLANG_UNIT_TEST(sourceMap)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_check()));
}

