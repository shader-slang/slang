// A parameter's passing mode decides whether its body may assign to it, whether the parameter is
// declared in the modern (`name : Type`) or the legacy (C-style) syntax. A modern-syntax `__ref`
// or `__ref_writeonly` parameter is writable, like `out` and `inout`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

func setRef(__ref x : int) { x = 1; }
func setRefWriteOnly(__ref_writeonly x : int) { x = 2; }
func setInout(inout x : int) { x = 3; }
func setOut(out x : int) { x = 4; }

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int v = 0;
    // CHECK: 1
    setRef(v);
    outputBuffer[0] = v;
    // CHECK-NEXT: 2
    setRefWriteOnly(v);
    outputBuffer[1] = v;
    // CHECK-NEXT: 3
    setInout(v);
    outputBuffer[2] = v;
    // CHECK-NEXT: 4
    setOut(v);
    outputBuffer[3] = v;
}
