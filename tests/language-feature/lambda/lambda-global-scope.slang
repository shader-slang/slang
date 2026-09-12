// Regression test for https://github.com/shader-slang/slang/issues/11963
// Two (or more) lambda expressions at module/global scope used to collide on the
// synthesized struct name `_slang_Lambda_`, producing `error[E30200]: conflicting
// declaration`. This compiles two module-scope lambdas together and checks that the
// entry point is emitted, i.e. the front-end no longer rejects the second lambda.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

IFunc<int> g1 = () => 3;
IFunc<int> g2 = () => 5;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: void computeMain
}
