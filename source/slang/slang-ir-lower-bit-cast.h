// slang-ir-lower-bit-cast.h
#pragma once

// This file defines an IR pass that lowers a BitCast<T>(U) operation, where T and U are struct
// types, into a series of bit-cast operations on basic-typed elements. It also defines a
// diagnostic pass that rejects user casts into a `DescriptorHandle<T>`; it runs earlier in the
// pipeline, so `lowerBitCast` only ever sees compiler-generated casts into a handle.

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

void lowerBitCast(IRModule* module, TargetProgram* targetReq, DiagnosticSink* sink);

/// Report an error for every `BitCast` whose destination type is or contains a
/// `DescriptorHandle<T>`, unless the operand has the same type, or the target represents a handle
/// as its native resource and the operand is that bare resource.
///
/// A handle is a `uint2` heap index on D3D, Vulkan and WebGPU, a `uint64_t` for texture and
/// sampler kinds under `spvBindlessTextureNV`, and the native resource on CUDA, CPU and Metal, so
/// a bit pattern produced from other data means different things on different targets. We
/// therefore treat a `bit_cast` into a handle as ill-formed and leave construction to the
/// `DescriptorHandle` constructors; a `bit_cast` out of a handle stays an ordinary same-size
/// reinterpretation. The check must run before AnyValue marshalling and byte-address-buffer
/// legalization, because both build handles with their own `BitCast` and nothing in the IR tells
/// those casts apart from the user's.
void diagnoseBitCastsToDescriptorHandle(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);

} // namespace Slang
