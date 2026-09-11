// A builtin arithmetic, comparison, logical, bitwise, or unary operator on a generic type
// parameter constrained to a sealed builtin marker interface (`T : __BuiltinFloatingPointType`,
// `T : __BuiltinIntegerType`, `T : __BuiltinLogicalType`) takes the same builtin-operator fast
// path a concrete basic type does (`SemanticsExprVisitor::convertToBuiltinArithmeticOp`), instead
// of falling through to full generic overload resolution against every visible `operator OP`
// candidate. Before this fix, only a concrete `BasicExpressionType` took the fast path, so a
// generic-heavy shader like the one in #12458 (thousands of `T(literal)` and `vector<T,C> * T`
// sites inside one generic function) spent the large majority of its compile time on generic
// argument inference for candidates that could never apply, at ~1.36 ms per repeated expression.
//
// `modF`/`modI` also pin a correctness invariant of the fix, not just its performance: `%` lowers
// to `FRem` or `IRem` depending on whether the operand element type is floating-point. Checking
// resolves that once, from the type's declared constraint when it is abstract, and stores it on
// the `BuiltinOperatorExpr` node (`elementTypeIsFloatingPoint`) for IR lowering to read back,
// because by lowering time the element type may still be an unspecialized generic parameter with
// no concrete `BaseType` to inspect directly. `modF<float>(5.25, 2.0)` verifies the resolved value
// (`1.25`) is the floating-point remainder, not `1` (the integer remainder truncation).

//TEST:INTERPRET(filecheck=CHECK):

T addF<T : __BuiltinFloatingPointType>(T a, T b) { return a + b; }
T modF<T : __BuiltinFloatingPointType>(T a, T b) { return a % b; }
T modI<T : __BuiltinIntegerType>(T a, T b) { return a % b; }
bool cmpF<T : __BuiltinFloatingPointType>(T a, T b) { return a < b; }
T logicalAndT<T : __BuiltinLogicalType>(T a, T b) { return a && b; }
T bitAndT<T : __BuiltinIntegerType>(T a, T b) { return a & b; }
T negF<T : __BuiltinFloatingPointType>(T a) { return -a; }

void main()
{
    // CHECK: 5.500000
    printf("%f\n", addF<float>(2.0, 3.5));
    // CHECK-NEXT: 1.250000
    printf("%f\n", modF<float>(5.25, 2.0));
    // CHECK-NEXT: 1
    printf("%d\n", modI<int>(7, 3));
    // CHECK-NEXT: 1
    printf("%d\n", cmpF<float>(1.0, 2.0) ? 1 : 0);
    // CHECK-NEXT: 1
    printf("%d\n", logicalAndT<bool>(true, true) ? 1 : 0);
    // CHECK-NEXT: 4
    printf("%d\n", bitAndT<int>(12, 6));
    // CHECK-NEXT: -3.500000
    printf("%f\n", negF<float>(3.5));
}
