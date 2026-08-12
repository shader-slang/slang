//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a by-value struct returned from a helper and then read field-by-field round-trips its fields correctly under the interpreter.
//META: intent=characterization
//META: covers=source/slang/slang-emit-c-like.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins struct-by-value return emission end-to-end. makePair returns a
// Pair {a, b}; the caller reads both fields and sums them. This drives
// the struct-return / FieldExtract emit path (slang-emit-c-like.cpp:2486)
// and, on slangi, the VM's struct value handling. With input 7 the result
// is 7 + 14 = 21. INTERPRET validates the value locally (no GPU / FileCheck
// needed), so this is the value-level companion to the emit-shape tests.
//TEST:INTERPRET(filecheck=CHECK):

struct Pair { int a; int b; };
Pair makePair(int x) { Pair p; p.a = x; p.b = x * 2; return p; }

void main()
{
    Pair p = makePair(7);
    //CHECK: sum=21
    printf("sum=%d\n", p.a + p.b);
}
