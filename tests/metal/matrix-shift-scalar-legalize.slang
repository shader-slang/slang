//TEST:SIMPLE(filecheck=CHECK):-target metal -stage compute -entry computeMain

RWStructuredBuffer<uint> output;

// A narrow matrix shifted by a wider scalar used to crash binary-op legalization because it
// assumed that every composite shift operand was a vector.
[noinline]
matrix<uint16_t, 2, 2> shiftMatrix(matrix<uint16_t, 2, 2> value, uint amount)
{
    return value << amount;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    matrix<uint16_t, 2, 2> value = {1, 2, 3, 4};
    //CHECK: << matrix<uint,int(2),int(2)>
    output[0] = uint(shiftMatrix(value, 1)[0][0]);
}
