//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Case 1: `<D : int>` — single type param with int constraint.
// Should get the helper note.
struct Foo1<D : int> {}
/*CHECK:
                ^^^ type 'int' cannot be used as a constraint
                use 'let' keyword to declare a generic value parameter: 'let D : int'
*/

// Case 2: `<each D : int>` — type pack param with int constraint.
// Should get the helper note.
struct Foo2<each D : int> {}
/*CHECK:
                     ^^^ type 'int' cannot be used as a constraint
                     use 'let' keyword to declare a generic value parameter: 'let D : int'
*/

// Case 3: `<D> where D : int` — where clause constraint.
// Should NOT get the helper note.
struct Foo3<D> where D : int {}
/*CHECK:
                         ^^^ type 'int' cannot be used as a constraint
*/
//CHECK-NOT: use 'let' keyword
