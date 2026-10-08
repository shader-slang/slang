//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Receiver mode does not participate in overload selection. The exact argument-type match below
// selects the instance method. The enclosing method is static and has no receiver, so semantic
// checking rejects the selected instance member.

struct Choice
{
    [nonmutating]
    int pick(int value) { return value; }

    static int pick(float value) { return int(value); }

    static int test()
    {
        int staticResult = pick(1.0);
        return pick(1);
//CHECK:       ^^^^ type cannot refer to non-static member
//CHECK:       ^^^^ type 'Choice' cannot be used to refer to non-static member 'pick'
    }
}
