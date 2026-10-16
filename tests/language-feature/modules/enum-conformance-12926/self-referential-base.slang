//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Companion to https://github.com/shader-slang/slang/issues/12926.
//
// `E`'s base `IFoo<E>` carries the constraint `E : __EnumType`, so checking the base evaluates
// `E`'s conformance while `E` itself is still being checked. That is a genuine cycle, and it must
// be reported against the offending base (`$inheritance` / `IFoo<E>`), not against `E`: linearizing
// `E`'s inheritance while `E` is mid-check must not be turned into a cyclic-reference diagnostic on
// `E` itself. This is the mid-check path guarded in `_calcInheritanceInfo`.
interface IFoo<T> where T : __EnumType {}
enum E : IFoo<E>
//CHECK: cyclic reference '$inheritance'
{
    a = 1,
    b = 2,
}
