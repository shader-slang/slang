//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=9e789f76018ea954bfb264040a694991a83baab0f112111a9b519188a6174ec6
//META: purpose=Each core-module scalar alias names the same builtin type it aliases: int32_t is int, uint32_t is uint, float32_t is float, float64_t is double and float16_t is half.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's scalar alias block introduces aliases,
// not distinct types. An overload set keyed on the builtin spellings
// (int/uint/float/double/half) is called with values declared through
// the alias spellings; overload resolution can only pick the exact
// match if the alias and the builtin denote the same type, so the
// CHECKed selector digits pin alias identity rather than convertibility.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int) { return 1; }
int probe(uint) { return 2; }
int probe(float) { return 3; }
int probe(double) { return 4; }
int probe(half) { return 5; }

void main()
{
    int32_t a = 1;
    uint32_t b = 1;
    float32_t c = 1;
    float64_t d = 1;
    float16_t h = 1;
    // CHECK: i32=1 u32=2 f32=3 f64=4 f16=5
    printf("i32=%d u32=%d f32=%d f64=%d f16=%d\n",
           probe(a), probe(b), probe(c), probe(d), probe(h));
}
