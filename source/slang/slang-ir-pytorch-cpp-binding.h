#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

// Diagnose every bodyless '[CudaKernel]' function (a forward declaration) with error E55104. This
// runs before the CUDA/PyTorch binding passes below, which read a kernel's parameters from its
// first block; a forward declaration has no block, so those passes would otherwise dereference a
// null first block and crash. For example, `[CudaKernel] void kernel();` is reported here rather
// than crashing a later pass.
void diagnoseBodylessKernelEntryPoints(IRModule* module, DiagnosticSink* sink);
void generatePyTorchCppBinding(IRModule* module, DiagnosticSink* sink);
void generateHostFunctionsForAutoBindCuda(IRModule* module, DiagnosticSink* sink);
void removeTorchKernels(IRModule* module);
void handleAutoBindNames(IRModule* module);
void generateDerivativeWrappers(IRModule* module, DiagnosticSink* sink);
void lowerBuiltinTypesForKernelEntryPoints(IRModule* module, DiagnosticSink* sink);
void removeTorchAndCUDAEntryPoints(IRModule* module);

} // namespace Slang
