//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#memory
//META: doc_section_digest=dc66ba2c5bed92e16925296728625769d680792be147891410ddc77e46d8052c
//META: purpose=A module-scope `static const T K = value;` declaration lowers to `globalConstant(<value>)` in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Memory table lists `globalConstant` for "Module-scope
// constant" with AST origin "`VarDecl` with `const`/`static const`
// at module scope". A constant `int` and a constant `float` show
// up as `globalConstant(<literal>)` lines in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static const int K = 42;
static const float PI = 3.14159;

uniform int a;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + K;
    fbuf[0] = PI;
}

// IR: let {{.*}}%K{{.*}}: Int{{.*}}= globalConstant(42 : Int)
// IR: let {{.*}}%PI{{.*}}: Float{{.*}}= globalConstant(3.14159 : Float)
