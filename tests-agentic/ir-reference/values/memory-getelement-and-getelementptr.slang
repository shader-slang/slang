//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#getelementptr-vs-getelement
//META: doc_section_digest=fe2ba8ba0ec9c96c4e3299305eb8c8e97cd80661da22fe64d61863c713e5b8ba
//META: purpose=Array indexing in lvalue position lowers to `getElementPtr`; indexing into a struct-field array value lowers to `getElement`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `GetElementPtr` vs `GetElement` notable-opcode note
// distinguishes the address-returning (`getElementPtr`) and
// value-returning (`getElement`) forms. A local array indexed by a
// dynamic index produces both opcodes: the writes use `getElementPtr`
// + `store`, while indexing the struct-field array value uses
// `getElement` directly.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct S
{
    int arr[4];
}

uniform S s;
uniform int a;
uniform int idx;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int local[4];
    local[0] = a;
    local[1] = a + 1;
    local[2] = a + 2;
    local[3] = a + 3;
    buf[0] = local[idx];
    buf[1] = s.arr[idx];
}

// IR-LABEL: func %main
// IR: getElementPtr(%local, 0 : Int)
// IR: getElementPtr(%local, %idx)
// IR: getElement(%{{.*}}, %idx)
