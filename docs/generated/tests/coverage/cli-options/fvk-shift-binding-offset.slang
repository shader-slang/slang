//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -fvk-t-shift <N> <space> adds N to the Vulkan binding inferred from a `register(tX)` semantic and silences the "D3D register without Vulkan binding or shift" warning for that register class, with `all` behaving like the matching explicit space.
//META: intent=characterization
//META: characterization-unverified=true
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::VulkanBindShift` arm of
// `OptionsParser::_parse` in slang-options.cpp (~30 uncovered lines). It reads
// the register class out of the flag spelling itself (`-fvk-t-shift`), then two
// operands: the shift amount and a descriptor space, where the literal `all`
// selects the every-space form.
//
// The bundle README previously listed this option as untargetable because the
// two-operand read appeared to swallow the input path. It does not: slang-test
// puts the source file first on the command line, so the two operands are read
// from the flag's own arguments and the option behaves normally.
//
// `tex` is declared `register(t3)`, so a `-fvk-t-shift 10` moves it to Vulkan
// binding 13; DEFAULT pins binding 3 from the same source. The E39029 warning
// that fires when a `register` has no Vulkan binding or shift also disappears for
// `tex` (but stays for `samp`, which has no `-fvk-s-shift`), which is a second,
// independent handle on the same option.
//
// Tagged characterization-unverified because supplying a t-shift also renumbers
// the *other*, unshifted parameters (`samp` 1 -> 0 and `outBuf` 2 -> 1 in the
// observed output) — auto-allocation is re-run once any shift is present. That
// side effect is pinned nowhere here, only `tex` is, but a reviewer should
// confirm the renumbering is intended before this behaviour is treated as fixed.

//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=SHIFT):-target spirv-asm -entry main -stage compute -fvk-t-shift 10 0
//TEST:SIMPLE(filecheck=SHIFTALL):-target spirv-asm -entry main -stage compute -fvk-t-shift 10 all

Texture2D<float4> tex : register(t3);
SamplerState samp : register(s1);
RWStructuredBuffer<float4> outBuf : register(u2);

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = tex.SampleLevel(samp, float2(0, 0), 0);
}

//DEFAULT: warning[E39029]: D3D register without Vulkan binding or shift
//DEFAULT: shader parameter 'tex' has a 'register' specified for D3D
//DEFAULT: OpDecorate %tex Binding 3

//SHIFT-NOT: shader parameter 'tex' has a 'register' specified for D3D
//SHIFT: shader parameter 'samp' has a 'register' specified for D3D
//SHIFT: OpDecorate %tex Binding 13

//SHIFTALL: OpDecorate %tex Binding 13
