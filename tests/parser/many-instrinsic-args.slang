//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry computeMain -line-directive-mode none

// HLSL: bar((int(0)), (1L), (2L), (3L), (4L), (5L), (6L), (7L), (8L), (9L), (10L), (11L))

__target_intrinsic(hlsl, "bar($0, $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)")
int foo(int x0, int x1, int x2, int x3, int x4, int x5, int x6, int x7, int x8, int x9, int x10, int x11);

[numthreads(1, 1, 1)]
void computeMain()
{
    foo(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}
