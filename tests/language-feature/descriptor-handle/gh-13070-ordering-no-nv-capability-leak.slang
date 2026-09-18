// `DescriptorHandle.lessThan` / `lessThanOrEquals` choose the native `uint64` vs `uint2` comparison by
// calling `usesNativeSpirvDescriptorHandle<T>()` — the query that owns the `case spvBindlessTextureNV:`
// arm. Front-end capability inference unions a `__target_switch`'s arms per target family before IR-time
// specialization, so a lone `case spvBindlessTextureNV:` arm would pin a switch's SPIR-V requirement to
// `{spirv, spvBindlessTextureNV}`; the query avoids that because its own `case spirv:`/`default` sibling
// collapses its inferred capability back to `{spirv}`, so callers do not inherit `spvBindlessTextureNV`.
// If that sibling regressed — or if an operator reintroduced a bare `case spvBindlessTextureNV:` arm — a
// plain-SPIR-V entry point that instantiates the operators for an *encodable* type (e.g. `Texture2D`)
// would be force-upgraded. This test compiles exactly that on plain SPIR-V (no spvBindlessTextureNV)
// with `-warnings-as-errors all`, so the E41012 profile-upgrade warning becomes a hard error if the
// capability scoping regresses. See shader-slang/slang#13070.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -warnings-as-errors all

[[vk::binding(0)]] RWStructuredBuffer<uint> outputBuffer;

// Encodable texture handles: comparing them instantiates `usesNativeSpirvDescriptorHandle<T>()` (via
// lessThan/lessThanOrEquals), whose capability scoping this test exercises.
uniform Texture2D<float>.Handle a;
uniform Texture2D<float>.Handle b;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // A successful compile (no implicit spvBindlessTextureNV upgrade) is the assertion.
    //CHECK: OpEntryPoint
    outputBuffer[0] = (a < b) ? 1u : 0u;
    outputBuffer[1] = (a <= b) ? 1u : 0u;
}
