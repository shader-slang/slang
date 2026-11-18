//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// Companion regression for shader-slang/slang#14578: kernel targets are
// excluded from the link-time preservation of globals with side-effecting
// initializers, because their initializers are handled by the global-init
// pass that injects selected initializers into the entry points, and an
// unreferenced global must keep being eliminated. The benign-ctor shape
// compiled for SPIR-V must therefore not contain the initializer call.

//CHECK: OpCapability
//CHECK-NOT: computeInitialValue

static int s = computeInitialValue();

int computeInitialValue()
{
    int sum = 0;
    for (int i = 0; i < 7; ++i)
        sum += i;
    return sum;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
