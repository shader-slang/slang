//TEST:INTERPRET(filecheck=CHECK):

// Values the interpreter can represent must still print. A `NativeString` is a plain
// pointer with a HostVM layout, so even a genuinely runtime `NativeString` selected by
// a runtime ternary (the pointer-sized counterpart of a runtime `String`, which is
// rejected -- see interpreter-runtime-string-diagnostic.slang) works; and a
// constant-foldable ternary folds to a string literal.

bool cmpF(float a, float b)
{
    return a < b;
}

void main()
{
    NativeString yes = "yes";
    NativeString no = "no";
    // CHECK: yes
    printf("%s\n", cmpF(1.0, 2.0) ? yes : no);

    // CHECK: true
    printf("%s\n", (1 < 2) ? "true" : "false");

    // These representable values must NOT be rejected: pins the no-over-rejection
    // contract, so a future over-broad guard that starts rejecting NativeString or a
    // folded literal fails here.
    // CHECK-NOT: error[E52014]
}
