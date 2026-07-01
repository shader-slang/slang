// Regression test for:
// https://github.com/shader-slang/slang/issues/11317
//
// `countof(localArr)` for a fixed-size local array used to return
// `sizeof(scalar component)` (e.g. `4` for `int[5]`, or `4` for
// `int2[3]` because the scalar component is `int`) instead of the array
// length. Two layers had to be fixed:
//
//   - lower-to-IR: `visitSizeOfLikeExpr` was taking an `ASTNaturalLayout`
//     constant-fold shortcut for any `SizeOfLikeExpr` whose
//     `dataLayoutType` was unset, including `CountOfExpr`. That branch
//     then fell through to a code path that picked `size.alignment` for
//     non-`SizeOfExpr` cases, which is unrelated to count.
//   - IR specialization: `maybeSpecializeCountOf` only folded
//     `kIROp_TypePack` / `kIROp_TupleType` operands, so any `CountOf`
//     that did reach IR survived as an unspecialized inst that slangi
//     had no handler for.
//
// Pin the fix across scalar-, vector-, and struct-element fixed arrays.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair { int x; int y; }

void main()
{
    int      a[5];
    int      b[7];
    float    c[5];
    double   d[5];
    uint16_t e[5];
    int2     f[3];
    Pair     g[2];

    //CHECK: a=5
    //CHECK-NEXT: b=7
    //CHECK-NEXT: c=5
    //CHECK-NEXT: d=5
    //CHECK-NEXT: e=5
    //CHECK-NEXT: f=3
    //CHECK-NEXT: g=2
    printf("a=%d\n", countof(a));
    printf("b=%d\n", countof(b));
    printf("c=%d\n", countof(c));
    printf("d=%d\n", countof(d));
    printf("e=%d\n", countof(e));
    printf("f=%d\n", countof(f));
    printf("g=%d\n", countof(g));
}
