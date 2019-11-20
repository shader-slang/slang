Slang Linking
=============

The slang feature around libraries and linking are a *work in progress*. Future versions of Slang are likely to change the API and binary compatibility. 

In many languages it is possible to compile source files into binaries - such as libraries and object files, and then these files can be combined perhaps with other source files to produce the final result, such as an executable. Slang now has experimental support for such a feature. 

To make such a feature work we need a few abilities

* To create a library from source (in our case a Slang IR serialized library)
* A way for symbols to be exported and imported
* Linkage - ability to combine source/libraries to produce things (such as other libraries, executables, etc)
* A way to specify libraries to be used in linkage

Libraries
---------

A library can be created from source from the Slang command line with the `-o` option naming the output file. We use `-no-codegen` because we don't want to generate any *target* code. This is important because of how the feature currently works, if it contains any types that have any kind of binding it should be considered platform specific. Also the file extension `slang-lib` must be given to indicate how the library will be stored. In this case we want the information stored in a Slang IR serialization which is what slang-lib indicates.

```
slangc -no-codegen tests/serialization/extern/module-a.slang -o tests/serialization/extern/module-a.slang-lib
```

From the slang API we, need to indicate the container format and to disable target code generation 

```
    SlangCompileRequest* compileRequest = ...;
    SlangCompileFlags slangCompileFlags = ...;
    
    slangCompileFlags |= SLANG_COMPILE_FLAG_NO_CODEGEN;

    spSetOutputContainerFormat(compileRequest, SLANG_CONTAINER_FORMAT_SLANG_MODULE);
```

When the compilation is completed the resulting serialized IR code can be accessed via 

```
// To get the contents as a blob - whose scope can be maintained by the application

{
    ComPtr<ISlangBlob> blob;
    if (SLANG_SUCCEEDED(spGetContainerCode(compileRequest, blob.writeRef())))
    {
        // Do something with the blob
    }
}

// To directly get the contents, will only remain valid as long as compileRequest stays in scope
// and with the associated spCompile
{
    size_t size;
    const void* data = spGetCompileRequestCode(compileRequest, size);
}
```

Libraries can contain entry points. It is necessary though that the entry point is specified during the compilation. This can be achieved by specifying a function as an entry point via the API or command line. Entry points can also be specified via the `[shader()]` attribute for example.

```
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // ... 
}
```

Symbols
-------

Linkage - the combining of libraries and source to produce something that is the combination, can be achieved in many ways. The mechanism used with Slang, is that items that are going to be imported or exported are named. As the Slang language supports function overloading and generics, it is not possible to just use a function name, or type name to uniquely identify the item. For that we need to include additional information to uniquely identify the item - and to do that we use 'mangled names'. 

Symbols within a module are additionally uniquely marked with the name of the module, and thus the module name becomes part of the mangled name. 

This also has implications for includes. For example say we have two modules, and they both #include a header file that contains a function. That function will appear as two separate functions - one defined in each module, because a #include is a textual the function will be defined once in each module and with each module name.

By default Slang will provide linkage symbols for all global symbols - types or functions, including declarations. In the future this is likely to change with declarations *not* being exported by default, as the declaration is made available just to forward define, not as a mechnism to descibe something to be imported. 

The `[__extern]` attribute is used to mark a declaration as having it's definition defined elsewhere. For example 

```
[__extern] Thing makeThing(int a, int b);
``` 

With types the attribute can also be used. For the moment though a type declaration that is externed still needs to be a type definition. Therefore to declare a type to be imported from elsewhere we actually have to write

```
[__extern] struct Thing {};
```

Which looks a little like a definition of `Thing` but because of the `[__extern]` it actually means that `Thing` is defined elsewhere.  

Without further language and API support, that every module has all of it's symbols include the module name becomes a problem. If we have two modules `module-a` and `module-b` - how do I use a function defined in `module-a` in `module-b`? There needs to be mechansism/s to allow access to the function as part of the other module. Until the language and API features are added to allow such support, the problem is side stepped by allowing the specifying of a module name. If `module-a` and `module-b` use the same module name they will be able to access one anothers symbols during linkage. 

Specifying the module name can be achieve on the command line with the `-module-name` option as in

```
slangc -module-name somename -no-codegen tests/serialization/extern/module-a.slang -o tests/serialization/extern/module-a.slang-lib
```

From the API the module name can be specified via 

```
spSetDefaultModuleName(compileRequest, "somename");
```

Linkage
-------

The linkage process used in Slang is purposefully fairly straight forwards. When linkage occurs all items with linkage have a identifying mangled name. That for some mangled names there could be multiple definitions - for example a declaration of a function and a definition of a function. If there are multiple definitions, linking aims to determine the one that is 'the best' is then this is used. A definition is better than a declaration. And a definition that is specific to the current target is taken as better than some other definition. 

If there are multiple identical definitions, or definitions where it cannot be determed if one is better than another then one is taken at effectively random. This therefore assumes that if there are multiple definitions that they are in effect the same. 

If after the linkage process there are declarations that are used, without a definition, then linkage will fail with an unfound symbol.

Specifying Libraries
--------------------
  
When compiling there needs to be a mechanism to specify library or libraries that will be used during linkage. From the command line this can be achieved with the `-r` option. For example...

```
slangc -target dxil -profile lib_6_3 computeMain.slang -entry computeMain -stage compute -r module-a.slang-lib -o linked.dxil
```

Note that multiple `-r` options can be specified on a command line to reference multiple libraries. 

From the API we use

```
SLANG_API SlangResult spAddLibraryReference(
    SlangCompileRequest*    request,
    const void* libData,
    size_t libDataSize)```
    
The libData/libDataSize is the binary data that was was either loaded from a `slang-lib` file or was the result of a compilation of a module retrieved via `spGetContainerCode`. 

As with -r command line option, multiple libraries can be added to a SlangCompileRequest and all will be searched for relevant symbolds during linking. 

