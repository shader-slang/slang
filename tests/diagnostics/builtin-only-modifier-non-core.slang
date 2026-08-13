//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

// Builtin-only modifiers (`__magic_type`/`__magic_enum`, `__builtin_type`,
// `__builtin_requirement`) are valid only on core-module declarations and must be rejected with
// error E31228 on a user declaration. Each struct is referenced from `main` to force its type to
// be realized, which is the point at which an unrejected `__magic_type` would build an internal
// type node around a bare user struct.

// Unregistered AST syntax-class name.
__magic_type(FloatType)
/*CHECK:
^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyFloat
{
}

// Name that is not any AST syntax class.
__magic_type(NotARealMagicTypeName)
/*CHECK:
^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyUnknown
{
}

// Registered magic name: the non-null-node construction path.
__magic_type(ConstantBufferType)
/*CHECK:
^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyConstantBuffer
{
}

// `BasicExpressionType::getBaseType` dereferences a `BuiltinTypeModifier` the user struct lacks.
__magic_type(BasicExpressionType)
/*CHECK:
^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyBasic
{
}

__magic_enum(FloatType)
/*CHECK:
^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^ '__magic_enum' can only be applied to declarations in the core module
*/
struct MyEnum
{
}

__builtin_type(0)
/*CHECK:
^^^^^^^^^^^^^^ error E31228
^^^^^^^^^^^^^^ '__builtin_type' can only be applied to declarations in the core module
*/
struct MyBuiltin
{
}

interface IMyInterface
{
    __builtin_requirement(0)
    /*CHECK:
    ^^^^^^^^^^^^^^^^^^^^^ error E31228
    ^^^^^^^^^^^^^^^^^^^^^ '__builtin_requirement' can only be applied to declarations in the core module
    */
    void f();
}

// Negative control: `__intrinsic_type` is valid user syntax and must be accepted
// (see tests/language-feature/enums/strongly-typed-id.slang).
__intrinsic_type(UInt)
struct MyIntrinsic
{
}

[numthreads(1, 1, 1)]
void main()
{
    MyFloat a;
    MyUnknown b;
    MyConstantBuffer c;
    MyBasic d;
    MyEnum e;
    MyBuiltin f;
    MyIntrinsic g;
}
