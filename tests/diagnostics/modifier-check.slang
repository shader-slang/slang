//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

public internal int v()
//diag:^^^^^^^^ duplicate modifier
//diag:^^^^^^^^ modifier 'internal' is redundant or conflicting with existing modifier 'public'
{}

__preserve_type_modifiers
/*diag:
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier not allowed
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier '__preserve_type_modifiers' is not allowed here.
*/
struct InvalidModifiedStruct {}

__preserve_type_modifiers
/*diag:
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier not allowed
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier '__preserve_type_modifiers' is not allowed here.
*/
extension float {}

struct Box<T> {}
__preserve_type_modifiers
/*diag:
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier not allowed
^^^^^^^^^^^^^^^^^^^^^^^^^ modifier '__preserve_type_modifiers' is not allowed here.
*/
extension<T> Box<T> {}

interface IQualifiedPayload
{
    associatedtype Payload;
}
__preserve_type_modifiers
extension<T : IDifferentiable> T : IQualifiedPayload
{
    typealias Payload = T;
}
typealias NonDifferentiable = no_diff float;

// Preserving the complete argument must also respect its excluded conformances.
// In particular, no_diff float cannot satisfy this extension's IDifferentiable bound.
NonDifferentiable.Payload invalidPayload;
//diag:           ^^^^^^^ member not found
//diag:           ^^^^^^^ 'Payload' is not a member of 'typeof(NonDifferentiable)'.
