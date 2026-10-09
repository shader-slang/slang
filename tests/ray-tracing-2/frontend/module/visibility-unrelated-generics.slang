//TEST:SIMPLE: -experimental-feature -entry main -stage compute -no-codegen -I $dirname

// Generic arguments unrelated to structural ray tracing must retain normal checking behavior,
// including when their module is imported after the ray-tracing contracts.
import slang.raytracing;
import visibility_unrelated_generics_lib;

[numthreads(1, 1, 1)]
void main()
{
    consumeUnrelatedGenericModule();
}
