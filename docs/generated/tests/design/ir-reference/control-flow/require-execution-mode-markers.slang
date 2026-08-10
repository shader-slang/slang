//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#other-control-flow-opcodes
//META: doc_section_digest=f5793a717c7bbfcde423407642d2b04f1b04ae204499660ecf91fed86254707e
//META: purpose=The three operand-less execution-mode markers RequireComputeDerivative, RequireMaximallyReconverges, and RequireQuadDerivatives each lower from their own core-module intrinsic call.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the three no-operand rows of the backend-hint group together,
// since the surface for all of them is the same shape -- an argument-less
// call to a core-module intrinsic -- and a single entry point can carry all
// three. The CHECKs assert each marker appears as its own instruction line
// inside the entry point's block, in call order, which is what shows the
// front end mapped each intrinsic to a distinct opcode rather than folding
// them into a shared decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(2, 2, 1)]
void main()
{
    __requireComputeDerivative();
    __requireMaximallyReconverges();
    __requireQuadDerivatives();
    buf[0] = n;
}

// CHECK-LABEL: func %main
// CHECK: RequireComputeDerivative
// CHECK-NEXT: RequireMaximallyReconverges
// CHECK-NEXT: RequireQuadDerivatives
