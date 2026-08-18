//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#barriers-and-synchronization
//META: doc_section_digest=46f99bd5a35473a9163ccbd42c4c906fa8dad92dac17c85d7c15483893456718
//META: purpose=`beginInvocationInterlock()` and `endInvocationInterlock()` are the Slang spellings that produce the nullary `BeginFragmentShaderInterlock` and `EndFragmentShaderInterlock` opcodes, which appear directly in a `fragment` entry point's `func %main` and reach SPIR-V as `OpBeginInvocationInterlockEXT` / `OpEndInvocationInterlockEXT` and GLSL as `beginInvocationInterlockARB()` / `endInvocationInterlockARB()`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two interlock rows of the barrier table together with the
// user-surface note that names their Slang spellings and the stage they
// require.
//
// The two calls carry no operands, so the only thing a dump can show is
// that the opcode is present and bracketing the guarded store; the IR
// directive anchors at `func %main` so the match cannot come from a
// pulled-in core-module body, and the store between them is what keeps the
// critical section from being emptied. The two emission directives pin the
// user-visible artifact on the targets that admit the capability: SPIR-V
// requests `SPV_EXT_fragment_shader_interlock` and emits the two
// `OpInvocationInterlock` instructions, GLSL requires the ARB extension
// and calls its ARB-suffixed functions.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main

RWTexture2D<float> rov;

float4 main(float4 pos : SV_Position) : SV_Target
{
    beginInvocationInterlock();
    rov[uint2(pos.xy)] = 1.0;
    endInvocationInterlock();
    return float4(1.0);
}

// IR-LABEL: func %main
// IR: BeginFragmentShaderInterlock
// IR: EndFragmentShaderInterlock

// SPV-DAG: OpExtension "SPV_EXT_fragment_shader_interlock"
// SPV-DAG: OpBeginInvocationInterlockEXT
// SPV-DAG: OpEndInvocationInterlockEXT

// GLSL-DAG: GL_ARB_fragment_shader_interlock
// GLSL-DAG: beginInvocationInterlockARB()
// GLSL-DAG: endInvocationInterlockARB()
