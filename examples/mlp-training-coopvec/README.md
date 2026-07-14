Slang "MLP-Training-CoopVec" Example
==========================

This example shows how to use Slang to train a feed-forward neural network
using automatic differentiation and the cooperative vector intrinsics. Also see the
"MLP-Training" example for the same task without using cooperative vector.

Floating-point `CoopVec` values participate directly in automatic differentiation. Operations
that require external layout information, such as matrix-vector multiplication, use focused
wrapper functions with custom derivatives.
