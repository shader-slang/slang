# Slang

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/kt9ch5niwkslk5p4/branch/master?svg=true)](https://ci.appveyor.com/project/tangent-vector/slang/branch/master)

Slang is a library for compiling real-time shader code.
It can be used with either existing HLSL or GLSL code, or with code written directly in Slang.
The library provides a variety of services that application developers can use to put together the shader compilation workflow they want.

Services provided by the Slang library include:

* Slang can scan ordinary HLSL or GLSL code that neglects to include `register` or `layout` bindings and "rewrite" that code to include explicit bindings for all shader parameters. This lets you write simple and clean code, but still get deterministic binding locations.

* Slang provides a full reflection API for shader parameters, with a uniform interface across HLSL, GLSL, and Slang. The reflection system does not silently drop unused/"dead" parameters, and it can even be used on libraries of shader code without compiling any entry points.

* *Work in progress:* Slang supports cross-compilation of either HLSL or Slang code to GLSL.

* You can directly get HLSL or GLSL source code as output from Slang, or you can let the library invoke lower-level code generation for you to get back DXBC or SPIR-V (DXIL support to come).

## Documentation

TODO

## Getting Started

TODO

## Testing

TODO

## Contributing

TODO

## Contributors

* Yong He
* Haomin Long
* Teguh Hofstee
* Tim Foley
