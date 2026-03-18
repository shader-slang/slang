//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Case 1: `<D : int>` — single type param with int constraint.
// Should get the scalar hint.
struct Foo1<D : int> {}
/*CHECK:
                ^^^ type 'int' cannot be used as a constraint
                use 'let' keyword to declare a generic value parameter: 'let D : int'
*/

// Case 2: `<each D : int>` — type pack param with int constraint.
// Should get the variadic hint.
struct Foo2<each D : int> {}
/*CHECK:
                     ^^^ type 'int' cannot be used as a constraint
                     use 'let each' to declare a variadic generic value parameter: 'let each D : int'
*/

// Case 3: `<D> where D : int` — where clause constraint.
// Should NOT get any hint.
struct Foo3<D> where D : int {}
/*CHECK:
                         ^^^ type 'int' cannot be used as a constraint
*/
//CHECK-NOT: use 'let' keyword
//CHECK-NOT: use 'let each'
