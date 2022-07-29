NVAPI Support
=============

Slang provides support for [NVAPI](https://developer.nvidia.com/nvapi) in several ways

* Slang allows the use of NVAPI directly, by the inclusion of the `#include "nvHLSLExtns.h"` header in your Slang code. Doing so will make all the NVAPI functions directly available and usable within your Slang source code.
* NVAPI is used to provide features implicitly for certain targets. For example support for [RWByteAddressBuffer atomics](target-compatibility.md) on HLSL based targets is supported currently via NVAPI.
* Direct and implicit NVAPI usage can be freely mixed. 

Direct usage of NVAPI
=====================

Direct usage of NVAPI just requires the inclusion of the appropriate NVAPI header, typically with `#include "nvHLSLExtns.h` within your Slang source. As is required by NVAPI before the `#include` it is necessary to specify the slot and perhaps space usage. For example a typical direct NVAPI usage inside a Slang source file might contain something like...

```
#define NV_SHADER_EXTN_SLOT u0 
#include "nvHLSLExtns.h"
```

In order for the include to work, it is necessary for the include path to include the folder that contains the nvHLSLExtns.h and associated headers.

Implicit usage of NVAPI
=======================

It is convenient and powerful to be able to directly use NVAPI calls, but will only work on such targets that support the mechansism, even if there is a way to support the functionality some other way.

Slang provides some cross platform features on HLSL based targets that are implemented via NVAPI. For example RWByteAddressBuffer atomics are supported on Vulkan, DX12 and CUDA. On DX12 they are made available via NVAPI, whilst CUDA and Vulkan have direct support. When compiling Slang code that uses RWByteAddressBuffer atomics Slang will emit HLSL code that use NVAPI. In order for the downstream compiler to be able to compile this HLSL it must be able to include the NVAPI header `nvHLSLExtns.h`. 

It worth discussing briefly how this mechanism works. Slang has a 'prelude' mechanism for different source targets. The prelude is a piece of text that is inserted before the source that is output from compiling the input Slang source code. There is a default prelude for HLSL that is something like 

```
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
```

If there are any calls to NVAPI implicitly from Slang source, then the following is emitted before the prelude

```
#define SLANG_HLSL_ENABLE_NVAPI 1
#define NV_SHADER_EXTN_SLOT u0
#define NV_SHADER_EXTN_REGISTER_SPACE space0
```

Thus causing the prelude to include nvHLSLExtns.h, and specifying the slot and potentially the space as is required for inclusion of nvHLSLExtns.h.

The actual values for the slot and optionally the space, are found by Slang examining the values of those values at the end of preprocessing input Slang source files. 

This means that if compile Slang source that has implicit use NVAPI, the slot and optionally the space must be defined. This can be achieved with a command line -D, throught the API or through having suitable `#define`s in the Slang source code.

It is worth noting if you *replace* the default HLSL prelude, and use NVAPI then it will be necessary to have something like the default HLSL prelude part of your custom prelude.

Downstream Compiler Include
---------------------------

There is a subtle detail that is perhaps worth noting here around the downstream compiler and `#include`s. When Slang outputs HLSL it typically does not contain any `#include`, because all of the `#include` in the original source code have been handled by Slang. Slang then outputs everything required to compile to the downstream compiler *without* any `#include`. When NVAPI is used explicitly this is still the case - the NVAPI headers are consumed by Slang, and then Slang will output HLSL that does not contain any `#include`.

The astute reader may have noticed that the new default Slang HLSL prelude *does* contain an include. So when outputs NVAPI calls from implicit use, this #include will be enabled.

```
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
```

This means that the *downstream* compiler (such as DXC and FXC) must be able to handle this include. 

As it turns out all the includes specified to Slang (via command line -I or through the API), are passed down to the include handlers for FXC and DXC. 

In the simplest use case where the path to `nvHLSLExtns.h` is specified in the include paths everything should 'just work' - as both Slang and the downstream compilers will see these include paths and so can handle the include. 

Things are more complicated if there is mixed implicit/explitic NVAPI usage and in the Slang source the include path is set up such that NVAPI is included with 

```
#include "nvapi/nvHLSLExtns.h"
```

This won't work directly with the implicit usage, as the downstream compiler includes as `"nvHLSLExtns.h"`. One way to work around this by altering the HLSL prelude such as the same `#include` is used. 

Links
-----

More details on how this works can be found in the following PR

* [Simplify workflow when using NVAPI #1556](https://github.com/shader-slang/slang/pull/1556)
