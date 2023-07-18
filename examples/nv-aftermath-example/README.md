Nsight Aftermath Crash Example
==============================

* Demonstrates use of aftermath API to capture a dump with a GPU crash
* Uses the [obfuscation feature](https://github.com/shader-slang/slang/blob/master/docs/user-guide/a1-03-obfuscation.md)
* Uses an `emit` source map
* Demonstrates use of file system compile products
* Forces a crash via time out, executing a shader that is purposefully slow
* Can be used to capture D3D and Vulkan (change the device type in the sample)
* When enabled GFX is built to use Aftermath it's debug layer 
  * This disables D3D debug layer, as not possible to have both enabled
* NOTE! Will only capture Aftermath DebugInfo with a *debug* build
  * Gfx enables debugging info on debug builds

The example requires aftermath SDK to be in the directory "external/nv-aftermath".

This example demo is not enabled by default, as it requires the [Nsight aftermath SDK](https://developer.nvidia.com/nsight-aftermath). 

Enabling requires 

* Passing "--enable-aftermath=true" to the command line of `premake`. 
* Having a copy of the [Nsight aftermath SDK](https://developer.nvidia.com/nsight-aftermath) in `externals/nv-aftermath` directory.

On windows the following would be reasonable..

```
premake vs2019 --deps=true --enable-aftermath=true
```

Typically D3D debug run produces the following files...

* fragment-0.dxil               - Fragment DXIL
* fragment-0.map                - The emit source map, maps locations in the the fragment kernel to the obfuscated source file
* vertex-0.dxil                 - Vertex DXIL
* vertex-0.map                  - The emit source map, maps locations in the vertex kernel to the obfuscated source file
* XXXX-obfuscated.map           - The obfuscated source map. Will be referenced by the other source maps. Maps obfuscated locations to the original source
* aftermath-dump-X.bin          - The Aftermath crash captures
* aftermath-debug-info-X.bin    - The Aftermath debug infos

Having emit source maps, can be useful as discussed in [the documentation](https://github.com/shader-slang/slang/blob/master/docs/user-guide/a1-03-obfuscation.md).

If Vulkan is enabled spv files will be emitted.

If emit source maps are disabled, emit source maps such as `fragment-0.map`/`vertex-0.map` will *not* be produced, as the mapping the the obfuscated source file are embedded into the emitted source and therefore kernels.

The example source describes how to switch between emit source files, and different devices. 

## Links

* [nsight aftermath](https://developer.nvidia.com/nsight-aftermath)
* [obfuscation](https://github.com/shader-slang/slang/blob/master/docs/user-guide/a1-03-obfuscation.md)
* [source map](https://github.com/source-map/source-map-spec)
