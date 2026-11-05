//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A static subscript has no receiver. Lookup from its accessor must therefore use a type base and
// reject an implicit reference to an instance member.

struct StaticSubscriptLookup
{
    int value;

    static __subscript(int index) -> int
    {
        get
        {
            return value;
//CHECK:           ^^^^^ type cannot refer to non-static member
//CHECK:           ^^^^^ type 'StaticSubscriptLookup' cannot be used to refer to non-static member 'value'
        }
    }
}
