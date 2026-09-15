//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Companion to https://github.com/shader-slang/slang/issues/12926.
//
// Exercises the path where an enum's inheritance is queried while the enum is still being
// checked: `E`'s base `IFoo<E>` carries the constraint `E : __EnumType`, whose check runs during
// `E`'s own base checking. That is a genuine cycle. The fix in `_calcInheritanceInfo` forces an
// enum to `ReadyForLookup` before linearizing, but must not force it here — doing so would emit a
// spurious `CyclicReference` on `E` and return without advancing. It skips when the enum is
// mid-check and lets the cycle be reported as usual.
interface IFoo<T> where T : __EnumType {}
enum E : IFoo<E>
//CHECK: cyclic reference
{
    a = 1,
    b = 2,
}
