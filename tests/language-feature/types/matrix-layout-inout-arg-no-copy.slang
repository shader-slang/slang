// An explicit layout that matches the default needs no temporary for an inout argument.

//TEST:SIMPLE(filecheck=SAME):-target hlsl -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DIFF):-target hlsl -entry main -stage compute -matrix-layout-column-major

struct S
{
    row_major float2x4 m;
}

RWStructuredBuffer<S> sb;

void bump(inout float2x4 x) { x[0][1] += 100; }

// SAME-LABEL: void main()
// SAME-NOT: m_0
// SAME: return;

// DIFF-LABEL: void main()
// DIFF: float2x4 [[TMP:_S[0-9]+]] = {{.*}}sb_0[int(0)].m_0
// DIFF: bump_0([[TMP]]);
[numthreads(1, 1, 1)]
void main()
{
    bump(sb[0].m);
}
