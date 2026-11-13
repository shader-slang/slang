// Alias requirements include conformance dependencies without prematurely validating them.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFORMANCE,non-exhaustive): -target hlsl -entry conformanceMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=EXTENSION_CONFORMANCE,non-exhaustive): -target hlsl -entry extensionConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry selfConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry selfConformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry conformanceMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry extensionConformanceMain -stage compute -no-codegen

struct Input<T>
{
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
