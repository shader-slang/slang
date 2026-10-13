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

    static __subscript(int index) -> int
    {
        get
        {
            return instanceMethod();
/*CHECK:
                   ^^^^^^^^^^^^^^ type cannot refer to non-static member
                   ^^^^^^^^^^^^^^ type 'Value' cannot be used to refer to non-static member 'instanceMethod'
*/

        }
    }
}

