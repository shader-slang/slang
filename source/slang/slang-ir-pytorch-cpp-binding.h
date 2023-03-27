#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

void generatePyTorchCppBinding(IRModule* module, DiagnosticSink* sink);
void removeTorchKernels(IRModule* module);

}

