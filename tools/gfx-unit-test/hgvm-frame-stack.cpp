#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-frame-stack.h"
#include "slang-hgvm/slang-hgvm-interpreter.h"

#include <string>

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
/// Compiles `source` and retargets every call to `placeholder` at `recursive`.
///
/// Slang rejects source-level recursion for the HGBC target, but recursive HGBC is valid input
/// for the VM. Both functions share a signature, so redirecting the call operand turns a bounded
/// call chain into real recursion without changing anything else in the program.
RefPtr<Program> compileRecursiveProgram(
    slang::IGlobalSession* globalSession,
    const char* source,
    const char* moduleName)
{
    auto compiled =
        compileHGVMTestProgram(globalSession, source, SLANG_DEBUG_INFO_LEVEL_NONE, moduleName);
    HGBCId placeholder = 0, recursive = 0;
    for (const auto& function : compiled->getDecodedProgram().functions)
    {
        auto name = compiled->findStringDecoration(function.id, HGBCDecorationKind::NameHint);
        if (name && *name == "placeholder")
            placeholder = function.id;
        if (name && *name == "recursive")
            recursive = function.id;
    }
    SLANG_CHECK_ABORT(placeholder && recursive);
    auto decoded = compiled->getDecodedProgram();
    for (auto& record : decoded.records)
        if (record.opcode == HGBCOpcode::Call && record.operands[2] == placeholder)
            record.operands[2] = recursive;
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(decoded, bytes));
    RefPtr<Program> program;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(Program::load(bytes.data(), bytes.size(), moduleName, program, error)));
    return program;
}

/// Recurses `depth` levels, adding each level to the root frame's `total` through `inout`.
/// Every level holds the native address of the same root-frame slot, so the sum is correct only
/// if frames never move while deeper frames are pushed.
const char* kAccumulateSource = R"(
    [noinline] void placeholder(inout uint total, uint depth) {}
    [noinline] void recursive(inout uint total, uint depth)
    {
        if (depth == 0)
            return;
        total += depth;
        placeholder(total, depth - 1);
    }
    [host] uint main()
    {
        uint total = 0;
        recursive(total, 3000);
        return total;
    }
)";

/// Recurses without bound, so the frame stack overflows.
const char* kUnboundedSource = R"(
    [noinline] uint placeholder(uint depth) { return depth; }
    [noinline] uint recursive(uint depth) { return placeholder(depth + 1) + 1; }
    [host] uint main() { return recursive(0); }
)";

uint32_t invokeMain(InterpreterCore& interpreter, Program* program, String& error)
{
    RefPtr<BoundFunction> function;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    Value result;
    if (SLANG_FAILED(interpreter.invoke(function, {}, result, error)))
        return 0;
    uint32_t value = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(result.getScalarData(ScalarType::UInt32, &value, sizeof(value), error)));
    return value;
}

InterpreterCore::Desc makeDesc(FrameStackKind kind, size_t byteLimit)
{
    InterpreterCore::Desc desc;
    desc.frameStackKind = kind;
    desc.frameStackByteLimit = byteLimit;
    // Small segments make a 3000-level recursion cross hundreds of segment boundaries.
    desc.frameStackSegmentSize = 4096;
    return desc;
}

bool contains(const String& text, const char* expected)
{
    return std::string(text.getBuffer()).find(expected) != std::string::npos;
}
} // namespace

SLANG_UNIT_TEST(hgvmFrameStackKeepsFrameAddresses)
{
    for (FrameStackKind kind : {FrameStackKind::Reserved, FrameStackKind::Segmented})
    {
        auto stack = createFrameStack(kind, 64 * 1024, 256);
        uint8_t* initialTop = stack->getTop();
        List<uint8_t*> frames;
        for (uint8_t i = 0; i < 100; ++i)
        {
            uint8_t* frame = stack->push(48, 16);
            SLANG_CHECK_ABORT(frame);
            SLANG_CHECK(uintptr_t(frame) % 16 == 0);
            std::memset(frame, i, 48);
            frames.add(frame);
        }
        // Every earlier frame still holds its bytes at its original address.
        for (Index i = 0; i < frames.getCount(); ++i)
        {
            SLANG_CHECK(frames[i][0] == uint8_t(i) && frames[i][47] == uint8_t(i));
            SLANG_CHECK(stack->containsLiveRange(frames[i], 48));
        }
        uint8_t* middleTop = frames[50] + 48;
        stack->popTo(middleTop);
        SLANG_CHECK(!stack->containsLiveRange(frames[99], 48));
        SLANG_CHECK(stack->containsLiveRange(frames[50], 48));
        stack->popTo(initialTop);
        SLANG_CHECK(!stack->containsLiveRange(frames[0], 48));

        // A push past the limit reports overflow instead of growing without bound.
        uint8_t* top = stack->getTop();
        bool overflowed = false;
        for (int i = 0; i < 10000 && !overflowed; ++i)
            overflowed = stack->push(1024, 16) == nullptr;
        SLANG_CHECK(overflowed);
        stack->popTo(top);
        SLANG_CHECK(stack->push(1024, 16) != nullptr);
    }
}

SLANG_UNIT_TEST(hgvmFrameStackDeepInOutRecursion)
{
    auto program = compileRecursiveProgram(
        unitTestContext->slangGlobalSession,
        kAccumulateSource,
        "hgvm_frame_stack_accumulate");
    for (FrameStackKind kind : {FrameStackKind::Reserved, FrameStackKind::Segmented})
    {
        InterpreterCore interpreter(makeDesc(kind, size_t(64) * 1024 * 1024));
        String error;
        SLANG_CHECK(invokeMain(interpreter, program, error) == 3000u * 3001u / 2u);
        SLANG_CHECK(error.getLength() == 0);
    }
}

SLANG_UNIT_TEST(hgvmFrameStackOverflowReportsCallChain)
{
    auto unbounded = compileRecursiveProgram(
        unitTestContext->slangGlobalSession,
        kUnboundedSource,
        "hgvm_frame_stack_unbounded");
    auto bounded = compileRecursiveProgram(
        unitTestContext->slangGlobalSession,
        kAccumulateSource,
        "hgvm_frame_stack_bounded");
    for (FrameStackKind kind : {FrameStackKind::Reserved, FrameStackKind::Segmented})
    {
        InterpreterCore interpreter(makeDesc(kind, 1024 * 1024));
        String error;
        RefPtr<BoundFunction> function;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.bindFunction(
            unbounded,
            unbounded->findFunctionExport("main")->id,
            function,
            error)));
        Value result;
        SLANG_CHECK(SLANG_FAILED(interpreter.invoke(function, {}, result, error)));
        SLANG_CHECK(contains(
            error,
            "HGVM stack overflow: the call to 'recursive' exceeds the frame stack limit of "
            "1048576 bytes"));
        SLANG_CHECK(contains(error, "\n  recursive ("));
        SLANG_CHECK(contains(error, " frames)\n  main"));

        // The failed invocation popped all of its frames, so the next one has the full stack.
        error = "";
        SLANG_CHECK(invokeMain(interpreter, bounded, error) == 3000u * 3001u / 2u);
        SLANG_CHECK(error.getLength() == 0);
    }
}
