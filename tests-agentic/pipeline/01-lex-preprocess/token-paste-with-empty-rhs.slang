//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=1f95bdbd2dae47141e9ce007dbaf0bb66550c8aacddbc7708ffa3f8f5b7ad00d
//META: purpose=Boundary probe of `##`: pasting an identifier with the empty token sequence yields just that identifier.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `JOIN(my_value, )` — the right operand of `##` is the empty token
// sequence. The paste must collapse to just the left operand, so
// the resulting identifier is `my_value`. If the paste produced an
// invalid token (or an extra trailing token), the lookup `my_value`
// would not resolve to the static constant.

//TEST:INTERPRET(filecheck=CHECK):

#define JOIN_(a, b) a##b
#define JOIN(a, b) JOIN_(a, b)

static const int my_value = 88;

void main()
{
    // CHECK: pasted=88
    printf("pasted=%d\n", JOIN(my_value,));
}
