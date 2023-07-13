Aftermath Crash Example
=======================

* Demonstrates use of aftermath API
* Uses source map obfuscation

The example requires aftermath SDK to be in the directory "external/nv-aftermath".

The example demo is not enabled by default. Enabling can be achieved by passing "--enable-aftermath=true" to the command line of premake. So on windows the following would be reasonable. 

```
premake vs2019 --deps=true --enable-aftermath=true
```