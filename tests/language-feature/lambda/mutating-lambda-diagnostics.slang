//TEST:SIMPLE(filecheck=CHECK): -target spirv

func acceptIFunc(f: IFunc<int>) {}
func acceptFuncType(f: functype()->int) {}

[numthreads(1, 1, 1)]
void computeMain()
{
    int captured = 0;
    let immutableCallback = ()! => ++captured;
    // CHECK: error[E39999]: no overload for '()' applicable to arguments of type ()
    immutableCallback();

    // CHECK: error[E30019]: type mismatch in expression
    // CHECK: expected an expression of type 'IFunc<int, >'
    acceptIFunc(()! => 1);
    // CHECK: error[E30019]: type mismatch in expression
    // CHECK: expected an expression of type '() -> int'
    acceptFuncType(()! => 2);
}
