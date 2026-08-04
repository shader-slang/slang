//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=Function parameters and non-static local variables belong to the Function address space; SPIRV emits mutable locals as `OpVariable` with `Function` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that non-static local variables land in the Function address space.
// A `Counter` struct local that gets written through `inout` cannot be SSA-folded away;
// CHECK asserts the SPIRV output contains an OpVariable with `Function` storage class,
// confirming the per-thread-invocation scope mapping.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

struct Counter { int val; };

// Non-inlined to preserve the local variable in SPIRV output.
[__NoInline]
int countUp(inout Counter c, int n)
{
    for (int i = 0; i < n; i++)
        c.val++;
    return c.val;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<int> output)
{
    Counter c;
    c.val = 0;
    output[0] = countUp(c, int(tid.x) + 1);
}

// The Counter struct local variable maps to OpVariable ... Function inside the entry point.
// CHECK: OpVariable {{.*}} Function
// Metal maps function-local structs to thread address space variables.
// METAL: thread Counter_
// WGSL passes function-local structs as ptr<function,...> arguments.
// WGSL: ptr<function, Counter_
