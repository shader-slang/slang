//TEST:INTERPRET(filecheck=CHECK):

// A *runtime* `String` value has no representation on the slangi CPU interpreter:
// `String` is a host-only managed object with no HostVM byte layout, so the value
// would land in a 0-byte working-set slot and `printf`'s `%s` handler would then
// dereference garbage as a `const char*` and SIGSEGV. The interpreter must instead
// report a clean diagnostic (error 52014). See issue #13017.
//
// The condition must be genuinely runtime (a call to `cmpF`, not a compile-time
// constant): a constant-foldable condition folds the ternary to a string literal,
// which is representable. interpreter-representable-string.slang covers the cases
// that must still work.

bool cmpF(float a, float b)
{
    return a < b;
}

void main()
{
    //CHECK: result code = 1
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    // Two E52014 diagnostics fire, order-independent: one for the runtime `string`
    // value (the ternary result), one for the synthesized varargs `Tuple` that
    // contains it, so the aggregate-containing-an-unrepresentable-type path cannot
    // silently regress.
    //CHECK-DAG: error[E52014]
    //CHECK-DAG: type 'string' is not representable by the Slang interpreter
    //CHECK-DAG: type 'Tuple' is not representable by the Slang interpreter
    // No internal-error/abort path may follow the diagnostics either.
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    printf("%s\n", cmpF(1.0, 2.0) ? "true" : "false");
}
