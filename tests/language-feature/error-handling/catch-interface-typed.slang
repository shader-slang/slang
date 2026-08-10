// Regression test for https://github.com/shader-slang/slang/issues/12343: calling a method on a
// caught interface-typed exception used to make simplify-cfg spin forever.
//
// Both halves of the shape are load-bearing, so do not "simplify" either one:
//   - the thrown/caught value must be interface(existential)-typed, not a concrete `ABC`, and
//   - the catch handler must *call a method* on it.
// The method call is what lowers to the hoistable `lookupWitness` / `extractExistential*`
// instructions whose re-parenting drove the non-termination; delete it, or replace `IFace` with a
// concrete type, and this file still compiles while testing nothing.
//
// A reintroduced hang surfaces as a suite/job timeout rather than a `FAILED test:` line, because
// slang-test only bounds compile time on its `-use-test-server` path.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute

interface IFace
{
    int getVal();
}

struct ABC : IFace
{
    int getVal() { return 42; }
}

// The CHECK lines are deliberately minimal liveness probes: termination is the property under
// test, so they assert only that compilation finished and produced an entry point. Tightening
// them into codegen-shape assertions would make this file fail for reasons unrelated to #12343.
//HLSL: void computeMain(
//SPV: OpEntryPoint
void computeMain(uint tid : SV_DispatchThreadID)
{
    do
    {
        IFace thrown = ABC();
        throw thrown;
    }
    catch(caught: IFace)
    {
        caught.getVal();
    }
}
