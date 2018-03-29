Slang "Hello World" Example
===========================

The goal of this example is to demonstrate an almost minimal application that uses Slang for shading, and D3D11 for rendering.

The `hello.slang` file contains simple vertex and fragment shader entry points.
The `hello.cpp` file contains the C++ application code, showing how to use the Slang C API to load and compile the shader code to DirectX shader bytecode (DXBC).

Note that this example is not intended to demonstrate good practices for integrating Slang into a production engine; the goal is merely to use the minimum amount of code possible to demonstrate a complete applicaiton that uses Slang.
