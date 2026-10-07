//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -DINVALID=1
//DIAGNOSTIC_TEST:SIMPLE(diag=VALID): -DINVALID=0

interface IFoo
{
    property float3 X { get; }
    property float3 Y { get; }
}

struct Foo : IFoo
{
#if INVALID
    property override float3 X { get; }
//CHECK:     ^^^^^^^^ modifier not allowed
//CHECK:     ^^^^^^^^ modifier 'override' is not allowed here.
    override property float3 Y { get; }
/*CHECK:
    ^^^^^^^^ modifier not allowed
    ^^^^^^^^ modifier 'override' is not allowed here.
*/
#else
    property float3 X { get; }
    property float3 Y { get; }
#endif
}

#if INVALID
struct InvalidModifiers
{
    // Declaration modifiers are invalid in both property type syntaxes.
    property static float3 X { get; }
//CHECK:     ^^^^^^ modifier not allowed
//CHECK:     ^^^^^^ modifier 'static' is not allowed here.
    property Y : override float3 { get; }
//CHECK:         ^^^^^^^^ modifier not allowed
//CHECK:         ^^^^^^^^ modifier 'override' is not allowed here.

    // The same semantic check also handles types outside properties.
    var value : public int;
//CHECK:        ^^^^^^ modifier not allowed
//CHECK:        ^^^^^^ modifier 'public' is not allowed here.
}
#endif

public struct ValidModifiers
{
    public property float3 X { get; }
    property unorm float3 U { get; }
    property snorm float3 S { get; }
    property no_diff float3 N { get; }
    property row_major float3x3 R { get; }
    property C : column_major float3x3 { get; }
}
