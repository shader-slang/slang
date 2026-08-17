//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// non-exhaustive: cascading notes for visibility errors

// Test that visibility diagnostics for constructors in various declaration
// shapes print "TypeName.init" instead of the internal "$init" name.
// Regression test for https://github.com/shader-slang/slang/issues/10955

struct IS
{
}

// Generic struct with synthesized constructor referencing less-visible type.
public struct GenericStruct<T>
//CHECK:      ^^^^^^^^^^^^^ 'GenericStruct.init' references less visible type 'IS'.
{
    public IS member;
//CHECK:      ^^^^^^ references less visible type
}

// Non-generic struct with a generic constructor referencing less-visible type.
public struct PlainStruct
{
    public __init<T>(IS v)
//CHECK:   ^^^^^^ 'PlainStruct.init' references less visible type 'IS'.
    {
    }
}

// Constructor declared inside an extension.
struct ExtTarget
{
}

extension ExtTarget
{
    public __init(IS v)
//CHECK:   ^^^^^^ 'ExtTarget.init' references less visible type
    {
    }
}
