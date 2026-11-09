#pragma once

#include "slang-hgvm-debug-map.h"
#include "slang-hgvm-debug.h"
#include "slang-hgvm.h"

namespace Slang::HGVM
{
enum class DebugCommand
{
    Continue,
    StepInstruction,
    NextInstruction,
    Step,
    Next,
    Finish,
    Stop,
};

enum class DebugStopReason
{
    Entry,
    Breakpoint,
    Step,
    Pause
};
enum class DebugControlAction
{
    Resume,
    Pause,
    Stop
};

struct DebugBreakpoint
{
    uint32_t id = 0;
    String file;
    uint32_t line = 0;
    String function;
    bool enabled = true;
    List<uint32_t> addresses;
};

/// Describes a logical stack entry reconstructed when the debugger inspects a stop.
struct DebugStackFrame : SourcePosition
{
    bool isInlined = false;
};

/// Owns a bounded preview or a shallow value with a reference for lazy child inspection.
/// References are valid only until execution resumes; no storage pointers are exposed.
struct DebugValueInfo
{
    String name;
    String typeName;
    String value;
    List<DebugValueInfo> children;
    bool childrenTruncated = false;
    uint64_t reference = 0;
    uint32_t namedChildren = 0;
    uint32_t indexedChildren = 0;
    bool isTextureMip = false;
};

class DebugSession;

/// Receives actual debugger stops. Frontends may inspect the session and edit breakpoints here.
class IDebugSessionListener
{
public:
    virtual ~IDebugSessionListener() = default;
    virtual DebugCommand onStopped(DebugSession& session) = 0;
    /// Services pending frontend requests on the execution thread without losing a stepping plan.
    virtual DebugControlAction onControl(DebugSession&) { return DebugControlAction::Resume; }
};

/// Owns debugger policy and an interpreter, independently of command syntax or transport.
/// The caller serializes preparation, execution and breakpoint edits on one thread. Callbacks
/// retain scope indices and call-site offsets; stack names and locations are decoded only on
/// demand.
class DebugSession : public RefObject, private IStepControl, private IDebugHandler
{
public:
    explicit DebugSession(const Desc& desc);
    ~DebugSession();
    Interpreter* getInterpreter() const { return m_interpreter; }
    DebugMap* getDebugMap() const { return m_map; }
    DebugInstructionAddress getAddress() const { return m_address; }
    uint32_t getHitBreakpoint() const { return m_hitBreakpoint; }
    bool wasTerminated() const { return m_terminated; }
    DebugStopReason getStopReason() const { return m_stopReason; }
    /// This is the only session operation that may be called from a different thread.
    void requestControl() { IStepControl::requestControl(); }
    const List<DebugBreakpoint>& getBreakpoints() const { return m_breakpoints; }

    SlangResult prepare(Program* program, String& error);
    SlangResult run(
        const String& entry,
        bool stopAtEntry,
        IDebugSessionListener* listener,
        String& error);
    uint32_t addBreakpoint(const String& file, uint32_t line);
    uint32_t addFunctionBreakpoint(const String& name);
    bool removeBreakpoint(uint32_t id);
    bool enableBreakpoint(uint32_t id, bool enabled);
    List<DebugBreakpoint> replaceSourceBreakpoints(const String& file, const List<uint32_t>& lines);
    List<DebugBreakpoint> replaceFunctionBreakpoints(const List<String>& names);
    List<DebugStackFrame> getStackTrace() const;
    SlangResult getVariables(
        Index frame,
        bool arguments,
        List<DebugValueInfo>& values,
        String& error,
        bool expand = true);
    SlangResult evaluateExpression(
        Index frame,
        const String& expression,
        DebugValueInfo& value,
        String& error,
        bool typeOnly = false,
        bool expand = true);
    /// Enumerates one level of children. References expire when execution resumes.
    SlangResult getValueChildren(
        uint64_t reference,
        uint32_t start,
        uint32_t count,
        List<DebugValueInfo>& values,
        String& error);

    /// Exports a selected mip during a stop without depending on frontend file handling.
    SlangResult exportTextureMip(
        uint64_t reference,
        SequentialByteWriter& destination,
        String& error);

private:
    enum class StoredValueKind
    {
        Value,
        Texture,
        TextureMip
    };
    struct ValueLocation
    {
        HGBCId type = 0;
        Address address;
        uint32_t offset = 0;
        bool heapData = false;
        uint32_t count = 0;
        uint32_t rowWidth = 0;
        Index snapshot = -1;
    };
    struct StoredValue
    {
        DebugCallId call;
        ValueLocation location;
        StoredValueKind kind = StoredValueKind::Value;
        uint32_t mip = 0;
    };
    struct ValueReader;
    struct ValueExpression;
    Dictionary<uint64_t, StoredValue> m_values;
    struct BufferSnapshot
    {
        Address buffer;
        HGBCId elementType = 0;
        uint32_t first = 0;
        uint32_t count = 0;
        uint32_t stride = 0;
        List<uint8_t> bytes;
    };
    List<BufferSnapshot> m_bufferSnapshots;
    uint64_t m_bufferSnapshotBytes = 0;
    void clearValues();
    uint64_t m_nextValueReference = 1;
    struct VariableContext
    {
        DebugCallId call;
        HGBCId scope = 0;
        HGBCId inlinedAt = 0;
        HGBCId function = 0;
        uint32_t pc = 0;
    };
    bool getVariableContext(Index frame, VariableContext& context) const;
    List<const DebugVariableInfo*> getVisibleVariables(const VariableContext& context) const;
    /// Tracks one active function call; inline scopes are expanded when a trace is requested.
    struct CallState
    {
        uint32_t callSite = 0;
        uint32_t scopeIndex = 0;
        uint32_t inlineSiteCount = 0;
        DebugCallId interpreterCall;
    };
    struct StopLocation
    {
        uint32_t address;
        Index sourceRange;
        Index function;
        bool blockEntry;
        uint32_t inlineSiteCount;
    };
    StepAction onStep(DebugInstructionAddress address, bool breakpointHit) override;
    StepAction onControl(DebugInstructionAddress address, StepAction action, bool breakpointHit)
        override;
    StepAction stopAt(DebugInstructionAddress address, DebugStopReason reason);
    void onFunctionEnter(DebugInstructionAddress callSite, DebugCallId call) override;
    void onFunctionExit() override;
    void onScope(uint32_t index, uint32_t inlineSiteCount) override;
    void resolveBreakpoints();
    void addTemporaryBreakpoint(uint32_t address);
    void clearTemporaryBreakpoints();
    void addCallerContinuation();
    List<uint32_t> findInlineContinuations(uint32_t address, uint32_t maxInlineSiteCount) const;
    StepAction prepareStep(DebugInstructionAddress address);
    void rememberSourceRange(uint32_t address);
    uint32_t findHitBreakpoint(uint32_t address) const;

    RefPtr<Interpreter> m_interpreter;
    RefPtr<Program> m_program;
    RefPtr<DebugMap> m_map;
    List<DebugBreakpoint> m_breakpoints;
    List<StopLocation> m_stopLocations;
    List<uint32_t> m_sourceKeys;
    HashSet<uint32_t> m_armedAddresses;
    HashSet<uint32_t> m_temporaryBreakpoints;
    List<CallState> m_callStack;
    IDebugSessionListener* m_listener = nullptr;
    DebugInstructionAddress m_address;
    DebugCommand m_command = DebugCommand::Continue;
    DebugStopReason m_stopReason = DebugStopReason::Entry;
    Index m_logicalDepth = 0;
    Index m_startDepth = 0;
    Index m_startCallDepth = 0;
    Index m_sourceRange = -1;
    uint32_t m_rangeStart = 0;
    uint32_t m_rangeEnd = 0;
    uint32_t m_previousAddress = 0;
    uint32_t m_startSourceKey = 0;
    uint64_t m_previousCallId = 0;
    uint64_t m_startCallId = 0;
    uint32_t m_nextBreakpoint = 1;
    uint32_t m_hitBreakpoint = 0;
    bool m_entryPending = false;
    bool m_stopAtEntry = false;
    bool m_running = false;
    bool m_stopped = false;
    bool m_terminated = false;
};
} // namespace Slang::HGVM
