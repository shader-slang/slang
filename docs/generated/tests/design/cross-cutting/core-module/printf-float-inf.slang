//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Boundary: the core/HLSL meta-module `printf` accepts a +inf float argument and formats it under the interpreter.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: +infinity as a printf %f argument.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float inf = 1.0 / 0.0;
    //CHECK: val=inf
    printf("val=%f\n", inf);
}
