# Address Spaces and Storage Classes

## Address Spaces

An address space defines a view into physical memory. When a physical memory object is mapped to an address
space, it is assigned an address (or address range) within the address space. Then,
the memory object can be accessed by the execution context associated with the address space.

In Slang, the following logical address spaces are defined:

<table>
<tr>
  <th>Address space</th>
  <th>Scope</th>
  <th>Description</th>
</tr>
<tr>
  <td>Host</td>
  <td>Process</td>
  <td>The host address space is the address space used by the host program. This is typically the virtual
    address space of the application process. The host address space is not usually directly accessible by a
    Slang program, except when the program is compiled for the C++ target.</td>
</tr>
<tr>
  <td>Device</td>
  <td>GPU context</td>
  <td>The device address space contains memory accessible by all threads in the GPU context.</td>
</tr>
<tr>
  <td>Group-shared</td>
  <td>Thread group</td>
  <td>The group-shared address space contains memory accessible by all threads in the thread group.</td>
</tr>
<tr>
  <td>Thread-private</td>
  <td>Thread</td>
  <td>The memory is private to the thread. It cannot be shared with other threads. Function-local memory
    is considered thread-private.</td>
</tr>
</table>

In Slang, address spaces are not directly accessible by name or keyword. They are described here for context.


> 📝 **Remark 1:** The term *logical address space* is used here. The logical address spaces may be
> implemented using one or more virtual or physical address spaces.

> 📝 **Remark 2:** The *GPU context* for Slang program execution is not defined by Slang. Instead, it is
> defined by the graphics/compute API used to launch Slang programs.


## Storage Classes

A storage class determines the memory location, the address space, and other characteristics of memory. In
Slang, the storage class for a variable is determined by its type, modifiers, and attributes. For example,
data in `ConstantBuffer<T>` belongs to the uniform storage class, and the storage class for a
`static groupshared` variable is group-shared.

The following storage classes are defined by Slang:

<table>
<tr>
  <th>Storage class</th>
  <th>Slang construct</th>
  <th>Address space</th>
  <th>Description</th>
</tr>
<tr>
  <td>Uniform</td>
  <td><code>uniform variable</code><br>
      <code>ConstantBuffer&lt;T&gt;</code> </td>
  <td>Device</td>
  <td>Uniform values are parameters of a Slang program. They are expected to remain constant over the lifetime of a launch/dispatch.</td>
</tr>
<tr>
  <td>Image</td>
  <td><code>Texture1D&lt;...&gt;</code><br><code>Texture2D&lt;...&gt;</code><br>etc.</td>
  <td>Device</td>
  <td>Storage class for image data.</td>
</tr>
<tr>
  <td>Push constant</td>
  <td><code>[push_constant]</code><br><code>[vk_push_constant]</code></td>
  <td>Device</td>
  <td>Small, frequently updated constants passed directly in the command stream. On Vulkan, these correspond to push constants.</td>
</tr>
<tr>
  <td>Storage buffer</td>
  <td><code>StructuredBuffer&lt;T&gt;</code><br><code>RWStructuredBuffer&lt;T&gt;</code></td>
  <td>Device</td>
  <td>Storage buffers are read-only or read-write buffers, typically shared between the host and Slang programs.</td>
</tr>
<tr>
  <td>Group-shared</td>
  <td><code>static groupshared</code> (global scope)</td>
  <td>Thread group</td>
  <td>Memory shared by a thread group.</td>
</tr>
<tr>
  <td>Function</td>
  <td>Function parameters and non-static local variable declarations</td>
  <td>Thread</td>
  <td>Visible only to the function invocation.</td>
</tr>
<tr>
  <td>Thread-local</td>
  <td><code>static</code> (global scope)</td>
  <td>Thread</td>
  <td>Global variables with a separate instance per thread.</td>
</tr>
<tr>
  <td>Input</td>
  <td>Entry point input parameters (non-uniform)</td>
  <td>Thread</td>
  <td>Inputs to a graphics shader stage from the system and the previous stage.</td>
</tr>
<tr>
  <td>Output</td>
  <td>Entry point output parameters<br>Entry point return value</td>
  <td>Thread</td>
  <td>Outputs from a graphics shader stage to the next stage.</td>
</tr>
<tr>
  <td>Specialization constant</td>
  <td><a href="../../../core-module-reference/attributes/specializationconstant-0e.html">[SpecializationConstant]</a><br>
      <a href="../../../core-module-reference/attributes/vk_specialization_constant.html">[vk::specialization_constant]</a></td>
  <td>None</td>
  <td>Vulkan specialization constants are compile-time constants whose values are fixed at pipeline creation
    time.</td>
</tr>
</table>


> 📝 **Remark 1:** Storage classes specific to graphics pipeline stages are not enumerated here. See
> [Graphics Shaders and Compute Kernels](shaders-and-kernels.md) for a description.

> 📝 **Remark 2:** A pointer to memory in one storage class is generally not interchangeable with a pointer to
> memory in another storage class. In particular, pointers to group-shared memory are not interchangeable
> across thread groups.

> 📝 **Remark 3:** Storage class "group-shared" and address space "group-shared" share the name intentionally.
