//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Companion to enum-self-referential-base-conversion.slang, for the enum -> tag conversion site.
// `E`'s base `IHas<int(E.a)>` coerces an `E` value to its tag type while `E` is still being
// checked, so resolving the base runs an enum -> tag conversion on a mid-check enum. That is a
// genuine cycle and must produce a clean cyclic-reference diagnostic, never a compilation abort.
// The enum -> tag conversion site in slang-check-conversion.cpp does not drive a mid-check enum to
// `ReadyForLookup`, so the checking machinery owns the cyclic-reference report.
interface IHas<let N : int> {}
enum E : IHas<int(E.a)>
//CHECK: E40002
{
    a = 1,
    b = 2,
}
