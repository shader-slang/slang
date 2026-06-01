//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E38205 (recursive-types-found-in-structured-buffer) - recursive type in structured buffer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38205
//META: catalog_name=recursive-types-found-in-structured-buffer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct that contains a pointer to itself, used as element
// type of a StructuredBuffer, triggers E38205.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_6_0

struct Node
{
    Node* next;
    int v;
}

StructuredBuffer<Node> sb;

[numthreads(1,1,1)]
void main() {}

// CHECK: E38205
