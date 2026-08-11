//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
//META: purpose=Boundary: `int8_t` is one of the narrow integer types supplied by `core.meta.slang`; the value -128 is its documented MIN and selects the `int8_t` overload via overload resolution.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower edge of the narrow signed integer type contributed
// by the core meta-module. If `int8_t` were not in the environment,
// the overload would not compile.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int8_t v) { return 1; }
int kind(int v)    { return 2; }

void main()
{
    int8_t x = -128;
    // CHECK: k=1 v=-128
    printf("k=%d v=%d\n", kind(x), int(x));
}
