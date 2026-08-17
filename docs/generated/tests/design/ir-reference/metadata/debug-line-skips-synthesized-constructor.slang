//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugline
//META: doc_section_digest=eb5cc6b339fed6275ffa177c8c0c47a41583954b25feb65903b85fa94849df4a
//META: purpose=Lowering emits no DebugLine markers inside a Slang-synthesized constructor, so a debugger cannot step into compiler-generated code.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out in the marker-emitting helper. Initializing a struct
// with a brace list makes lowering synthesize a constructor, which is a
// function the user never wrote and has no source range worth stopping on. The
// CHECKs bracket that synthesized function: they enter at its header, forbid
// any marker before the next user-written function begins, and then require
// the user's own entry point to be marked -- so the absence is attributable to
// the constructor being synthesized rather than to debug info being off.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

struct Point
{
    float x;
    float y;
}

uniform RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Point p = { float(tid.x), 2.0 };
    gOut[0] = p.x + p.y;
}

// The synthesized constructor is mangled from the struct name plus `$init`.
// CHECK: {{^}}func %Pointx5Fx24init {{.*}}Func(
// CHECK-NOT: DebugLine(
// CHECK: {{^}}func %main {{.*}}Func(Void,
// CHECK: DebugLine(
