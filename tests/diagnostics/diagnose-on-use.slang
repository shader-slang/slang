//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):

// `[__diagnoseOnUse]` is a general declaration attribute: any reference to a declaration
// carrying it is an error reporting the attribute's message. These cases pin the
// user-declaration surface of the attribute directly (the builtin `operator*` fallbacks
// exercise it in matrix-vector-mul-hint.slang): references to functions, variables and
// types; the constructor-versus-coercion reference boundary; interaction with
// `[deprecated]` and `[RemovedSince]`; the selected-vs-unselected overload candidate
// behavior; and the usage attributes' argument validation.

// A function carrying the attribute reports its message at every call.
[__diagnoseOnUse("use newFunc instead")]
int oldFunc(int x) { return x; }
int newFunc(int x) { return x + 1; }

int useOldFunc()
{
    return oldFunc(1);
}

// A variable and a struct are diagnosed the same way at their references.
[__diagnoseOnUse("read from current instead")]
static const int legacySetting = 1;

[__diagnoseOnUse("LegacyGrid is retired; use Grid")]
struct LegacyGrid
{
    int cells[4];
};

int useLegacyDecls(LegacyGrid g)
{
    return legacySetting + g.cells[0];
}

// Among overload candidates, the attribute fires only when the carrying candidate is the
// one resolution selects: an exact match wins over a conversion, so `pick(1)` stays
// clean while `pick(2.5)` must select the carrying overload and report the error.
int pick(int x) { return x; }
[__diagnoseOnUse("pick(float) is not for floating-point use")]
int pick(float x) { return int(x); }

int usePick()
{
    return pick(1);
}

int usePickedFloat()
{
    return pick(2.5);
}

// A declaration carrying both `[deprecated]` and `[__diagnoseOnUse]` reports only the
// error: the usage-attribute checks stop at the first attribute that applies, in the
// order removed, error-on-use, then deprecation.
[deprecated("old news")]
[__diagnoseOnUse("gone for good")]
int bothAttrs() { return 1; }

int useBothAttrs()
{
    return bothAttrs();
}

// A declaration that is already removed in the module's language version reports the
// removal error instead of the error-on-use: removal is checked first.
[RemovedSince(0, "removed now")]
[__diagnoseOnUse("unreachable message")]
int removedFunc() { return 1; }

int useRemovedFunc()
{
    return removedFunc();
}

// The attribute takes exactly one string argument; a missing and a surplus argument are
// both diagnosed at the attribute use site.
[__diagnoseOnUse()]
int noArg() { return 1; }

[__diagnoseOnUse("one", "two")]
int twoArgs() { return 1; }

// The message must be a string literal.
[__diagnoseOnUse(3)]
int nonString() { return 1; }

// The same user-reachable surplus-argument hazard applies to the older usage attributes;
// their validation now diagnoses the count instead of asserting it.
[deprecated("a", "b")]
int deprecatedTwoArgs() { return 1; }

[RemovedSince(0, "gone", "extra")]
int removedThreeArgs() { return 1; }

// The constructor boundary: a multi-argument type call materializes a reference to the
// selected `__init`, so an `__init` carrying the attribute reports the error; a
// single-argument construction is an explicit coercion and routes through the conversion
// machinery instead of declaration reference checking, so it is not diagnosed.
struct DiagCtor
{
    [__diagnoseOnUse("this constructor is retired")]
    __init(int a, int b) { a + b; }
    __init(int a) { a; }
}

DiagCtor multiArg = DiagCtor(1, 2);
DiagCtor singleArg = DiagCtor(5);

// Declaration checking validates the attribute arguments first, in declaration order:
// CHECK: error[E31001]: wrong number of attribute arguments
// CHECK: attribute '__diagnoseOnUse' expects 1 arguments (0 provided)
// CHECK: error[E31001]: wrong number of attribute arguments
// CHECK: attribute '__diagnoseOnUse' expects 1 arguments (2 provided)
// CHECK: error[E33071]: expected a string literal
// CHECK: error[E31001]: wrong number of attribute arguments
// CHECK: attribute 'deprecated' expects 1 arguments (2 provided)
// CHECK: error[E31001]: wrong number of attribute arguments
// CHECK: attribute 'RemovedSince' expects 2 arguments (3 provided)

// The parameter-type reference to LegacyGrid is diagnosed while signatures are checked,
// before any function body runs.
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'LegacyGrid' is an error: LegacyGrid is retired; use Grid

// References in the bodies follow.
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'oldFunc' is an error: use newFunc instead
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'legacySetting' is an error: read from current instead

// `pick(1)` resolves to the plain overload and reports nothing.
// CHECK-NOT: use of 'pick' is an error

// `pick(2.5)` selects the carrying overload.
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'pick' is an error: pick(float) is not for floating-point use

// The error-on-use attribute wins over deprecation, and only the error is reported.
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'bothAttrs' is an error: gone for good
// CHECK-NOT: has been deprecated

// The removal error wins over the error-on-use for a declaration already removed in this
// language version.
// CHECK: error[E31207]: use of removed declaration
// CHECK: removedFunc has been removed since language version '0': removed now
// CHECK-NOT: E31233

// The constructor boundary is checked last: the multi-argument construction of DiagCtor
// reports the error, and the single-argument construction that follows is a coercion and
// reports nothing.
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of 'DiagCtor.init' is an error: this constructor is retired
// CHECK-NOT: use of 'DiagCtor.init' is an error
