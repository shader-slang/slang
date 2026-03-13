
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry main -target spirv -emit-spirv-directly

void main()
{
    spirv_asm
    {
        unknownOpCode
//CHECK:^^^^^^^^^^^^^ unrecognized SPIR-V opcode
//CHECK:^^^^^^^^^^^^^ unrecognized spirv opcode: unknownOpCode
    };
    spirv_asm
    {
        5
    };
}
