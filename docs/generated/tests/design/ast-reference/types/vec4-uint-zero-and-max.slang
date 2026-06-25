//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<uint, 4>: lanes hold the full 32-bit unsigned range [0, 0xFFFFFFFF]; both edges survive per-lane.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: uint4 covers (uint, 4). The documented 32-bit
// unsigned range bookends both fit in the lanes.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint4 v = uint4(0u, 1u, 0x80000000u, 0xFFFFFFFFu);
    // CHECK: v=0,1,-2147483648,-1
    // printf %d treats uint as signed, so MAX shows as -1.
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
