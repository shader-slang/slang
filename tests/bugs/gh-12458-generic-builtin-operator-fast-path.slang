// A builtin arithmetic, comparison, logical, bitwise, or unary operator on a generic type
// parameter constrained to a sealed builtin marker interface (`T : __BuiltinFloatingPointType`,
// `T : __BuiltinIntegerType`, `T : __BuiltinLogicalType`) takes the same builtin-operator fast
// path a concrete basic type does (`SemanticsExprVisitor::convertToBuiltinArithmeticOp`), instead
// of falling through to full generic overload resolution against every visible `operator OP`
// candidate. Before this fix, only a concrete `BasicExpressionType` took the fast path, so a
// generic-heavy shader like the one in #12458 (thousands of `T(literal)` sites inside one generic
// function) spent the large majority of its compile time on generic argument inference for
// candidates that could never apply, at ~1.36 ms per repeated expression.
//
// The widened path is reached only when both operands have the SAME type: `T OP T` for a scalar
// `T`, or `vector<T,N> OP vector<T,N>`/`matrix<T,R,C> OP matrix<T,R,C>` for a same-shape composite
// whose element type is `T` (the operand-unwrap-then-classify path `vaddF`/`vmodI` below cover). A
// MIXED shape like `vector<T,C> * T` does not reach it: `coerceOperandsOfBuiltinBinaryExpr` only
// takes the fast-path branch when the two operand types are equal, so a mixed vector*scalar pair
// on an abstract `T` still falls through to full overload resolution today.
//
// `modF`/`modI` also pin a correctness invariant of the fix, not just its performance: `%` lowers
// to `FRem` or `IRem` depending on whether the operand element type is floating-point. Checking
// resolves that once, from the type's declared constraint when it is abstract, and stores it on
// the `BuiltinOperatorExpr` node (`elementTypeIsFloatingPoint`) for IR lowering to read back,
// because by lowering time the element type may still be an unspecialized generic parameter with
// no concrete `BaseType` to inspect directly. `modF<float>(5.25, 2.0)` verifies the resolved value
// (`1.25`) is the floating-point remainder, not `1` (the integer remainder truncation). `vmodI`
// pins the same selection for a vector element.
//
// `eqT` (not `logicalAndT`/`&&`) is what actually exercises the `__BuiltinLogicalType` branch of
// `classifyBuiltinArithmeticElementType`: `&&`/`||` are short-circuiting and map to
// `BuiltinOperationKind::Unknown` in `convertToBuiltinArithmeticOp`, which returns null before any
// element-type classification runs, so they always resolve through normal overload resolution
// exactly as before this PR, on any `T`. Equality (`==`) IS fast-pathed for `bool`, so `eqT` is the
// case that actually proves the sealed `__BuiltinLogicalType` conformance witness is recognized.
//
// The negative case -- the fast path must be DECLINED, not just correctly taken -- is covered by
// the companion `gh-12458-generic-builtin-operator-fast-path-decline.slang`: a plain,
// unconstrained generic parameter must yield no conformance witness to any of the three sealed
// interfaces and fall through to ordinary overload resolution, which then fails to find a
// matching `operator+` and reports a normal diagnostic. That file also explains why the stronger
// negative case the fix's own `isWitnessUncheckedOptional` guard exists for -- a `where optional
// T : __BuiltinFloatingPointType` constraint used outside an `if (T is ...)` check -- is not
// covered anywhere yet: instantiating it with a concretely non-conforming type currently crashes
// the compiler for a reason unrelated to this PR (shader-slang/slang#13018).
//
// A THIRD negative case -- `vector<T,N> == vector<T,N>` specifically -- is covered separately by
// `gh-12458-generic-vector-equality-decline.slang` (a `COMPARE_COMPUTE` test, not `INTERPRET`:
// the `all(equal(...))` lowering this case resolves to is not yet supported by the bytecode
// interpreter's codegen, an unrelated, pre-existing gap). `glsl.meta.slang` declares `bool
// operator==<T:__BuiltinArithmeticType,N>(vector<T,N>, vector<T,N>)` (`[OverloadRank(15)]`,
// `[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm)]` -- visible on every target, not just GLSL
// scope) that reduces vector equality to a single `bool` via `all(equal(...))`, and real code
// (`tests/metal/math-vector.slang`'s generic `Test_Vector<T:__BuiltinFloatingPointType,N>`)
// relies on `vec == vec` type-checking as `bool` directly in a boolean context. Before this PR, a
// generic element type was never recognized by `classifyBuiltinArithmeticElementType`, so
// `vector<T,N> == vector<T,N>` for abstract `T` always fell through to that stdlib overload.
// Widening the classification to recognize generic `T` made the fast path fire here too,
// producing the RAW per-component `vector<bool,N>` instead -- which fails to coerce to `bool` and
// broke that real test. Vectors have no declarable stdlib overload for the raw per-component form
// (it exists only as this fast path's own `BuiltinOperatorExpr` construction), so unlike every
// other operator, `==`/`!=` on a GENERIC-element-type vector must keep falling through to the
// stdlib overload instead of taking the fast path; only a CONCRETE element type keeps this
// function's pre-existing (`isGLSLOperatorScope()`-gated) vector-equality behavior unchanged.
//
// `notT` pins a FOURTH case, on unary `!` specifically: `__BuiltinLogicalType` is implemented by
// `bool` AND every builtin integer type (it also backs bitwise-operator codegen elsewhere), so a
// `T : __BuiltinLogicalType` parameter may be instantiated with `int`, not just `bool` --
// `notT<int>(5)` below is exactly that. Unary `!` must still decline the fast path for such a
// `T`, because its result has to be `bool`-shaped: `classifyBuiltinArithmeticElementType` tracks
// this with two separate flags, `isBool` (true only for a genuinely concrete `bool` element,
// never provable for a generic parameter) and `isLogical` (true for `__BuiltinLogicalType`
// conformance broadly, safe for `==`/`eqT` above but NOT for `!`). `notT<int>(5)` verifies the
// result is `0` (5 is truthy, so `!5` is `false`, via normal overload resolution's int-to-bool
// conversion), not a raw `Not` misapplied to the integer bit pattern.

//TEST:INTERPRET(filecheck=CHECK):

T addF<T : __BuiltinFloatingPointType>(T a, T b) { return a + b; }
T modF<T : __BuiltinFloatingPointType>(T a, T b) { return a % b; }
T modI<T : __BuiltinIntegerType>(T a, T b) { return a % b; }
bool cmpF<T : __BuiltinFloatingPointType>(T a, T b) { return a < b; }
bool eqT<T : __BuiltinLogicalType>(T a, T b) { return a == b; }
T bitAndT<T : __BuiltinIntegerType>(T a, T b) { return a & b; }
T negF<T : __BuiltinFloatingPointType>(T a) { return -a; }
T vaddF<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b) { return (a + b).x; }
T vmodI<T : __BuiltinIntegerType>(vector<T, 2> a, vector<T, 2> b) { return (a % b).y; }
T notT<T : __BuiltinLogicalType>(T a) { return !a; }

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
    printf("%d\n", eqT<bool>(true, true) ? 1 : 0);
    // CHECK-NEXT: 4
    printf("%d\n", bitAndT<int>(12, 6));
    // CHECK-NEXT: -3.500000
    printf("%f\n", negF<float>(3.5));
    // CHECK-NEXT: 6.500000
    printf("%f\n", vaddF<float>(vector<float, 3>(1.0, 2.0, 3.0), vector<float, 3>(5.5, 6.0, 7.0)));
    // CHECK-NEXT: 2
    printf("%d\n", vmodI<int>(vector<int, 2>(7, 8), vector<int, 2>(3, 3)));
    // CHECK-NEXT: 0
    printf("%d\n", notT<int>(5));
}
