//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// A conversion to a type other than the one a throwing call returns is a call of its own, so a
// `try` on it does not cover the throwing call in its operand (#13489). The checks match only the
// uncovered inner call, because whether the conversion itself also gets "callee in 'try' does not
// throw" depends on how the conversion's callee is classified.

enum MyError
{
    Failed
}

int f() throws MyError { throw MyError.Failed; }

uint unsignedConstructor() throws MyError
{
    // CHECK: error[E30094]: callee may throw, use 'try'
    // CHECK-NEXT: try-nested-conversion-operand.slang:[[# @LINE+1]]:22
    return try uint(f());
}

float floatConstructor() throws MyError
{
    // CHECK: error[E30094]: callee may throw, use 'try'
    // CHECK-NEXT: try-nested-conversion-operand.slang:[[# @LINE+1]]:23
    return try float(f());
}

float floatCast() throws MyError
{
    // CHECK: error[E30094]: callee may throw, use 'try'
    // CHECK-NEXT: try-nested-conversion-operand.slang:[[# @LINE+1]]:24
    return try (float)f();
}

// CHECK-NOT: error[E30094]
