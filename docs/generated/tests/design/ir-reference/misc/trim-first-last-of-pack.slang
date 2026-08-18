//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#pack-and-expansion
//META: doc_section_digest=23248f45180c2ed6ca7d90a9cde90609e58c45a8a68d6fc6c5245f5bc4c2e1df
//META: purpose=`__trimFirst(pack)` and `__trimLast(pack)` lower to `TrimFirstOfPack` / `TrimLastOfPack` insts whose result is again a pack value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the trim helpers produce a narrower pack rather than a scalar:
// the CHECKs require each trim inst's result type to be a `ValuePackType`,
// which distinguishes them from the `__first` / `__last` extracts that yield a
// single slot. `countof` around the trim keeps the result live so the inst
// survives to the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int trimmedFirstCount<let each D : int>()
{
    return countof(__trimFirst(D));
}

int trimmedLastCount<let each D : int>()
{
    return countof(__trimLast(D));
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = trimmedFirstCount<10, 20, 30>();
    outputBuffer[1] = trimmedLastCount<10, 20, 30>();
}

// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= TrimFirstOfPack(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= TrimLastOfPack(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
