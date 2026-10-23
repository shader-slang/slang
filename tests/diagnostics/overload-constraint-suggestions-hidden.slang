//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that no constraint is suggested when its type arguments are inaccessible.

import overload_constraint_suggestions_library;

// The interface is public, but its specialization contains an internal type.
void caller<T>(T value) { Holder.need(value); }
/*CHECK:
                                     ^ type argument doesn't conform to interface
                                     ^ type argument 'T' does not conform to the required interface 'IBound<Holder.Secret>'
*/

//CHECK: see declaration of public static func Holder.need<U> -> void
