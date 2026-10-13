//TEST:SIMPLE(filecheck=IR): -target spirv -entry computeMain -stage compute -no-codegen -dump-ir

// The inline SPIR-V operand is the only use of `value` in the lambda body. Selected-expression
// finalization must visit that operand and retain `value` as closure state.

int copyWithInlineSpirv(int value)
{
    let copy = () => spirv_asm
    {
        result : $$int = OpCopyObject $value;
    };
    return copy();
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = copyWithInlineSpirv(42);
}

// IR-LABEL: [nameHint("copyWithInlineSpirv._slang_Lambda_copyWithInlineSpirv_{{[0-9]+}}")]
// IR: field([[VALUE_FIELD:%[A-Za-z0-9_]+]], Int)
// IR-LABEL: [nameHint("copyWithInlineSpirv._slang_Lambda_copyWithInlineSpirv_{{[0-9]+}}.()")]
// IR: [[CAPTURED_VALUE:%[A-Za-z0-9_]+]] : Int = get_field({{.*}}, [[VALUE_FIELD]])
// IR: SPIRVAsm
