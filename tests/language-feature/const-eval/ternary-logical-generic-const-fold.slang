// Regression test: `?:`, `&&`, and `||` applied to symbolic (generic value parameter)
// integer operands fold through `BuiltinOperationIntVal` (the `Conditional`/`And`/`Or`
// operation kinds) and resolve to concrete array sizes once the generic is specialized.
// This exercises the path that previously produced a `FuncCallIntVal`.
//TEST:INTERPRET(filecheck=CHECK):

int test<int N, int M>()
{
    int arrCond[(N > M) ? N : M];                   // ?:  -> Conditional
    int arrAnd[((N > 0) && (M > 0)) ? (N + M) : 1]; // &&  -> And
    int arrOr[((N > 5) || (M > 0)) ? N : 1];        // ||  -> Or
    return arrCond.getCount() + arrAnd.getCount() + arrOr.getCount();
}

void main()
{
    // test<3,5>: max(3,5)=5; (3>0 && 5>0)?(3+5):1 = 8; (3>5 || 5>0)?3:1 = 3; total = 16.
    printf("%d\n", test<3, 5>());
    //CHECK: 16
}
