//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Pins the Slang-mode multi-dimensional array declaration ordering
// (shader-slang/slang#5870): a type-position bracket chain binds the last
// bracket outermost, a C-style declarator binds the first bracket outermost,
// and the mixed spelling `int[2][3] arr[5][4]` equals `int[2][3][4][5] arr`
// (specification/types-array.md, Declaration Syntax groups (2)/(5) and
// Remark 2), so GLSL-mode or other ordering changes cannot silently shift
// these meanings.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Type position: `int[2][3]` is a 3-element array of `int[2]`s.
    int[2][3] a;
    outputBuffer[0] = a.getCount();    // CHECK: 3
    outputBuffer[1] = a[0].getCount(); // CHECK: 2

    // C-style declarator: `int b[2][3]` is a 2-element array of `int[3]`s.
    int b[2][3];
    outputBuffer[2] = b.getCount();    // CHECK: 2
    outputBuffer[3] = b[0].getCount(); // CHECK: 3

    // Mixed spelling: every level's element count must match the equivalent
    // pure type-position spelling.
    int[2][3] arr[5][4];
    int[2][3][4][5] arr2;
    outputBuffer[4] = arr.getCount();        // CHECK: 5
    outputBuffer[5] = arr[0].getCount();      // CHECK: 4
    outputBuffer[6] = arr[0][0].getCount();   // CHECK: 3
    outputBuffer[7] = arr[0][0][0].getCount(); // CHECK: 2
    outputBuffer[8] = arr2.getCount();        // CHECK: 5
    outputBuffer[9] = arr2[0].getCount();     // CHECK: 4
    outputBuffer[10] = arr2[0][0].getCount();  // CHECK: 3
    outputBuffer[11] = arr2[0][0][0].getCount(); // CHECK: 2
}
