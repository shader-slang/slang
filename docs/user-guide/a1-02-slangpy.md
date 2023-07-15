---
layout: user-guide
---

Using Slang to Write PyTorch Kernels
=========================================================
If you are a PyTorch user seeking to write complex, high-performance, and automatically differentiated kernel functions using a per-thread programming model, we invite you to try Slang. Slang is a cutting-edge shading language that provides a straightforward way to define kernel functions that run incredibly fast in graphics applications. With the latest addition of automatic differentiation and PyTorch interop features, Slang offers an efficient solution for developing auto-differentiated kernels that run at lightning speed with a strongly typed, per-thread programming model.

One of the primary advantages of a per-thread programming model in kernel programming is the elimination of concerns regarding maintaining masks for branches. When developing a kernel in Slang, you can use all control flow statements, composite data types (structs, arrays, etc.), and function calls without additional effort. Code created with these language constructs can be automatically differentiated by the compiler without any restrictions. Additionally, Slang is a strongly typed language, which ensures that you will never encounter type errors at runtime. Most code errors can be identified as you type thanks to the [compiler's coding assistance service](https://marketplace.visualstudio.com/items?itemName=shader-slang.slang-language-extension), further streamlining the development process.

In addition, using a per-thread programming model also results in more optimized memory usage. When writing a kernel in Slang, most intermediate results do not need to be written out to global memory and then read back, reducing global memory bandwidth consumption and the delay caused by these memory operations. As a result, a Slang kernel can typically run at higher efficiency compared to the traditional bulk-synchronous programming model.

## Getting Started with slangpy

In this tutorial, we will use a simple example to walk through the steps to use Slang in your PyTorch project.

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

This function is self-explanatory. To use it in PyTorch, we need to write a GPU kernel function (that maps to a 
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
Here, we mark the function with the `[TorchEntryPoint]` attribute, so it will be exported to Python. In the function body, we call `TorchTensor<float>.zerosLike` to allocate a 2D-tensor that has the same size as the input.
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

In the following section, we walk through how to use Slang to generate a backward propagation function
for `square`, and expose it to PyTorch as an autograd function.

First we need to tell Slang compiler that we need the `square` function to be considered a differentiable function, so Slang compiler can generate a backward derivative propagation function for it:
```csharp
[Differentiable]
float square(float x)
{
    return x * x;
}
```
This is done by simply adding a `[Differentiable]` attribute to our `square`function.

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
    bwd_diff(square)(dpInput, gradInElem);
    grad_propagated[globalIdx.xy] = dpInput.d;
}
```

Note that the function follows the same structure of `square_fwd_kernel`, with the only difference being that
instead of calling into `square` to compute the forward value for each tensor element, we are calling `bwd_diff(square)`
that represents the automatically generated backward propagation function of `square`.
`bwd_diff(square)` will have the following signature:
```csharp
void bwd_diff_square(inout DifferentialPair<float> dpInput, float dOut);
```

Where the first parameter, `dpInput` represents a pair of original and derivative value for `input`, and the second parameter,
`dOut`, represents the initial derivative with regard to some latent variable that we wish to back-prop through. The resulting
derivative will be stored in `dpInput.d`. For example:

```csharp
// construct a pair where the primal value is 3, and derivative value is 0.
var dp = diffPair(3.0);
bwd_diff(square)(dp, 1.0);
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
x = torch.tensor([[3.0, 4.0],[0.0, 1.0]], requires_grad=True, device='cuda')
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
## Specializing shaders using slangpy

`slangpy.loadModule` allows specialization parameters to be specified since it might be easier to write shaders with placeholder definitions that can be substituted at load-time.
For instance, here's a sphere tracer that uses a _compile-time_ specialization parameter for its maximum number of steps (`N`):

```csharp
float sphereTrace<let N:int>(Ray ray, SDF sdf)
{
    var pt = ray.o;
    for (int i = 0; i < N; i++)
    {
        pt += sdf.eval(pt) * ray.d;
    }

    return pt;
}

float render(Ray ray)
{
    // Use N=20 for sphere tracing.
    float3 pt = sphereTrace<20>(ray, sdf);
    return shade(pt, sdf.normal());
}
```

However, instead of using a fixed `20` steps, the renderer can be configured to use an arbitrary compile-time constant.

```csharp
// Compile-time constant. Expect "MAX_STEPS" to be set by the loadModule call.
static const uint kMaxSteps = MAX_STEPS;

float render(Ray ray)
{
    float3 pt = sphereTrace<kMaxSteps>(ray, sdf);
    return shade(pt, sdf.normal());
}
```

Then multiple versions of this shader can be compiled from Python using the `defines` argument:
```python
import slangpy

sdfRenderer20Steps = slangpy.loadModule('sdf.slang', defines={"MAX_STEPS": 20})
sdfRenderer50Steps = slangpy.loadModule('sdf.slang', defines={"MAX_STEPS": 50})
...
```

This is often helpful for code re-use, parameter sweeping, comparison/ablation studies, and more, from the convenience of Python.

## Back-propagating Derivatives through Complex Access Patterns

In most common scenarios, a kernel function will access input tensors in a complex pattern instead of mapping
1:1 from an input element to an output element, like the `square` example shown above. When you have a kernel
function that access many different elements from the input tensors and use them to compute an output element,
the derivatives of each input element can't be represented directly as a function parameter, like the `x` in `square(x)`.

Consider a 3x3 box filtering kernel that computes for each pixel in a 2D image, the average value of its 
surrounding 3x3 pixel block. We can write a Slang function that computes the value of an output pixel:
```csharp
float computeOutputPixel(TensorView<float> input, uint2 pixelLoc)
{
    int width = input.size(0);
    int height = input.size(1);

    // Track the sum of neighboring pixels and the number
    // of pixels currently accumulated.
    int count = 0;
    float sumValue = 0.0;

    // Iterate through the surrounding area.
    for (int offsetX = -1; offsetX <= 1; offsetX++)
    {
        // Skip out of bounds pixels.
        int x = pixelLoc.x + offsetX;
        if (x < 0 || x >= width) continue;

        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            int y = pixelLoc.y + offsetY;
            if (y < 0 || y >= height) continue;
            sumValue += input[x, y];
            count++;
        }
    }

    // Comptue the average value.
    sumValue /= count;

    return sumValue;
}
```

We can define our kernel function to compute the entire output image by calling `computeOutputPixel`:

```csharp
[CudaKernel]
void boxFilter_fwd(TensorView<float> input, TensorView<float> output)
{
    uint2 pixelLoc = (cudaBlockIdx() * cudaBlockDim() + cudaThreadIdx()).xy;
    int width = input.dim(0);
    int height = input.dim(1);
    if (pixelLoc.x >= width) return;
    if (pixelLoc.y >= height) return;

    float outputValueAtPixel = computeOutputPixel(input, pixelLoc)

    // Write to output tensor.
    output[pixelLoc] = outputValueAtPixel;
}
```

How do we define the backward derivative propagation kernel? Note that in this example, there
isn't a function like `square` that we can just mark as `[Differentiable]` and
call `bwd_diff(square)` to get back the derivative of an input parameter.

In this example, the input comes from multiple elements in a tensor. How do we propagate the
derivatives to those input elements?

The solution is to wrap tensor access with a custom function:
```csharp
float getInputElement(
    TensorView<float> input,
    TensorView<float> inputGradToPropagateTo,
    uint2 loc)
{
    return input[loc];
}
```

Note that the `getInputElement` function simply returns `input[loc]` and is not using the
`inputGradToPropagateTo` parameter. That is intended. The `inputGradToPropagateTo` parameter
is used to hold the backward propagated derivatives of each input element, and is reserved for later use.

Now we can replace all direct accesses to `input` with a call to `getInputElement`. The
`computeOutputPixel` can be implemented as following:

```csharp
[Differentiable]
float computeOutputPixel(
    TensorView<float> input,
    TensorView<float> inputGradToPropagateTo,
    uint2 pixelLoc)
{
    int width = input.dim(0);
    int height = input.dim(1);

    // Track the sum of neighboring pixels and the number
    // of pixels currently accumulated.
    int count = 0;
    float sumValue = 0.0;

    // Iterate through the surrounding area.
    for (int offsetX = -1; offsetX <= 1; offsetX++)
    {
        // Skip out of bounds pixels.
        int x = pixelLoc.x + offsetX;
        if (x < 0 || x >= width) continue;

        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            int y = pixelLoc.y + offsetY;
            if (y < 0 || y >= height) continue;
            sumValue += getInputElement(input, inputGradToPropagateTo, uint2(x, y));
            count++;
        }
    }

    // Comptue the average value.
    sumValue /= count;

    return sumValue;
}
```

The main changes compared to our original version of `computeOutputPixel` are:
- Added a `inputGradToPropagateTo` parameter.
- Modified `input[x,y]` with a call to `getInputElement`.
- Added a `[Differentiable]` attribute to the function.

With that, we can define our backward kernel function:

```csharp
[CudaKernel]
void boxFilter_bwd(
    TensorView<float> input,
    TensorView<float> resultGradToPropagateFrom,
    TensorView<float> inputGradToPropagateTo)
{
    uint2 pixelLoc = (cudaBlockIdx() * cudaBlockDim() + cudaThreadIdx()).xy;
    int width = input.dim(0);
    int height = input.dim(1);
    if (pixelLoc.x >= width) return;
    if (pixelLoc.y >= height) return;

    bwd_diff(computeOutputPixel)(input, inputGradToPropagateTo, pixelLoc);
}
```

The kernel function simply calls `bwd_diff(computeOutputPixel)` without taking any return values from the call
and without writing to any elements in the final `inputGradToPropagateTo` tensor. But when exactly does the propagated
output get written to the output gradient tensor (`inputGradToPropagateTo`)?

And that logic is defined in our final piece of code:
```csharp
[BackwardDerivativeOf(getInputElement)]
void getInputElement_bwd(
    TensorView<float> input,
    TensorView<float> inputGradToPropagateTo,
    uint2 loc,
    float derivative)
{
    float oldVal;
    inputGradToPropagateTo.InterlockedAdd(loc, derivative, oldVal);
}
```

Here, we are providing a custom defined backward propagation function for `getInputElement`.
In this function, we simply add `derivative` to the element in `inputGradToPropagateTo` tensor.

When we call `bwd_diff(computeOutputPixel)` in `boxFilter_bwd`, the Slang compiler will automatically
differentiate all operations and function calls in `computeOutputPixel`. By wrapping the tensor element access
with `getInputElement` and by providing a custom backward propagation function of `getInputElement`, we are effectively
telling the compiler what to do when a derivative propagates to an input tensor element. Inside the body
of `getInputElement_bwd`, we define what to do then: atomically adds the derivative propagated to the input element
in the `inputGradToPropagateTo` tensor. Therefore, after running `boxFilter_bwd`, the `inputGradToPropagateTo` tensor will contain all the
back propagated derivative values.

Again, to understand all the details of the automatic differentiation system, please refer to the 
[Automatic Differentiation](07-autodiff.md) chapter for a detailed explanation.

## Builtin Library Support for PyTorch Interop

As shown in previous tutorial, Slang has defined the `TorchTensor<T>` and `TensorView<T>` type for interop with PyTorch
tensors. The `TorchTensor<T>` represents the CPU view of a tensor and provides methods to allocate a new tensor object.
The `TensorView<T>` represents the GPU view of a tensor and provides accessors to read write tensor data.

Following is a list of built-in methods and attributes for PyTorch interop.

### `TorchTensor` methods

#### `static TorchTensor<T> TorchTensor<T>.alloc(uint x, uint y, ...)`
Allocates a new PyTorch tensor with the given dimensions. If `T` is a vector type, the length of the vector is implicitly included as the last dimension.
For example, `TorchTensor<float3>.alloc(4, 4)` allocates a 3D tensor of size `(4,4,3)`.

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
Marks a function as a CUDA device function, and ensures the compiler to include it in the generated CUDA source.

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

The same transform rules apply to parameter types.
