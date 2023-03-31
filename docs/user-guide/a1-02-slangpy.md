---
layout: user-guide
---

Using Slang to Write PyTorch Kernels
=========================================================
If you are a PyTorch user seeking to write complex, high-performance, and automatically differentiated kernel functions using a per-thread programming model, we invite you to try Slang. Slang is a cutting-edge shading language that provides a straightforward way to define kernel functions that run incredibly fast in graphics applications. With the latest addition of automatic differentiation and PyTorch interop features, Slang offers an efficient solution for developing auto-differentiated kernels that run at lightning speed with a strongly typed, per-thread programming model.

One of the primary advantages of a per-thread programming model in kernel programming is the elimination of concerns regarding maintaining masks for branches. When developing a kernel in Slang, you can use all control flow statements, composite data types (structs, arrays, etc.), and function calls without additional effort. Code created with these language constructs can be automatically differentiated by the compiler without any restrictions. Additionally, Slang is a strongly typed language, which ensures that you will never encounter type errors at runtime. Most code errors can be identified as you type thanks to the [compiler's coding assistance service](https://marketplace.visualstudio.com/items?itemName=shader-slang.slang-language-extension), further streamlining the development process.

In addition, using a per-thread programming model also results in more optimized memory usage. When writing a kernel in Slang, most intermediate results do not need to be written out to global memory and then read back, reducing global memory bandwidth consumption and the delay caused by these memory operations. As a result, a Slang kernel can typically run at higher efficiency compared to the traditional bulk-synchronous programming model.

## Getting Started with slangpy

In this tutorial, we will use a simple example to walkthrough the steps to use Slang in your PyTorch project.

### Writing a simple kernel function as a Slang module

Assume we want to write a kernel function that computes `x*x` for each element in the input tensor in Slang. To do so,
we start by creating a `square.slang` file:

```csharp
// square.slang
float square(float x)
{
    return x * x;
}
```

This function is self explanatory. To use it in PyTorch, we need to write a GPU kernel function (that maps to a 
`__global__` CUDA function) that defines how to compute each element of the input tensor. So we continue to write
the following Slang function:

```csharp
[CudaKernel]
void square_fwd_kernel(TensorView<float> input, TensorView<float> output)
{
    uint3 globalIdx = cudaBlockIdx() * cudaBlockDim() + cudaThreadIdx();

    if (globalIdx.x > input.size(0) || globalIdx.y > input.size(1))
        return;
    float result = square(input[globalIdx.xy]);
    output[globalIdx.xy] = result;
}
```

This code follows the standard pattern of a typical CUDA kernel function. It takes as input
two tensors, `input` and `output`. 
It first obtains the global dispatch index of the current thread and performs range check to make sure we don't read or write out
of the bounds of input and output tensors, and then calls `square()` to compute the per-element result, and
store it at the corresponding location in `output` tensor.

With a kernel function defined, we then need to expose a CPU(host) function that defines how this kernel is dispatched:
```csharp
[TorchEntryPoint]
TorchTensor<float> square_fwd(TorchTensor<float> input)
{
    var result = TorchTensor<float>.zerosLike(input);
    let blockCount = uint3(1);
    let groupSize = uint3(result.size(0), result.size(1), 1);
    __dispatch_kernel(square_fwd_kernel, blockCount, groupSize)(input, result);
    return result;
}
```
Here, we mark the function with the `[TorchEntryPoint]` attribute so it will be exported to Python. In the function body, we call `TorchTensor<float>.zerosLike` to allocate a 2D-tensor that has the same size as the input.
`zerosLike` returns a `TorchTensor<float>` object that represents a CPU handle of a PyTorch tensor.
Then we launch `square_fwd_kernel` with the `__dispatch_kernel` syntax. Note that we can directly pass
`TorchTensor<float>` arguments to a `TensorView<float>` parameter and the compiler will automatically convert
the type and obtain a view into the tensor that can be accessed by the GPU kernel function.

### Calling Slang module from Python

Next, let's see how we can call the `square_fwd` function we defined in the Slang module.
To do so, we use a python package called `slangpy`. You can obtain it with

```bash
pip install slangpy
```

With that, you can use the following code to call `square_fwd` from Python:

```python
import torch
import slangpy

m = slangpy.loadModule("square.slang")

x = torch.randn(2,2)
print(f"X = {x}")
y = m.square_fwd(x)
print(f"Y = {y.cpu()}")
```

Result output:
```
X = tensor([[ 0.1407,  0.6594],
        [-0.8978, -1.7230]])
Y = tensor([[0.0198, 0.4349],
        [0.8060, 2.9688]])
```

And that's it! `slangpy.loadModule` uses JIT compilation to compile your Slang source into CUDA binary.
It may take a little longer the first time you execute the script, but the compiled binaries will be cached and as
long as the kernel code is not changed, future runs will not rebuild the CUDA kernel.

Because the PyTorch JIT system requires `ninja`, you need to make sure `ninja` is installed on your system
and is discoverable from the current environment, you also need to have a C++ compiler available on the system.
On Windows, this means that Visual Studio need to be installed.

### Exposing an automatically differentiated kernel to PyTorch

The above example demonstrates how to write a simple kernel function in Slang and call it from Python.
Another major benefit of using Slang is that the Slang compiler support generating backward derivative
propagation functions automatically.

In the following section, we walkthrough how to use Slang to generate a backward propagation function
for `square`, and expose it to PyTorch as an autograd function.

First we need to tell Slang compiler that we need the `square` function to be considered a differentiable function so Slang compiler can generate a backward derivative propagation function for it:
```csharp
[BackwardDifferentiable]
float square(float x)
{
    return x * x;
}
```
This is done by simply adding a `[BackwardDifferentiable]` attribute to our `square`function.

With that, we can now define `square_bwd_kernel` that performs backward propagation as:

```csharp
[CudaKernel]
void square_bwd_kernel(TensorView<float> input, TensorView<float> grad_out, TensorView<float> grad_propagated)
{
    uint3 globalIdx = cudaBlockIdx() * cudaBlockDim() + cudaThreadIdx();

    if (globalIdx.x > input.size(0) || globalIdx.y > input.size(1))
        return;

    DifferentialPair<float> dpInput = diffPair(input[globalIdx.xy]);
    var gradInElem = grad_out[globalIdx.xy];
    __bwd_diff(square)(dpInput, gradInElem);
    grad_propagated[globalIdx.xy] = dpInput.d;
}
```

Note that the function follows the same structure of `square_fwd_kernel`, with the only difference being that
instead of calling into `square` to compute the forward value for each tensor element, we are calling `__bwd_diff(square)`
that represents the automatically generated backward propagation function of `square`.
`__bwd_diff(square)` will have the following signature:
```csharp
void __bwd_diff_square(inout DifferentialPair<float> dpInput, float dOut);
```

Where the first parameter, `dpInput` represents a pair of original and derivative value for `input`, and the second parameter,
`dOut`, represents the initial derivative with regard to some latent variable that we wish to backprop through. The resulting
derivative will be stored in `dpInput.d`. For example:

```csharp
// construct a pair where the primal value is 3, and derivative value is 0.
var dp = diffPair(3.0);
__bwd_diff(square)(dp, 1.0);
// dp.d is now 6.0
```

Similar to `square_fwd`, we can define the host side function `square_bwd` as:

```csharp
[TorchEntryPoint]
TorchTensor<float> square_bwd(TorchTensor<float> input, TorchTensor<float> grad_out)
{
    var grad_propagated = TorchTensor<float>.zerosLike(input);
    let blockCount = uint3(1);
    let groupSize = uint3(input.size(0), input.size(1), 1);
    __dispatch_kernel(square_bwd_kernel, blockCount, groupSize)(input, grad_out, grad_propagated);
    return grad_propagated;
}
```

You can refer to [this documentation](07-autodiff.md) for a detailed reference of Slang's automatic differentiation feature.

With this, the python script `slangpy.loadModule("square.slang")` will now return
a scope that defines two functions, `square_fwd` and `square_bwd`. We can then use these
two functions to define a PyTorch autograd function class:

```python
m = slangpy.loadModule("square.slang")

class MySquareFuncInSlang(torch.autograd.Function):
    @staticmethod
    def forward(ctx, input):
        ctx.save_for_backward(input)
        return m.square_fwd(input)

    @staticmethod
    def backward(ctx, grad_output):
        [input] = ctx.saved_tensors
        return m.square_bwd(input, grad_output)
```

Now we can use the autograd function `MySquareFuncInSlang` in our python script:

```python
x = torch.tensor([[3.0, 4.0],[0.0, 1.0]], requires_grad=True, device=cuda_device)
print(f"X = {x}")
y_pred = MySquareFuncInSlang.apply(x)
loss = y_pred.sum()
loss.backward()
print(f"dX = {x.grad.cpu()}")
```

Output:
```
X = tensor([[3., 4.],
        [0., 1.]], device='cuda:0', requires_grad=True)
dX = tensor([[6., 8.],
        [0., 2.]])
```


## Builtin Library Support for PyTorch Interop

As shown in previous tutorial, Slang has defined the `TorchTensor<T>` and `TensorView<T>` type for interop with PyTorch
tensors. The `TorchTensor<T>` represents the CPU view of a tensor and provides methods to allocate a new tensor object.
The `TensorView<T>` represents the GPU view of a tensor and provides accesors to read write tensor data.

Following is a list of builtin methods and attributes for PyTorch interop.

### `TorchTensor` methods

#### `static TorchTensor<T> TorchTensor<T>.alloc(uint x, uint y, ...)`
Allocates a new PyTorch tensor with the given dimensions.

#### `static TorchTensor<T> TorchTensor<T>.emptyLike(TorchTensor<T> other)`
Allocates a new PyTorch tensor that has the same dimensions as `other` without initializing it.

#### `static TorchTensor<T> TorchTensor<T>.zerosLike(TorchTensor<T> other)`
Allocates a new PyTorch tensor that has the same dimensions as `other` and initialize it to zero.

#### `uint TorchTensor<T>.dims()`
Returns the tensor's dimension count.

#### `uint TorchTensor<T>.size(int dim)`
Returns the tensor's size (in number of elements) at `dim`.

#### `uint TorchTensor<T>.stride(int dim)`
Returns the tensor's stride (in bytes) at `dim`.

### `TensorView` methods

#### `TensorView<T>.operator[uint x, uint y, ...]`
Provide an accessor to data content in a tensor.

#### `TensorView<T>.operator[vector<uint, N> index]`
Provide an accessor to data content in a tensor, indexed by a uint vector.
`tensor[uint3(1,2,3)]` is equivalent to `tensor[1,2,3]`.

#### `uint TensorView<T>.dims()`
Returns the tensor's dimension count.

#### `uint TensorView<T>.size(int dim)`
Returns the tensor's size (in number of elements) at `dim`.

#### `uint TensorView<T>.stride(int dim)`
Returns the tensor's stride (in bytes) at `dim`.

#### `void TensorView<T>.fillZero()`
Fills the tensor with zeros. Modifies the tensor in-place.

#### `void TensorView<T>.fillValue(T value)`
Fills the tensor with the specified value, modifies the tensor in-place.

#### `T* TensorView<T>.data_ptr_at(vector<uint, N> index)`
Returns a pointer to the element at `index`.

#### `void TensorView<T>.InterlockedAdd(vector<uint, N> index, T val, out T oldVal)`
Atomically add `val` to element at `index`. 

#### `void TensorView<T>.InterlockedMin(vector<uint, N> index, T val, out T oldVal)`
Atomically computes the min of `val` and the element at `index`. Available for 32 and 64 bit integer types only.

#### `void TensorView<T>.InterlockedMax(vector<uint, N> index, T val, out T oldVal)`
Atomically computes the max of `val` and the element at `index`. Available for 32 and 64 bit integer types only.

#### `void TensorView<T>.InterlockedAnd(vector<uint, N> index, T val, out T oldVal)`
Atomically computes the bitwise and of `val` and the element at `index`. Available for 32 and 64 bit integer types only.

#### `void TensorView<T>.InterlockedOr(vector<uint, N> index, T val, out T oldVal)`
Atomically computes the bitwise or  of `val` and the element at `index`. Available for 32 and 64 bit integer types only.

#### `void TensorView<T>.InterlockedXor(vector<uint, N> index, T val, out T oldVal)`
Atomically computes the bitwise xor  of `val` and the element at `index`. Available for 32 and 64 bit integer types only.

#### `void TensorView<T>.InterlockedExchange(vector<uint, N> index, T val, out T oldVal)`
Atomically swaps `val` into the element at `index`. Available for `float` and 32/64 bit integer types only.

#### `void TensorView<T>.InterlockedCompareExchange(vector<uint, N> index, T compare, T val)`
Atomically swaps `val` into the element at `index` if the element equals to `compare`. Available for `float` and 32/64 bit integer types only.

### CUDA Support Functions

#### `cudaThreadIdx()`
Returns the `threadIdx` variable in CUDA.

#### `cudaBlockIdx()`
Returns the `blockIdx` variable in CUDA.

#### `cudaBlockDim()`
Returns the `blockDim` variable in CUDA.

#### `syncTorchCudaStream()`
Waits for all pending CUDA kernel executions to complete on host.

### Attributes for PyTorch Interop

#### `[CudaKernel]` attribute
Marks a function as a CUDA kernel (maps to a `__global__` function)

#### `[TorchEntryPoint]` attribute
Marks a function for export to Python. Functions marked with `[TorchEntryPoint]` will be accessible from a loaded module returned by `slangpy.loadModule`.

#### `[CudaDeviceExport]` attribute
Marks a function as a cuda device function, and ensures the compiler to include it in the generated cuda source.

## Type Marshalling Between Slang and Python

The return types and parameters types of an exported `[TorchEntryPoint]` function can be a basic type (e.g. `float`, `int` etc.), a vector type (e.g. `float3`), a `TorchTensor<T>` type, an array type, or a struct type.

When you use struct or array types in the function signature, it will be exposed as a Python tuple.
For example,
```csharp
struct MyReturnType
{
    TorchTensor<T> tensors[3];
    float v;
}

[TorchEntryPoint]
MyReturnType myFunc()
{
    ...
}
```

Calling `myFunc` from python will result in a python tuple in the form of
```
[[tensor, tensor, tensor], float]
```

The same transform rules applies to parameter types.
