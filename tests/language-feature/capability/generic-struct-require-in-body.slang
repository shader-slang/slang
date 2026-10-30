// A `[require(...)]` on a user-defined generic type must be enforced wherever the
// type is used, not only in entry-point signatures, exactly as for the non-generic
// spelling. The `*NG` cases are non-generic twins: GENERICNG, ENUMNG, NESTEDENUMNG,
// STATICNG, OUTERMAKENG and INTRINSICNG must report the same diagnostics as the rows they
// follow. LATENG and COREINTRINSIC record known gaps instead, so their expected output is
// empty: a non-generic type named only as a generic argument before its declaration is still
// dropped, and so is a core-module intrinsic generic such as `CoopVec<float, 4>`.
//
// Regression test for https://github.com/shader-slang/slang/issues/13579.
//
// Each entry point uses its own diag tag because only the compiled entry point's
// diagnostics fire, and every invocation is exhaustive so an unexpected extra
// diagnostic fails the test. Several rows reach the type by two paths. In GENERIC,
// `read(gInput)` names `MetalOnly<3>` both through the callee's type and through
// `gInput`, whose own declaration requires `metal`. The callee is visited first, so
// the "see using of" caret sits on `read`; if the generic-argument path regressed,
// the caret would move to `gInput` rather than the error disappearing. The caret is
// what pins the path under test.

// Field read of a `uniform` of the generic struct type.
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD): -target hlsl -profile sm_6_5 -entry mainField -stage compute -no-codegen
// The issue's reproducer: the value is only passed to a generic helper.
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC): -target hlsl -profile sm_6_5 -entry mainGenericHelper -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERICNG): -target hlsl -profile sm_6_5 -entry mainGenericHelperNG -stage compute -no-codegen
// A local variable of the generic struct type, never constructed.
//DIAGNOSTIC_TEST:SIMPLE(diag=LOCAL): -target hlsl -profile sm_6_5 -entry mainLocal -stage compute -no-codegen
// Element read through a builtin generic resource wrapping the struct.
//DIAGNOSTIC_TEST:SIMPLE(diag=BUFFER): -target hlsl -profile sm_6_5 -entry mainBuffer -stage compute -no-codegen
// A requiring non-generic struct nested in a generic parent (`Outer<3>.In`), whose
// decl-ref is a member of a specialized generic rather than a direct reference.
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target hlsl -profile sm_6_5 -entry mainNested -stage compute -no-codegen
// A requiring generic struct nested in a non-generic parent (`Holder.In<3>`).
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTEDGEN): -target hlsl -profile sm_6_5 -entry mainNestedGeneric -stage compute -no-codegen
// The struct appears in a non-generic helper's parameter type (`helper(MetalOnly<3> a)`). The
// caret on `helper` pins the path through the callee's signature, where GENERIC's caret pins the
// generic argument.
//DIAGNOSTIC_TEST:SIMPLE(diag=HELPERSIG): -target hlsl -profile sm_6_5 -entry mainHelperSig -stage compute -no-codegen
// A static member accessed through the specialized struct (`MetalOnly<3>.make()`).
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target hlsl -profile sm_6_5 -entry mainStatic -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=STATICNG): -target hlsl -profile sm_6_5 -entry mainStaticNG -stage compute -no-codegen
// A static member of a requiring struct nested in a generic parent (`Outer<3>.In.make()`).
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTERMAKE): -target hlsl -profile sm_6_5 -entry mainOuterMake -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTERMAKENG): -target hlsl -profile sm_6_5 -entry mainOuterMakeNG -stage compute -no-codegen
// The struct is named only as a generic argument, before its declaration in the file.
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE): -target hlsl -profile sm_6_5 -entry mainLate -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=LATENG): -target hlsl -profile sm_6_5 -entry mainLateNG -stage compute -no-codegen
// A requiring `__intrinsic_type` declared outside the core module.
//DIAGNOSTIC_TEST:SIMPLE(diag=INTRINSIC): -target hlsl -profile sm_6_5 -entry mainIntrinsic -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=INTRINSICNG): -target hlsl -profile sm_6_5 -entry mainIntrinsicNG -stage compute -no-codegen
// A requiring generic enum, passed to a generic helper.
//DIAGNOSTIC_TEST:SIMPLE(diag=ENUM): -target hlsl -profile sm_6_5 -entry mainEnum -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=ENUMNG): -target hlsl -profile sm_6_5 -entry mainEnumNG -stage compute -no-codegen
// A case of a requiring enum nested in a generic parent (`Outer<3>.E.A`).
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTEDENUM): -target hlsl -profile sm_6_5 -entry mainNestedEnum -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTEDENUMNG): -target hlsl -profile sm_6_5 -entry mainNestedEnumNG -stage compute -no-codegen
// Positive case: every entry point compiles for the target the struct requires.
// A core-module intrinsic generic is excluded from requirement inference (see
// `isCoreModuleIntrinsicTypeDecl`), so naming `CoopVec<float, 4>` only as a generic argument
// is not diagnosed on a target without cooperative vectors, and the expected output is empty.
//DIAGNOSTIC_TEST:SIMPLE(diag=COREINTRINSIC): -target metal -entry mainCoreIntrinsic -stage compute -no-codegen
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

[require(metal)]
struct MetalOnlyNG
{
    uint value;

    static uint make()
    {
        return 1;
    }
}

[require(metal)]
enum GenericEnum<let N : int>
{
    A,
    B
}

[require(metal)]
enum PlainEnum
{
    A,
    B
}

struct Outer<let N : int>
{
    [require(metal)]
    struct In
    {
        uint value;

        static uint make()
        {
            return 1;
        }
    }

    [require(metal)]
    enum E
    {
        A,
        B
    }
}

struct Holder
{
    [require(metal)]
    struct In<let N : int>
    {
        uint value;
    }

    [require(metal)]
    enum E
    {
        A,
        B
    }
}

struct PlainHolder
{
    [require(metal)]
    struct In
    {
        uint value;

        static uint make()
        {
            return 1;
        }
    }
}

uniform MetalOnly<3> gInput;
uniform MetalOnlyNG gInputNG;
uniform GenericEnum<3> gEnum;
uniform PlainEnum gEnumNG;
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
void mainGenericHelperNG()
/*GENERICNG:
     ^^^^^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^^^^^ entrypoint 'mainGenericHelperNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gInputNG);
//GENERICNG:     ^^^^ see using of 'MetalOnlyNG'
}
//GENERICNG: see definition of 'MetalOnlyNG'
//GENERICNG: see declaration of 'require'

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
//STATIC:        ^^^^^^^^^ see using of 'MetalOnly'
}
//STATIC: see definition of 'MetalOnly'
//STATIC: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainStaticNG()
/*STATICNG:
     ^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^ entrypoint 'mainStaticNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = MetalOnlyNG.make();
//STATICNG:      ^^^^^^^^^^^ see using of 'MetalOnlyNG'
}
//STATICNG: see definition of 'MetalOnlyNG'
//STATICNG: see declaration of 'require'

uint typeOnly<T>()
{
    return 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainOuterMake()
/*OUTERMAKE:
     ^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^ entrypoint 'mainOuterMake' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = Outer<3>.In.make();
//OUTERMAKE:              ^^ see using of 'In'
}
//OUTERMAKE: see definition of 'In'
//OUTERMAKE: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainOuterMakeNG()
/*OUTERMAKENG:
     ^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^ entrypoint 'mainOuterMakeNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = PlainHolder.In.make();
//OUTERMAKENG:               ^^ see using of 'In'
}
//OUTERMAKENG: see definition of 'In'
//OUTERMAKENG: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainCoreIntrinsic()
{
    gOutput[0] = typeOnly<CoopVec<float, 4> >();
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

[shader("compute")]
[numthreads(1, 1, 1)]
void mainLateNG()
{
    gOutput[0] = typeOnly<DeclaredLaterNG>();
}

[require(metal)]
__intrinsic_type(UInt)
struct UserIntrinsic<let N : int>
{
}

[require(metal)]
__intrinsic_type(UInt)
struct UserIntrinsicNG
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainIntrinsic()
/*INTRINSIC:
     ^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^ entrypoint 'mainIntrinsic' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = typeOnly<UserIntrinsic<3> >();
//INTRINSIC:     ^^^^^^^^ see using of 'UserIntrinsic'
}
//INTRINSIC: see definition of 'UserIntrinsic'
//INTRINSIC: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainIntrinsicNG()
/*INTRINSICNG:
     ^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^ entrypoint 'mainIntrinsicNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = typeOnly<UserIntrinsicNG>();
//INTRINSICNG:   ^^^^^^^^ see using of 'UserIntrinsicNG'
}
//INTRINSICNG: see definition of 'UserIntrinsicNG'
//INTRINSICNG: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainEnum()
/*ENUM:
     ^^^^^^^^ unavailable features in entry point
     ^^^^^^^^ entrypoint 'mainEnum' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gEnum);
//ENUM:          ^^^^ see using of 'GenericEnum'
}
//ENUM: see definition of 'GenericEnum'
//ENUM: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainEnumNG()
/*ENUMNG:
     ^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^ entrypoint 'mainEnumNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = read(gEnumNG);
//ENUMNG:        ^^^^ see using of 'PlainEnum'
}
//ENUMNG: see definition of 'PlainEnum'
//ENUMNG: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNestedEnum()
/*NESTEDENUM:
     ^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^ entrypoint 'mainNestedEnum' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = (uint)Outer<3>.E.A;
//NESTEDENUM:                     ^ see using of 'E'
}
//NESTEDENUM: see definition of 'E'
//NESTEDENUM: see declaration of 'require'

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNestedEnumNG()
/*NESTEDENUMNG:
     ^^^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^^^ entrypoint 'mainNestedEnumNG' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    gOutput[0] = (uint)Holder.E.A;
//NESTEDENUMNG:                 ^ see using of 'E'
}
//NESTEDENUMNG: see definition of 'E'
//NESTEDENUMNG: see declaration of 'require'

[require(metal)]
struct DeclaredLater<let N : int>
{
    uint value;
}

[require(metal)]
struct DeclaredLaterNG
{
    uint value;
}
