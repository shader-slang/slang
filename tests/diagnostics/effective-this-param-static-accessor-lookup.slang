//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// An accessor nested under a static subscript has no instance receiver. An unqualified call to an
// instance method must therefore be rejected instead of being treated as if implicit `this` were
// available.

struct Value
{
    int field;

    int instanceMethod()
    {
        return 0;
    }

    int overloadedInstanceMethod(int value) { return value; }
    int overloadedInstanceMethod(float value) { return int(value); }

    static __subscript(int index) -> int
    {
        get
        {
            return instanceMethod();
/*CHECK:
                   ^^^^^^^^^^^^^^ type cannot refer to non-static member
                   ^^^^^^^^^^^^^^ type 'Value' cannot be used to refer to non-static member 'instanceMethod'
*/

            overloadedInstanceMethod(1);
/*CHECK:
            ^^^^^^^^^^^^^^^^^^^^^^^^ type cannot refer to non-static member
            ^^^^^^^^^^^^^^^^^^^^^^^^ type 'Value' cannot be used to refer to non-static member 'overloadedInstanceMethod'
*/

            this.instanceMethod();
//CHECK:    ^^^^ 'this' used in callable without an instance receiver
//CHECK:    ^^^^ 'this' expression cannot be used in a callable that has no instance receiver

            (this).field;
//CHECK:     ^^^^ 'this' used in callable without an instance receiver
//CHECK:     ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
        }
    }

    static int explicitParenthesizedThis()
    {
        return (this).field;
//CHECK:        ^^^^ 'this' used in callable without an instance receiver
//CHECK:        ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
    }
}
