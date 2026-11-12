//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#diagnostics-during-lowering
//META: doc_section_digest=8da9811f8488ad6ce5409491fafa35c23411a9a65708775cf2a3ad6c64768b12
//META: purpose=A `switch` body that discards nothing (`{ }` or `{ ; }`) takes the no-cases early return silently: no unreachable-code warning is reported.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary for the no-case-label switch report: bodies that discard no
// statement must not warn. The CHECK-NOT proves E41000 never appears while the
// positive CHECK proves lowering did complete and produced `func %main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    switch (a) { }
    switch (a) { ; }
    buf[0] = a;
}

// CHECK-NOT: E41000
// CHECK: func %main
