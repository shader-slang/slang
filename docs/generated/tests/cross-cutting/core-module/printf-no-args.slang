//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the core/HLSL meta-module `printf` accepts a format string with zero format specifiers (no variadic args).
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: 0 variadic args — printf with a literal-only format
// string. Variadic arg count 0 is the lower edge.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    //CHECK: just-a-literal
    printf("just-a-literal\n");
}
