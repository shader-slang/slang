Glslang Generated
=================

This directory holds files that are generated that are need to build glslang.

https://github.com/shader-slang/glslang

In the context of Slang we build a glslang shared library. This shared library is built a project created by the regular Slang premake. That process is discussed later as there are a bunch of other steps that need to be completed before we can create the Slang glslang project and build it. 

First we need to create the `build_info.h` file in glslang. We can do this from the command line via (assuming we are in the Slang root directory). 

```
% cd external/glslang
% python build_info.py . -i build_info.h.tmpl -o ../glslang-generated/glslang/build_info.h
```

The glslang project is dependent on 'spirv-tools', so the next step is to set them up. How to do this is described in the README.md file in the external/spirv-tools-generated folder. 

## Creating and building the Slang glslang project

In normal operation our premake5.lua does not build glslang because that is a slow process, so it must be specified explicitly on the command line. 

For visual studio

```
% premake vs2015 --build-glslang=true
```

For gcc or clang (add -cc=clang)

```
% premake gmake --build-glslang=true
```

Then just build Slang as usual in visual studio, or from the linux command line for example

```
% make config=release_x64
% make config=release_x86
```