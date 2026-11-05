//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

// Core-only builtin annotations must be rejected with error E31229 on a user declaration.
// Ordinary modules can use __magic_type only for the executable shader-stage interfaces.
// Each struct below is referenced from main to force type realization,
// where an unrejected annotation would build an invalid internal type node.

// Unregistered AST syntax-class name.
__magic_type(FloatType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyFloat
{
}

// Name that is not any AST syntax class.
__magic_type(NotARealMagicTypeName)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyUnknown
{
}

// Registered magic name: the non-null-node construction path.
__magic_type(ConstantBufferType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyConstantBuffer
{
}

// `BasicExpressionType::getBaseType` dereferences a `BuiltinTypeModifier` the user struct lacks.
__magic_type(BasicExpressionType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct MyBasic
{
}

// A stage interface AST type cannot be constructed around a struct declaration.
__magic_type(MissShaderType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct WrongStageKind
{
}

// Stage inputs remain ordinary structs; their old magic AST spelling is not available.
__magic_type(MissInputType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
interface WrongInputKind
{
}

// Abstract AST classes must never reach DeclRefType::create.
__magic_type(RayTracingStageInterfaceType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
interface AbstractStageKind
{
}

__magic_type(RayTracingStageInterfaceType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_type' can only be applied to declarations in the core module
*/
struct AbstractStructKind
{
}

__magic_enum(FloatType)
/*CHECK:
^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^ '__magic_enum' can only be applied to declarations in the core module
*/
struct MyEnum
{
}

__builtin_type(0)
/*CHECK:
^^^^^^^^^^^^^^ error E31229
^^^^^^^^^^^^^^ '__builtin_type' can only be applied to declarations in the core module
*/
struct MyBuiltin
{
}

interface IMyInterface
{
    __builtin_requirement(0)
    /*CHECK:
    ^^^^^^^^^^^^^^^^^^^^^ error E31229
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
    WrongStageKind wrongStage;
    AbstractStructKind abstractStruct;
    MyEnum e;
    MyBuiltin f;
    MyIntrinsic g;
}
