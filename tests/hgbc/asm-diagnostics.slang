//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

float undefinedLocal(float value)
{
    return hgbc_asm {
        result : $$float = OpAdd $$float $value %missing;
        // CHECK: error[E29126]
        // CHECK: HGBC id '%missing' is not defined before this use
    };
}

float misplacedResult(float value)
{
    return hgbc_asm {
        result : $$float = OpSin $value;
        // CHECK: error[E29128]
        // CHECK: the result marker may appear only once, as the result of the final instruction
        OpPrint $$String $"not final";
    };
}

[host]
float main(float value)
{
    hgbc_asm {
        OpDefinitelyUnknown;
        // CHECK: error[E29119]
        // CHECK: unrecognized HGBC opcode: OpDefinitelyUnknown
    };
    return undefinedLocal(value) + misplacedResult(value);
}
