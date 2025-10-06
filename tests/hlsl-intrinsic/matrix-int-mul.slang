//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

//CHECK-NOT: error 39999: ambiguous call to 'mul'

[shader("compute")]
void main() {
    int2x2 matrixA = {
        1, 2,
        3, 4
    };
    int2 vecA = {
        1, 2
    };

    int2x2 matrixB = mul(matrixA, matrixA); // M * M
    int2   vecB    = mul(vecA,    matrixA); // V * M
    int2   vecC    = mul(matrixA, vecA);    // M * V
}
