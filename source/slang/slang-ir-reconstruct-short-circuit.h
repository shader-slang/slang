// slang-ir-reconstruct-short-circuit.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetRequest;

// Rebuild `&&`/`||` expressions from their short-circuit diamond form before
// phi elimination, for targets whose source language short-circuits natively.
void reconstructShortCircuit(IRModule* inModule, TargetRequest* req);
} // namespace Slang
