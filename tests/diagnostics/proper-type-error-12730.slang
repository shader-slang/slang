//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test for https://github.com/shader-slang/slang/issues/12730
//
// When a value/expression is used where a type is expected, CoerceToProperType
// fails. Previously the failed proper-type check could leave a null (or, on other
// failure branches, a stale) type in the resulting TypeExp; the fix normalizes
// every failure on the diagnostic path to ErrorType. The observable behavior must
// be unchanged: the ExpectedAType diagnostic still fires exactly once, and the
// ErrorType-typed member must suppress cascading errors when used downstream.

int gValue = 5;

struct Foo
{
    // `gValue` is a value, not a type -> ExpectedAType. The field's type becomes
    // ErrorType (previously null) so that downstream uses do not cascade.
    gValue field;
    //CHECK: expected a type
    //CHECK: expected a type, got a 'int' for 'gValue'
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    // Downstream uses of the failed-type member must not produce additional
    // errors: ErrorType propagates and suppresses cascading diagnostics.
    f.field = 3;
    let y = f.field + 1;
    //CHECK-NOT: error[
}
