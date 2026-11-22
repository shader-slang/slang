// An imported GLSL atomic-counter type is not a supported global unless GLSL parsing supplied
// the binding and offset layout consumed by its backing-storage rewrite.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute

#language slang 2025
import glsl;

atomic_uint counter;
//CHECK: global `atomic_uint` declarations require a GLSL `layout(binding = ...)` qualifier
//CHECK: this atomic counter has no GLSL binding layout

[numthreads(1, 1, 1)]
void main()
{
    atomicCounterIncrement(counter);
}
