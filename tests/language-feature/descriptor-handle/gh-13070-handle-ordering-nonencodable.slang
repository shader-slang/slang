// `DescriptorHandle.lessThan`/`lessThanOrEquals` compare in the handle's native representation: a handle
// that is a native `uint64` under `spvBindlessTextureNV` (the texture/sampler types, i.e. those for which
// `isBindlessTextureNVEncodable` is true) compares as a single 64-bit value, while any other handle
// compares as a `uint2` component-wise. The split keys on the type, not on `DescriptorKind`:
// `SubpassInput` (lowers to `IRSubpassInputType`, shares `DescriptorKind.Texture`) and `TextureBuffer`
// (lowers to `IRTextureBufferType`, shares `DescriptorKind.UniformTexelBuffer`) are NOT encodable, so
// their handles are `uint2`-represented even though their kinds look texture/texel-like. See
// shader-slang/slang#13070.
//
// Each operator is exercised on each representation in its own entry point so the check for one cannot
// be satisfied by the other: a `uint2` comparison extracts 32-bit components (`OpCompositeExtract %uint`)
// and uses no 64-bit integer (`%ulong`), whereas a `uint64` comparison loads a `%ulong` and extracts no
// component. Each entry asserts the representation it must use and forbids the other, so regressing
// either operator on either representation fails the test.

//TEST:SIMPLE(filecheck=SUBLT): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry subLt
//TEST:SIMPLE(filecheck=SUBLE): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry subLe
//TEST:SIMPLE(filecheck=TBUFLT): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry texBufLt
//TEST:SIMPLE(filecheck=TBUFLE): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry texBufLe
//TEST:SIMPLE(filecheck=TEXLT): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry texLt
//TEST:SIMPLE(filecheck=TEXLE): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry texLe

[[vk::binding(0)]] RWStructuredBuffer<uint> outputBuffer;

uniform SubpassInput<float4>.Handle subA;
uniform SubpassInput<float4>.Handle subB;
uniform TextureBuffer<float4>.Handle tbufA;
uniform TextureBuffer<float4>.Handle tbufB;
uniform Texture2D<float>.Handle texA;
uniform Texture2D<float>.Handle texB;

// Non-encodable (uint2, component-wise): must extract 32-bit components, must not use a 64-bit integer.
//SUBLT-DAG: OpCompositeExtract %uint
//SUBLT-NOT: %ulong
[shader("compute")][numthreads(1, 1, 1)]
void subLt() { outputBuffer[0] = (subA < subB) ? 1u : 0u; }

//SUBLE-DAG: OpCompositeExtract %uint
//SUBLE-NOT: %ulong
[shader("compute")][numthreads(1, 1, 1)]
void subLe() { outputBuffer[0] = (subA <= subB) ? 1u : 0u; }

// TextureBuffer shares `DescriptorKind.UniformTexelBuffer` but is not encodable, so it too is uint2.
//TBUFLT-DAG: OpCompositeExtract %uint
//TBUFLT-NOT: %ulong
[shader("compute")][numthreads(1, 1, 1)]
void texBufLt() { outputBuffer[0] = (tbufA < tbufB) ? 1u : 0u; }

//TBUFLE-DAG: OpCompositeExtract %uint
//TBUFLE-NOT: %ulong
[shader("compute")][numthreads(1, 1, 1)]
void texBufLe() { outputBuffer[0] = (tbufA <= tbufB) ? 1u : 0u; }

// Encodable (uint64): must load a 64-bit value, must not extract 32-bit components.
//TEXLT-DAG: OpLoad %ulong
//TEXLT-NOT: OpCompositeExtract %uint
[shader("compute")][numthreads(1, 1, 1)]
void texLt() { outputBuffer[0] = (texA < texB) ? 1u : 0u; }

//TEXLE-DAG: OpLoad %ulong
//TEXLE-NOT: OpCompositeExtract %uint
[shader("compute")][numthreads(1, 1, 1)]
void texLe() { outputBuffer[0] = (texA <= texB) ? 1u : 0u; }
