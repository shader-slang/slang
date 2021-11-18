// unit-test-json-native.cpp

#include "../../source/core/slang-rtti-info.h"

#include "../../source/compiler-core/slang-json-native.h"
#include "../../source/compiler-core/slang-json-parser.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

namespace { // anonymous 

struct SomeStruct
{
    typedef SomeStruct ThisType;

    bool operator==(const ThisType& rhs) const
    {
        return a == rhs.a && b == rhs.b && s == rhs.s && list == rhs.list;
    }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    int a = 0;
    float b = 2.0f;
    String s;
    List<String> list;

    static const StructRttiInfo g_rttiInfo;
};

} // anonymous

static const StructRttiInfo _makeSomeStructRtti()
{
    SomeStruct obj;
    StructRttiBuilder builder(&obj, "SomeStruct", nullptr);

    builder.addField("a", &obj.a);
    builder.addField("b", &obj.b);
    builder.addField("s", &obj.s);
    builder.addField("list", &obj.list);

    return builder.make();
}
/* static */const StructRttiInfo SomeStruct::g_rttiInfo = _makeSomeStructRtti();

static SlangResult _check()
{
    // Convert into a JSON string

    SomeStruct s;
    s.list.add("Hello!");
    s.s = "There";

    // Try serializing it out

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    DiagnosticSink sink(&sourceManager, &JSONLexer::calcLexemeLocation);

    RefPtr<JSONContainer> container(new JSONContainer(&sourceManager));

    String json;
    {
        NativeToJSONConverter converter(container, &sink);

        JSONValue value;
        SLANG_RETURN_ON_FAIL(converter.convert(GetRttiInfo<SomeStruct>::get(), &s, value));

        // Convert into a string
        JSONWriter writer(JSONWriter::IndentationStyle::Allman);
        container->traverseRecursively(value, &writer);

        json = writer.getBuilder();
    }

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

    // Convert back to native
    {
        JSONToNativeConverter converter(container, &sink);

        {
            SomeStruct readS;
            SLANG_RETURN_ON_FAIL(converter.convert(readValue, GetRttiInfo<SomeStruct>::get(), &readS));

            // Should be equal
            SLANG_CHECK(readS == s);
        }
    }

    return SLANG_OK;
}

SLANG_UNIT_TEST(JSONNative)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_check()));

}
