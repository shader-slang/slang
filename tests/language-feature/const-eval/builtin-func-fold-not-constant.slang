// Calls that are not folded stay non-constant (shader-slang/slang#13973): a user-defined function
// with the same body as `max`, `abs` of an unsigned value (whose result differs between targets),
// and a function marked with an operator kind that the attribute does not accept.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

int myMax(int x, int y)
{
    return x > y ? x : y;
}

[__constantFoldAs(0)]
int notFolded(int x)
{
    return x;
}

RWStructuredBuffer<int> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int arr[myMax(8, 4)];
    //CHECK:     ^ E39999
    int arrAbs[abs(3u)];
    //CHECK:      ^ E39999
    int arrNotFolded[notFolded(3)];
    //CHECK:                  ^ E39999
    arr[0] = buf[0];
    arrAbs[0] = buf[1];
    arrNotFolded[0] = buf[2];
    buf[3] = arr[0] + arrAbs[0] + arrNotFolded[0];
}
