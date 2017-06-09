Spire "Hello World" Example
===========================

The goal of this example is to demonstrate an almost minimal application that uses Spire for shading, and D3D11 for rendering.

The `hello.spire` file contains a simple declaration of a Spire *shader module*, along with a *pipeline declaration* that will be used for mapping shader code to the capabilities of the "engine" (in this case, just vertex and fragment shaders).
The `hello.cpp` file contains the C++ application code, showing how to use the Spire C API to load and compile the shader code, and construct a (trivial) executable shader from Spire modules.

Note that this example is not intended to demonstrate good practices for integrating Spire into a production engine; the goal is merely to use the minimum amount of code possible to demonstrate a complete applicaiton that uses Spire.
