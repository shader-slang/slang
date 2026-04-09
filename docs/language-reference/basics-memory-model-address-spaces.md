# Address Spaces

An address space defines a view into memory. It determines how memory objects are addressed and
accessed by threads. In Slang, the address space for a variable is determined by its type, modifiers, and
attributes. For example, data in `ConstantBuffer<T>` belongs to the uniform address space, and the address
space for a `static groupshared` variable is group-shared.

The following address spaces are defined:

<table>
<tr>
  <th>Address space</th>
  <th>Slang construct</th>
  <th>Instance scope</th>
  <th>Description</th>
</tr>
<tr>
  <td>Uniform</td>
  <td><code>uniform</code><br>
      <code>ConstantBuffer&lt;T&gt;</code> </td>
  <td>All threads</td>
  <td>Uniform values are parameters of a Slang program. They are expected to remain constant over the lifetime of a launch/dispatch.</td>
</tr>
<tr>
  <td>Image</td>
  <td><a href="../../../core-module-reference/types/texture1d-08.html">Texture1D&lt;...&gt;</a><br>
      <a href="../../../core-module-reference/types/texture2d-08.html">Texture2D&lt;...&gt;</a><br>etc.</td>
  <td>All threads</td>
  <td>Images are multi-dimensional arrays of texels, typically containing color, depth, stencil, and similar data.</td>
</tr>
<tr>
  <td>Push constant</td>
  <td><a href="../../../core-module-reference/attributes/push_constant.html">[push_constant]</a><br>
      <a href="../../../core-module-reference/attributes/vk_push_constant.html">[vk_push_constant]</a></td>
  <td>All threads</td>
  <td>Push constants are small, frequently updated constants passed directly through the command stream. On Vulkan, these correspond to push constants.</td>
</tr>
<tr>
  <td>Storage buffer</td>
  <td><a href="../../../core-module-reference/types/structuredbuffer-0a/index.html">StructuredBuffer&lt;T&gt;</a><br>
      <a href="../../../core-module-reference/types/rwstructuredbuffer-012c/index.html">RWStructuredBuffer&lt;T&gt;</a></td>
  <td>All threads</td>
  <td>Storage buffers are read-only or read-write buffers, typically shared between the host and Slang programs.</td>
</tr>
<tr>
  <td>Group-shared</td>
  <td><code>static groupshared</code> (global scope)</td>
  <td>Thread group</td>
  <td>Group-shared variable instances are shared by a thread group.</td>
</tr>
<tr>
  <td>Function</td>
  <td>Function parameters and non-static local variable declarations</td>
  <td>Thread</td>
  <td>Function parameters and local variables are visible only to the function invocation.</td>
</tr>
<tr>
  <td>Thread-local</td>
  <td><code>static</code> (global scope)</td>
  <td>Thread</td>
  <td>Thread-local variables have separate instances per thread.</td>
</tr>
<tr>
  <td>Input</td>
  <td>Entry point input parameters (non-uniform)</td>
  <td>Thread</td>
  <td>Entry point inputs to a graphics shader stage come from the system and the previous stage.</td>
</tr>
<tr>
  <td>Output</td>
  <td>Entry point output parameters<br>Entry point return value</td>
  <td>Thread</td>
  <td>Entry point outputs from a graphics shader stage are inputs to the next stage.</td>
</tr>
<tr>
  <td>Specialization constant</td>
  <td><a href="../../../core-module-reference/attributes/specializationconstant-0e.html">[SpecializationConstant]</a><br>
      <a href="../../../core-module-reference/attributes/vk_specialization_constant.html">[vk::specialization_constant]</a></td>
  <td>All threads</td>
  <td>Vulkan specialization constants have values that are fixed at pipeline creation time.</td>
</tr>
<tr>
  <td>Host</td>
  <td>None</td>
  <td>Host process</td>
  <td>The host address space is used by the host program. This is typically the virtual
    address space of the application process. The host address space is not usually directly accessible by a
    Slang program, except when the program is compiled for the C++ target.</td>
</tr>
</table>

> 📝 **Remark 1:** Address spaces specific to graphics pipeline stages are not enumerated here. See
> [Graphics Shaders and Compute Kernels](shaders-and-kernels.md) for a description.

> 📝 **Remark 2:** A pointer to memory in one address space is generally not interchangeable with a pointer to
> memory in another address space. In particular, pointers to group-shared memory are not interchangeable
> between thread groups.

> 📝 **Remark 3:** Address spaces in Slang are roughly equivalent to SPIR-V storage classes.
