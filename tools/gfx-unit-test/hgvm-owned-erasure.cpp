#include "core/slang-dictionary.h"
#include "slang-hgvm/slang-hgvm-debug.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include "slang-hgvm/slang-hgvm-types.h"
#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
enum Id : HGBCId
{
    UInt = 1,
    Void,
    String,
    Node,
    Payload,
    OtherPayload,
    Box,
    Union,
    Destination,
    UnionPointer,
    FunctionType,
    Name,
    ChildField,
    ValueField,
    Number,
    PayloadTag,
    EmptyTag,
    OtherTag,
    InvalidTag,
    NullBox,
    NullUnion,
    Main,
    Entry,
    Object,
    Aggregate,
    Boxed,
    Tagged,
    ExtractedBox,
    Converted,
    Extracted,
    Printed,
    Empty,
    VoidResult,
    Local,
    Loaded,
    AliasConverted,
    FailureResult,
    Export,
};

enum class Failure
{
    None,
    WrongType,
    WrongCase,
    WrongConstructionType,
    InvalidCase,
    Narrowing,
    Uninitialized,
};

HGBC::Record& append(
    HGBC::Program& program,
    HGBCSectionTag section,
    HGBCOpcode opcode,
    HGBC::Vector<uint32_t> operands)
{
    if (program.sections.empty() || program.sections.back().tag != section)
    {
        HGBC::Section value;
        value.tag = section;
        program.sections.add(value);
    }
    program.sections.back().recordIndices.add(program.records.size());
    HGBC::Record record;
    record.opcode = opcode;
    record.operands = std::move(operands);
    program.records.add(std::move(record));
    return program.records.back();
}

/// Builds real wire operations independently of compiler lowering. A nominal struct owns an
/// allocated class, and two different union cases deliberately carry that same struct type.
RefPtr<Program> makeProgram(Failure failure)
{
    HGBC::Program code;
    auto section = HGBCSectionTag::TYPE;
    append(code, section, HGBCOpcode::UIntType, {UInt});
    append(code, section, HGBCOpcode::VoidType, {Void});
    append(code, section, HGBCOpcode::StringType, {String});
    append(code, section, HGBCOpcode::ClassType, {Node});
    append(code, section, HGBCOpcode::StructType, {Payload, ChildField, Node, ValueField, UInt});
    append(
        code,
        section,
        HGBCOpcode::StructType,
        {OtherPayload, ChildField, Node, ValueField, UInt});
    append(code, section, HGBCOpcode::BoxType, {Box});
    append(
        code,
        section,
        HGBCOpcode::TaggedUnionType,
        {Union, 7, Payload, 8, UInt, 9, Void, 10, Payload});
    append(
        code,
        section,
        HGBCOpcode::TaggedUnionType,
        {Destination, 20, Payload, 21, UInt, 22, Void});
    append(code, section, HGBCOpcode::PtrType, {UnionPointer, Union});
    append(code, section, HGBCOpcode::FuncType, {FunctionType, Void});
    section = HGBCSectionTag::LITR;
    append(code, section, HGBCOpcode::StringLit, {String, Name, 4}).payload = {'m', 'a', 'i', 'n'};
    append(code, section, HGBCOpcode::StringLit, {String, ChildField, 5})
        .payload = {'c', 'h', 'i', 'l', 'd'};
    append(code, section, HGBCOpcode::StringLit, {String, ValueField, 5})
        .payload = {'v', 'a', 'l', 'u', 'e'};
    append(code, section, HGBCOpcode::IntLit, {UInt, Number, 42});
    append(code, section, HGBCOpcode::IntLit, {UInt, PayloadTag, 7});
    append(code, section, HGBCOpcode::IntLit, {UInt, EmptyTag, 9});
    append(code, section, HGBCOpcode::IntLit, {UInt, OtherTag, 8});
    append(code, section, HGBCOpcode::IntLit, {UInt, InvalidTag, 99});
    append(code, section, HGBCOpcode::NullLit, {Box, NullBox});
    append(code, section, HGBCOpcode::NullLit, {Union, NullUnion});
    section = HGBCSectionTag::EXPT;
    append(
        code,
        section,
        HGBCOpcode::Export,
        {Export, uint32_t(HGBCExportKind::Function), Name, Main});
    section = HGBCSectionTag::FUNC;
    append(code, section, HGBCOpcode::Func, {FunctionType, Main});
    append(code, section, HGBCOpcode::Block, {Entry});
    append(code, section, HGBCOpcode::AllocateObject, {Node, Object});
    append(code, section, HGBCOpcode::MakeStruct, {Payload, Aggregate, Object, Number});
    append(code, section, HGBCOpcode::Box, {Box, Boxed, Payload, Aggregate});
    append(code, section, HGBCOpcode::MakeTaggedUnion, {Union, Tagged, PayloadTag, Boxed});
    append(code, section, HGBCOpcode::GetTaggedUnionPayload, {Box, ExtractedBox, Tagged});
    append(
        code,
        section,
        HGBCOpcode::ConvertTaggedUnion,
        {Destination, Converted, Tagged, 7, 20, 8, 21, 9, 22, 10, 20});
    append(code, section, HGBCOpcode::ExtractTaggedUnion, {Payload, Extracted, Converted, 20});
    append(code, section, HGBCOpcode::FieldExtract, {UInt, Printed, Extracted, ValueField});
    append(code, section, HGBCOpcode::Print, {UInt, Printed});
    append(code, section, HGBCOpcode::MakeTaggedUnion, {Union, Empty, EmptyTag, NullBox});
    append(code, section, HGBCOpcode::ExtractTaggedUnion, {Void, VoidResult, Empty, 9});
    append(code, section, HGBCOpcode::Var, {UnionPointer, Local});
    append(code, section, HGBCOpcode::Store, {Local, Tagged});
    append(code, section, HGBCOpcode::Load, {Union, Loaded, Local});
    append(
        code,
        section,
        HGBCOpcode::ConvertTaggedUnion,
        {Union, AliasConverted, Loaded, 7, 10, 8, 8, 9, 9, 10, 10});
    // The old destination and replacement share a single immutable box.
    append(code, section, HGBCOpcode::Store, {Local, AliasConverted});
    switch (failure)
    {
    case Failure::None:
        break;
    case Failure::WrongType:
        append(code, section, HGBCOpcode::Unbox, {OtherPayload, FailureResult, ExtractedBox});
        break;
    case Failure::WrongCase:
        append(code, section, HGBCOpcode::ExtractTaggedUnion, {Payload, FailureResult, Tagged, 10});
        break;
    case Failure::WrongConstructionType:
        append(code, section, HGBCOpcode::MakeTaggedUnion, {Union, FailureResult, OtherTag, Boxed});
        break;
    case Failure::InvalidCase:
        append(
            code,
            section,
            HGBCOpcode::MakeTaggedUnion,
            {Union, FailureResult, InvalidTag, Boxed});
        break;
    case Failure::Narrowing:
        append(
            code,
            section,
            HGBCOpcode::ConvertTaggedUnion,
            {Destination, FailureResult, Tagged, 9, 22});
        break;
    case Failure::Uninitialized:
        append(code, section, HGBCOpcode::GetTaggedUnionCase, {UInt, FailureResult, NullUnion});
        break;
    }
    append(code, section, HGBCOpcode::Return, {});
    append(code, section, HGBCOpcode::BlockEnd, {});
    append(code, section, HGBCOpcode::FunctionEnd, {});
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(code, bytes));
    RefPtr<Program> program;
    Slang::String error;
    auto result =
        Program::load(bytes.data(), bytes.size(), "owned-erasure-runtime", program, error);
    if (SLANG_FAILED(result))
        fprintf(stderr, "%s\n", error.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    return program;
}

struct Output : IHGVMOutputSink
{
    Slang::String text;
    void writeLine(std::string_view line) override
    {
        text.append(line.data(), Index(line.size()));
        text.append("\n");
    }
};

/// Stops after extraction has printed its value, while the enclosing frame still owns its boxes.
struct StopAfterOutput : IStepControl
{
    Output* output = nullptr;
    StepAction onStep(DebugInstructionAddress, bool) override
    {
        return output->text.getLength() ? StepAction::Stop : StepAction::Step;
    }
};

struct Events : IEventSink
{
    Dictionary<const void*, uint64_t> heap;
    Dictionary<const void*, uint64_t> payloads;
    uint32_t boxCount = 0;
    bool valid = true;

    void onEvent(const TrackingEvent& event) noexcept override
    {
        switch (event.kind)
        {
        case TrackingEventKind::HeapAlloc:
            valid &= !heap.containsKey(event.heap.address);
            heap[event.heap.address] = event.heap.byteSize;
            boxCount += event.heap.objectKind == HeapObjectKind::Box;
            if (event.heap.objectKind == HeapObjectKind::Box)
                valid &= event.heap.typeId == Payload;
            break;
        case TrackingEventKind::HeapFree:
            valid &= heap.containsKey(event.heap.address);
            heap.remove(event.heap.address);
            break;
        case TrackingEventKind::PayloadAlloc:
            valid &= !payloads.containsKey(event.payload.address);
            payloads[event.payload.address] = event.payload.byteSize;
            break;
        case TrackingEventKind::PayloadFree:
            valid &= payloads.containsKey(event.payload.address);
            payloads.remove(event.payload.address);
            break;
        default:
            break;
        }
    }
};
} // namespace

SLANG_UNIT_TEST(hgvmOwnedErasureRuntime)
{
    struct TestCase
    {
        Failure failure;
        const char* error;
    };
    TestCase cases[] = {
        {Failure::None, nullptr},
        {Failure::WrongType, "different concrete type"},
        {Failure::WrongCase, "different active case"},
        {Failure::WrongConstructionType, "different concrete type"},
        {Failure::InvalidCase, "invalid case"},
        {Failure::Narrowing, "excludes the active case"},
        {Failure::Uninitialized, "uninitialized tagged union"},
    };
    for (const auto& item : cases)
    {
        auto program = makeProgram(item.failure);
        RefPtr<Events> events = new Events;
        RefPtr<Output> output = new Output;
        Desc desc;
        desc.enableEventTracking = true;
        desc.eventSink = events;
        desc.outputSink = output;
        {
            Interpreter interpreter(desc);
            Slang::String error;
            auto result = interpreter.call(program, "main", error);
            if (item.error)
            {
                SLANG_CHECK(SLANG_FAILED(result));
                if (!error.contains(item.error))
                    fprintf(stderr, "%s\n", error.getBuffer());
                SLANG_CHECK(error.contains(item.error));
            }
            else
            {
                if (SLANG_FAILED(result))
                    fprintf(stderr, "%s\n", error.getBuffer());
                SLANG_CHECK(SLANG_SUCCEEDED(result));
            }
            SLANG_CHECK(output->text == "42\n");
            SLANG_CHECK(events->valid);
            SLANG_CHECK(events->heap.getCount() == 0 && events->payloads.getCount() == 0);
        }
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        SLANG_CHECK(events->boxCount == 1);
#endif
    }
}

SLANG_UNIT_TEST(hgvmOwnedErasureCancellation)
{
    auto program = makeProgram(Failure::None);
    RefPtr<Events> events = new Events;
    RefPtr<Output> output = new Output;
    StopAfterOutput control;
    control.output = output;
    Desc desc;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    desc.outputSink = output;
    desc.stepControl = &control;
    Interpreter interpreter(desc);
    Slang::String error;
    SLANG_CHECK(interpreter.call(program, "main", error) == SLANG_E_ABORT);
    SLANG_CHECK(output->text == "42\n");
    SLANG_CHECK(events->valid);
    SLANG_CHECK(events->heap.getCount() == 0 && events->payloads.getCount() == 0);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(events->boxCount == 1);
#endif
}
