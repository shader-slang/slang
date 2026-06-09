//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

// Test that errors from synthesized methods are followed by a general note
// naming the method being synthesized.

struct MyVec<T : __BuiltinFloatingPointType> : IDifferentiable
{
    typedef MyVec<T> Differential;
    T x;
    T y;
};

// CHECK: some of the above errors may have been produced while synthesizing method '$__syn_dzero' of 'MyVec'
// CHECK: some of the above errors may have been produced while synthesizing method '$__syn_dadd' of 'MyVec'
