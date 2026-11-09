#include "slang-hgvm-debug-session.h"

#include "core/slang-io.h"
#include "slang-hgvm-staged-records.h"

namespace Slang::HGVM
{
/// Normalizes source spelling once when resolving a breakpoint, never in an execution callback.
static String normalizeDebugPath(const String& path)
{
    String result;
    if (SLANG_FAILED(Path::getCanonical(path, result)))
        result = Path::simplify(path);
#if SLANG_WINDOWS_FAMILY
    result = result.toLower();
#endif
    return result;
}

/// Matches a recorded source interval during breakpoint resolution, including inline call sites.
static bool matchesDebugPosition(const SourcePosition& position, const String& path, uint32_t line)
{
    return position.line && position.line <= line &&
           line <= (position.endLine ? position.endLine : position.line) &&
           normalizeDebugPath(position.path) == path;
}

DebugSession::DebugSession(const Desc& desc)
{
    Desc debugDesc = desc;
    if (debugDesc.gpuExecution == GPUExecutionMode::Default)
        debugDesc.gpuExecution = GPUExecutionMode::Synchronous;
    debugDesc.stepControl = this;
    debugDesc.debugHandler = this;
    m_interpreter = new Interpreter(debugDesc);
    m_callStack.reserve(64);
}

DebugSession::~DebugSession() = default;

void DebugSession::clearValues()
{
    m_values.clear();
    m_bufferSnapshots.clear();
    m_bufferSnapshotBytes = 0;
}

SlangResult DebugSession::prepare(Program* program, String& error)
{
    SLANG_RELEASE_ASSERT(!m_running);
    uint64_t id = 0;
    SLANG_RETURN_ON_FAIL(m_interpreter->prepareProgram(program, id, error));
    clearTemporaryBreakpoints();
    if (m_map)
        for (uint32_t address : m_armedAddresses)
            m_interpreter->setBreakpoint({m_map->getProgramId(), address}, false);
    m_armedAddresses.clear();
    m_program = program;
    m_map = m_interpreter->getDebugMap(id);
    m_stopLocations.clear();
    m_sourceKeys.clear();
    Dictionary<String, uint32_t> sourceKeys;
    for (const auto& range : m_map->getSourceRanges())
    {
        const auto& location = range.location;
        StringBuilder key;
        key << location.path << ':' << location.line;
        m_sourceKeys.add(
            location.line
                ? sourceKeys.getOrAddValue(key.produceString(), uint32_t(sourceKeys.getCount() + 1))
                : 0);
    }
    Index blockIndex = 0;
    const auto& blocks = m_map->getBlockStarts();
    const auto& functions = m_map->getFunctions();
    // Walk source-instruction boundaries once, including an initial ownership update. A source
    // range can be entered at an interior block, so retain both range starts and block entries.
    for (Index f = 0; f < functions.getCount(); ++f)
    {
        Index previousRange = -2;
        bool blockEntry = true;
        uint32_t inlineSiteCount = 0;
        for (uint32_t pc = functions[f].entry; pc < functions[f].end;)
        {
            while (blockIndex < blocks.getCount() && blocks[blockIndex] <= pc)
            {
                blockEntry = true;
                ++blockIndex;
            }
            const auto* header = m_interpreter->getRecord({id, pc});
            if ((header->opcodeAndFlags & StagedRecord::kDebugRecordBit) &&
                (header->opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                    uint32_t(HGBCOpcode::DebugScope))
                inlineSiteCount =
                    reinterpret_cast<const StagedRecord::DebugScope*>(header)->inlineSiteCount;
            if (!(header->opcodeAndFlags &
                  (StagedRecord::kDebugRecordBit | StagedRecord::kInstructionContinuationBit)))
            {
                Index range = m_map->findSourceRange(pc);
                if (blockEntry || range != previousRange)
                    m_stopLocations.add({pc, range, f, blockEntry, inlineSiteCount});
                previousRange = range;
                blockEntry = false;
            }
            pc += header->recordWordCount;
        }
    }
    resolveBreakpoints();
    return SLANG_OK;
}

void DebugSession::resolveBreakpoints()
{
    SLANG_RELEASE_ASSERT(!m_running || m_stopped);
    if (!m_map)
        return;
    for (uint32_t address : m_armedAddresses)
        if (!m_temporaryBreakpoints.contains(address))
            m_interpreter->setBreakpoint({m_map->getProgramId(), address}, false);
    m_armedAddresses.clear();
    for (auto& breakpoint : m_breakpoints)
    {
        breakpoint.addresses.clear();
        String path = normalizeDebugPath(breakpoint.file);
        Index lastFunction = -1;
        bool previousMatch = false;
        for (const auto& stop : m_stopLocations)
        {
            bool matches = false;
            if (breakpoint.function.getLength())
                matches = stop.function != lastFunction &&
                          m_map->getFunctions()[stop.function].name == breakpoint.function;
            else if (stop.sourceRange >= 0)
            {
                const auto& loc = m_map->getSourceRanges()[stop.sourceRange].location;
                matches = matchesDebugPosition(loc, path, breakpoint.line);
                for (const auto& site : loc.inlineSites)
                    matches |= matchesDebugPosition(site, path, breakpoint.line);
            }
            bool entry = !previousMatch || stop.function != lastFunction || stop.blockEntry;
            previousMatch = matches;
            lastFunction = stop.function;
            if (!matches || !entry)
                continue;
            breakpoint.addresses.add(stop.address);
            if (breakpoint.enabled && m_armedAddresses.add(stop.address))
                m_interpreter->setBreakpoint({m_map->getProgramId(), stop.address});
        }
    }
}

uint32_t DebugSession::addBreakpoint(const String& file, uint32_t line)
{
    DebugBreakpoint breakpoint;
    breakpoint.id = m_nextBreakpoint++;
    breakpoint.file = file;
    breakpoint.line = line;
    m_breakpoints.add(breakpoint);
    resolveBreakpoints();
    return breakpoint.id;
}

uint32_t DebugSession::addFunctionBreakpoint(const String& name)
{
    DebugBreakpoint breakpoint;
    breakpoint.id = m_nextBreakpoint++;
    breakpoint.function = name;
    m_breakpoints.add(breakpoint);
    resolveBreakpoints();
    return breakpoint.id;
}

bool DebugSession::removeBreakpoint(uint32_t id)
{
    for (Index i = 0; i < m_breakpoints.getCount(); ++i)
        if (m_breakpoints[i].id == id)
        {
            m_breakpoints.removeAt(i);
            resolveBreakpoints();
            return true;
        }
    return false;
}

bool DebugSession::enableBreakpoint(uint32_t id, bool enabled)
{
    for (auto& breakpoint : m_breakpoints)
        if (breakpoint.id == id)
        {
            breakpoint.enabled = enabled;
            resolveBreakpoints();
            return true;
        }
    return false;
}

SlangResult DebugSession::run(
    const String& entry,
    bool stopAtEntry,
    IDebugSessionListener* listener,
    String& error)
{
    SLANG_RELEASE_ASSERT(m_program && listener && !m_running);
    m_listener = listener;
    m_entryPending = true;
    m_stopAtEntry = stopAtEntry;
    m_terminated = false;
    m_callStack.clear();
    m_logicalDepth = 0;
    m_running = true;
    SlangResult result = m_interpreter->call(m_program, entry.getBuffer(), error);
    m_running = false;
    clearTemporaryBreakpoints();
    m_stopped = false;
    clearValues();
    m_listener = nullptr;
    m_callStack.clear();
    m_logicalDepth = 0;
    return m_terminated && result == SLANG_E_ABORT ? SLANG_OK : result;
}

void DebugSession::onFunctionEnter(DebugInstructionAddress callSite, DebugCallId call)
{
    m_callStack.add({callSite.wordOffset, 0, 0, call});
    ++m_logicalDepth;
}

void DebugSession::onFunctionExit()
{
    SLANG_RELEASE_ASSERT(m_callStack.getCount());
    m_logicalDepth -= 1 + m_callStack.getLast().inlineSiteCount;
    m_callStack.removeLast();
}

void DebugSession::onScope(uint32_t index, uint32_t inlineSiteCount)
{
    auto& callState = m_callStack.getLast();
    m_logicalDepth -= callState.inlineSiteCount;
    callState.scopeIndex = index;
    callState.inlineSiteCount = inlineSiteCount;
    m_logicalDepth += inlineSiteCount;
}

/// Arms an executable continuation after applying any intervening scope and value updates.
void DebugSession::addTemporaryBreakpoint(uint32_t address)
{
    const uint64_t programId = m_map->getProgramId();
    for (;;)
    {
        const auto* header = m_interpreter->getRecord({programId, address});
        if (!(header->opcodeAndFlags & StagedRecord::kDebugRecordBit))
            break;
        address += header->recordWordCount;
    }
    if (m_temporaryBreakpoints.add(address))
        m_interpreter->setBreakpoint({programId, address});
}

/// Removes stepping's ownership without clearing a breakpoint set by the user at the same address.
void DebugSession::clearTemporaryBreakpoints()
{
    for (uint32_t address : m_temporaryBreakpoints)
        if (!m_armedAddresses.contains(address))
            m_interpreter->setBreakpoint({m_map->getProgramId(), address}, false);
    m_temporaryBreakpoints.clear();
}

/// Uses the saved call site to stop after the current physical frame returns, including cleanup.
/// The entry frame has no caller; finishing it is allowed to complete the program without a stop.
void DebugSession::addCallerContinuation()
{
    if (m_callStack.getCount() < 2)
        return;
    uint32_t callSite = m_callStack.getLast().callSite;
    const auto* call = m_interpreter->getRecord({m_map->getProgramId(), callSite});
    addTemporaryBreakpoint(callSite + call->recordWordCount);
}

/// Finds conservative continuation addresses for leaving deeper inline scopes in this function.
///
/// Consider `main() { print(outer()); }`, where outer inlines an inner function containing an if.
/// There is no runtime return address for inner. Instead, executing a DebugScope setter changes
/// the logical inline depth. Scan the entire physical function, including blocks before the
/// current PC, and collect the first executable record after each setter whose inline-site count
/// is at most maxInlineSiteCount. Adjacent setters are combined: only the last context is visible
/// at the next executable record. DebugValue records are skipped so their updates precede a stop.
///
/// This deliberately returns a superset of exits. It needs neither a contiguous inline address
/// range nor a prediction of the taken branch, and handles multiple exits and backward edges.
/// Unreachable candidates never fire. Recursive invocations or other paths can reach a candidate
/// in the wrong context, so onStep must still check the live stack depth and call identity.
/// Physical returns do not execute a scope setter and are covered separately by the saved caller
/// continuation. Addresses in generated cleanup have no remaining body scopes to scan.
///
/// This is the replaceable inline-continuation policy. It could later use explicit continuation
/// metadata or control-flow analysis without adding work to onScope or changing interpreter APIs.
List<uint32_t> DebugSession::findInlineContinuations(uint32_t address, uint32_t maxInlineSiteCount)
    const
{
    List<uint32_t> result;
    for (const auto& function : m_map->getFunctions())
    {
        if (address < function.entry || address >= function.end)
            continue;
        bool pendingScope = false;
        uint32_t inlineSiteCount = 0;
        uint32_t pc = function.entry;
        for (; pc < function.end;)
        {
            const auto* header = m_interpreter->getRecord({m_map->getProgramId(), pc});
            if (header->opcodeAndFlags & StagedRecord::kDebugRecordBit)
            {
                if ((header->opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                    uint32_t(HGBCOpcode::DebugScope))
                {
                    inlineSiteCount =
                        reinterpret_cast<const StagedRecord::DebugScope*>(header)->inlineSiteCount;
                    pendingScope = true;
                }
            }
            else
            {
                if (pendingScope && inlineSiteCount <= maxInlineSiteCount)
                    result.add(pc);
                pendingScope = false;
            }
            pc += header->recordWordCount;
        }
        // The body range ends where mandatory cleanup begins. A final scope setter applies
        // there too, even when no ordinary instruction remains in the body.
        if (pendingScope && inlineSiteCount <= maxInlineSiteCount)
            result.add(pc);
        break;
    }
    return result;
}

/// Chooses the next execution action while stopped, arming temporary bits for next and finish.
StepAction DebugSession::prepareStep(DebugInstructionAddress address)
{
    clearTemporaryBreakpoints();
    switch (m_command)
    {
    case DebugCommand::Stop:
        m_terminated = true;
        return StepAction::Stop;
    case DebugCommand::Continue:
        return StepAction::Continue;
    case DebugCommand::Step:
    case DebugCommand::StepInstruction:
        return StepAction::Step;
    case DebugCommand::NextInstruction:
        {
            const auto* header = m_interpreter->getRecord(address);
            if (!(header->opcodeAndFlags & StagedRecord::kInternalOpcodeBit) &&
                (header->opcodeAndFlags & StagedRecord::kOpcodeMask) == uint32_t(HGBCOpcode::Call))
            {
                addTemporaryBreakpoint(address.wordOffset + header->recordWordCount);
                return StepAction::Continue;
            }
            return StepAction::Step;
        }
    case DebugCommand::Finish:
        if (uint32_t count = m_callStack.getLast().inlineSiteCount)
            for (uint32_t pc : findInlineContinuations(address.wordOffset, count - 1))
                addTemporaryBreakpoint(pc);
        addCallerContinuation();
        return StepAction::Continue;
    case DebugCommand::Next:
        {
            // Ancestor frames can be reached without a source location. Replan there using the
            // original logical-depth ceiling, which includes the saved callers' inline depths.
            Index baseDepth = m_logicalDepth - m_callStack.getLast().inlineSiteCount;
            SLANG_RELEASE_ASSERT(baseDepth <= m_startDepth);
            uint32_t maxInlineSiteCount = uint32_t(m_startDepth - baseDepth);
            const auto& functions = m_map->getFunctions();
            // Every branch destination is a block start. Keeping those candidates, even on the
            // same source line, lets the PC comparison in onStep detect loop backedges.
            for (const auto& stop : m_stopLocations)
            {
                const auto& function = functions[stop.function];
                if (function.entry <= address.wordOffset && address.wordOffset < function.end &&
                    stop.inlineSiteCount <= maxInlineSiteCount)
                    addTemporaryBreakpoint(stop.address);
            }
            for (uint32_t pc : findInlineContinuations(address.wordOffset, maxInlineSiteCount))
                addTemporaryBreakpoint(pc);
            addCallerContinuation();
            return StepAction::Continue;
        }
    }
    SLANG_UNREACHABLE("Unhandled debugger command");
}

void DebugSession::rememberSourceRange(uint32_t address)
{
    m_sourceRange = m_map->findSourceRange(address);
    const auto& ranges = m_map->getSourceRanges();
    m_rangeStart = m_sourceRange >= 0 ? ranges[m_sourceRange].startWordOffset : 0;
    m_rangeEnd = m_sourceRange + 1 < ranges.getCount() ? ranges[m_sourceRange + 1].startWordOffset
                                                       : UINT32_MAX;
}

uint32_t DebugSession::findHitBreakpoint(uint32_t address) const
{
    for (const auto& breakpoint : m_breakpoints)
        if (breakpoint.enabled)
            for (uint32_t location : breakpoint.addresses)
                if (location == address)
                    return breakpoint.id;
    return 0;
}

StepAction DebugSession::onStep(DebugInstructionAddress address, bool breakpointHit)
{
    uint64_t callId = m_callStack.getCount() ? m_callStack.getLast().interpreterCall.value : 0;
    bool temporaryHit = breakpointHit && m_temporaryBreakpoints.contains(address.wordOffset);
    bool userHit =
        breakpointHit && (!temporaryHit || m_armedAddresses.contains(address.wordOffset));
    // Record bits are shared by every recursive invocation. Ignore wrong-depth temporary hits
    // without changing previous-PC tracking or replacing the original stepping plan. A user
    // breakpoint at the same address always takes precedence over the depth condition.
    if (temporaryHit && !userHit)
    {
        bool wrongCall = m_callStack.getCount() > m_startCallDepth ||
                         (m_callStack.getCount() == m_startCallDepth && callId != m_startCallId);
        bool wrongDepth = m_command == DebugCommand::Finish ? m_logicalDepth >= m_startDepth
                          : m_command == DebugCommand::Next ? m_logicalDepth > m_startDepth
                                                            : false;
        if (wrongCall || wrongDepth)
            return StepAction::Continue;
    }
    bool backwards = callId == m_previousCallId && address.wordOffset <= m_previousAddress;
    m_previousCallId = callId;
    m_previousAddress = address.wordOffset;
    bool entry = m_entryPending;
    m_entryPending = false;
    // The usual callback does only integer comparisons. Source decoding is
    // needed only when stepping reaches a candidate source boundary or an actual breakpoint.
    bool stop = userHit || (entry && m_stopAtEntry);
    if (entry && !stop)
        return StepAction::Continue;
    if (!stop)
    {
        switch (m_command)
        {
        case DebugCommand::Continue:
            return StepAction::Continue;
        case DebugCommand::StepInstruction:
            stop = true;
            break;
        case DebugCommand::NextInstruction:
            stop = m_callStack.getCount() <= m_startCallDepth;
            break;
        case DebugCommand::Finish:
            stop = m_logicalDepth < m_startDepth;
            break;
        case DebugCommand::Next:
            if (m_logicalDepth > m_startDepth)
                return StepAction::Continue;
            [[fallthrough]];
        case DebugCommand::Step:
            if (address.wordOffset < m_rangeStart || address.wordOffset >= m_rangeEnd)
                rememberSourceRange(address.wordOffset);
            if (m_sourceRange >= 0 && m_sourceKeys[m_sourceRange])
                stop = m_logicalDepth != m_startDepth || backwards ||
                       m_sourceKeys[m_sourceRange] != m_startSourceKey;
            break;
        case DebugCommand::Stop:
            return StepAction::Stop;
        }
    }
    if (!stop)
    {
        if (m_command == DebugCommand::Next)
        {
            // Same-line hits keep the existing candidates. Only returning to an ancestor with
            // no source location requires a new physical function's stepping plan.
            return m_callStack.getCount() == m_startCallDepth ? StepAction::Continue
                                                              : prepareStep(address);
        }
        return StepAction::Step;
    }
    // The interpreter already tested the break bit. Resolve the user-facing breakpoint number
    // only when reporting a hit, keeping breakpoint lookup out of ordinary instruction stepping.
    m_hitBreakpoint = userHit ? findHitBreakpoint(address.wordOffset) : 0;
    return stopAt(
        address,
        userHit ? DebugStopReason::Breakpoint
        : entry ? DebugStopReason::Entry
                : DebugStopReason::Step);
}

StepAction DebugSession::stopAt(DebugInstructionAddress address, DebugStopReason reason)
{
    clearTemporaryBreakpoints();
    m_entryPending = false;
    m_stopReason = reason;
    m_address = address;
    m_stopped = true;
    m_command = m_listener->onStopped(*this);
    m_stopped = false;
    clearValues();
    m_startDepth = m_logicalDepth;
    m_startCallDepth = m_callStack.getCount();
    m_startCallId = m_callStack.getCount() ? m_callStack.getLast().interpreterCall.value : 0;
    m_previousCallId = m_startCallId;
    m_previousAddress = address.wordOffset;
    rememberSourceRange(address.wordOffset);
    m_startSourceKey = m_sourceRange >= 0 ? m_sourceKeys[m_sourceRange] : 0;
    return prepareStep(address);
}

StepAction DebugSession::onControl(
    DebugInstructionAddress address,
    StepAction action,
    bool breakpointHit)
{
    m_address = address;
    m_stopped = true;
    auto control = m_listener->onControl(*this);
    m_stopped = false;
    clearValues();
    if (control == DebugControlAction::Stop)
    {
        m_terminated = true;
        return StepAction::Stop;
    }
    if (control == DebugControlAction::Pause)
    {
        m_hitBreakpoint = 0;
        return stopAt(address, DebugStopReason::Pause);
    }
    // A request may have changed the current record's breakpoint. Consult the sets only here,
    // after servicing control, and preserve the previous stepping plan for a transparent update.
    breakpointHit = m_armedAddresses.contains(address.wordOffset) ||
                    m_temporaryBreakpoints.contains(address.wordOffset);
    return action == StepAction::Step || breakpointHit ? onStep(address, breakpointHit) : action;
}

List<DebugBreakpoint> DebugSession::replaceSourceBreakpoints(
    const String& file,
    const List<uint32_t>& lines)
{
    String path = normalizeDebugPath(file);
    List<DebugBreakpoint> previous, retained;
    for (const auto& breakpoint : m_breakpoints)
        if (!breakpoint.function.getLength() && normalizeDebugPath(breakpoint.file) == path)
            previous.add(breakpoint);
        else
            retained.add(breakpoint);
    Index first = retained.getCount();
    for (uint32_t line : lines)
    {
        DebugBreakpoint breakpoint;
        breakpoint.file = file;
        breakpoint.line = line;
        for (const auto& old : previous)
            if (old.line == line)
            {
                breakpoint.id = old.id;
                break;
            }
        if (!breakpoint.id)
            breakpoint.id = m_nextBreakpoint++;
        retained.add(breakpoint);
    }
    m_breakpoints = _Move(retained);
    resolveBreakpoints();
    List<DebugBreakpoint> result;
    for (Index i = first; i < m_breakpoints.getCount(); ++i)
        result.add(m_breakpoints[i]);
    return result;
}

List<DebugBreakpoint> DebugSession::replaceFunctionBreakpoints(const List<String>& names)
{
    List<DebugBreakpoint> previous, retained;
    for (const auto& breakpoint : m_breakpoints)
        (breakpoint.function.getLength() ? previous : retained).add(breakpoint);
    Index first = retained.getCount();
    for (const auto& name : names)
    {
        DebugBreakpoint breakpoint;
        breakpoint.function = name;
        for (const auto& old : previous)
            if (old.function == name)
            {
                breakpoint.id = old.id;
                break;
            }
        if (!breakpoint.id)
            breakpoint.id = m_nextBreakpoint++;
        retained.add(breakpoint);
    }
    m_breakpoints = _Move(retained);
    resolveBreakpoints();
    List<DebugBreakpoint> result;
    for (Index i = first; i < m_breakpoints.getCount(); ++i)
        result.add(m_breakpoints[i]);
    return result;
}

List<DebugStackFrame> DebugSession::getStackTrace() const
{
    List<DebugStackFrame> result;
    for (Index i = m_callStack.getCount(); i-- > 0;)
    {
        uint32_t pc =
            i + 1 == m_callStack.getCount() ? m_address.wordOffset : m_callStack[i + 1].callSite;
        const auto& scope = m_map->getScope(m_callStack[i].scopeIndex);
        SourcePosition position;
        if (auto location = m_map->findSourceLocation(pc))
            position = *location;
        if (scope.functionName.getLength())
            position.functionName = scope.functionName;
        if (!position.functionName.getLength())
            for (const auto& function : m_map->getFunctions())
                if (function.entry <= pc && pc < function.end)
                    position.functionName = function.name;
        // For inner -> outer -> main within one physical call, inner and outer are inlined.
        // The last reconstructed entry represents the physical call and stays untagged.
        Index inlineCount = scope.inlineSites.getCount();
        result.add({position, inlineCount != 0});
        for (Index j = 0; j < inlineCount; ++j)
            result.add({scope.inlineSites[j], j + 1 < inlineCount});
    }
    return result;
}
} // namespace Slang::HGVM
