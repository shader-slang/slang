NVAPI Support
=============

Slang provides support for [NVAPI](https://developer.nvidia.com/nvapi) in several ways

* Slang allows the use of NVAPI directly, by the inclusion of the `#include "nvHLSLExtns.h"` header in your Slang code. Doing so will make all the NVAPI functions directly available and usabe within your Slang source code.
* Some targets use NVAPI to provide cross platform features. For example support for [RWByteAddressBuffer atomics](target-compatibility.md) on HLSL based targets is supported currently via NVAPI. 

It should also be pointed out that is possible to mix the use of direct and implict NVAPI usage. 

Direct usage of NVAPI
=====================

Direct usage of NVAPI just requires the inclusion of the appropriate NVAPI header, typically with `#include "nvHLSLExtns.h` within your Slang source. As is required by NVAPI before the include it is necessary to specify the slot and perhaps space usage. So a typical usage inside a Slang file might look something like

```
#define NV_SHADER_EXTN_SLOT u0 
#include "nvHLSLExtns.h"
```

In order for the include to work, it is necessary for the include path to include the folder that contains the nvHLSLExtns.h and associated headers.

Implicit usage of NVAPI
=======================

It is convenient and powerful to be able to directly use NVAPI calls, but will only work on such targets that support the mechansism, even if there is a way to support the functionality some other way.

Slang provides some cross platform features on HLSL based targets that are implemented via NVAPI. For example RWByteAddressBuffer atomics are supported on Vulkan, DX12 and CUDA. On DX12 they are made available via NVAPI, whilst CUDA and Vulkan have direct support. When compiling Slang code that uses RWByteAddressBuffer atomice Slang will emit HLSL code that use NVAPI. In order for the downstream compiler to be able to compile this HLSL it must be able to include the NVAPI header `nvHLSLExtns.h`. 

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

Links
-----

More details on how this works can be found in the following PR

* [Simplify workflow when using NVAPI #1556](https://github.com/shader-slang/slang/pull/1556)