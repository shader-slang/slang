//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that failed calls suggest missing constraints, including when a concrete overload
// hides the generic candidate.

interface IBase {}
interface IDerived : IBase {}
interface IUnrelated {}

void derivedOp<U : IDerived>(U value) {}
void derivedOp(int2 value) {}
void unrelatedOp<U : IUnrelated>(U value) {}
void unrelatedOp(uint value) {}
void singleOp<U : IDerived>(U value) {}
/*CHECK:
     ^^^^^^^^ see declaration of func singleOp<U> -> void
     ^^^^^^^^ see declaration of func singleOp<U> -> void
*/
void tiedOp<U : IDerived>(U value) {}
/*CHECK:
     ^^^^^^ candidate: func tiedOp<U> -> void
*/
void tiedOp(int2 value) {}
/*CHECK:
     ^^^^^^ candidate: func tiedOp(int2) -> void
     ^^^^^^ argument 0 does not match: expected 'vector<int,2>', got 'T'
*/
void tiedOp(uint2 value) {}
/*CHECK:
     ^^^^^^ candidate: func tiedOp(uint2) -> void
     ^^^^^^ argument 0 does not match: expected 'vector<uint,2>', got 'T'
*/

void replaceBound<T : IBase>(T value) { derivedOp(value); }
/*CHECK:
                                                  ^^^^^ type mismatch in expression
                                                  ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
                  ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
void addBound<T : IBase>(T value) { unrelatedOp(value); }
/*CHECK:
                                                ^^^^^ type mismatch in expression
                                                ^^^^^ expected an expression of type 'uint', got 'T'
              ^ consider constraining 'T' as 'T : IBase & IUnrelated' to satisfy an overload's requirement 'IUnrelated'
*/
void singleCandidate<T : IBase>(T value) { singleOp(value); }
/*CHECK:
                                                   ^ type argument doesn't conform to interface
                                                   ^ type argument 'T' does not conform to the required interface 'IDerived'
                     ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
void tiedCandidates<T : IBase>(T value) { tiedOp(value); }
/*CHECK:
                                                ^ no overload for 'tiedOp' applicable to arguments of type (T)
                                                ^ type argument 'T' does not conform to the required interface 'IDerived'
                    ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
void unconstrained<T>(T value) { singleOp(value); }
/*CHECK:
                                         ^ type argument doesn't conform to interface
                                         ^ type argument 'T' does not conform to the required interface 'IDerived'
                   ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
void preserveBound<T : IBase & IUnrelated>(T value) { derivedOp(value); }
/*CHECK:
                                                                ^^^^^ type mismatch in expression
                                                                ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
                   ^ consider constraining 'T' as 'T : IUnrelated & IDerived' to satisfy an overload's requirement 'IDerived'
*/
void whereBound<T>(T value) where T : IBase { derivedOp(value); }
/*CHECK:
                                                        ^^^^^ type mismatch in expression
                                                        ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
                ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/

// Check that a refinement is preferred over an unrelated constraint regardless of order.
void prefer<U : IUnrelated>(U value) {}
void prefer<U : IDerived>(U value) {}
void prefer(int2 value) {}
void preferRefinement<T : IBase>(T value) { prefer(value); }
/*CHECK:
                                                   ^^^^^ type mismatch in expression
                                                   ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
                      ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/

// Check that duplicate requirements from different overloads produce one suggestion.
void duplicate<U : IDerived>(U value, int other) {}
/*CHECK:
     ^^^^^^^^^ candidate: func duplicate<U> -> void
*/
void duplicate<U : IDerived>(U value, float other) {}
/*CHECK:
     ^^^^^^^^^ candidate: func duplicate<U> -> void
*/
void duplicateRequirements<T : IBase>(T value) { duplicate(value, 0); }
/*CHECK:
                                                          ^ no overload for 'duplicate' applicable to arguments of type (T, int)
                                                          ^ type argument 'T' does not conform to the required interface 'IDerived'
                                                          ^ type argument 'T' does not conform to the required interface 'IDerived'
                           ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
