//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

// `OpLoad` reads the value stored at its address operand. The diagnostic must point to the
// source-level assembly expression rather than its compiler-generated IR operand wrapper, which
// has no source location.

static int uninitializedGlobal;
RWStructuredBuffer<int> output;

int readAssemblyArgument(__constref int value)
{
    return value;
}

void writeAssemblyArgument(out int value)
{
    value = 1;
}

int initializeLocalWithAssembly()
{
    int value;
    spirv_asm
    {
        OpConstant $$int %one 1;
        OpStore &value %one;
    };
    return value;
}

int initializeLocalAfterAssemblyLabel()
{
    int value;
    int source = 1;
    spirv_asm
    {
        // The analysis treats only stores before the first `OpLabel` or block terminator as
        // unconditional. It therefore treats this store as possible rather than definite.
        OpUnreachable;
        %later = OpLabel;
        OpStore &value $source;
    };
    return value;
/*CHECK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
*/
}

int storeUninitializedSourceWithAssembly()
{
    int source;
    int destination = 0;
    spirv_asm
/*CHECK:
    ^^^^^^^^^ use of uninitialized variable
    ^^^^^^^^^ use of uninitialized variable 'source'
*/
    {
        OpStore &destination $source;
    };
    return destination;
}

int copyUninitializedSourceWithAssembly()
{
    int source;
    int destination = 0;
    spirv_asm
/*CHECK:
    ^^^^^^^^^ use of uninitialized variable
    ^^^^^^^^^ use of uninitialized variable 'source'
*/
    {
        OpCopyMemory &destination &source;
    };
    return destination;
}

int copySizedUninitializedSourceWithAssembly()
{
    int source;
    int destination = 0;
    spirv_asm
/*CHECK:
    ^^^^^^^^^ use of uninitialized variable
    ^^^^^^^^^ use of uninitialized variable 'source'
*/
    {
        OpConstant $$uint %four 4;
        OpCopyMemorySized &destination &source %four;
    };
    return destination;
}

int initializeLocalWithAssemblyCopy()
{
    int source = 1;
    int destination;
    spirv_asm
    {
        OpCopyMemory &destination &source;
    };
    return destination;
}

// The uninitialized-value analysis has no target layout with which to prove that a byte-counted
// copy covers the complete destination. This remains true when the literal byte count happens to
// match the size of `int` on this target.
int copySizedIntoLocalWithAssembly()
{
    int source = 1;
    int destination;
    spirv_asm
    {
        OpConstant $$uint %four 4;
        OpCopyMemorySized &destination &source %four;
    };
    return destination;
/*CHECK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'destination' may be uninitialized on some paths; it is only conditionally assigned
*/
}

int partiallyInitializeLocalWithSizedAssemblyCopy()
{
    int source = 1;
    int destination;
    spirv_asm
    {
        OpConstant $$uint %one 1;
        OpCopyMemorySized &destination &source %one;
    };
    return destination;
/*CHECK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'destination' may be uninitialized on some paths; it is only conditionally assigned
*/
}

int readAfterStoreWithinAssembly()
{
    int value;
    int result = spirv_asm
    {
        OpConstant $$int %one 1;
        OpStore &value %one;
        OpLoad $$int result &value;
    };
    return result;
}

int readBeforeStoreWithinAssembly()
{
    int value;
    int result = spirv_asm
//CHECK:         ^^^^^^^^^ use of uninitialized variable
//CHECK:         ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpLoad $$int %loaded &value;
        OpConstant $$int %one 1;
        OpStore &value %one;
        OpCopyObject $$int result %loaded;
    };
    return result;
}

int readUninitializedElementThroughAssemblyAddress()
{
    int2 value;
    bool selectFirst = true;
    int result = spirv_asm
//CHECK:         ^^^^^^^^^ use of uninitialized variable
//CHECK:         ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpConstant $$int %zero 0;
        %element : $$__Addr<int> = OpAccessChain &value %zero;
        %copy : $$__Addr<int> = OpCopyObject %element;
        %selected : $$__Addr<int> = OpSelect $selectFirst %copy %copy;
        result : $$int = OpLoad %selected;
    };
    return result;
}

int initializeLocalThroughAssemblyCopyObject()
{
    int value;
    int source = 1;
    spirv_asm
    {
        %address : $$__Addr<int> = OpCopyObject &value;
        OpStore %address $source;
    };
    return value;
}

int readUninitializedThroughAssemblyBitcast()
{
    int value;
    int result = spirv_asm
//CHECK:         ^^^^^^^^^ use of uninitialized variable
//CHECK:         ^^^^^^^^^ use of uninitialized variable 'value'
    {
        %address : $$__Addr<int> = OpBitcast &value;
        result : $$int = OpLoad %address;
    };
    return result;
}

int readUninitializedThroughAssemblyPtrAccessChain()
{
    int value;
    int result = spirv_asm
//CHECK:         ^^^^^^^^^ use of uninitialized variable
//CHECK:         ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpConstant $$int %zero 0;
        %address : $$__Addr<int> = OpPtrAccessChain &value %zero;
        result : $$int = OpLoad %address;
    };
    return result;
}

int readUninitializedThroughAssemblyCall()
{
    int value;
    int result = spirv_asm
//CHECK:         ^^^^^^^^^ use of uninitialized variable
//CHECK:         ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpFunctionCall $$int result $readAssemblyArgument &value;
    };
    return result;
}

int initializeLocalThroughAssemblyCall()
{
    int value;
    spirv_asm
    {
        OpFunctionCall $$void %ignored $writeAssemblyArgument &value;
    };
    return value;
}

int readUninitializedThroughExportedAssemblyAddress()
{
    int value;
    let address = spirv_asm
    {
        result : $$__Addr<int> = OpCopyObject &value;
    };
    return *address;
/*CHECK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 'value'
*/
}

int initializeThroughExportedAssemblyAddress()
{
    int value;
    int source = 1;
    let address = spirv_asm
    {
        result : $$__Addr<int> = OpCopyObject &value;
    };
    *address = source;
    return value;
}

int partiallyInitializeLocalThroughAssemblyAddress()
{
    int2 value;
    int source = 1;
    spirv_asm
    {
        OpConstant $$int %zero 0;
        %element : $$__Addr<int> = OpAccessChain &value %zero;
        OpStore %element $source;
    };
    return value.y;
//CHECK:         ^ possible use of uninitialized variable
//CHECK:         ^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
}

[numthreads(1, 1, 1)]
void main()
{
    int value = spirv_asm
//CHECK:        ^^^^^^^^^ use of uninitialized global variable
//CHECK:        ^^^^^^^^^ use of uninitialized global variable 'uninitializedGlobal'
    {
        OpLoad $$int result &uninitializedGlobal
    };
    output[0] = value + initializeLocalWithAssembly() +
                initializeLocalAfterAssemblyLabel() +
                storeUninitializedSourceWithAssembly() +
                copyUninitializedSourceWithAssembly() +
                copySizedUninitializedSourceWithAssembly() +
                initializeLocalWithAssemblyCopy() +
                copySizedIntoLocalWithAssembly() +
                partiallyInitializeLocalWithSizedAssemblyCopy() +
                readAfterStoreWithinAssembly() +
                readBeforeStoreWithinAssembly() +
                readUninitializedElementThroughAssemblyAddress() +
                initializeLocalThroughAssemblyCopyObject() +
                readUninitializedThroughAssemblyBitcast() +
                readUninitializedThroughAssemblyPtrAccessChain() +
                readUninitializedThroughAssemblyCall() +
                initializeLocalThroughAssemblyCall() +
                readUninitializedThroughExportedAssemblyAddress() +
                initializeThroughExportedAssemblyAddress() +
                partiallyInitializeLocalThroughAssemblyAddress();
}
