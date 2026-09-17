//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression for the re-entrancy guard on the enum tag-conversion sites in
// slang-check-conversion.cpp (raised in review of issue #12443).
//
// `E`'s base `IHasE<E(0)>` constructs an `E` value from an integer (the int -> enum conversion
// site) while `E` itself is still being checked -- a genuine cycle. Driving a mid-check enum to
// `ReadyForLookup` from the conversion site used to abort compilation with an internal error
// (E99997). The `isBeingChecked` guard defers to the checking machinery instead, so the cycle is
// reported cleanly against the offending base (`$inheritance` / `IHasE<E(0)>`) rather than as a
// spurious diagnostic on `E` or an abort. This mirrors the guard PR #12927 added in
// `_calcInheritanceInfo` for `enum E : IFoo<E>` with `IFoo<T : __EnumType>`.
interface IHasE<let V : E> {}
enum E : IHasE<E(0)>
//CHECK: cyclic reference '$inheritance'
{
    a = 0,
    b = 1,
}
