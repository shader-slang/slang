// Alias requirements propagate across forward references and recursive aliases.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=SELF,non-exhaustive): -target hlsl -entry recursiveMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD,non-exhaustive): -target hlsl -entry forwardMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=MUTUAL,non-exhaustive): -target hlsl -entry mutualMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=EARLY_ENTRY,non-exhaustive): -target hlsl -entry earlyMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry recursiveMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry forwardMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry mutualMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry earlyMain -stage compute -no-codegen

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

struct Input<T>
{
}

struct ForwardEntryContext
{
    typealias Element = MetalOnly;
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
