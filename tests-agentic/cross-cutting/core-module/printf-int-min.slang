//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the core/HLSL meta-module `printf` accepts int32 MIN and formats it correctly under the interpreter.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: int MIN as a printf %d argument.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    //CHECK: min=-2147483648
    printf("min=%d\n", -2147483648);
}
