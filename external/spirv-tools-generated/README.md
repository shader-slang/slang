Slang SPIR-V Tools
==================

The spir-v tools are needed in the Slang project in order to be able to build glslang. We don't necessarily want/neet to build all the spir-v tools - but we do need the files that are generated as part of this process. Those files are then stored in this folder, so that they can just be used without needing to be created as part of the Slang build process.

To build spirv-tools we need [cmake](https://cmake.org/download/). On windows we can use cmake with the gui interface. 

Inside the `external/spirv-tools` directory make a directory `build.vs` which is where we are going to generate all the files.

You may need to make sure you have the other dependencies that spirv-tools requires as described on their github main page...

https://github.com/KhronosGroup/SPIRV-Tools

At the time of writing in `external/spirv-tools` the following were needed

```
git clone https://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers
git clone https://github.com/google/effcee.git external/effcee
git clone https://github.com/google/re2.git external/re2
```

Next run the cmake gui. Set the source path to be `external/spirv-tools` (in the slang directory), and then set the 'where to build binaries' to `external/spirv-tools/build.vs` (or however you named that file. Then click `configure` and once that is done 'generate'. 

Now go into to the `build.vs` directory and open `spirv-tools.sln` with Visual Studio and compile. This will generate many of the files needed, once regular C++/C compilation has started all of the files should have been created. 

Take the files with '.inc' and '.h' extensions from the build.vs directory and copy it into this directory.