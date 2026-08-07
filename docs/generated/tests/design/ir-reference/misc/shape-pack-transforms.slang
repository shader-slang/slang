//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#pack-and-expansion
//META: doc_section_digest=bee2a58322e072cc00e82140814df0fcca180d4ab4d912ad3941f22a06a1ccff
//META: purpose=The four shape helpers lower to `ShapeConcat(left, right, axis)`, `ShapePermute(pack, order)`, `ShapeSwap(pack, dim0, dim1)`, and `ShapeReduce(pack, axis)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand arity the doc gives for each shape transform by
// declaring one generic per helper over a tile type whose extents are a value
// pack. The CHECKs pin the operand count of each inst (three, two, three, two)
// and require every result to be a `ValuePackType`, so a helper that dropped an
// operand or folded to a scalar would not match.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
#lang slang 2026

struct Tile<T : ICoopElement, let each D : int>
{
    Tile<T, __shapePermute(D, Order)> permute<let each Order : int>()
    { Tile<T, __shapePermute(D, Order)> res; return res; }

    Tile<T, __shapeReduce(D, Axis)> reduce<let Axis : int>()
    { Tile<T, __shapeReduce(D, Axis)> res; return res; }
}

Tile<T, __shapeConcat(A, B, Axis)> cat<let Axis : int, T : ICoopElement, let each A : int, let each B : int>(Tile<T, A> l, Tile<T, B> r)
{ Tile<T, __shapeConcat(A, B, Axis)> res; return res; }

Tile<T, __shapeSwap(D, I, J)> swp<let I : int, let J : int, T : ICoopElement, let each D : int>(Tile<T, D> t)
{ Tile<T, __shapeSwap(D, I, J)> res; return res; }

void use16x64(Tile<float, 16, 64> t) {}
void use32x16(Tile<float, 32, 16> t) {}
void use16x1(Tile<float, 16, 1> t) {}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    Tile<float, 16, 32> a;
    use16x64(cat<1>(a, a));
    use32x16(a.permute<1, 0>());
    use32x16(swp<0, 1>(a));
    use16x1(a.reduce<1>());
    outputBuffer[0] = 1;
}

// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= ShapeConcat(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= ShapePermute(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= ShapeSwap(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
// IR-DAG: : ValuePackType({{.*}}){{[ 	]*}}= ShapeReduce(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
