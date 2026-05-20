//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=88bd1a1de6592fbd4873bdcc84f730186523fdd89ee42e0e9473c34ee75a0278
//META: purpose=Per-entry-point step 8 attaches an IRLayoutDecoration to the stub IRFunc; the merged entry-point func carries a [layout(%N)] decoration that the reflection API queries.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Row 8 of the per-entry-point steps table:
//   builder->addLayoutDecoration(irFunc, irEntryPointLayout)
// is what makes the entry point's layout queryable. After the
// layout module merges into the post-link IR, `func %main` carries
// a `[layout(%N)]` decoration whose operand resolves to an
// EntryPointLayout(...) instruction.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: func %main
