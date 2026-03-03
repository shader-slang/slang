//TEST(compute):SIMPLE(filecheck=CHECK_EXPLICIT): -target spirv-asm -entry computeMain -stage compute -DEXPLICIT
//TEST(compute):SIMPLE(filecheck=CHECK_OVERLOAD): -target spirv-asm -entry computeMain -stage compute -DOVERLOAD

enum CoopMatMatrixLayout
{
    RowMajor = 0,
    ColumnMajor = 1,
};

int Load<let me : CoopMatMatrixLayout>(int inVal)
{
    return int(inVal % 10);
}

#if !defined(EXPLICIT)
int Load<let Dim : uint, let ClampMode : uint>(int inVal, int inVal2)
{
    return Load<CoopMatMatrixLayout.RowMajor>(inVal);
}
#endif

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;

#if defined(EXPLICIT)
    //CHECK_EXPLICIT: error 30019: expected an expression of type 'CoopMatMatrixLayout', got 'int'
    //CHECK_EXPLICIT: note: explicit conversion from 'int' to 'CoopMatMatrixLayout' is possible
    int outVal = Load<0>(tid);

#elif defined(OVERLOAD)
    //CHECK_OVERLOAD-NOT: error {{[1-9][0-9]*}}
    int outVal = Load<CoopMatMatrixLayout.RowMajor>(tid);
#endif
}
