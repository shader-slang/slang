Slang Glslang
=============

This directory holds files that are generated that are needed to build glslang. The github repository that holds slangs current version of glslang is 

https://github.com/shader-slang/glslang

Building glslang depends on 

* external/spirv-headers
* external/spirv-tools

These are not external projects but files produces elsewhere and placed in the slang project.

* external/spirv-tools-generated
* external/glslang-external

To get the latest version of one of the submodules you can use

```
% git pull origin master
```

Make sure you have compatible versions of 'external/spirv-header', 'external/spirv-tools' and 'glslang' before you start!

In the context of Slang we build glslang as a Slang specific shared library/dll. The library is built from a project created by the regular Slang premake. That process is discussed later as there are a bunch of other steps that need to be completed before we can create the Slang glslang project and build it. 

First we need to create the `build_info.h` file in glslang. We can do this from the command line via (assuming we are in the Slang root directory). 

```
% cd external/glslang
% python build_info.py . -i build_info.h.tmpl -o ../glslang-generated/glslang/build_info.h
```

Next we need to create glslang/Include/revision.h. In notes it seems like this may not be needed much longer, but was needed for this build. We run the l/unix shell script `make-revision`. On windows it worked fine within cygwin (and did the previous python step). We then copy the file to `external/glslang-generated/glslang/Include/revision.h`.

The glslang project is dependent on 'spirv-tools', so the next step is to set them up. How to do this is described in the README.md file in the `external/spirv-tools-generated` folder. 

## Creating and building the Slang glslang project

In normal operation `premake5.lua` does not build glslang because it is a slow process, so it must be specified explicitly on the command line for `premake`. 

For Visual Studio 

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