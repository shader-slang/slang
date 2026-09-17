//TEST:SIMPLE: -experimental-feature -entry main -stage compute -no-codegen -I $dirname

// Loading the trusted structural ray-tracing module must not enable its special
// generic-argument checks while a later, unrelated module is being checked.
import slang.raytracing;
import visibility_unrelated_generics_lib;

[numthreads(1, 1, 1)]
void main()
{
    consumeUnrelatedGenericModule();
}
