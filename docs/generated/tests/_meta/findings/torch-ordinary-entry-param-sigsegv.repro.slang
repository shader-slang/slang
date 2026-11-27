// Repro for finding torch-ordinary-entry-param-sigsegv.
// `slangc <this> -target torch` crashes with SIGSEGV (exit 139) on an ordinary
// compute entry point that takes a parameter. Without the parameter it
// compiles and emits an empty PYBIND11_MODULE.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
}
