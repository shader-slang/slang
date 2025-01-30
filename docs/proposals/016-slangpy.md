# SP#016: SlangPy

## Status

Author: Chris Cummings, Benedikt Bitterli, Sai Bangaru, Yong He

Status: Design Review

Implementation:

Reviewed by:

## Background

With first class support of automatic differentiation, Slang makes it easy to write differentiable SIMT kernel code that seamlessly interops with GPU accelerated graphics features such as ray tracing. Slang has the potential to bridge traditional graphics applications with machine learning techniques, to help researchers and developers in both the Graphics and the AI communities to rapidly prototype and deploy high-performance, reusable SIMT kernel code for their neural applications.

However, making kernel code easy to write is half the story. Today slang users who want to use slang from within python, e.g. to train a neural graphics MLP, have to write thousands of lines of glue code to expose Slang and simple graphics apis to PyTorch. We want to make it effortless to get going with neural graphics algorithms, so we want to remove the need for this boilerplate.

## Goals

Since most of the AI community is on Python, we propose to introduce a Python package called `slangpy` that allows python code to call Slang shaders easily. 

It is worth noting before going into SlangPy goals further, we actually tried a previous version of Slang<->Python binding in the SlangTorch project. The design here is highly influenced by learnings there — those learnings are summarized below, but broadly, we want to expose the full range of Slang features to python, allowing python code to run all forms of Slang code not just on CUDA, but on all target graphics APIs that Slang supports, without a hard dependency on PyTorch.
With that in mind, the SlangPy package is to enable rapid prototyping of graphics/machine learning applications with Slang being the kernel language. To achieve this goal, SlangPy should offer the following features:

- Python abstraction of GPU features, with simple to use functions and classes to initialize and control GPU execution.
- Hide unnecessary low level graphics API concepts such as command encoding and parameter passing, and allow use of the graphics pipeline with a couple of lines of python code instead of thousands of lines of C++ code.
- Allow Slang code to be called from Python as directly as possible through automatic boilerplate generation to allow calling any slang function on a tensor/array domain.
- Support authoring differentiable renderers by allowing automatic differentiation of graphics shader entrypoints for both rasterization and raytracing pipelines. 
- Interop with PyTorch without hard dependency on PyTorch. Available for users who need to work with PyTorch, but for those who don't, there is no need to install pytorch to use SlangPy.

## Proposed Approach

The key ideas behind `slangpy` are:

- We use Slang's reflection API to understand the structure of Slang code, and expose the Slang functions as callable functions inside python objects.
- When loading a Slang module into Python, you connect that module to a SlangPy device — this device is where the slang functions should run when called, and allows you to specify e.g. which gpu, which api [Vulkan, OptiX, CUDA] etc.
- SlangPy's binding model aims to make simple, unambiguous calls very easy, with little to no extra syntax. However for complex or ambiguous calls, a 'map' method is provided that allows the user to be explicit about how to vectorize a function and resolve any ambiguities.
- The execution model of SlangPy is that codegen and shader compilation for a given function all happens at the function call time. We then cache the generated code on disk so that subsequent runs have minimal overhead.


We will develop of SlangPy in a parallel GitHub repo to slang, named slangpy. This is because we would like SlangPy to have a different development and release cadence from the slang compiler, and we want to avoid having slang users who are building from source pull in unrelated dependencies if they are not using SlangPy.

In keeping with Slang's general feature process, the SlangPy feature set will be treated as experimental - and may stay this way for quite some time. This is because the space is fairly dynamic, and we expect as we try to use this with real developers, the interfaces may change quite a lot.

### Calling Slang Entrypoints From Python
The most basic level of service that SlangPy provides is to allow Python code to run shaders written in Slang. For example, given an `add` function in Slang:

```hlsl
// A simple function that adds two numbers together
float add(float a, float b)
{
    return a + b;
}
```

The user can define a wrapping compute shader entrypoint to run the `add` function on two input buffers, and store the result in an output buffer:

```hlsl
// example.slang

[numthreads(128, 1, 1)]
void addBuffer(NDBuffer<float, 1> a, NDBuffer<float, 1> b, NDBuffer<float, 1> result, int i : SV_DispatchThreadID)
{
     result[i] = add(a[i], b[i]);
}
```

Here the `NDBuffer<T, N>` type is defined by SlangPy to represent a `N`-dimensional structured buffer of element type `T`.

With SlangPy, the user can run this `addBuffer` compute shader with a few Python calls:

```python
# example.py

import slangpy as spy

# Create a slangpy GPU execution context
device = spy.init_device()

# Load a slang the module
module = spy.Module.load_from_file(device, "example.slang")

# Create numpy arrays
a = numpy.random.rand(1024)
b = numpy.random.rand(1024)
r = numpy.zeros(1024);

# Create buffers for kernel call
bufferA = spy.NDBuffer(device, shape = (1024,))
bufferB = spy.NDBuffer(device, shape = (1024,))
bufferRs = spy.NDBuffer(device, shape = (1024,))

# Populate buffer contents
bufferA.from_numpy(a)
bufferB.from_numpy(b)
bufferRs.from_numpy(r)

# Call the function and print the result
module.addBuffer.dispatch(1024, bufferA, bufferB, bufferRs)
print(bufferRs)
```

As you can see, calling Slang code using raw dispatch requires the user to write a lot of boilerplate code in both Slang and Python. To simplify this task, SlangPy also provides a next level of services: marshaling of common Python types and automatic wrapping of functions into kernels, to eliminate the need to explicitly define the compute shader entry point in Slang, and to create buffers manually in Python.

### Calling Slang Functions Directly From Python

The following code achieves the same `add` function, without writing boilerplate for the compute shader entrypoint, or buffer marshalling in Python.

First, our slang module needs to provide a definition for `add`, but no need to define `addBuffer` entrypoint.

```hlsl
// example.slang

// A simple function that adds two numbers together
float add(float a, float b)
{
    return a + b;
}

```

And we will allow this function to be called from Python directly:

```python
import slangpy as spy

# Create a SlangPy GPU execution context
device = spy.init_device()

# Load a slang the module
module = spy.Module.load_from_file(device, "example.slang")

# Call the function and print the result
pa = numpy.random.rand(1024)
pb = numpy.random.rand(1024)

result = module.add(pa, pb)
print(result)
```

When the user calls `add` directly in this style, SlangPy will use Slang's reflection API to find out the parameter types of `a` and `b`, and infer how to wrap the function into a kernel with the runtime type of Python variables `pa` and `pb`. Since the `add` is taking two float scalar parameters, and the user is passing two numpy arrays, SlangPy knows it needs to generate a wrapper kernel entrypoint that dispatches the `add` function over a thread grid of size 1024, and create temporary buffers to hold the input and output contents.

### Device Object
The user starts with creating a SlangPy `device`. This represents a GPU execution context from where you can load slang modules and run Slang code on a GPU device. You can specify what target API you want to run the Slang  code on. Currently we support creating a D3D12, Vulkan or CUDA+Optix backed device.

### Type Marshaling

SlangPy generates code that marshals between Slang and Python. The binding model we use is designed to make the common case easy, without requiring boilerplate. When the binding code detects ambiguity, then the call will fail and require the user to make a more explicit call. 
The result is that we are aligned on a principle to make the common case easy, but don't try to be clever when there is ambiguity, and the user is required to call  `map()` on the function to explicitly map buffer dimensions to parameters.

#### Marshaling of basic types

A basic scalar integer or float can be passed to a slang function as is:
```hlsl
// test.slang
void test(int x, float y) { … }
```

```python
# test.py
module = spy.Module.load_from_file(device, “test.slang”)

module.test(5, 6.0) // OK, passing a basic scalar types as is.
```

#### Marshaling of array, vector and matrix types

You can also map Python and numpy arrays to vectors, matrices, or arrays:
```
// test.slang
void test1(float3 x) {}
void test1(float3 x[2]) {}
void test2(float3x3 x) {}
```

```
# test.py
module = spy.Module.load_from_file(device, “test.slang”)

module.test1([5, 6, 7]) // OK, passing a matching size array to a vector.
module.test2([[1,2,3],[5, 6, 7]]) // OK, passing matching size array to a vector array.
module.test2(numpy.zeros(3,2)) // OK, passing a numpy array of matching dimensions.
module.test3(numpy.zeros(3,3)) // OK, passing a numpy array of matching dimensions.
```

#### Marshaling of a struct types

SlangPy should allow users to pass a python dictionary to a function parameter that accepts a struct:
```hlsl
// test.slang
struct MyType { int x; float y; }
void test(MyType v) { ... }
```

```python
# test.py
module = spy.Module.load_from_file(device, “test.slang”)

module.test({"x":5,  "y":6.0}) # OK, passing a tuple to a struct type.
```

In case of ambiguity, the user can be explicit from the python side what Slang type should the dictionary be converted to, before used as argument:

```python
# test.py
module.test({'_type': 'MyType', 'x': 5, 'y': 6.0})
```

### NDBuffer
SlangPy provides an `NDBuffer` type to represent a device-side buffer that can hold any data:

A buffer can be created by calling the `NDBuffer` constructor:

```python
image_1 = spy.NDBuffer(device, dtype=module.Pixel, shape=(16, 16))
image_2 = spy.NDBuffer(device, dtype=module.Pixel, shape=(16, 16))
```

The buffer can be initialized from a numpy array:

```python
image_2.from_numpy(0.1 * np.random.rand(16 * 16 * 3).astype(np.float32))
```

Buffers can be used to call Slang functions directly:

```python
result = module.add(image_1, image_2)
```

In this scenario, `module.add` is the Python side object that maps to the `add` function in `example.slang`. Recall that `add` is defined as:

```hlsl
float add(float a, float b)
{
    return a + b;
}
```

When this Slang function is called with `a` and `b` being buffers, SlangPy will automatically generate a compute shader entrypoint that dispatches to `add` for every corresponding element in buffer `a` and `b`, and then invokes the generated compute shader. This is called automatic __broadcasting__. SlangPy will also automatically allocate a new buffer to hold the result of the computation and return the buffer object from the call to `module.add`.
If a more complex pattern of buffer element to execution thread mapping is required, SlangPy provides the 'map' function to be explicit about what argument dimensions map to what execution threads (kernel dimensions).

On the Python side, we also provide a `NDDifferentiableBuffer` type that acts as a pair of `NDBuffer`s of the same shape, to hold both the primal and the gradient values. A `NDDifferentiableBuffer` object can be passed directly to a `DifferentialPair` typed parameter in Slang.

### Function Objects

For each Slang function, SlangPy will create a python side function object during loading of the slang module, which will allow users to invoke from python.

The function object provides methods and properties to allow invoking the slang function in various ways.

Specifically, a function object provides:

- `call(args...)`: Call the function with a given set of arguments. This will generate and compile a new kernel if need be, then immediately dispatch it and return any results.
- `append_to(args...)`: Similar to call, but appends the dispatch to a command list for future submission rather than immediate dispatch.
- `constants(dict[str,any])`: specify link time constants to specialize the function.
- `set()`: set additional global uniform parameters.
- `map()`: specifies how to map each argument dimensions to kernel dimensions.
- `bwds` property: returns a function object for the backward derivative propagation function.
- `return_type()`: specify the desired return type of the function when called from python.

### Interop With Pytorch

Slangpy supports creating a NDBuffer from a pytorch tensor:

```python
t = torch.tensor(...)
buffer = spy.NDBuffer(t)
```

And you can create a PyTorch tensor from an NDBuffer:

```python
torchTensor = buffer.torch()
```

### Example: Training An MLP with SlangPy

Along with the core functionalities of SlangPy, we should also ship a MLP library in SlangPy's release package, to make using and training MLPs easy.

You can write a slang function that uses an MLP to compute a value:
```slang
// example.slang
import mlp;

[Differentiable]
float[8] compute(MLP<32,8> mlp, float input[32])
{
     return mlp.run(input);
}
```
And you can call `compute` from python as:

```python
import slangpy as spy

device = spy.init_device();
module = device.load_module_from_file(“example.slang”)
                                      
N = 128
inputTensor = spy.Tensor.numpy(device, numpy.random.rand(N, 32).astype(numpy.float32))
outputTensor = spy.Tensor.numpy(device, numpy.random.rand(N, 8).astype(numpy.float32))
outputTensor = module.compute(mlp_weights, inputTensor, _result=Tensor)
```

To train an MLP, you can define a loss function in Slang, and call the backward derivative of `loss` and use it in a training loop:

```slang
// example.slang

[Differentiable]
float loss(MLP<32,8> mlp, float input[8], float target[8])
{
    float result = 0.0;
    let output = compute(mlp, input);
    for (int i = 0; i < 8; i++) result += square(output[i], target[i])
    return result;
}

void updateWeights(inout float param, float grad, float step)
{
    param -= grad * step;
}
```

In python code, you can invoke the backward derivative of `loss` with `module.loss.bwds`:

```python
# example.py
from slangpy.types import Tensor

max_iterations = 1e5 
mlp_weights = Tensor.from_numpy(device, numpy.random.rand(32, 8).astype(numpy.float32))
mlp_weights = mlp_weights.with_grads()

lossBuffer = Tensor.zeros(device, dtype=module.float, shape=(N, ))
lossGrad = Tensor.from_numpy(device, numpy.ones(shape=(N, ), dtype=numpy.float32))
lossBuffer = lossBuffer.with_grads(grad_in=lossGrad)

targetOutput = Tensor.from_numpy(device, numpy.random.rand(N, 8).astype(numpy.float32))

learning_rate = 1e-4

for iter in range(iterations)::
    mlp_weights.grad_out.clear()
    module.loss.bwds(mlp_weights, inputTensor, targetOutput, _result=lossBuffer)

    module.updateWeights(mlp_weights.flatten_dtype(), mlp_weights.grad_out.flatten_dtype(), learning_rate)

    currentLoss = lossBuffer.to_numpy().mean()

    print(f'Iteration {iter}. Loss:{currentLoss}')

```

## Learnings from Slang Torch

We currently provide a slang-torch python package to allow Slang code to beused as a PyTorch kernel. Slang-torch also provides python functions to load a slang module, and call an exported Slang function from python.

The main feedback we received from slang-torch users is that it still requires developers to write a substantial amount of boilerplate both in Slang to wrap ordinary functions in entrypoints, and in Python to setup the parameters for the kernel call. These boilerplate setups slow down iteration.

Another issue with slang-torch is that it is tightly coupled with PyTorch tensors. For users who are not working with PyTorch, this dependency is very inconvenient.

Finally, slang-torch uses CUDA as the only execution target, while SlangPy is cross-platform and can run using D3D, Vulkan and CUDA/Optix as the underlying API to bring access to a much wider range of GPU features.

This proposal aims at addressing these issues in slang-torch, and make it truly effortless to use Slang from Python.
