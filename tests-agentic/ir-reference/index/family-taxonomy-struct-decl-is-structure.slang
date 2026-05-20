//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#family-taxonomy
//META: doc_section_digest=a53f1a310fda9b5efc9c45940da27ed7132b7f00cc82073c20d9fb7f689da0bc
//META: purpose=Per the family taxonomy, a struct declaration belongs to the Structure family; the IR dump shows a struct parent opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc routes module-organization opcodes (functions,
// structs, witness tables, globals) to structure.md. A bare
// struct declaration that is reachable from the entry point
// should therefore appear as a `struct` opcode in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

struct Point
{
    int x;
    int y;
};

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Point p;
    p.x = 1;
    p.y = 2;
    buf[0] = p.x + p.y;
}

// CHECK: struct
