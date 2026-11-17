//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:26+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/values.md#defaultconstruct
//META: doc_section_digest=70fa32ba53aa5066ea3a76e6f36848a1e76640420b67965c0cf7cf95009cbcf5
//META: purpose=Default-initialising a tuple or a fixed-size array does not emit `defaultConstruct`; `emitDefaultConstruct` decomposes it into `makeTuple` / an array maker built from zero literals.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the aggregate arms of `emitDefaultConstruct` that the
// refreshed doc lists: `makeTuple` for a tuple and `makeArray` for an
// array, so the nullary `defaultConstruct` never survives for them.
// The array has identical zero elements, which the builder collapses to
// the `makeArrayFromElement` form of the array maker (the sibling
// `aggregate-makearrayfromelement.slang` covers that opcode itself).
// Both values are passed to a `[noinline]` callee so they stay live.
// CHECK asserts the tuple is `makeTuple(0 : Int, 0 : Float)`, the array
// is built from a `0 : Int` literal, and no `defaultConstruct` appears
// in `main`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[noinline]
int pick(int a[3], Tuple<int, float> t, int i) { return a[i] + t._0; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[3] = {};
    Tuple<int, float> t = {};
    buf[tid.x] = pick(a, t, buf[1]);
}

// IR-LABEL: func %main
// IR-NOT: defaultConstruct
// IR: : Array(Int, 3 : Int){{[[:space:]]}}= makeArray{{(FromElement)?}}(0 : Int{{.*}})
// IR: : tuple_type(Int, Float){{[[:space:]]}}= makeTuple(0 : Int, 0 : Float)
// IR-NOT: defaultConstruct
