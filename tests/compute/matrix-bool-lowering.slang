//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute

// Companion of the matrix-bool-lowering tests under tests/{glsl,metal,wgsl,spirv}:
// bool matrix construction, logical and comparison operators, and reductions on
// the CPU and CUDA targets, where bool matrices are emitted as Matrix<bool,R,C>
// instead of being legalized to vector arrays.

//TEST_INPUT:ubuffer(data=[1 0], stride=4):name inputBuffer
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

// Booleans are loaded from a buffer at runtime so the compiler can't constant-fold them
static bool trueVal;
static bool falseVal;

struct MatrixWrapper {
    bool2x2 mat1 = bool2x2(falseVal, falseVal, falseVal, falseVal);
    bool2x3 mat2 = bool2x3(trueVal, trueVal, falseVal, falseVal, falseVal, trueVal);
}

bool allElementsTrue(bool2x2 matrix)
{
    return trueVal
        && matrix[0][0]
        && matrix[0][1]
        && matrix[1][0]
        && matrix[1][1];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Load true/false values from input buffer to avoid constant folding
    trueVal = inputBuffer[0] != 0;
    falseVal = inputBuffer[1] != 0;

    // Test bool matrix construction, covering every shape-dependent constructor
    // (2x3/3x2, 2x4/4x2, 3x4/4x3 take the scalar-list constructors that select
    // their layout at instantiation)
    bool2x2 mat1 = bool2x2(trueVal, falseVal, falseVal, trueVal);
    bool3x3 mat2 = bool3x3(
        trueVal, falseVal, trueVal,
        falseVal, trueVal, falseVal,
        trueVal, falseVal, trueVal
    );
    bool2x4 mat3 = bool2x4(
        trueVal, falseVal, trueVal, falseVal,
        trueVal, falseVal, trueVal, falseVal
    );
    bool3x4 mat4 = bool3x4(
        trueVal, falseVal, trueVal, falseVal,
        falseVal, trueVal, falseVal, trueVal,
        trueVal, falseVal, trueVal, falseVal
    );
    bool4x3 mat5 = bool4x3(
        trueVal, falseVal, trueVal,
        falseVal, trueVal, falseVal,
        trueVal, falseVal, trueVal,
        falseVal, trueVal, falseVal
    );
    bool3x2 mat6 = bool3x2(trueVal, falseVal, falseVal, trueVal, trueVal, falseVal);
    bool4x2 mat7 = bool4x2(trueVal, trueVal, falseVal, falseVal, trueVal, falseVal, falseVal, trueVal);

    // Test bool matrix element access
    bool val1 = mat1[0][0];
    bool val2 = mat2[2][1];

    // Test bool matrix row access
    bool2 row = mat1[1];
    bool3 row3 = mat2[0];

    // Test logical and comparison operators
    bool2x2 not_mat = !mat1;
    bool2x2 and_mat = mat1 && bool2x2(trueVal, trueVal, falseVal, falseVal);
    bool2x2 eq_mat = mat1 == bool2x2(trueVal, falseVal, falseVal, trueVal);
    bool2x2 or_mat = mat1 || bool2x2(trueVal, trueVal, falseVal, falseVal);
    bool2x2 xor_mat = mat1 ^ bool2x2(trueVal, trueVal, falseVal, falseVal);

    // Test element assignment
    mat1[0][1] = trueVal;
    mat2[1][2] = trueVal;

    // Test passing bool matrices to functions
    bool anded = allElementsTrue(mat1);

    // Test structs with bool matrix fields
    MatrixWrapper wrapper = {};

    // Test any/all operations
    bool2x2 all_true = bool2x2(trueVal, trueVal, trueVal, trueVal);
    bool2x2 all_false = bool2x2(falseVal, falseVal, falseVal, falseVal);
    bool2x2 mixed = bool2x2(trueVal, falseVal, trueVal, falseVal);

    bool test_all_true = all(all_true);      // all elements true -> true
    bool test_all_false = all(all_false);    // all elements false -> false
    bool test_all_mixed = all(mixed);        // some elements false -> false
    bool test_any_true = any(all_true);      // some elements true -> true
    bool test_any_false = any(all_false);    // no elements true -> false
    bool test_any_mixed = any(mixed);        // some elements true -> true

    // Float matrix negation rides the same unary-op macro as bool `!`
    float2x2 fmat = float2x2(1.0, 2.0, 3.0, 4.0);
    float2x2 neg_fmat = -fmat;

    // Store results
    outputBuffer[0] = val1;
    // CHECK: 1
    outputBuffer[1] = val2;
    // CHECK-NEXT: 0
    outputBuffer[2] = row.x;
    // CHECK-NEXT: 0
    outputBuffer[3] = row.y;
    // CHECK-NEXT: 1
    outputBuffer[4] = row3.y;
    // CHECK-NEXT: 0
    outputBuffer[5] = not_mat[0][0];
    // CHECK-NEXT: 0
    outputBuffer[6] = and_mat[0][0];
    // CHECK-NEXT: 1
    outputBuffer[7] = mat1[0][1];
    // CHECK-NEXT: 1
    outputBuffer[8] = mat3[0][1];
    // CHECK-NEXT: 0
    outputBuffer[9] = anded;
    // CHECK-NEXT: 0
    outputBuffer[10] = wrapper.mat1[0][0] || wrapper.mat2[0][0];
    // CHECK-NEXT: 1
    outputBuffer[11] = test_all_true;
    // CHECK-NEXT: 1
    outputBuffer[12] = test_all_false;
    // CHECK-NEXT: 0
    outputBuffer[13] = test_all_mixed;
    // CHECK-NEXT: 0
    outputBuffer[14] = test_any_true;
    // CHECK-NEXT: 1
    outputBuffer[15] = test_any_false;
    // CHECK-NEXT: 0
    outputBuffer[16] = test_any_mixed;
    // CHECK-NEXT: 1
    outputBuffer[17] = mat4[2][3];
    // CHECK-NEXT: 0
    outputBuffer[18] = mat5[3][2];
    // CHECK-NEXT: 0
    outputBuffer[19] = mat6[2][1];
    // CHECK-NEXT: 0
    outputBuffer[20] = mat7[3][1];
    // CHECK-NEXT: 1
    outputBuffer[21] = eq_mat[0][0];
    // CHECK-NEXT: 1
    outputBuffer[22] = or_mat[1][0];
    // CHECK-NEXT: 0
    outputBuffer[23] = xor_mat[0][1];
    // CHECK-NEXT: 1
    outputBuffer[24] = int(neg_fmat[0][0]);
    // CHECK-NEXT: FFFFFFFF
    // (-1; the buffer dump prints negative ints as unsigned hex)
    outputBuffer[25] = mat2[1][2];
    // CHECK-NEXT: 1
}
