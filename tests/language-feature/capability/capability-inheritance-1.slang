//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// Test if InheritanceDecl fail to conform to parent capabilities

// mismatch of target abstract atoms
[require(hlsl)]
interface IFoo0
{
}
[require(hlsl)]
[require(spirv)]
struct Foo0 : IFoo0
/*CHECK:
       ^^^^ dependencies not compatible on target
       ^^^^ 'Foo0' has dependencies that are not compatible on the required compilation target 'spirv'.
              ^^^^^ see declaration of 'IFoo0'
*/
{
}

// mismatch of target abstract atoms
[require(hlsl)]
[require(spirv)]
interface IFoo1
{
}
[require(hlsl)]
struct Foo1 : IFoo1
/*CHECK:
       ^^^^ subtype missing target/stage support
       ^^^^ subtype 'Foo1' must have the same target/stage support as the supertype; 'Foo1' is missing 'spirv'
              ^^^^^ see declaration of 'IFoo1'
*/
{
}

// subtype is superset
[require(sm_4_0)]
interface IFoo2
{
}
[require(sm_5_0)]
struct Foo2 : IFoo2
/*CHECK:
       ^^^^ capability incompatible with supertype
       ^^^^ 'Foo2' uses capability 'sm_5_0' that is incompatable with the supertype
              ^^^^^ see declaration of 'IFoo2'
*/
{
}

// subtype is subset, no error
[require(sm_5_0)]
interface IFoo3
{
}
[require(sm_4_0)]
struct Foo3 : IFoo3
{
}

// mismatch of stage abstract atoms
[require(hlsl, compute)]
interface IFoo4
{
}
[require(hlsl, fragment)]
struct Foo4 : IFoo4
/*CHECK:
       ^^^^ subtype missing target/stage support
       ^^^^ subtype 'Foo4' must have the same target/stage support as the supertype; 'Foo4' is missing 'compute'
              ^^^^^ see declaration of 'IFoo4'
*/
{
}

// mismatch of stage abstract atoms
[require(fragment)]
[require(vertex)]
interface IFoo5
{
}
[require(fragment)]
struct Foo5 : IFoo5
/*CHECK:
       ^^^^ subtype missing target/stage support
       ^^^^ subtype 'Foo5' must have the same target/stage support as the supertype; 'Foo5' is missing 'vertex'
              ^^^^^ see declaration of 'IFoo5'
*/
{
}

[require(hlsl)]
[numthreads(1,1,1)]
void computeMain()
{
}
