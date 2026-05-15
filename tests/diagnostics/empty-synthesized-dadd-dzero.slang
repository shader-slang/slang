//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that the compiler warns when the user explicitly assigns a custom
// `Differential` type whose own fields are all non-differentiable.  In that
// case the synthesized `dadd`/`dzero` bodies cannot set up any gradient and
// return uninitialized state.

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    int flag; // non-differentiable field inside the Differential type itself

    static MyDiff dzero() { return { 0 }; }
    static MyDiff dadd(MyDiff a, MyDiff b) { return { 0 }; }
};

struct MyType : IDifferentiable
/*CHECK:
       ^^^^^^ cannot synthesize complete differential method
       ^^^^^^ cannot synthesize a complete 'dzero' for type 'MyType' because its 'Differential' type contains fields that are not differentiable. Provide a user-defined 'dzero' implementation.
       ^^^^^^ cannot synthesize complete differential method
       ^^^^^^ cannot synthesize a complete 'dadd' for type 'MyType' because its 'Differential' type contains fields that are not differentiable. Provide a user-defined 'dadd' implementation.
*/
{
    typealias Differential = MyDiff;
    int id;
};
