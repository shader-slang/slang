//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#pre-link-mandatory-passes
//META: doc_section_digest=5bba61a76153063b8f5fcca3dfc0394ae8fb3e1c8ed03e1852e561d6fe1ee9b3
//META: purpose=The pre-link pipeline is target-agnostic, so the same source produces the same LOWER-TO-IR shape on every text-emit target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the page's framing claim that the same mandatory passes run for
// every shader target. One source file is compiled with seven different
// `-target` values, and all seven runs share a single CHECK prefix, so the
// identical set of assertions has to hold in each dump. The assertions pick
// shapes each phase is responsible for -- the SSA-promoted local from C1,
// the surviving ordinary call, and the entry-point decorations from A6 --
// which is what makes an agreement across targets evidence that the same
// passes ran rather than a coincidence of trivial output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target glsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cpp -dump-ir -o /dev/null -entry main -stage compute

RWStructuredBuffer<int> outBuf;
uniform int a;

int helper(int x)
{
    int t = x + 1;
    return t * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = helper(a);
}

// CHECK: ### LOWER-TO-IR:
// CHECK-DAG: func %helper
// CHECK-DAG: add(%{{[A-Za-z0-9_]+}}, 1 : Int)
// CHECK-DAG: mul(%{{[A-Za-z0-9_]+}}, 2 : Int)
// CHECK-DAG: {{\[}}numThreads(1 : Int, 1 : Int, 1 : Int){{\]}}
// CHECK-DAG: call %helper(%a)
