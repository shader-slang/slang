// Entry-point signature validation must honor the capability `[require(...)]`
// of a *generic* struct type the same way it does for a non-generic struct.
//
// Regression test for https://github.com/shader-slang/slang/issues/9489:
// `Foo<int>` used as a `vertex` entry-point parameter, where `Foo` requires the
// `cpp` capability, previously compiled silently for the `spirv` target. It now
// reports E36107 just as the non-generic spelling `Foo` always did.
//
// Each entry point uses its own diag tag because a `DIAGNOSTIC_TEST` invocation
// must match every annotation for its tag, and only the compiled entry point's
// diagnostics fire. PARAM, DUP, RET, MIXED, SPECIALIZE and BODYONLY are exhaustive, so each
// pins exactly one "see using of" note per use site. The other error invocations are
// non-exhaustive and pin only the notes they are about. Carets align to the
// columns of the preceding source line.

// Plain generic parameter.
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM): -target spirv -emit-spirv-directly -entry main -stage vertex
// Generic parameter nested inside a builtin wrapper (array).
//DIAGNOSTIC_TEST:SIMPLE(diag=ARR,non-exhaustive): -target spirv -emit-spirv-directly -entry mainArr -stage vertex
// Multiple contributing generic structs.
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTI,non-exhaustive): -target spirv -emit-spirv-directly -entry mainMulti -stage vertex
// Generic return type.
//DIAGNOSTIC_TEST:SIMPLE(diag=RET): -target spirv -emit-spirv-directly -entry mainRet -stage vertex
// A generic and a non-generic requiring struct side by side: each is reported at its own
// parameter.
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED): -target spirv -emit-spirv-directly -entry mainMixed -stage vertex
// A generic entry point whose specialization argument is a requiring generic struct.
//DIAGNOSTIC_TEST:SIMPLE(diag=SPECIALIZE): -target spirv -emit-spirv-directly -entry mainSpecialize -stage vertex -specialize Foo<int>
// Same generic struct on two parameters: both use sites must be reported (the
// hash-consed `Val` for `Foo<int>` must not collapse to a single note).
//DIAGNOSTIC_TEST:SIMPLE(diag=DUP): -target spirv -emit-spirv-directly -entry mainDup -stage vertex
// Generic struct nested inside a builtin generic wrapper (`ConstantBuffer<...>`).
//DIAGNOSTIC_TEST:SIMPLE(diag=CB,non-exhaustive): -target spirv -emit-spirv-directly -entry mainCB -stage vertex
// Generic struct nested inside `Optional<...>`.
//DIAGNOSTIC_TEST:SIMPLE(diag=OPT,non-exhaustive): -target spirv -emit-spirv-directly -entry mainOpt -stage vertex
// Generic struct used only as a generic *argument* of a non-requiring user
// wrapper (`Wrap<Foo<int>>`): the requirement comes solely from the inner `Foo`,
// exercising the generic-arg recursion path.
//DIAGNOSTIC_TEST:SIMPLE(diag=NEST,non-exhaustive): -target spirv -emit-spirv-directly -entry mainNest -stage vertex
// Generic struct reached only via a substituted *field* of a non-requiring user
// wrapper (`struct Field<T> { Foo<T> f; }` used as `Field<int>`): exercises the
// field-substitution recursion path.
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD,non-exhaustive): -target spirv -emit-spirv-directly -entry mainField -stage vertex
// Non-generic wrapper whose field is a generic requiring struct
// (`struct NonGenField { Foo<int> f; }`): also reached via field substitution.
//DIAGNOSTIC_TEST:SIMPLE(diag=NGFIELD,non-exhaustive): -target spirv -emit-spirv-directly -entry mainNgField -stage vertex
// `out` parameter direction must be checked the same as `in`.
//DIAGNOSTIC_TEST:SIMPLE(diag=OUT,non-exhaustive): -target spirv -emit-spirv-directly -entry mainOut -stage vertex
// `inout` parameter direction must be checked the same as `in`/`out`.
//DIAGNOSTIC_TEST:SIMPLE(diag=INOUT,non-exhaustive): -target spirv -emit-spirv-directly -entry mainInOut -stage vertex
// Multi-level wrapper nesting (`Wrap<Wrap<Foo<int>>>`): both recursion arms must
// keep descending and the depth guard must not fire too early.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEEP,non-exhaustive): -target spirv -emit-spirv-directly -entry mainDeep -stage vertex
// A requiring generic struct used only in the body of a clean-signature entry
// point (passed to a helper) is diagnosed by general capability inference rather
// than by the signature check, so its only note points at the helper call.
// generic-struct-require-in-body.slang covers other body uses.
//DIAGNOSTIC_TEST:SIMPLE(diag=BODYONLY): -target spirv -emit-spirv-directly -entry mainBodyOnly -stage vertex
// Positive case: the cpp requirement is satisfied for the cpp target, so the same
// code must compile. We assert no E36107 *and* that compilation reaches code
// emission (the entry-point rename warning E40100 only fires on success), so the
// case cannot pass by failing for some unrelated reason.
//DIAGNOSTIC_TEST:SIMPLE(diag=CPP): -target cpp -entry main -stage vertex
//CPP-NOT: E36107
//CPP: E40100
// Profile-implicit-upgrade path: a generic struct requiring a higher shader model
// than the requested profile contributes to the "implicitly upgraded" warning
// (this path uses the augmented capability set, not the bare entry-point set).
//DIAGNOSTIC_TEST:SIMPLE(diag=UPGRADE,non-exhaustive): -target hlsl -profile sm_5_0 -capability hlsl -entry mainUpgrade -stage compute
//UPGRADE: E41012
//UPGRADE: sm_6_5

[require(cpp)]
/*PARAM:
 ^^^^^^^ see declaration of 'require'
*/
//DUP: see declaration of 'require'
//DUP: see declaration of 'require'
struct Foo<T>
/*PARAM:
       ^^^ see definition of 'Foo'
*/
//DUP: see definition of 'Foo'
//DUP: see definition of 'Foo'
{
    int x;
}

[require(cpp)]
struct Bar<T>
{
    int y;
}

[require(cpp)]
struct Baz
{
    int z;
}

// A generic wrapper with no capability requirement of its own; used to verify the
// requirement is found when `Foo<int>` appears only as a generic argument.
struct Wrap<T>
{
    T x;
}

// A generic wrapper with no requirement of its own that *contains* a `Foo<T>`
// field; used to verify the requirement is found via field substitution.
struct Field<T>
{
    Foo<T> f;
}

// A non-generic wrapper whose field is a generic requiring struct; the
// requirement must still be found via the field's substituted type.
struct NonGenField
{
    Foo<int> f;
}

[shader("vertex")]
void main(Foo<int> a)
/*PARAM:
     ^^^^ unavailable features in entry point
     ^^^^ entrypoint 'main' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
          ^^^ see using of 'Foo'
*/
{
}

[shader("vertex")]
void mainArr(Foo<int> a[4])
/*ARR:
     ^^^^^^^ unavailable features in entry point
*/
//ARR: see using of 'Foo'
{
}

[shader("vertex")]
void mainMulti(Foo<int> a, Bar<int> b)
/*MULTI:
     ^^^^^^^^^ unavailable features in entry point
               ^^^ see using of 'Foo'
                           ^^^ see using of 'Bar'
*/
{
}

[shader("vertex")]
void mainDup(Foo<int> a, Foo<int> b)
/*DUP:
     ^^^^^^^ unavailable features in entry point
     ^^^^^^^ entrypoint 'mainDup' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
             ^^^ see using of 'Foo'
                         ^^^ see using of 'Foo'
*/
{
}

[shader("vertex")]
void mainCB(ConstantBuffer<Foo<int> > a)
/*CB:
     ^^^^^^ unavailable features in entry point
*/
//CB: see using of 'Foo'
{
}

[shader("vertex")]
void mainOpt(Optional<Foo<int> > a)
/*OPT:
     ^^^^^^^ unavailable features in entry point
*/
//OPT: see using of 'Foo'
{
}

[shader("vertex")]
void mainNest(Wrap<Foo<int> > a)
/*NEST:
     ^^^^^^^^ unavailable features in entry point
*/
//NEST: see using of 'Foo'
{
}

[shader("vertex")]
void mainField(Field<int> a)
/*FIELD:
     ^^^^^^^^^ unavailable features in entry point
*/
//FIELD: see using of 'Foo'
{
}

[shader("vertex")]
void mainNgField(NonGenField a)
/*NGFIELD:
     ^^^^^^^^^^^ unavailable features in entry point
*/
//NGFIELD: see using of 'Foo'
{
}

[shader("vertex")]
void mainOut(out Foo<int> a)
/*OUT:
     ^^^^^^^ unavailable features in entry point
*/
//OUT: see using of 'Foo'
{
    a = Foo<int>();
}

[shader("vertex")]
void mainInOut(inout Foo<int> a)
/*INOUT:
     ^^^^^^^^^ unavailable features in entry point
*/
//INOUT: see using of 'Foo'
{
}

[shader("vertex")]
void mainDeep(Wrap<Wrap<Foo<int> > > a)
/*DEEP:
     ^^^^^^^^ unavailable features in entry point
*/
//DEEP: see using of 'Foo'
{
}

void helper(Foo<int> a)
{
}

[shader("vertex")]
void mainBodyOnly(int x)
/*BODYONLY:
     ^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^ entrypoint 'mainBodyOnly' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
*/
{
    helper(Foo<int>());
/*BODYONLY:
    ^^^^^^ see using of 'Foo'
*/
}
//BODYONLY: see definition of 'Foo'
//BODYONLY: see declaration of 'require'

[shader("vertex")]
Foo<int> mainRet()
/*RET:
         ^^^^^^^ unavailable features in entry point
         ^^^^^^^ entrypoint 'mainRet' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
^^^ see using of 'Foo'
*/
{
    Foo<int> f = { 0 };
    return f;
}
//RET: see definition of 'Foo'
//RET: see declaration of 'require'

[shader("vertex")]
void mainMixed(Foo<int> a, Baz b)
/*MIXED:
     ^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^ entrypoint 'mainMixed' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
               ^^^ see using of 'Foo'
                           ^^^ see using of 'Baz'
*/
{
}
//MIXED: see definition of 'Foo'
//MIXED: see definition of 'Baz'
//MIXED: see declaration of 'require'
//MIXED: see declaration of 'require'

[shader("vertex")]
void mainSpecialize<T>()
/*SPECIALIZE:
     ^^^^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^^^^ entrypoint 'mainSpecialize' uses features that are not available in 'vertex' stage for 'spirv' compilation target.
*/
{
}
//SPECIALIZE: see using of 'Foo'
//SPECIALIZE: see definition of 'Foo'
//SPECIALIZE: see declaration of 'require'

// A generic struct requiring a higher shader model; used to exercise the
// profile-implicit-upgrade path rather than the hard incompatibility path.
[require(hlsl, sm_6_5)]
struct NeedsSM65<T>
{
    T x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainUpgrade(NeedsSM65<int> p)
{
}
