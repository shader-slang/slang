//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// HGBC validation failures name the source line that produced the rejected record, so an invalid
// instruction can be traced without a disassembler. The interpreter keeps line tables by default.
//
// The record below declares a `float` result for a `uint` addition, which the validator rejects.

[host]
void main()
{
    uint one = hgbc_asm { result : $$uint = OpIntLit 1; };
    float wrong = hgbc_asm {
        result : $$float = OpAdd $$uint $one $one;
    };
    print(wrong);
}

// CHECK: result code = 1
// CHECK: OpAdd at byte offset {{[0-9]+}} result and operand types do not match (at {{.*}}validation-error-source-location.slang:12:
