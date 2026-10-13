//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Kernel dispatch IR cannot represent an implicit receiver, so bound and unbound instance-method
// spellings are both rejected after selecting the kernel declaration.

struct KernelReceiver
{
    [mutating]
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void kernel()
    {}

    [nonmutating]
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void readOnlyKernel()
    {}

    [shader("compute")]
    [numthreads(1, 1, 1)]
    static void staticKernel()
    {}
}

void testBoundKernelReceiver()
{
    var receiver = KernelReceiver();
    let dispatch = __dispatch_kernel(receiver.kernel, uint3(1, 1, 1), uint3(1, 1, 1));
//CHECK:           ^^^^^^^^^^^^^^^^^ non-static function reference not allowed
//CHECK:           ^^^^^^^^^^^^^^^^^ non-static function reference 'kernel' is not allowed here.
}

void testUnboundKernelReceiver()
{
    let dispatch =
        __dispatch_kernel(KernelReceiver::readOnlyKernel, uint3(1, 1, 1), uint3(1, 1, 1));
//CHECK:           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ non-static function reference not allowed
//CHECK:           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ non-static function reference 'readOnlyKernel' is not allowed here.
}

void testStaticKernelSelectedThroughValue(KernelReceiver receiver)
{
    let dispatch =
        __dispatch_kernel(receiver.staticKernel, uint3(1, 1, 1), uint3(1, 1, 1));
}

struct DispatchSizeReceiver
{
    [mutating]
    uint3 getSize()
    {
        return uint3(1, 1, 1);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void dispatchTarget()
{}

void testKernelThreadGroupSizeReceiver()
{
    let receiver = DispatchSizeReceiver();
    let dispatch = __dispatch_kernel(dispatchTarget, receiver.getSize(), uint3(1, 1, 1));
//CHECK:                                                             ^ mutating method cannot be called on immutable value
//CHECK:                                                             ^ mutating method 'getSize' cannot be called on an immutable value
}

struct SetterOnlyDispatchSize
{
    property uint3 size
    {
        set {}
    }
}

void testSetterOnlyDispatchSize(SetterOnlyDispatchSize storage)
{
    let dispatch = __dispatch_kernel(dispatchTarget, uint3(1, 1, 1), storage.size);
//CHECK:                                                                     ^^^^ property or subscript cannot be read
//CHECK:                                                                     ^^^^ cannot read this property because it has no `get` or `ref` accessor
}
