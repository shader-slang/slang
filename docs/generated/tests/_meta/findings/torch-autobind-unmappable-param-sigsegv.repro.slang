// Repro for finding torch-autobind-unmappable-param-sigsegv.
// An [AutoPyBindCUDA] kernel with a parameter type that translateToHostType
// cannot map (here `String`) diagnoses E56001 and then crashes with SIGSEGV.
[AutoPyBindCUDA]
[CudaKernel]
void myKernel(String s)
{
}
