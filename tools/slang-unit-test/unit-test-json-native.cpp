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

struct ArrayStyleOptionalFieldsTestStruct
{
    String toolName;
    List<String> args;
    String optionalNote;

    static const StructRttiInfo g_rttiInfo;
};

static const StructRttiInfo _makeArrayStyleOptionalFieldsTestStructRtti()
{
    ArrayStyleOptionalFieldsTestStruct obj;
    StructRttiBuilder builder(&obj, "ArrayStyleOptionalFieldsTestStruct", nullptr);
    builder.addField("toolName", &obj.toolName);
    builder.addField("args", &obj.args);
    builder.addField("optionalNote", &obj.optionalNote, StructRttiInfo::Flag::Optional);
    return builder.make();
}
/* static */ const StructRttiInfo ArrayStyleOptionalFieldsTestStruct::g_rttiInfo =
    _makeArrayStyleOptionalFieldsTestStructRtti();

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

static SlangResult _checkStructFields(
    const StructRttiInfo& info,
    const char* const* expectedFields,
    Index expectedFieldCount)
{
    if (info.m_fieldCount != expectedFieldCount)
    {
        SLANG_CHECK(info.m_fieldCount == expectedFieldCount);
        return SLANG_FAIL;
    }

    for (Index i = 0; i < expectedFieldCount; ++i)
    {
        SLANG_CHECK(String(info.m_fields[i].m_name) == expectedFields[i]);
    }

    return SLANG_OK;
}

// Test that externally consumed test-server protocol structures keep their layout-compatible field
// lists. VK-GL-CTS compiles this header into pre-built binaries, so even Optional RTTI fields can
// break protocol compatibility.
static SlangResult _checkProtocolCompatibility()
{
    struct ExpectedExecuteToolTestArgsLayout
    {
        String toolName;
        List<String> args;
    };

    struct ExpectedExecutionResultLayout
    {
        String stdOut;
        String stdError;
        String debugLayer;
        int32_t result;
        int32_t returnCode;
    };

    SLANG_CHECK(
        sizeof(TestServerProtocol::ExecuteToolTestArgs) ==
        sizeof(ExpectedExecuteToolTestArgsLayout));
    SLANG_CHECK(
        alignof(TestServerProtocol::ExecuteToolTestArgs) ==
        alignof(ExpectedExecuteToolTestArgsLayout));
    SLANG_CHECK(
        sizeof(TestServerProtocol::ExecutionResult) == sizeof(ExpectedExecutionResultLayout));
    SLANG_CHECK(
        alignof(TestServerProtocol::ExecutionResult) == alignof(ExpectedExecutionResultLayout));

    static const char* const executeToolFields[] = {"toolName", "args"};
    SLANG_RETURN_ON_FAIL(_checkStructFields(
        TestServerProtocol::ExecuteToolTestArgs::g_rttiInfo,
        executeToolFields,
        SLANG_COUNT_OF(executeToolFields)));

    static const char* const executionResultFields[] = {
        "stdOut",
        "stdError",
        "debugLayer",
        "result",
        "returnCode",
    };
    SLANG_RETURN_ON_FAIL(_checkStructFields(
        TestServerProtocol::ExecutionResult::g_rttiInfo,
        executionResultFields,
        SLANG_COUNT_OF(executionResultFields)));

    return SLANG_OK;
}

SLANG_UNIT_TEST(TestServerProtocolCompatibility)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_checkProtocolCompatibility()));
}

// Test that array-style JSON-RPC works with Optional fields (backward compatibility).
// This simulates an older client sending fewer array elements than the struct has fields,
// where the missing fields are marked as Optional.
static SlangResult _checkArrayStyleOptionalFields()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    auto typeMap = JSONNativeUtil::getTypeFuncsMap();

    // Test: Array-style parsing with a missing Optional field.
    // Old clients send: ["toolName", ["arg1", "arg2"]]
    // New struct has: toolName, args, optionalNote (Optional)
    {
        const char* json = R"(["render-test", ["-api", "vk"]])";
        ArrayStyleOptionalFieldsTestStruct args;

        DiagnosticSink sink(&sourceManager, &JSONLexer::calcLexemeLocation);
        RefPtr<JSONContainer> container(new JSONContainer(&sourceManager));

        String contents(json);
        SourceFile* sourceFile =
            sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
        SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

        JSONLexer lexer;
        lexer.init(sourceView, &sink);

        JSONBuilder builder(container);
        JSONParser parser;
        SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

        JSONToNativeConverter converter(container, &typeMap, &sink);
        SLANG_RETURN_ON_FAIL(converter.convertArrayToStruct(
            builder.getRootValue(),
            GetRttiInfo<ArrayStyleOptionalFieldsTestStruct>::get(),
            &args));

        SLANG_CHECK(args.toolName == "render-test");
        SLANG_CHECK(args.args.getCount() == 2);
        SLANG_CHECK(args.args[0] == "-api");
        SLANG_CHECK(args.args[1] == "vk");
        SLANG_CHECK(args.optionalNote.getLength() == 0);
    }

    // Test: Array-style parsing with all fields provided (including Optional)
    {
        const char* json = R"(["render-test", ["-api", "vk"], "diagnostic-note"])";
        ArrayStyleOptionalFieldsTestStruct args;

        DiagnosticSink sink(&sourceManager, &JSONLexer::calcLexemeLocation);
        RefPtr<JSONContainer> container(new JSONContainer(&sourceManager));

        String contents(json);
        SourceFile* sourceFile =
            sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
        SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

        JSONLexer lexer;
        lexer.init(sourceView, &sink);

        JSONBuilder builder(container);
        JSONParser parser;
        SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

        JSONToNativeConverter converter(container, &typeMap, &sink);
        SLANG_RETURN_ON_FAIL(converter.convertArrayToStruct(
            builder.getRootValue(),
            GetRttiInfo<ArrayStyleOptionalFieldsTestStruct>::get(),
            &args));

        SLANG_CHECK(args.toolName == "render-test");
        SLANG_CHECK(args.args.getCount() == 2);
        SLANG_CHECK(args.optionalNote == "diagnostic-note");
    }

    // Test: Array-style parsing rejects a missing required field.
    {
        const char* json = R"(["render-test"])";
        ArrayStyleOptionalFieldsTestStruct args;

        DiagnosticSink sink(&sourceManager, &JSONLexer::calcLexemeLocation);
        RefPtr<JSONContainer> container(new JSONContainer(&sourceManager));

        String contents(json);
        SourceFile* sourceFile =
            sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
        SourceView* sourceView = sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

        JSONLexer lexer;
        lexer.init(sourceView, &sink);

        JSONBuilder builder(container);
        JSONParser parser;
        SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &sink));

        JSONToNativeConverter converter(container, &typeMap, &sink);
        SLANG_CHECK(SLANG_FAILED(converter.convertArrayToStruct(
            builder.getRootValue(),
            GetRttiInfo<ArrayStyleOptionalFieldsTestStruct>::get(),
            &args)));
        SLANG_CHECK(sink.getErrorCount() > 0);
    }

    return SLANG_OK;
}

SLANG_UNIT_TEST(ArrayStyleOptionalFields)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_checkArrayStyleOptionalFields()));
}
