// slang-ir-reconstruct-short-circuit.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetRequest;

// Rebuild `&&`/`||` expressions from their short-circuit diamond form before
// phi elimination, on the targets whose emitters the pass's fold-veto
// mirrors cover (CUDA and CPU/C++ source emission).
void reconstructShortCircuit(IRModule* inModule, TargetRequest* req);
} // namespace Slang
