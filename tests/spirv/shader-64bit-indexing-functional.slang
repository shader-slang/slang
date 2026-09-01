//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvShader64BitIndexingEXT

// End-to-end functional test for 64-bit indexing (issue #11967, follow-up to #11538/PR #11541).
//
// The plumbing tests (shader-64bit-indexing{,-require,-callgraph,-multi-entry,-negative}.slang)
// only verify that requiring the capability -- via the `[Shader64BitIndexing]` attribute or the
// `[require(spvShader64BitIndexingEXT)]` path -- unions the `spvShader64BitIndexingEXT` capability /
// extension / execution mode into the entry point. This test verifies end-to-end that an actual
// 64-bit index value flows through to the buffer access without being truncated to 32 bits: a
// `uint64_t` index whose value is >= 2^32 indexes a `RWStructuredBuffer` directly, and the emitted
// SPIR-V access chain must consume that 64-bit index verbatim (no `OpUConvert`/`OpSConvert`
// narrowing it to 32 bits first).
//
// A true >2^32-element *execution* test is impractical (it needs an >8 GB device buffer), so this
// is a codegen guard on the direct buffer subscript, which preserves the 64-bit index to the
// access chain because `RWStructuredBuffer`'s subscript is already generic over the index type
// (`__generic<TIndex : __BuiltinIntegerType> __subscript(TIndex index)` in `hlsl.meta.slang`).
//
// KNOWN LIMITATION (tracked in issue #11990): indexing through the `IArray<T>` / `IRWArray<T>`
// interface constraint currently truncates a 64-bit index to 32 bits, because those interface
// subscript *requirements* are declared `__subscript(int index)` (`core.meta.slang`). Fixing that
// requires type-checker changes (witness synthesis for a generic subscript requirement on the
// built-in `Array`/`vector`/`matrix` magic types, and preserving an integer-constrained generic
// index in `getMatchingIntType`); it is deferred to #11990 as a maintainer design decision. This
// test therefore exercises only the direct buffer path, which is the real-world 64-bit-indexing
// case.

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // 0x100000000 = 2^32: a value that requires more than 32 bits, so a 32-bit
    // truncation of the index would drop the high bit and corrupt the access.
    uint64_t index = (uint64_t)dispatchThreadID.x + 0x100000000ull;
    outputBuffer[index] = 1;
}

// The capability opt-in from `[Shader64BitIndexing]` must still be emitted. These appear near the
// top of the module (before the function body), so they are checked before the body CHECKs below.
// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %computeMain Shader64BitIndexingEXT

// A 64-bit unsigned integer type is emitted, and the >= 2^32 offset constant uses it. Capture the
// constant's id so the index computation below can be tied to this specific >= 2^32 value.
// CHECK-DAG: %[[ULONG:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// CHECK-DAG: %[[OFFSET:[A-Za-z0-9_]+]] = OpConstant %[[ULONG]] 4294967296

// The index is computed at 64-bit width and reaches the access chain verbatim: capture the
// 64-bit index id at its definition, require it to be the OpIAdd of the >= 2^32 offset constant
// (so the test pins the >= 2^32 value specifically, not just any 64-bit add), then require the
// SAME id to be the access-chain index operand. If the index were truncated to 32 bits, an
// OpUConvert/OpSConvert would sit in between and the access-chain index would be a different
// (32-bit) id, so this pairing would not match.
// CHECK: %[[INDEX:[A-Za-z0-9_]+]] = OpIAdd %[[ULONG]] {{.*}}%[[OFFSET]]{{.*}}
// CHECK: OpAccessChain {{.*}} %[[INDEX]]
