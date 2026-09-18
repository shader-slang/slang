// Regression for shader-slang/slang#12990: a matrix layout modifier is accepted on a
// function return type (it qualifies the return type, not the function declaration).

//TEST:INTERPRET(filecheck=CHECK):

row_major float2x3 makeMatrix()
{
    return float2x3(1, 2, 3, 4, 5, 6);
}

void main()
{
    float2x3 m = makeMatrix();
    // m is filled 1..6, so element [1][2] is 6.
    printf("%d\n", (int)m[1][2]);
    // CHECK: 6
}
