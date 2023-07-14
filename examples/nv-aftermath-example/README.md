Nsight Aftermath Crash Example
==============================

* Demonstrates use of aftermath API
* Uses source map obfuscation
* Demonstrates use of file system compile products

The example requires aftermath SDK to be in the directory "external/nv-aftermath".

The example demo is not enabled by default. Enabling can be achieved by passing "--enable-aftermath=true" to the command line of premake. So on windows the following would be reasonable. 

```
premake vs2019 --deps=true --enable-aftermath=true
```

* [nsight aftermath](https://developer.nvidia.com/nsight-aftermath)
* [obfuscation](https://github.com/shader-slang/slang/blob/master/docs/user-guide/a1-03-obfuscation.md)
* [source map](https://github.com/source-map/source-map-spec)
