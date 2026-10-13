// A `[require(...)]` on a user-defined generic struct must be enforced wherever the
// struct is used, not only in entry-point signatures: the non-generic spelling of
// each use below has always reported E36107.
//
// Regression test for https://github.com/shader-slang/slang/issues/13579.
//
// Each entry point uses its own diag tag because only the compiled entry point's
// diagnostics fire, and every invocation is exhaustive so an unexpected extra
// diagnostic fails the test.

// Field read of a `uniform` of the generic struct type.
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target hlsl -profile sm_6_5 -entry mainField -stage compute -no-codegen
// The issue's reproducer: the value is only passed to a generic helper.
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target hlsl -profile sm_6_5 -entry mainGenericHelper -stage compute -no-codegen
// A local variable of the generic struct type, never constructed.
//DIAGNOSTIC_TEST:SIMPLE(diag=LOCAL): -target hlsl -profile sm_6_5 -entry mainLocal -stage compute -no-codegen
// Element read through a builtin generic resource wrapping the struct.
//DIAGNOSTIC_TEST:SIMPLE(diag=BUFFER): -target hlsl -profile sm_6_5 -entry mainBuffer -stage compute -no-codegen
// A requiring non-generic struct nested in a generic parent (`Outer<3>.In`), whose
// decl-ref is a member of a specialized generic rather than a direct reference.
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target hlsl -profile sm_6_5 -entry mainNested -stage compute -no-codegen
// A requiring generic struct nested in a non-generic parent (`Holder.In<3>`).
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTEDGEN): -target hlsl -profile sm_6_5 -entry mainNestedGeneric -stage compute -no-codegen
// The struct appears only in a non-entry-point helper's signature; the value is
// never constructed, so no synthesized `init` carries the requirement.
//DIAGNOSTIC_TEST:SIMPLE(diag=HELPERSIG): -target hlsl -profile sm_6_5 -entry mainHelperSig -stage compute -no-codegen
// A static member accessed through the specialized struct (`MetalOnly<3>.make()`).
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target hlsl -profile sm_6_5 -entry mainStatic -stage compute -no-codegen
// The struct is named only as a generic argument, before its declaration in the file.
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE): -target hlsl -profile sm_6_5 -entry mainLate -stage compute -no-codegen
// Positive case: every entry point compiles for the target the struct requires.
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL): -target metal -no-codegen

[require(metal)]
struct MetalOnly<let N : int>
{
    uint value;

    static uint make()
    {
        return 1;
    }
}

struct Outer<let N : int>
{
    [require(metal)]
    struct In
    {
        uint value;
    }
}

struct Holder
{
    [require(metal)]
    struct In<let N : int>
    {
        uint value;
    }
}

uniform MetalOnly<3> gInput;
uniform Outer<3>.In gNested;
uniform Holder.In<3> gNestedGeneric;
StructuredBuffer<MetalOnly<3> > gBuffer;
RWStructuredBuffer<uint> gOutput;

uint read<T>(T x)
{
    return 1;
}

uint helper(MetalOnly<3> a)
{
    return a.value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainField()
/*FIELD:
     ^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^ entrypoint 'mainField' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = gInput.value;
//FIELD:         ^^^^^^ see using of 'MetalOnly'
}
//FIELD: see definition of 'MetalOnly'
//FIELD: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainGenericHelper()
/*GENERIC:
     ^^^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^^^ entrypoint 'mainGenericHelper' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gInput);
//GENERIC:       ^^^^ see using of 'MetalOnly'
}
//GENERIC: see definition of 'MetalOnly'
//GENERIC: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainLocal()
/*LOCAL:
     ^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^ entrypoint 'mainLocal' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    MetalOnly<3> x;
/*LOCAL:
    ^^^^^^^^^ see using of 'MetalOnly'
*/
    x.value = 1;
    gOutput[0] = x.value;
}
//LOCAL: see definition of 'MetalOnly'
//LOCAL: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainBuffer()
/*BUFFER:
     ^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^ entrypoint 'mainBuffer' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = gBuffer[0].value;
//BUFFER:        ^^^^^^^ see using of 'MetalOnly'
}
//BUFFER: see definition of 'MetalOnly'
//BUFFER: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNested()
/*NESTED:
     ^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^ entrypoint 'mainNested' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gNested);
//NESTED:        ^^^^ see using of 'In'
}
//NESTED: see definition of 'In'
//NESTED: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNestedGeneric()
/*NESTEDGEN:
     ^^^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^^^ entrypoint 'mainNestedGeneric' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gNestedGeneric);
//NESTEDGEN:     ^^^^ see using of 'In'
}
//NESTEDGEN: see definition of 'In'
//NESTEDGEN: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainHelperSig()
/*HELPERSIG:
     ^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^ entrypoint 'mainHelperSig' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = helper(gInput);
//HELPERSIG:     ^^^^^^ see using of 'MetalOnly'
}
//HELPERSIG: see definition of 'MetalOnly'
//HELPERSIG: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainStatic()
/*STATIC:
     ^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^ entrypoint 'mainStatic' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = MetalOnly<3>.make();
//STATIC:                     ^^^^ see using of 'MetalOnly'
}
//STATIC: see definition of 'MetalOnly'
//STATIC: see declaration of 'require'

uint typeOnly<T>()
{
    return 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainLate()
/*LATE:
     ^^^^^^^^ unavailable features in entry point
     ^^^^^^^^ entrypoint 'mainLate' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = typeOnly<DeclaredLater<3> >();
//LATE:          ^^^^^^^^ see using of 'DeclaredLater'
}
//LATE: see definition of 'DeclaredLater'
//LATE: see declaration of 'require'

[require(metal)]
struct DeclaredLater<let N : int>
{
    uint value;
}
