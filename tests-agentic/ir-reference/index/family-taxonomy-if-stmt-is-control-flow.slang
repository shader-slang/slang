//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#family-taxonomy
//META: doc_section_digest=a53f1a310fda9b5efc9c45940da27ed7132b7f00cc82073c20d9fb7f689da0bc
//META: purpose=Per the family taxonomy, branches and block terminators belong to the Control-flow family; an if statement lowers to an ifElse/conditional terminator with multiple blocks.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc routes block/parameters/branches/function exits
// to control-flow.md. A natural `if` should therefore produce
// multiple basic blocks and a conditional-branch terminator in
// the IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r;
    if (a > 0)
        r = a;
    else
        r = -a;
    buf[0] = r;
}

// CHECK: block %{{[A-Za-z_0-9]+}}
// CHECK: ifElse
