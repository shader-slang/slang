//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
float main(float value)
{
    hgbc_asm
    {
        OpPrint $$String $"hgbc_asm";
    };

    return hgbc_asm
    {
        %one : $$float = OpFloatLit 1065353216;
        %sum : $$float = OpAdd $$float $value %one;
        result = OpSin $$float %sum;
    };
}

// CHECK: OpFloatLit 1
// CHECK: OpPrint
// CHECK: OpAdd
// CHECK: OpSin
