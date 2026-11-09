#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-debug-session.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include "slang-hgvm/slang-hgvm-image-io.h"
#include "slang-hgvm/slang-hgvm-staged-records.h"

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
struct DebugTestOutput : IHGVMOutputSink
{
    String text;
    void writeLine(std::string_view line) override
    {
        text.append(line.data(), Index(line.size()));
        text.append("\n");
    }
};

struct RecordingSteps : IStepControl, IDebugHandler
{
    List<DebugInstructionAddress> stops;
    List<DebugInstructionAddress> breakpointHits;
    StepAction action = StepAction::Step;
    Index depth = 0;
    Index maximumDepth = 0;
    Index scopes = 0;
    uint32_t maximumInlineSiteCount = 0;
    StepAction onStep(DebugInstructionAddress address, bool breakpointHit) override
    {
        stops.add(address);
        if (breakpointHit)
            breakpointHits.add(address);
        return action;
    }
    void onFunctionEnter(DebugInstructionAddress, DebugCallId) override
    {
        ++depth;
        maximumDepth = Math::Max(maximumDepth, depth);
    }
    void onFunctionExit() override { SLANG_CHECK(depth-- > 0); }
    void onScope(uint32_t, uint32_t inlineSiteCount) override
    {
        SLANG_CHECK(depth > 0);
        ++scopes;
        maximumInlineSiteCount = Math::Max(maximumInlineSiteCount, inlineSiteCount);
    }
};

struct RecordingStops : IDebugSessionListener
{
    List<List<DebugStackFrame>> traces;
    DebugCommand command = DebugCommand::Continue;
    DebugCommand onStopped(DebugSession& session) override
    {
        traces.add(session.getStackTrace());
        return command;
    }
};

struct AllocationCounter : IEventSink
{
    Index allocations = 0;
    Index frees = 0;
    void onEvent(const TrackingEvent& event) noexcept override
    {
        allocations += event.kind == TrackingEventKind::HeapAlloc;
        frees += event.kind == TrackingEventKind::HeapFree;
    }
};
} // namespace

SLANG_UNIT_TEST(hgvmDebuggerRecordControl)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[host] void main() { print(1); print(2); }",
        SLANG_DEBUG_INFO_LEVEL_STANDARD);
    RecordingSteps control;
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.stepControl = &control;
    desc.outputSink = output;
    Interpreter interpreter(desc);
    String error;
    uint64_t id;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.prepareProgram(program, id, error)));
    auto map = interpreter.getDebugMap(id);
    SLANG_CHECK_ABORT(map && map->getFunctions().getCount() == 1);
    const auto& function = map->getFunctions()[0];
    List<uint32_t> instructions;
    for (uint32_t pc = function.entry; pc < function.end;)
    {
        const auto* record = interpreter.getRecord({id, pc});
        SLANG_CHECK(!(record->opcodeAndFlags & StagedRecord::kDebugRecordBit));
        instructions.add(pc);
        pc += record->recordWordCount;
    }
    SLANG_CHECK_ABORT(instructions.getCount() > 1);
    uint32_t target = instructions[1];
    auto original = *interpreter.getRecord({id, target});
    interpreter.setBreakpoint({id, target});
    auto armed = *interpreter.getRecord({id, target});
    SLANG_CHECK(armed.opcodeAndFlags == (original.opcodeAndFlags | StagedRecord::kBreakpointBit));
    SLANG_CHECK(armed.recordWordCount == original.recordWordCount);
    control.action = StepAction::Continue;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
    SLANG_CHECK_ABORT(control.stops.getCount() == 2);
    SLANG_CHECK(control.stops[0].wordOffset == function.entry);
    SLANG_CHECK(control.stops[1].wordOffset == target);
    SLANG_CHECK_ABORT(control.breakpointHits.getCount() == 1);
    SLANG_CHECK(control.breakpointHits[0].wordOffset == target);
    SLANG_CHECK(output->text == "1\n2\n");

    // A breakpoint still reports a hit when the callback was already due for a single step.
    // Entry can also carry a breakpoint; the flag must not depend on which condition fired first.
    interpreter.setBreakpoint({id, function.entry});
    control.stops.clear();
    control.breakpointHits.clear();
    control.action = StepAction::Step;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
    SLANG_CHECK_ABORT(control.breakpointHits.getCount() == 2);
    SLANG_CHECK(control.breakpointHits[0].wordOffset == function.entry);
    SLANG_CHECK(control.breakpointHits[1].wordOffset == target);
    interpreter.setBreakpoint({id, function.entry}, false);

    interpreter.setBreakpoint({id, target}, false);
    SLANG_CHECK(interpreter.getRecord({id, target})->opcodeAndFlags == original.opcodeAndFlags);
    control.stops.clear();
    control.breakpointHits.clear();
    control.action = StepAction::Step;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
    SLANG_CHECK(control.stops.getCount() >= instructions.getCount());
    SLANG_CHECK(control.breakpointHits.getCount() == 0);
    for (Index i = 0; i < instructions.getCount(); ++i)
        SLANG_CHECK(control.stops[i].wordOffset == instructions[i]);

    control.stops.clear();
    control.action = StepAction::Stop;
    output->text = {};
    SLANG_CHECK(interpreter.call(program, "main", error) == SLANG_E_ABORT);
    SLANG_CHECK(control.stops.getCount() == 1 && output->text.getLength() == 0);
}

SLANG_UNIT_TEST(hgvmDebuggerManagedSourceBreakpoint)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "class Node { int value; __init(int v) { value = v; } }\n"
        "[noinline] Node identity(Node node)\n"
        "{\n"
        "    return node;\n"
        "}\n"
        "[host] void main()\n"
        "{\n"
        "    var node = new Node(7);\n"
        "    print(identity(node).value);\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_managed_breakpoint");
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.outputSink = output;
    DebugSession session(desc);
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    uint32_t breakpoint = session.addBreakpoint("hgvm_managed_breakpoint.slang", 4);
    const auto& locations = session.getBreakpoints()[0].addresses;
    SLANG_CHECK_ABORT(locations.getCount() > 0);
    bool hasOwnershipBoundary = false;
    for (uint32_t location : locations)
    {
        const auto* record =
            session.getInterpreter()->getRecord({session.getDebugMap()->getProgramId(), location});
        SLANG_CHECK(!(record->opcodeAndFlags & StagedRecord::kInstructionContinuationBit));
        hasOwnershipBoundary |= (record->opcodeAndFlags & StagedRecord::kInternalOpcodeBit) != 0;
    }
    // Returning a class starts with ownership bookkeeping; its byte store is a continuation.
    SLANG_CHECK(hasOwnershipBoundary);
    struct Listener : IDebugSessionListener
    {
        uint32_t expected = 0;
        int stops = 0;
        DebugCommand onStopped(DebugSession& session) override
        {
            SLANG_CHECK(session.getHitBreakpoint() == expected);
            ++stops;
            return DebugCommand::Continue;
        }
    } listener;
    listener.expected = breakpoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(listener.stops == 1);
    SLANG_CHECK(output->text == "7\n");
}

SLANG_UNIT_TEST(hgvmDebuggerInlineControlFlow)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[noinline] int marker(int x) { return x; }\n"
        "[ForceInline] int choose(int x)\n"
        "{\n"
        "    if (x > 0) return marker(x);\n"
        "    return marker(-x);\n"
        "}\n"
        "[host] void main()\n"
        "{\n"
        "    for (int i = -1; i < 2; ++i)\n"
        "    {\n"
        "        print(choose(i));\n"
        "        print(marker(10));\n"
        "    }\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_debugger");
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.outputSink = output;
    DebugSession session(desc);
    uint32_t breakpoint = session.addFunctionBreakpoint("marker");
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    RecordingStops listener;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK_ABORT(listener.traces.getCount() == 6);
    for (Index i = 0; i < 6; ++i)
    {
        const auto& trace = listener.traces[i];
        SLANG_CHECK_ABORT(trace.getCount() == (i % 2 ? 2 : 3));
        SLANG_CHECK(trace[0].functionName == "marker");
        SLANG_CHECK(!trace[0].isInlined);
        SLANG_CHECK(trace.getLast().functionName == "main");
        SLANG_CHECK(!trace.getLast().isInlined);
        SLANG_CHECK(trace.getLast().line == (i % 2 ? 12 : 11));
        if (!(i % 2))
        {
            SLANG_CHECK(trace[1].functionName == "choose");
            SLANG_CHECK(trace[1].isInlined);
            SLANG_CHECK(trace[1].line == (i < 4 ? 5 : 4));
        }
    }
    SLANG_CHECK(output->text == "1\n10\n0\n10\n1\n10\n");
    session.enableBreakpoint(breakpoint, false);
    listener.traces.clear();
    // The call itself is inlined away. Its call-site location must still resolve to instructions.
    uint32_t callSite = session.addBreakpoint("hgvm_debugger.slang", 11);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(listener.traces.getCount() >= 3);
    for (const auto& trace : listener.traces)
        SLANG_CHECK(trace.getLast().line == 11);
    session.removeBreakpoint(callSite);
    listener.traces.clear();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(listener.traces.getCount() == 0);
    session.enableBreakpoint(breakpoint, true);
    listener.traces.clear();
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(listener.traces.getCount() == 6);
}

SLANG_UNIT_TEST(hgvmDebuggerCancellationAndFailure)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "class Node { int value; __init(int v) { value = v; } }\n"
        "[noinline] void leaf(Node node) { print(node.value); }\n"
        "[noinline] void outer(Node node) { var local = new Node(node.value + 1); leaf(local); }\n"
        "[host] void main() { var root = new Node(1); outer(root); }\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD);
    RefPtr<AllocationCounter> counter = new AllocationCounter;
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.eventSink = counter;
    desc.enableEventTracking = true;
    desc.outputSink = output;
    DebugSession session(desc);
    session.addFunctionBreakpoint("leaf");
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    RecordingStops listener;
    listener.command = DebugCommand::Stop;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(session.wasTerminated() && output->text.getLength() == 0);
    SLANG_CHECK_ABORT(listener.traces.getCount() == 1);
    SLANG_CHECK(listener.traces[0].getCount() == 3);
    SLANG_CHECK(counter->allocations == counter->frees);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(counter->allocations == 2);
#endif
    listener.command = DebugCommand::Continue;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(!session.wasTerminated() && output->text.getLength());
    SLANG_CHECK(counter->allocations == counter->frees);

    program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[noinline] void leaf() { String message = \"debug failure\"; hgbc_asm { OpFail $message; "
        "}; }\n"
        "[host] void main() { leaf(); }\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD);
    RecordingSteps control;
    control.action = StepAction::Continue;
    desc.stepControl = &control;
    desc.debugHandler = &control;
    Interpreter interpreter(desc);
    SLANG_CHECK(SLANG_FAILED(interpreter.call(program, "main", error)));
    SLANG_CHECK(error.contains("debug failure"));
    SLANG_CHECK(control.depth == 0 && control.maximumDepth == 2 && control.scopes > 0);
    SLANG_CHECK(control.stops.getCount() == 1);
}

SLANG_UNIT_TEST(hgvmDebuggerRecursiveCalls)
{
    // Slang currently rejects source recursion for HGBC, but recursive HGBC is valid. Retarget
    // this same-signature call in the serialized program to exercise the interpreter contract.
    auto source = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[noinline] int leaf(int x) { return x; }\n"
        "[noinline] int placeholder(int x) { return x; }\n"
        "[noinline] int recurse(int x) { if (x == 0) return leaf(x); return placeholder(x - 1); }\n"
        "[host] void main() { print(recurse(3)); }\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD);
    HGBCId placeholder = 0, recurse = 0;
    for (const auto& function : source->getDecodedProgram().functions)
    {
        auto name = source->findStringDecoration(function.id, HGBCDecorationKind::NameHint);
        if (name && *name == "placeholder")
            placeholder = function.id;
        if (name && *name == "recurse")
            recurse = function.id;
    }
    SLANG_CHECK_ABORT(placeholder && recurse);
    auto decoded = source->getDecodedProgram();
    for (auto& record : decoded.records)
        if (record.opcode == HGBCOpcode::Call && record.operands[2] == placeholder)
            record.operands[2] = recurse;
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(decoded, bytes));
    RefPtr<Program> program;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(Program::load(bytes.data(), bytes.size(), "recursive", program, error)));
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.outputSink = output;
    DebugSession session(desc);
    session.addFunctionBreakpoint("leaf");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    RecordingStops listener;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK_ABORT(listener.traces.getCount() == 1);
    const auto& trace = listener.traces[0];
    SLANG_CHECK_ABORT(trace.getCount() == 6);
    SLANG_CHECK(trace[0].functionName == "leaf" && trace[5].functionName == "main");
    for (const auto& frame : trace)
        SLANG_CHECK(!frame.isInlined);
    for (Index i = 1; i < 5; ++i)
        SLANG_CHECK(trace[i].functionName == "recurse");
    SLANG_CHECK(output->text == "0\n");

    // Finish the second recursive invocation while it still has deeper calls ahead of it.
    // Each deeper return visits the very same temporary address in recurse, and must be ignored.
    struct FinishRecursiveCall : IDebugSessionListener
    {
        uint32_t breakpoint = 0;
        Index stops = 0;
        DebugCommand onStopped(DebugSession& session) override
        {
            auto trace = session.getStackTrace();
            ++stops;
            if (stops == 1)
            {
                SLANG_CHECK(trace.getCount() == 2);
                return DebugCommand::Continue;
            }
            if (stops == 2)
            {
                SLANG_CHECK(trace.getCount() == 3);
                session.enableBreakpoint(breakpoint, false);
                return DebugCommand::Finish;
            }
            SLANG_CHECK(stops == 3 && trace.getCount() == 2);
            SLANG_CHECK(session.getHitBreakpoint() == 0);
            return DebugCommand::Continue;
        }
    } finish;
    session.enableBreakpoint(session.getBreakpoints()[0].id, false);
    finish.breakpoint = session.addFunctionBreakpoint("recurse");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &finish, error)));
    SLANG_CHECK(finish.stops == 3);
}

SLANG_UNIT_TEST(hgvmDebuggerTemporaryBreakpoints)
{
    // Inspect record bits from ordinary program output while nexti is running through work.
    // This verifies that stepping installs a real continuation breakpoint, not just the same
    // user-visible stops obtained by single-stepping every callee instruction.
    struct InspectContinuation : IHGVMOutputSink
    {
        DebugSession* session = nullptr;
        uint32_t continuation = 0;
        Index writes = 0;
        void writeLine(std::string_view) override
        {
            auto* record = session->getInterpreter()->getRecord(
                {session->getDebugMap()->getProgramId(), continuation});
            bool armed = (record->opcodeAndFlags & StagedRecord::kBreakpointBit) != 0;
            SLANG_CHECK(armed == (writes < 2));
            ++writes;
        }
    };
    struct StepOverCall : IDebugSessionListener
    {
        uint32_t continuation = 0;
        uint32_t userBreakpoint = 0;
        Index stops = 0;
        bool cancel = false;
        DebugCommand onStopped(DebugSession& session) override
        {
            if (++stops == 1)
                return DebugCommand::NextInstruction;
            SLANG_CHECK(stops == 2);
            SLANG_CHECK(session.getHitBreakpoint() == userBreakpoint);
            if (cancel)
                return DebugCommand::Stop;
            SLANG_CHECK(session.getAddress().wordOffset == continuation);
            auto* record = session.getInterpreter()->getRecord(session.getAddress());
            SLANG_CHECK(
                bool(record->opcodeAndFlags & StagedRecord::kBreakpointBit) ==
                bool(userBreakpoint));
            if (userBreakpoint)
                session.enableBreakpoint(userBreakpoint, false);
            return DebugCommand::Continue;
        }
    } listener;
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[noinline] void work() { print(1); print(2); }\n"
        "[host] void main() {\n"
        "    work();\n"
        "    print(3);\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_temporary");
    RefPtr<InspectContinuation> output = new InspectContinuation;
    Desc desc;
    desc.outputSink = output;
    DebugSession session(desc);
    output->session = &session;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    // Use the user's source breakpoint to locate the first executable instruction after work.
    uint32_t userBreakpoint = session.addBreakpoint("hgvm_temporary.slang", 4);
    const auto& locations = session.getBreakpoints()[0].addresses;
    SLANG_CHECK_ABORT(locations.getCount() == 1);
    listener.continuation = output->continuation = locations[0];
    session.enableBreakpoint(userBreakpoint, false);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", true, &listener, error)));
    SLANG_CHECK(listener.stops == 2 && output->writes == 3);

    // A user breakpoint sharing the temporary bit survives removal of the temporary owner.
    session.enableBreakpoint(userBreakpoint, true);
    listener.stops = output->writes = 0;
    listener.userBreakpoint = userBreakpoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", true, &listener, error)));
    SLANG_CHECK(listener.stops == 2 && output->writes == 3);
    listener.userBreakpoint = 0;

    // A user breakpoint in the callee can interrupt nexti and cancel the run. The abandoned
    // continuation must not remain armed after the interpreter unwinds.
    listener.userBreakpoint = session.addFunctionBreakpoint("work");
    listener.cancel = true;
    listener.stops = output->writes = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", true, &listener, error)));
    SLANG_CHECK(session.wasTerminated() && listener.stops == 2 && output->writes == 0);
    SLANG_CHECK(
        !(session.getInterpreter()
              ->getRecord({session.getDebugMap()->getProgramId(), output->continuation})
              ->opcodeAndFlags &
          StagedRecord::kBreakpointBit));
    session.enableBreakpoint(listener.userBreakpoint, false);
    listener.userBreakpoint = 0;
    listener.cancel = false;

    // A failed call never reaches its continuation. Its temporary bits must still be removed.
    program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[noinline] void work() { print(1); String message = \"stepping failure\";\n"
        "    hgbc_asm { OpFail $message; }; }\n"
        "[host] void main() {\n"
        "    work();\n"
        "    print(3);\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_failure");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    uint32_t failureContinuation = session.addBreakpoint("hgvm_failure.slang", 5);
    const auto& failureLocations = session.getBreakpoints().getLast().addresses;
    SLANG_CHECK_ABORT(failureLocations.getCount() == 1);
    output->continuation = failureLocations[0];
    session.enableBreakpoint(failureContinuation, false);
    listener.stops = output->writes = 0;
    SLANG_CHECK(SLANG_FAILED(session.run("main", true, &listener, error)));
    SLANG_CHECK(error.contains("stepping failure"));
    SLANG_CHECK(listener.stops == 1 && output->writes == 1);
    auto* record = session.getInterpreter()->getRecord(
        {session.getDebugMap()->getProgramId(), output->continuation});
    SLANG_CHECK(!(record->opcodeAndFlags & StagedRecord::kBreakpointBit));
}

SLANG_UNIT_TEST(hgvmDebuggerNestedInlineSiblings)
{
    for (const char* attribute : {"ForceInline", "__unsafeForceInlineEarly"})
    {
        StringBuilder source;
        source << "[noinline] int marker(int x) { return x + 1; }\n"
                  "[ForceInline] int inner(int x) { return marker(x); }\n"
                  "["
               << attribute
               << "] int outer(int x) { return inner(x) + inner(x + 1); }\n"
                  "[host] void main() { print(outer(1)); }\n";
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_STANDARD);
        RefPtr<DebugTestOutput> output = new DebugTestOutput;
        Desc desc;
        desc.outputSink = output;
        DebugSession session(desc);
        String error;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
        session.addFunctionBreakpoint("marker");
        RecordingStops listener;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
        SLANG_CHECK_ABORT(listener.traces.getCount() == 2);
        for (const auto& trace : listener.traces)
        {
            SLANG_CHECK_ABORT(trace.getCount() == 4);
            SLANG_CHECK(trace[0].functionName == "marker" && trace[0].line == 1);
            SLANG_CHECK(trace[1].functionName == "inner" && trace[1].line == 2);
            SLANG_CHECK(trace[2].functionName == "outer" && trace[2].line == 3);
            SLANG_CHECK(trace[3].functionName == "main" && trace[3].line == 4);
            SLANG_CHECK(
                !trace[0].isInlined && trace[1].isInlined && trace[2].isInlined &&
                !trace[3].isInlined);
        }
        SLANG_CHECK(output->text == "5\n");

        // Scope callbacks receive nested inline depth directly, even while continuing without
        // instruction stops. The callback does not need a debug map to track that depth.
        RecordingSteps control;
        control.action = StepAction::Continue;
        desc.stepControl = &control;
        desc.debugHandler = &control;
        Interpreter interpreter(desc);
        output->text = {};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
        SLANG_CHECK(control.maximumInlineSiteCount == 2);
        SLANG_CHECK(control.depth == 0 && control.maximumDepth == 2);
        SLANG_CHECK(control.stops.getCount() == 1);
        SLANG_CHECK(output->text == "5\n");
    }
}

SLANG_UNIT_TEST(hgvmDebuggerGPUExecutionModes)
{
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    if (!(unitTestContext->enabledApis & RenderApiFlag::Vulkan))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = rhi::DeviceType::Vulkan;
    deviceDesc.slang.slangGlobalSession = unitTestContext->slangGlobalSession;
    deviceDesc.debugCallback = unitTestContext->debugCallback;
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[shader(\"compute\")] [numthreads(1,1,1)]\n"
        "void update(RWStructuredBuffer<uint> buffer) { buffer[0] += 1; }\n"
        "[host] void main()\n"
        "{\n"
        "    let source = RuntimeArray<uint>::alloc(1);\n"
        "    source[0] = 7;\n"
        "    let buffer = RWStructuredBuffer<uint>::alloc(1);\n"
        "    copy(buffer, 0, source, 0, 1);\n"
        "    dispatch(update, uint3(1,1,1))(buffer);\n"
        "    dispatch(update, uint3(1,1,1))(buffer);\n"
        "    print(7);\n"
        "    print(readBack(buffer, 0));\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_gpu_modes");

    struct Events : IEventSink
    {
        uint32_t submissions = 0;
        uint32_t completions = 0;
        void onEvent(const TrackingEvent& event) noexcept override
        {
            submissions += event.kind == TrackingEventKind::CommandBufferSubmit;
            completions += event.kind == TrackingEventKind::PipelineExecutionComplete;
        }
        void check(bool synchronous)
        {
            SLANG_CHECK(submissions == (synchronous ? 3u : 0u));
            SLANG_CHECK(completions == (synchronous ? 2u : 0u));
        }
    };
    struct Output : DebugTestOutput
    {
        Events* events = nullptr;
        bool synchronous = false;
        void writeLine(std::string_view line) override
        {
            if (events && !text.getLength())
                events->check(synchronous);
            DebugTestOutput::writeLine(line);
        }
    };
    struct Listener : IDebugSessionListener
    {
        Events* events = nullptr;
        bool synchronous = false;
        uint32_t stops = 0;
        DebugCommand onStopped(DebugSession& session) override
        {
            ++stops;
            events->check(synchronous);
            String error;
            List<DebugValueInfo> locals;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.getVariables(0, false, locals, error)));
            DebugValueInfo value;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                session.evaluateExpression(0, "buffer", value, error, false, false)));
            SLANG_CHECK(value.indexedChildren == 1 && value.reference != 0);
            uint64_t reference = value.reference;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(session.evaluateExpression(0, "buffer[0]", value, error, true)));
            events->check(synchronous);
            List<DebugValueInfo> children;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(session.getValueChildren(reference, 0, 1, children, error)));
            SLANG_CHECK_ABORT(children.getCount() == 1);
            SLANG_CHECK(children[0].value == "9");
            SLANG_CHECK(events->completions == 2);
            SLANG_CHECK(events->submissions == (synchronous ? 3u : 1u));
            return DebugCommand::Continue;
        }
    };
    for (bool debugging : {false, true})
        for (auto mode :
             {GPUExecutionMode::Default, GPUExecutionMode::Buffered, GPUExecutionMode::Synchronous})
        {
            bool synchronous = mode == GPUExecutionMode::Synchronous ||
                               (mode == GPUExecutionMode::Default && debugging);
            RefPtr<Events> events = new Events;
            RefPtr<Output> output = new Output;
            output->events = debugging ? nullptr : events.get();
            output->synchronous = synchronous;
            Desc desc;
            desc.device = device;
            desc.gpuExecution = mode;
            desc.enableEventTracking = true;
            desc.eventSink = events;
            desc.outputSink = output;
            String error;
            if (debugging)
            {
                DebugSession session(desc);
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
                session.addBreakpoint("hgvm_gpu_modes.slang", 11);
                Listener listener;
                listener.events = events;
                listener.synchronous = synchronous;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
                SLANG_CHECK(listener.stops == 1);
            }
            else
            {
                Interpreter interpreter(desc);
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
            }
            SLANG_CHECK(output->text == "7\n9\n");
        }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(hgvmDebuggerTextureMips)
{
    if (!(unitTestContext->enabledApis & RenderApiFlag::Vulkan))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = rhi::DeviceType::Vulkan;
    deviceDesc.slang.slangGlobalSession = unitTestContext->slangGlobalSession;
    deviceDesc.debugCallback = unitTestContext->debugCallback;
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        "[host] void main()\n"
        "{\n"
        "    uint width = 7, height = 3, one = 1, zero = 0;\n"
        "    let texture = hgbc_asm {\n"
        "        result : $$Texture2D<float> = OpAllocateResource\n"
        "            Texture Texture2D $__getHGBCNaturalTextureFormat<float>()\n"
        "            DeviceLocal ShaderResource $$float $width $height $one 3 1 0\n"
        "            ShaderResource CopySource CopyDestination RenderTarget;\n"
        "    };\n"
        "    for (uint mip = 0; mip < 3; ++mip)\n"
        "    {\n"
        "        float value = float(mip) + 0.5;\n"
        "        hgbc_asm { OpClearTexture $$Texture2D<float> $texture $zero\n"
        "            $mip $one $zero $one $value; };\n"
        "    }\n"
        "    print(7);\n"
        "    print(8);\n"
        "}\n",
        SLANG_DEBUG_INFO_LEVEL_STANDARD,
        "hgvm_texture_mips");
    struct Events : IEventSink
    {
        uint32_t submissions = 0;
        void onEvent(const TrackingEvent& event) noexcept override
        {
            submissions += event.kind == TrackingEventKind::CommandBufferSubmit;
        }
    };
    struct Writer : SequentialByteWriter
    {
        std::vector<uint8_t> bytes;
        bool failWrites = false;
        SlangResult write(std::span<const uint8_t> source) override
        {
            if (failWrites)
                return SLANG_FAIL;
            bytes.insert(bytes.end(), source.begin(), source.end());
            return SLANG_OK;
        }
    };
    struct Listener : IDebugSessionListener
    {
        Events* events = nullptr;
        uint64_t previousMip = 0;
        uint32_t stops = 0;
        DebugCommand onStopped(DebugSession& session) override
        {
            String error;
            Writer writer;
            if (stops++)
            {
                SLANG_CHECK(SLANG_FAILED(session.exportTextureMip(previousMip, writer, error)));
                SLANG_CHECK(error.contains("expired"));
                return DebugCommand::Continue;
            }
            SLANG_CHECK(events->submissions == 0);
            DebugValueInfo texture;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                session.evaluateExpression(0, "texture", texture, error, false, false)));
            SLANG_CHECK(texture.namedChildren == 3);
            List<DebugValueInfo> mips, fields;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(session.getValueChildren(texture.reference, 0, 0, mips, error)));
            SLANG_CHECK_ABORT(mips.getCount() == 3);
            SLANG_CHECK(
                mips[0].value == "7 x 3" && mips[1].value == "3 x 1" && mips[2].value == "1 x 1");
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(session.getValueChildren(mips[1].reference, 0, 0, fields, error)));
            SLANG_CHECK_ABORT(fields.getCount() == 3);
            SLANG_CHECK(
                fields[0].value == "3" && fields[1].value == "1" && fields[2].value == "R32Float");
            SLANG_CHECK(events->submissions == 0);
            SLANG_CHECK(SLANG_FAILED(session.exportTextureMip(texture.reference, writer, error)));
            SLANG_CHECK(error.contains("Select a texture mip"));
            for (uint32_t mip = 0; mip < 3; ++mip)
            {
                writer.bytes.clear();
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(session.exportTextureMip(mips[mip].reference, writer, error)));
                DecodedTextureImage image;
                std::string decodeError;
                SLANG_CHECK_ABORT(decodeTextureEXR(writer.bytes, image, decodeError));
                SLANG_CHECK(image.width == Math::Max(1u, 7u >> mip));
                SLANG_CHECK(image.height == Math::Max(1u, 3u >> mip));
                for (size_t i = 0; i < image.rgba.size(); i += 4)
                {
                    SLANG_CHECK(image.rgba[i] == float(mip) + 0.5f);
                    SLANG_CHECK(
                        image.rgba[i + 1] == 0 && image.rgba[i + 2] == 0 && image.rgba[i + 3] == 1);
                }
            }
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            SLANG_CHECK(events->submissions == 1);
#endif
            writer.failWrites = true;
            SLANG_CHECK(SLANG_FAILED(session.exportTextureMip(mips[0].reference, writer, error)));
            SLANG_CHECK(error.contains("write"));
            previousMip = mips[1].reference;
            return DebugCommand::Continue;
        }
    } listener;
    RefPtr<Events> events = new Events;
    RefPtr<DebugTestOutput> output = new DebugTestOutput;
    Desc desc;
    desc.device = device;
    desc.gpuExecution = GPUExecutionMode::Buffered;
    desc.outputSink = output;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    DebugSession session(desc);
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.prepare(program, error)));
    session.addBreakpoint("hgvm_texture_mips.slang", 16);
    session.addBreakpoint("hgvm_texture_mips.slang", 17);
    listener.events = events;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session.run("main", false, &listener, error)));
    SLANG_CHECK(listener.stops == 2);
    SLANG_CHECK(output->text == "7\n8\n");
}

SLANG_UNIT_TEST(hgvmTextureEXRFormats)
{
    struct Case
    {
        rhi::Format format;
        uint8_t pixel[8];
        float expected[4];
    };
    const Case cases[] = {
        {rhi::Format::RG16Float, {0, 0x3c, 0, 0xc0}, {1, -2, 0, 1}},
        {rhi::Format::RG16Unorm, {0xff, 0xff, 0, 0}, {1, 0, 0, 1}},
        {rhi::Format::RG8Snorm, {0x80, 0x7f}, {-1, 1, 0, 1}},
        {rhi::Format::RG16Snorm, {0, 0x80, 0xff, 0x7f}, {-1, 1, 0, 1}},
        {rhi::Format::RGBA8UnormSrgb, {128, 0, 255, 128}, {0.2158605f, 0, 1, 128.f / 255}},
        {rhi::Format::BGRA8Unorm, {0, 128, 255, 255}, {1, 128.f / 255, 0, 1}},
    };
    for (const auto& test : cases)
    {
        TextureInfo info;
        info.format = test.format;
        info.width = info.height = 2;
        rhi::SubresourceLayout layout{};
        layout.size = {2, 2, 1};
        layout.rowPitch = 32;
        layout.sizeInBytes = 64;
        uint8_t bytes[64] = {};
        size_t pixelSize = rhi::getFormatInfo(test.format).blockSizeInBytes;
        for (size_t y = 0; y < 2; ++y)
            for (size_t x = 0; x < 2; ++x)
                memcpy(bytes + y * layout.rowPitch + x * pixelSize, test.pixel, pixelSize);
        std::vector<uint8_t> exr;
        std::string error;
        SLANG_CHECK_ABORT(encodeTextureEXR(info, layout, bytes, exr, error));
        DecodedTextureImage image;
        SLANG_CHECK_ABORT(decodeTextureEXR(exr, image, error));
        SLANG_CHECK(image.width == 2 && image.height == 2);
        for (size_t i = 0; i < image.rgba.size(); ++i)
            SLANG_CHECK(Math::Abs(image.rgba[i] - test.expected[i % 4]) < 0.00001f);
        info.format = rhi::Format::RGB10A2Unorm;
        SLANG_CHECK(!encodeTextureEXR(info, layout, bytes, exr, error));
        SLANG_CHECK(error.find("not supported") != std::string::npos);
    }
}
