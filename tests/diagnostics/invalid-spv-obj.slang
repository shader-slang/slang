//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main() {
    int var2 = spirv_asm {
        result: $$int = OpIMul $(15) %invalidId
//CHECK:                              ^^^^^^^^^ undefined SPIR-V id
//CHECK:                              ^^^^^^^^^ SPIRV id '%invalidId' is not defined in the current assembly block location
    };
}
