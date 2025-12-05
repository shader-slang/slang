// unit-test-json-native.cpp

#include "../../source/compiler-core/slang-json-native.h"
#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-test-server-protocol.h"
#include "../../source/core/slang-rtti-info.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{ // anonymous

struct OtherStruct
{
    typedef OtherStruct ThisType;

    bool operator==(const ThisType& rhs) const { return f == rhs.f && value == rhs.value; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    float f = 1.0f;
    String value;

    static const StructRttiInfo g_rttiInfo;
};

static const StructRttiInfo _makeOtherStructRtti()
{
    OtherStruct obj;
    StructRttiBuilder builder(&obj, "OtherStruct", nullptr);
    builder.addField("f", &obj.f);
    builder.addField("value", &obj.value);
    return builder.make();
}
/* static */ const StructRttiInfo OtherStruct::g_rttiInfo = _makeOtherStructRtti();

struct SomeStruct
{
    typedef SomeStruct ThisType;

    bool operator==(const ThisType& rhs) const
    {
        return a == rhs.a && b == rhs.b && s == rhs.s && list == rhs.list &&
               boolValue == rhs.boolValue && structList == rhs.structList &&
               makeConstArrayView(fixedArray) == makeConstArrayView(rhs.fixedArray);
    }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    int a = 0;
    float b = 2.0f;
    String s;
    List<String> list;
    bool boolValue = false;

    List<OtherStruct> structList;

    int fixedArray[2] = {0, 0};

    static const StructRttiInfo g_rttiInfo;
};

static const StructRttiInfo _makeSomeStructRtti()
{
    SomeStruct obj;
    StructRttiBuilder builder(&obj, "SomeStruct", nullptr);

    builder.addField("a", &obj.a);
    builder.addField("b", &obj.b);
    builder.addField("s", &obj.s);
    builder.addField("list", &obj.list);
    builder.addField("boolValue", &obj.boolValue);
    builder.addField("structList", &obj.structList);
    builder.addField("fixedArray", &obj.fixedArray);

    return builder.make();
}
/* static */ const StructRttiInfo SomeStruct::g_rttiInfo = _makeSomeStructRtti();

} // namespace


static SlangResult _check()
{
    // Convert into a JSON string

    SomeStruct s;
    s.list.add("Hello!");
    s.s = "There";
    s.boolValue = true;

    OtherStruct o;
    o.f = 27.5f;
    o.value = "This works!";

    s.structList.add(o);
    s.fixedArray[1] = 8;
    s.fixedArray[0] = -1;

    // Try serializing it out

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    DiagnosticSink sink(&sourceManager, &JSONLexer::calcLexemeLocation);

    auto typeMap = JSONNativeUtil::getTypeFuncsMap();

    RefPtr<JSONContainer> container(new JSONContainer(&sourceManager));

    String json;
    {
        NativeToJSONConverter converter(container, &typeMap, &sink);

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
        SourceFile* sourceFile =
            sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
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
        JSONToNativeConverter converter(container, &typeMap, &sink);

        {
            SomeStruct readS;
            SLANG_RETURN_ON_FAIL(
                converter.convert(readValue, GetRttiInfo<SomeStruct>::get(), &readS));

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

// Helper to parse JSON string and convert to native type
template<typename T>
static SlangResult _parseJsonToNative(
    const char* jsonStr,
    T* outValue,
    SourceManager* sourceManager,
    RttiTypeFuncsMap* typeMap)
{
    DiagnosticSink sink(sourceManager, &JSONLexer::calcLexemeLocation);
    RefPtr<JSONContainer> container(new JSONContainer(sourceManager));

    String contents(jsonStr);
    SourceFile* sourceFile =
        sourceManager->createSourceFileWithString(PathInfo::makeUnknown(), contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());

    JSONLexer lexer;
    lexer.init(sourceView, &sink);

    JSONBuilder builder(container);
    JSONParser parser;
    SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

    JSONToNativeConverter converter(container, typeMap, &sink);
    SLANG_RETURN_ON_FAIL(
        converter.convert(builder.getRootValue(), GetRttiInfo<T>::get(), outValue));

    return SLANG_OK;
}

// Helper to serialize native type to JSON string
template<typename T>
static SlangResult _serializeNativeToJson(
    const T* value,
    String& outJson,
    SourceManager* sourceManager,
    RttiTypeFuncsMap* typeMap)
{
    DiagnosticSink sink(sourceManager, nullptr);
    RefPtr<JSONContainer> container(new JSONContainer(sourceManager));

    NativeToJSONConverter converter(container, typeMap, &sink);
    JSONValue jsonValue;
    SLANG_RETURN_ON_FAIL(converter.convert(GetRttiInfo<T>::get(), value, jsonValue));

    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    container->traverseRecursively(jsonValue, &writer);
    outJson = writer.getBuilder().produceString();

    return SLANG_OK;
}

// Test that test-server protocol structures can be serialized/deserialized correctly.
static SlangResult _checkProtocolRoundTrip()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    auto typeMap = JSONNativeUtil::getTypeFuncsMap();

    // Test 1: ExecuteToolTestArgs round-trip
    {
        const char* json = R"({"toolName": "render-test", "args": ["-api", "vk"]})";
        TestServerProtocol::ExecuteToolTestArgs args;
        SLANG_RETURN_ON_FAIL(_parseJsonToNative(json, &args, &sourceManager, &typeMap));
        SLANG_CHECK(args.toolName == "render-test");
        SLANG_CHECK(args.args.getCount() == 2);
    }

    // Test 2: ExecutionResult round-trip
    {
        const char* json =
            R"({"stdOut": "ok", "stdError": "", "debugLayer": "", "result": 0, "returnCode": 0})";
        TestServerProtocol::ExecutionResult result;
        SLANG_RETURN_ON_FAIL(_parseJsonToNative(json, &result, &sourceManager, &typeMap));
        SLANG_CHECK(result.stdOut == "ok");
        SLANG_CHECK(result.result == 0);
        SLANG_CHECK(result.returnCode == 0);
    }

    // Test 3: ExecutionResult serialization
    {
        TestServerProtocol::ExecutionResult result;
        result.stdOut = "output";
        result.stdError = "";
        result.debugLayer = "";
        result.result = 0;
        result.returnCode = 0;

        String jsonOutput;
        SLANG_RETURN_ON_FAIL(_serializeNativeToJson(&result, jsonOutput, &sourceManager, &typeMap));

        // Verify expected fields are present
        SLANG_CHECK(jsonOutput.indexOf("stdOut") >= 0);
        SLANG_CHECK(jsonOutput.indexOf("result") >= 0);
    }

    return SLANG_OK;
}

SLANG_UNIT_TEST(TestServerProtocolRoundTrip)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_checkProtocolRoundTrip()));
}
