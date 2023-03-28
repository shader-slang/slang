---
layout: user-guide
---

Using Slang to Write PyTorch Kernels
=========================================================

If you are a PyTorch user looking for a way to write complex, high performance and automatically differentiated kernel functions in a per-thread instead of full-tensor style, give Slang a try. Slang is evolved from on a traditional shading language that were designed
to provide a simple way to define kernel functions that runs extremely fast in graphics applications. With the latest addition of
automatic differentiation and PyTorch interop features, Slang provides a streamlined solution to author auto-differentiated kernels
that runs at the speed of light with a strongly typed, per-thread programming model.

# Getting Started with `slangpy`

In this tutorial, we will use a simple example to walkthrough the steps to use Slang in your PyTorch project.

## Writing a simple kernel function as a Slang module

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

    if (globalIdx.x > input.size(0) || globalIdx.x > input.size(1))
        return;
    float result = square(input.load(globalIdx.x, globalIdx.y));
    output.store(globalIdx.x, globalIdx.y, result);
}
```

This code follows the standard pattern of a typical CUDA kernel function. It takes as input
two tensors, `input` and `output`. 
It first obtains the global dispatch index of the current thread and performs range check to make sure we don't read or write out
of the bounds of input and output tensors, and then calls `square()` to compute the per-element result, and
finally calls `output.store` to store the result into the corresponding location.

With a kernel function defined, we then need to expose a CPU(host) function that defines how this kernel is dispatched:
```csharp
[TorchEntryPoint]
TorchTensor<float> square_fwd(TorchTensor<float> input)
{
    var result = TorchTensor<float>.alloc(input.size(0), input.size(1));
    let blockCount = uint3(1);
    let groupSize = uint3(result.size(0), result.size(1), 1);
    __dispatch_kernel(square_fwd_kernel, blockCount, groupSize)(input, result);
    return result;
}
```
Here, we first call `TorchTensor<float>.alloc` to allocate a 2D-tensor that has the same size as the input.
This function returns a `TorchTensor<float>` object that represents a CPU handle of a PyTorch tensor.
Then we launch `square_fwd_kernel` with the `__dispatch_kernel` syntax. Note that we can directly pass
`TorchTensor<float>` arguments to a `TensorView<float>` parameter and the compiler will automatically convert
the type and obtain a view into the tensor that can be accessed by the GPU kernel function.

## Calling Slang module from Python

Next, let's see how we can call the `square_fwd` function we defined in the Slang module.
To do so, we use a python package called `slangpy`. You can obtain it with

```bash
pip install slangpy
```

With that, you can use the following code code call Slang function from Python:

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
It may take a little longer the first time you execute the script, but the result will be cached and as
long as the kernel code is not changed, future runs will not rebuild the CUDA kernel.

Because the PyTorch JIT system requires `ninja`, you need to make sure `ninja` is installed on your system
and is discoverable from the current environment.

## Exposing an automatically differentiated kernel to PyTorch

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

With that, we can now define `square_bwd_kernel` that performance backward propagation as:

```csharp
[CudaKernel]
void square_bwd_kernel(TensorView<float> input, TensorView<float> grad_in, TensorView<float> grad_output)
{
    uint3 globalIdx = cudaBlockIdx() * cudaBlockDim() + cudaThreadIdx();

    if (globalIdx.x > input.size(0) || globalIdx.x > input.size(1))
        return;

    DifferentialPair<float> dpInput = diffPair(input.load(globalIdx.x, globalIdx.y));
    var gradInElem = grad_in.load(globalIdx.x, globalIdx.y);
    __bwd_diff(square)(dpInput, gradInElem);
    grad_output.store(globalIdx.x, globalIdx.y, dpInput.d);
}
```

Note that the function follows the same structure of `square_fwd_kernel`, with the only difference being that
instead of calling into `square` to compute the forward value for each tensor element, we are calling `__bwd_diff(square)`
that represents the automatically generated backward propagation function of `squre`.
`__bwd_diff(squre)` will have the following signature:
```csharp
void __bwd_diff_squre(inout DifferentialPair<float> dpInput, float dOut);
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

Similarly to `squre_fwd`, we can define the host side function `square_bwd` as:

```csharp
[TorchEntryPoint]
TorchTensor<float> square_bwd(TorchTensor<float> input, TorchTensor<float> grad_in)
{
    var grad_out = TorchTensor<float>.alloc(input.size(0), input.size(1));
    let blockCount = uint3(1);
    let groupSize = uint3(input.size(0), input.size(1), 1);
    __dispatch_kernel(square_bwd_kernel, blockCount, groupSize)(input, grad_in, grad_out);
    return grad_out;
}
```

You can refer [this documentation](07-autodiff.md) for a detailed reference of Slang's automatic differentiation system.

With this, the python script `slangpy.loadModule("square.slang")` will now return
a scope that defines two functions, `square_fwd` and `square_bwd`. We can then use these
two functions to define a PyTorch autograd kernel class:

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

