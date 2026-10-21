// A struct's type aliases contribute their requirements even when the struct is
// used only as a generic argument and the alias is never named by the entry point.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,non-exhaustive): -target hlsl -entry main -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT,non-exhaustive): -target hlsl -entry directMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=TRANSITIVE,non-exhaustive): -target hlsl -entry transitiveMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SUBSTITUTED,non-exhaustive): -target hlsl -entry substitutedMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS_GENERIC,non-exhaustive): -target hlsl -entry aliasGenericMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SELF,non-exhaustive): -target hlsl -entry recursiveMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD,non-exhaustive): -target hlsl -entry forwardMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=MUTUAL,non-exhaustive): -target hlsl -entry mutualMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=EARLY_ENTRY,non-exhaustive): -target hlsl -entry earlyMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFORMANCE,non-exhaustive): -target hlsl -entry conformanceMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=EXTENSION_CONFORMANCE,non-exhaustive): -target hlsl -entry extensionConformanceMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=STAGE,non-exhaustive): -target hlsl -entry stageMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry main -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry transitiveMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry substitutedMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry aliasGenericMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry recursiveMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry forwardMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry mutualMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry earlyMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry methodMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry staticMethodMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry selfConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry selfConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry conformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry extensionConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry fragmentMain -stage fragment -no-codegen

// Entry-point validation must infer requirements before reading types declared later.
[numthreads(1, 1, 1)]
void earlyMain(uniform Input<ForwardEntryContext> input)
/*EARLY_ENTRY:
     ^^^^^^^^^ error E36107
*/
{
}

[require(metal)]
struct MetalOnly
{
}

struct Context
{
    typealias Element = MetalOnly;
}

struct Input<T>
{
}

// Explicit stage requirements on aliased types apply just like target requirements.
[require(fragment)]
struct FragmentOnly
{
}

struct FragmentContext
{
    typealias Element = FragmentOnly;
}

[numthreads(1, 1, 1)]
void stageMain(uniform Input<FragmentContext> input)
/*STAGE:
     ^^^^^^^^^ error E36107
*/
{
}

void fragmentMain(uniform Input<FragmentContext> input)
{
}

struct ForwardEntryContext
{
    typealias Element = MetalOnly;
}

[numthreads(1, 1, 1)]
void main(uniform Input<Context> input)
/*GENERIC:
     ^^^^ error E36107
*/
{
}

[numthreads(1, 1, 1)]
void directMain(uniform Context input)
/*DIRECT:
     ^^^^^^^^^^ error E36107
*/
{
}

struct OuterContext
{
    typealias Inner = Context;
}

[numthreads(1, 1, 1)]
void transitiveMain(uniform Input<OuterContext> input)
/*TRANSITIVE:
     ^^^^^^^^^^^^^^ error E36107
*/
{
}

[require(metal)]
struct MetalElement<T>
{
}

struct GenericContext<T>
{
    typealias Element = MetalElement<T>;
}

[numthreads(1, 1, 1)]
void substitutedMain(uniform GenericContext<int> input)
/*SUBSTITUTED:
     ^^^^^^^^^^^^^^^ error E36107
*/
{
}

// The alias's generic target has requirements beyond those of its argument `int`.
struct AliasOfGeneric
{
    typealias Element = GenericContext<int>;
}

[numthreads(1, 1, 1)]
void aliasGenericMain(uniform Input<AliasOfGeneric> input)
/*ALIAS_GENERIC:
     ^^^^^^^^^^^^^^^^ error E36107
*/
{
}

// Following a self alias must terminate and still collect subsequent requirements.
struct RecursiveContext
{
    typealias Self = RecursiveContext;
    typealias Element = MetalOnly;
}

[numthreads(1, 1, 1)]
void recursiveMain(uniform Input<RecursiveContext> input)
/*SELF:
     ^^^^^^^^^^^^^ error E36107
*/
{
}

// Requirements must propagate even when the dependency is declared later.
struct ForwardContext
{
    typealias Element = LaterContext;
}

struct LaterContext
{
    typealias Element = MetalOnly;
}

[numthreads(1, 1, 1)]
void forwardMain(uniform Input<ForwardContext> input)
/*FORWARD:
     ^^^^^^^^^^^ error E36107
*/
{
}

// B can be visited before A's restricted alias is checked. Both structs must
// eventually receive that requirement, independent of this declaration order.
struct A
{
    typealias BAlias = B;
    typealias Restricted = MetalOnly;
}

struct B
{
    typealias AAlias = A;
}

[numthreads(1, 1, 1)]
void mutualMain(uniform Input<B> input)
/*MUTUAL:
     ^^^^^^^^^^ error E36107
*/
{
}

// An unused restricted method does not restrict its containing struct's type.
struct MethodContext
{
    [require(metal)]
    void metalMethod()
    {
    }
}

[numthreads(1, 1, 1)]
void methodMain(uniform Input<MethodContext> input)
{
}

[require(hlsl)]
void hlslHelper()
{
}

// The alias infers a requirement on the struct's type, but does not declare that
// requirement on its static method. The method only promises support for HLSL.
public struct StaticMethodContext
{
    typealias Element = MetalOnly;

    [require(hlsl)]
    public static void hlslMethod()
    {
        hlslHelper();
    }
}

[numthreads(1, 1, 1)]
void staticMethodMain()
{
    StaticMethodContext.hlslMethod();
}

interface IAliasValue
{
}

struct AliasHolder<T : IAliasValue>
{
}

// The alias uses its enclosing type's conformance. Inferring its requirements must
// not request conformance validation, which requires those requirements first.
struct SelfConformingAlias : IAliasValue
{
    typealias Box = AliasHolder<SelfConformingAlias>;
}

[numthreads(1, 1, 1)]
void selfConformanceMain(uniform Input<SelfConformingAlias> input)
{
}

[require(metal)]
interface IMetalAliasValue
{
}

struct MetalAliasValue : IMetalAliasValue
{
}

struct MetalAliasHolder<T : IMetalAliasValue>
{
}

// Reading a conformance's type dependencies still preserves their requirements.
struct ConstrainedAliasContext
{
    typealias Box = MetalAliasHolder<MetalAliasValue>;
}

[numthreads(1, 1, 1)]
void conformanceMain(uniform Input<ConstrainedAliasContext> input)
/*CONFORMANCE:
     ^^^^^^^^^^^^^^^ error E36107
*/
{
}

struct ExtensionAliasValue<T>
{
}

interface IExtensionAliasValue
{
}

extension<T : IMetalAliasValue> ExtensionAliasValue<T> : IExtensionAliasValue
{
}

struct ExtensionAliasHolder<T : IExtensionAliasValue>
{
}

// The outer conformance's endpoint types are unrestricted. Its generic extension
// argument carries another witness whose supertype supplies the Metal requirement.
struct ExtensionAliasContext
{
    typealias Box = ExtensionAliasHolder<ExtensionAliasValue<MetalAliasValue>>;
}

[numthreads(1, 1, 1)]
void extensionConformanceMain(uniform Input<ExtensionAliasContext> input)
/*EXTENSION_CONFORMANCE:
     ^^^^^^^^^^^^^^^^^^^^^^^^ error E36107
*/
{
}
