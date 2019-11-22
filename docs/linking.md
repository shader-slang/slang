Slang Linking
=============

The Slang feature around libraries and linking are a *work in progress*. Future versions of Slang are likely to change the API and binary compatibility. Also note that currently reflection is mainly unsupported. 

In many languages it is possible to compile source files into binaries such as libraries and object files. Later these files can be combined perhaps with other source files to produce a result such as an executable. Slang now has experimental support for such a feature. 

To make such a feature work we need a few abilities

* To create a library from source 
* A way for symbols to be exported and imported
* Linkage - ability to combine source/libraries to produce things (such as other libraries, executables, etc)
* A way to specify libraries to be used in linkage


Binding
-------

Due to how the feature currently works, if a library contains any kind of binding (implicit or explicit) it should be considered platform specific. When linking with libraries, the libraries will not supply any reflection, other than minimal entry point information. Any source compiled and linked against does follow normal Slang rules in terms of implicit binding assignments and reflection.  

In practice this means that libraries either have to have no binding or manual binding. If any library has manual binding, this binding must be ensured to not clash with either implict or explicit binding in the source or other linked libraries. It is not guaranteed that Slang or downstream compilers will report binding clashes. 

If platform independent libraries are linked against source compilation then appropriate reflection and normal Slang implicit binding rules apply.

Libraries
---------

A library can be created from source from the Slang command line with the `-o` option naming the output file. The filename currently requires a `slang-lib` or `slang-module` extension. This extension will mean the library will be stored as serialized Slang IR. The option `-no-codegen` is also typically specified because we don't want to generate any *target* code, such as dxil, spir-v and so forth when producing a library, just the Slang IR. 

For example

```
slangc -no-codegen tests/serialization/extern/module-a.slang -o tests/serialization/extern/module-a.slang-lib
```

From the slang API we need to indicate the container format and to disable target code generation 

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
    // and without subsequent calls to spCompile on the request
    {
        size_t size;
        const void* data = spGetCompileRequestCode(compileRequest, size);
    }
```

Libraries can contain entry points. It is necessary though that the entry point is specified during the compilation producing the library. This can be achieved by specifying a function as an entry point via the API or command line through the normal mechanisms. 

Entry points can also be specified in source via the `[shader()]` attribute. For example

```
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // ... 
}
```

Slang libraries are stored as serialized Slang IR. That Slang IR currently maintains no forward or backward compatibility, and a new version of Slang may either produce incompatible IR, or be unable to consume previous versions IR.

By default Slang stores serialized Slang IR in a compressed `lite` format. It can also be stored without any addition compression. These options can be specified via the command line via

```
-ir-compression lite
-ir-compression none
```

They are also available via the API through the `spProcessCommandLineArguments` function. 

Note that Slang can consume and process either `lite` or `none` styles transparently. Also mixing compressed libraries with uncompressed libraries also works. 

Symbols
-------

Linkage is the process of combining of libraries and source to produce something. The mechanism used with Slang, is that items that are going to be imported or exported are named. As the Slang language supports function overloading and generics, it is not possible to just use a function name or a type name to uniquely identify an item. Thus exported and imported names are 'mangled' to include this additional information. This is not unlike how C++ produces mangled names for symbols for similar reasons.

Symbols within a module are additionally uniquely marked with the name of the module, and thus the module name becomes part of the mangled name. This has implications for includes. For example say we have two modules, and they both #include a header file that contains a function. That function will appear as two separate functions - one defined in each module, because a #include is a textual the function will be defined once in each module and with each module name.

By default Slang will provide linkage symbols for all global symbols - types or functions, including declarations. In the future this is likely to change with declarations *not* being exported by default. 

To mark a declaration as having it's definition defined elsewhere outside the current module, the `[__extern]` attribute can be used. For example 

```
[__extern] Thing makeThing(int a, int b);
``` 

With types the `[__extern]` attribute can also be used. For the moment though a type declaration that is externed still needs to be a type definition. Therefore to declare a type to be imported from elsewhere we actually have to write

```
[__extern] struct Thing {};
```

Which looks a little like a definition of `Thing` but because of the `[__extern]` it actually means that `Thing` is defined elsewhere. 

The mechanism for making other symbols available from a library and/or source within Slang is through `import`. Currently though `import` does not support importing of libraries. 

This means that a work around (aka hack) is needed so linkage works with a library. The short term fix, until importing of libraries is available, is to allow overriding of the name of the module - typically so that all symbols appear in the same module name. For example say we have two modules `module-a` and `module-b`, if `module-a` and `module-b` use the same module name they will be able to access one anothers symbols during linkage. 

Specifying the module name can be achieve on the command line with the `-module-name` option as in

```
slangc -module-name somename -no-codegen tests/serialization/extern/module-a.slang -o tests/serialization/extern/module-a.slang-lib
```

From the API the module name can be specified via 

```
spSetDefaultModuleName(compileRequest, "somename");
```

The `[__extern]` attribute and the use of module names, are temporary functionality until more complete support for libraries and linkage is implemented. 

Linkage
-------

The linkage process used in Slang is purposefully fairly straight forwards. When linkage occurs all items with linkage have a identifying mangled name. That for some mangled names there could be multiple definitions - for example a declaration of a function and a definition of that function. If there are multiple definitions, linking aims to determine the one that is 'best' which is then used. A definition is better than a declaration. And a definition that is specific to the current target is taken as better than some other definition. 

If there are multiple identical definitions, or definitions where it cannot be determed if one is better than another then one is taken at effectively random. This therefore assumes that if there are multiple definitions that they are in effect the same. 

If after the linkage process there are declarations that are used, without a definition, then linkage will fail with an unfound symbol.

Specifying Libraries
--------------------
  
When compiling there needs to be a mechanism to specify library or libraries that will be used during linkage. From the command line this can be achieved with the `-r` option. For example...

```
slangc -target dxil -profile sm_6_3 computeMain.slang -entry computeMain -stage compute -r module-a.slang-lib -o linked.dxil
```

Note that multiple `-r` options can be specified on a command line to reference multiple libraries. 

From the API use

```
SLANG_API SlangResult spAddLibraryReference(
    SlangCompileRequest*    request,
    const void* libData,
    size_t libDataSize)
```
    
The libData/libDataSize is the binary data that was was either loaded from a `slang-lib` file or was the result of a compilation of a module and the library contents retrieved via `spGetContainerCode` or `spGetCompileRequestCode`. 

As with -r command line option, multiple libraries can be added to a SlangCompileRequest and all will be searched for relevant symbolds during linking. 

Reflection
----------

In general reflection is *not* available post linkage currently. A subset of functionality is available though including

* spGetReflection
* spReflection_getEntryPointCount
* spReflection_getEntryPointByIndex
* spReflectionEntryPoint_getName

That other reflection functions currently either do not work or may *crash*. 

Other issues
------------

In languages such as C++, if I want to define a struct on the stack I need it's definition

```
struct Thing;

int main()
{
    // Won't compile we need the *definition* not just a declaration
    Thing thing;
    return 0;
}
```

It does not matter that Thing might be defined in another library or object file. For compilation of main, we have to have the definition of Thing. 

In Slang and libraries this is not the case. If I have a file `module-a.slang`
 
```
struct Thing
{
    int a; 
    float b;
};
```

And then I compile another file and include the library that contains Thing I *can* just use the Thing type 

```

// I have to say that Thing is externally defined
[__extern] struct Thing {};

void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // This is fine in Slang - as long is Thing *is* defined in a referenced library
    Thing thing;
    // ... 
}
```

This is possible because when linkage occurs the code is actually *Slang IR*, and Slang IR contains the information necessary for the use of the Thing type. 

It also worth noting that if the `-obfuscate` feature is used, all libraries and source must also have `-obfuscate` enabled, for linking to work. 


Examples
--------

Examples of compiling of libraries and linking with them can be found within tests/serialization. 