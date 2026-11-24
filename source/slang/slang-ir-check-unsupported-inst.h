#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

/// BACKSTOP: reports host-only HGBC bytecode (`hgbc_asm`) that reached device code generation.
///
/// The capability system should reject these uses before linking, but it currently misses them in
/// two cases, so this check exists only until those are fixed:
/// - a `[require(hgbc)]` static method of a generic type, such as `RWStructuredBuffer<T>::alloc`,
///   does not contribute its requirement to the caller;
/// - a generic entry point does not take on the requirements of the conforming methods it is
///   specialized with, such as a `Grid.dispatch` lambda's `operator()`.
/// Without this check the emitter crashes or reports an internal error. Remove it once the
/// capability system reports these uses itself.
void checkHostOnlyInstsInDeviceCode(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
