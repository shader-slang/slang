// The `x / -1` / `x % -1` guard in BuiltinOperationIntVal::tryFoldImpl, reached via symbolic
// (generic-value) folds that resolve once the generic is specialized. The guard computes
// `x / -1` as a wrapping negate (`-x`) and `x % -1` as `0`, instead of dividing -- a real divide
// would trap (SIGFPE) on `INT_MIN / -1`.
//TEST:INTERPRET(filecheck=CHECK):

int divNeg1<int N>()
{
    int a[N / -1]; // N / -1 == -N (wrapping negate, never an actual divide)
    return a.getCount();
}
int modNeg1<int N>()
{
    int a[(N % -1) + 5]; // N % -1 == 0  ->  size 5
    return a.getCount();
}

void main()
{
    printf("%d\n", divNeg1<-12>()); // -(-12) = 12
    //CHECK: 12
    printf("%d\n", modNeg1<99>()); // 0 + 5 = 5
    //CHECK: 5
}
