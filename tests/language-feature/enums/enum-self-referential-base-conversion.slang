//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `E`'s base `IHasE<E(0)>` constructs an `E` value from an integer while `E` itself is still
// being checked, so resolving the base runs an int -> enum conversion on a mid-check enum. That
// is a genuine cycle and must be reported as a clean cyclic-reference diagnostic against the
// offending base (`$inheritance` / `IHasE<E(0)>`), never as a compilation abort. The int -> enum
// conversion site in slang-check-conversion.cpp only drives an enum to `ReadyForLookup` when it is
// not already being checked, so it does not force an in-progress enum here; `_calcInheritanceInfo`
// guards the enum-inheritance path the same way.
interface IHasE<let V : E> {}
enum E : IHasE<E(0)>
//CHECK: cyclic reference '$inheritance'
{
    a = 0,
    b = 1,
}
