// Builtin-operator fast-path coverage for compile-time-constant contexts. These operators
// must fold at compile time (static const initializers, generic value arguments) -- the path
// that produces a decl-free `BuiltinOperationIntVal`. Covers concrete folding, the shift fold
// (shared `_tryFoldConstantShift`), and *symbolic* folding that re-evaluates on substitution
// (`N * 2` with a generic value parameter `N`), which must yield a single canonical value.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 3 * 2 + 1; // -> 7

int getN<let N : int>()
{
    return N;
}

// `N * 2` is symbolic here (depends on the generic value parameter); it must fold on
// substitution when `getDouble` is instantiated, then feed the next generic argument.
int getDouble<let N : int>()
{
    return getN<N * 2>();
}

void main()
{
    printf("%d\n", K); // CHECK: 7

    printf("%d\n", getN<(1 << 4)>());  // shift fold -> CHECK: 16
    printf("%d\n", getN<(100 % 7)>()); // modulo fold -> CHECK: 2
    printf("%d\n", getN<(255 & 0xF)>()); // bitwise fold -> CHECK: 15

    printf("%d\n", getDouble<5>());  // symbolic N*2 -> CHECK: 10
    printf("%d\n", getDouble<21>()); // symbolic N*2 -> CHECK: 42
}
