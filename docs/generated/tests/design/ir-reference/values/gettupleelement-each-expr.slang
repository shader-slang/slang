//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#reshape-and-pack-helpers
//META: doc_section_digest=e496f872e856a7097c0418a390ebd876b5c205231e5aaf9b8d85f80f7da0bd56
//META: purpose=An `each` expression over a `Tuple<expand each T>` parameter is the surface that emits `getTupleElement(tuple, index)`; positional member access is not, because it rewrites to `swizzle`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one lowering site that produces `getTupleElement`. A
// variadic function whose parameter is a `Tuple<expand each T>` and
// whose body iterates it with `expand ... each t` yields an `Expand`
// region whose block parameter is the pack index; inside that region
// the element read is a `getTupleElement` taking the loaded tuple and
// that index. CHECK anchors at `func %dump` and asserts the index-typed
// operand shape, which is what separates this opcode from the `swizzle`
// that `t._0` would have produced.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[noinline]
void dump<each T : IInteger>(Tuple<expand each T> t)
{
    int i = 0;
    expand buf[i++] = (each t).toInt();
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    dump(makeTuple(int(tid.x), int(tid.y)));
}

// IR-LABEL: func %dump
// IR: : tuple_type(%{{[A-Za-z0-9_]+}})	= load(%{{[A-Za-z0-9_]+}})
// IR: getTupleElement(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
