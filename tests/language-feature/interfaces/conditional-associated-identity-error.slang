//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

import "conditional-associated-identity-module";

struct MixedWrapper<T : IMixedValue> { T.Value value; }

T readMixed<T>(MixedWrapper<MixedBox<T>> box) where MixedBox<T> : IMixedValue
{
    return box.value.payload;
    /*diag:
                     ^^^^^^^ member not found
                     ^^^^^^^ 'payload' is not a member of 'MixedBox<T>.Value'.
    */
}

// A sealed base interface does not prevent an open child from admitting an external
// conformance with a different associated type, even for an otherwise fixed nominal family.
extension EscapeBox<int> : IOpenValue
{
    typealias Value = float;
}

struct EscapeWrapper<T : IClosedValue> { T.Value value; }

T readEscape<T>(EscapeWrapper<EscapeBox<T>> box) where EscapeBox<T> : IClosedValue
{
    return box.value.payload;
    /*diag:
                     ^^^^^^^ member not found
                     ^^^^^^^ 'payload' is not a member of 'EscapeBox<T>.Value'.
    */
}
