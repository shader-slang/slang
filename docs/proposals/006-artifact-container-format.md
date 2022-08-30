Artifact Container Format
=========================

This proposal is for a file hierarchy based structure that can be used to represent compile results and more generally a 'shader cache'. Ideally it would feature

* Does not require an extensive code base to implement
* Flexible and customizable for specific use cases
* Possible to produce a simple fast implementation 
* Use simple and open standards where appropriate
* Where possible/appropriate human readable/alterable
* A way to merge, or split out contents that is flexible and easy. Ideally without using Slang tooling.

Should be able to store

* Compiled kernels
* Reflection/layout information
* Diagnostic information
* "meta" information detailing user specific, and product specific information
* Source
* Debug information
* Binding meta data
* Customizable, and user specified additional information

API support needs to allow

* Interchangable use of static shader cache/slang compilation/combination of the two
  * Implies compilation needs to be initiated in some way that is compatible with shader cache keying
* Ability to store compilations as they are produced

Needs to be able relate and group products such that with suitable keys, it is relatively fast and easy to find appropriate results.

It's importance/relevance

* Provides a way to represent complex compilation results 
* Could be used to support an open standard around 'shader cache'
* Provide a standard 'shader cache' system that can be used for Slang tooling and customers 
* Supports Slang tooling and language features

Status
------

## Gfx

There is a run time shader cache that is implemented in gfx.

There is some work around a file system backed shader cache in gfx.

## Artifact System

The Artifact system exists and provides a mechanism to transport source/compile results through the Slang compiler. It already supports most of the different items that need to be stored.

Artifact has support for "containers". An artifact container is an artifact that can contain other artifacts. Support for different 'file system' style container formats is also implemented. The currently supported underlying container formats supported are

* Zip
* Riff 
  * Riff without compression
  * Riff with deflate
  * Riff with LZ4 compression
  
Additionally the mechanisms already implemented support   
  
* The OS filesystem
* A virtual file system 
* A 'chroot' of the file system (using RelativeFileSystem)

In order to access a file system via artifact, is as simple as adding a modification to the default handler to load the container, and to implement `expandChildren`, which will allow traversal of the container. In general the design is 'lazy' in design. Children are not expanded, unless requested, and files not decompressed unless required. The caching system also provides a caching mechanism such that a representation such as uncompressed blob can be associated with the artifact.

Very little code is needed to support this behavior because the IExtFileArtifactRepresentation and the use of the ISlangFileSystemExt interface, mean it works using the existing already used and tested mechanisms.

It is a desired feature of the container format that it can be represented as 'file system', and have the option of being human readable where appropraite. Doing so allows

* Third party to develop tools/formats that suit their specific purposes
* Allows different containers to used
* Is generally simple to understand
* Allows editing and manipulating of contents using pre-existing extensive and cross platform tools
* Is a simple basis

This documents is about how to structure the file system to represent a 'shader cache' like scenario. 

Incorporating into the Artifact system will require a Payload type. It may be acceptable to use `ArtifactPayload::CompileResults`. The IArtifactHandler will need to know how to interpret the contents. This will need to occur lazily at the `expandChildren` level. This will create IArtifacts for the children that some aspects are lazily evaluated, and others are interpretted at the expansion. For example setting up the ArtifactDesc will need to happen at expansion.

Background
----------

The background section should explain where things stand in the language/compiler today, along with any relevant concepts or terms of art from the wider industry.
If the proposal is about solving a problem, this section should clearly illustrate the problem.
If the proposal is about improving a design, it should explain where the current design falls short.

Related Work
------------

* Shader cache system as part of gfx (https://github.com/lucy96chen/slang/tree/shader-cache)
* Lumberyard [shader cache](https://docs.aws.amazon.com/lumberyard/latest/userguide/mat-shaders-custom-dev-cache-intro.html)
* Unreal [FShaderCache](https://docs.unrealengine.com/5.0/en-US/fshadercache-in-unreal-engine/)
* Unreal [Derived Data Cache - DDC](https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/DerivedDataCache/)
* Microsoft [D3DSCache](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/ShaderCache.md) 

Lumberyard uses the zip format for its '.pak' format. 

Microsoft D3DSCache provides a binary keyed key-value store. 

## Gfx

Gfx has a runtime shader cache based on `PipelineKey`, `ComponentKey` and `ShaderCache`. ShaderCache is a key value store. 

A key for a pipeline is a combination of 

```
    PipelineStateBase* pipeline;
    Slang::ShortList<ShaderComponentID> specializationArgs;
```

`ShaderComponentID` can be created on the ShaderCache, from 

```
    Slang::UnownedStringSlice typeName;
    Slang::ShortList<ShaderComponentID> specializationArgs;
    Slang::HashCode hash;
```    
    
For reflected types, a type name is generated if specialized.
    
The shader cache can be thought of as being parameterized by the pipeline and associated specialization args. It appears to currently only support specialization types.    
    
Gfx does not appear to support any serialization/file representation.    
    
Proposed Approach
-----------------

Explain the idea in enough detail that a reader can concretely know what you are proposing to do. Anybody who is just going to *use* the resulting feature/system should be able to read this and get an accurate idea of what that experience will be like.

## Usage scenarios

It is worth discussing in a little more detail usage scenarios. 

1) Being able to find and load a kernel based on some key
2) Being able to compile some implementation on demand
3) Being able to compile and cache some combination on demand
4) Being able to compile and store to a container or file/s on demand

For scenario 1, there are perhaps two usage styles that might be desirable. One might be to have a compilation interface, and be able to use that to invoke a 'compilation', but would in fact just pull out the result from a shader cache. A challenge here is what is this 'compilation interface' and how do options on that map to keys to look up the result.

You could also imagine a scenario where a shader cache is loaded as a 'artifact hierarchy'. This hierarchy or something like it is probably what is backing the 'compilation interface'. Both views are useful. The artifact hierarchy provides a mechanism to find out what is in the cache. It would also provide a way to lookup a kernel that doesn't require a way to map 'compilation options' to a result. 

We may want a mechanism, on being given compilation options can produce a suitable 'key'. Ideas around key are discussed in following section.

For scenario 2, we need an interface that can capture 'compilation'. The most obvious kind of interface would be

```
struct Options
{
    Includes ...;
    Optimizations ...;
    Miscellaneous ...;
    Source* source[];
    EntryPoint ... ;
    ...
    CompilerSpecific options;
}

ICompiler
{
    Result compile(const Options* options, IArtifact** outArtifact);
}
```

This is similar to the `IDownstreamCompiler` interface. The implication being that the is some mapping from 'options' to the cached result (if there is one). There are problem around this, because

* Items may be specified multiple times 
* Ideally the hash would or at least could remain stable with updated to options 
* Also ideally the user might want control over what constitutes a new version/key
* Calculating a hash is fairly complicated, and would need to take into account ordering

Another option might be to split common options from options that are likely to be modified per compilation. For example

```
struct Options
{
    const char* name;               ///< Human readable name
    Includes ...;
    Optimizations ...;
    Miscellaneous ...;
    Source* source[];
    ...
    CompilerSpecific options;
}

struct CompileOptions
{
    Stage stage ...;
    SpecializationArgs ...;
    EntryPoint ...;
};

ICompiler
{
    Result createOptions(Options* options, IOptions* outOptions);

    Result compile(IOptions* options, const CompileOptions* compileOptions, IArtifact** outArtifact);
}
```

Having the split greatly simplifies the key production, because we can use the unique human name, and the very much simpler values of CompileOptions to produce a key.

Another idea might be to split out compilation options from naming and other aspects. 

In scenario 3 we want to cache results somewhere. 

It should be noted *by design* a `IArtifactContainers` children is *not* a mechanism that automatically updates some underlying representation, such as files on the file system. Once a IArtifactContainer has been expanded, it allows for manipulation of the children (for example adding and removing). The typical way to produce a zip from an artifact hierachy would be to call a function that writes it out as such. This is not something that happens incrementally. 

For an in memory caching scenario this choice works well. We can update the artifact hierarchy as needed and all is good.

In scenario 4 need to be made to the backing representation.

It seems this most logically happens as part of the compilation interface implementation. The Artifact system doesn't need to know anything about such caching directly.

Once a compilation is complete, an implementation could save the result in Artifact hierarchy and write out a representation to disk from that part of the hierarchy. For some file systems doing this on demand is probably not a great idea. For example the Zip file system does not free memory directly when a file is deleted. Perhaps as part of the interface there needs to be a way to 'flush' cached data to backing store. Lastly there could be a mechanism to write out the changes (or the new archive).

## Cache keys

### Hashing source

The fastest/simplest way to hash source, is to take the blob and hash that. Unfortunately a problem occurse due line ending character changing, or other white space changes produce a different hash. A way to work around this would be to use a tokenizer, or a 'simplified' tokenizer that only handles the necessary special cases - for example white space in a string is important. Such a solution does not require an AST or rely on a specific tokenizer. 

The process would be

1) Find simplified tokens appending together with ` ` 
2) Take the hash of that 

Another approach would be to hash each "token" as produced. Doing so doesn't require memory allocation for the concatination. You could special case short strings or single chars, and hash longer strings.

Why not use the Slang AST? You could do but it assumes

1) You can produce an AST for the input source - this is not generally true as source could be CUDA, C++, etc 
2) The AST would have to be produced post preprocessing - because prior to preprocessing it may not be valid source

If we are are relying on dependencies specified at least in part by `#include`, it means the preprocessor must have already been executed for Slang.

If the source is being passed down to other downstream compilers this isn't the case. We could potentially run the preprocessor for other source varities, but there will be problems with system files which won't be locatable in general. 

The other disadvantage around using the AST is that it requires the extra work and space of parsing. 

If we wanted to use Slang lexer it would imply the hash process would be 

1) Load the file
2) Lex the file
3) Preprocess the file (to get dependencies). Throw these tokens away.
4) Hash the original lex of the files tokens

For hashing slang language and probably HLSL we can use the Slang preprocessor tokenizer, and hash the tokens (actually probably just the token text). 

For other languages we may want to use some simple lexer. A problem with using a lexer at all is that it adds a great amount of complexity to a stand alone implementation. 

We could side step the issues around source generation if we push that problem back onto users. If they are using code generation they will also have to provide a string that uniquely identifies the generation that is being used. This perhaps being a requirement for a persistant cache. For a temporary runtime cache, we can allow hash generation from source.

### How to handle unnamed compilation options?

We cannot produce a hash for a compilation in general without having access to the source, as the source can change if it's produced on demand. We cannot in general have source available - most developers will not want to ship with source. Additionally we cannot demand that generated source is always available.

As a fall back position, we could produce a hash that took into account all of these factors. The hash could only produced *after* compilation, as it would require the list of dependencies, and the source. Producing such a hash after compilation, is workable for a runtime cache, but is not very useful for a persistant cache, because the key could only be produced after a compilation.



### Where are options stored?

### How do we alter some options of a compilation?

It is easy to imagine that for some targets, or some versions of a shader there is a need to change options. 

It may be necessary to allow changes to some options on a per compilation level, but the compilation still use the same key. Here the 'key' provides an indirection. The compilation is tweaked for specific needs, and the application has the advantage of the indirection of not having to know the specifics of the compilation.

### Input filenames/compilation options is not enough to unquely define a compilation, because source can be injected. How do we handle this?

## Discussion



## Configuration and Identification

A compilation can be configured in many ways. Including

* The source including source injection
* Preprocessor defines
* Compile options - optimization, debug information, include paths, libraries 
* Specialization types and values
* Target and target specific features API/tools/operating system
* Specific version of slang and/or downstream compilers
* Pipeline aspects

In general we probably don't want to use the combination of source and/or the above options as a 'key'. Such a key would be hard and slow to produce. It would not be something that could be created and used easily by an application. Moreover it is commonly useful to be able to name results such that the actual products can be changed and have things still work. 

You could imagine some set of options could be named, and further attribution appended to that. For example compile options for a standardized type, and source could be named, but versions with different specializations applied on top of that. Such a system would provide some thing similar to how the gfx shader cache works.

## Obfuscation

For some use cases the amount of information about the contents of the shader cache need to be limited.

At a minimum there needs to be mechanisms to be able to strip out information that is not needed for use on a target. 

There probably also additionally needs to be a way to specify items such that names, such as type names, source names, entry point names, compile options and so forth are not trivially contained in the format, as their existance could leak sensitive information about the specifics of a compilation.

## Indexing

## Deduping source

When compiling shaders, typically much of the source is shared. Unfortunately it is not generally possible to just save 'used source', because some source can be generated on demand. One way this is already performed by users is to use a specialized include handler, that will inject the necessary code. 

It is not generally possible therefore to identify source by path, or unique identity (as used by the slang file system interface). 

It is also the case that compilations can be performed where the source is passed by contents, and the name is not set, or not unique. 

The `slang-repro` system already handles these cases, and outputs a map from the input path to the potentially 'uniquified' name within the repro.

You could imagine a container holding a folder of source that is shared between all the kernels. In general it would additionally require a map of each kernel that would map names to uniqified files.

In the `slang-repro` mechanism the source is actually stored in a 'flat' manner, with the actual looked up paths stored within a map for the compilation. It would be preferable if the source could be stored in a hiearchy similar to the file system it originates. This would be possible for source that are on a file system, but would in general lead to deeper and more complex hierarchies contained in container. 

Including source, provides a way to distribute a 'compilation' much like the `slang-repro` file. It may also be useful such that a shader could be recompiled on a target. This could be for many reasons - allowing support for future platforms, allowing recompilation to improve performance or allowing compilation to happen on client machines for rare scenarios on demand.

## Manifest or association

A typical container will contain kernels - in effect blobs. The blobs themselves, or the blob names are not going to be sufficient to express the amount of information that is necessary to meet the goals laid out at the start of this document. Some extra information may be user supplied. Some extra information might be user based to know how to classify different kernels. Therefore it is necessary to have some system to handle this metadata. 

As previously discussed the underlying container format is a file system. Some limited information could be infered from the filename. For example a .spv extension file is probably SPIR-V blob. For more rich meta data describing a kernel something more is needed. Two possible approaches could be to have a 'manifest' that described the contents of the container. Another approach would to have a file associated with the kernel that describes it's contents.

Single Manifest Pros

* Single file describes contents
* Probably faster to load and use
* Reduces the amount of extra files
* Everything describing how the contents is to be interpretted is all in one place

Single Manifest Cons

* Not possible to easily add and remove contents - requires editing of the manifest, or tooling 
  * Extra tooling specialized tooling was deemed undesirable in original problem description
* Manifest could easily get out of sync with the contents

Associated Files Pros

* Simple 
* Can use normal file system tooling to manipulate
* The contents of the container is implied by the contents of the file system
  * Easier to keep in sync
  
Associated Files Cons

* Requires traversal of the container 'file system' to find the contents
* Might mean a more 'loose' association between results

Another possible way of doing the association is via a directory structure. The directory might contain the 'manifest' for that directory. 

Given that we want the format to represent a file system, and that we would want it to be easy and intuitive how to manipulate the represtation, using a single manifest is probably ruled out. It remains to be seen which is preferable in practice, but it seems likely that using 'associated files' is probably the way to go.

## How to represent data

As previously discussed, unless there is a very compelling reason not to we want to use representations that are open standards and easy to use. We also need such representations to be resilient to changes. It is important that file formats can be human readable or easily changable into something that is human readable. For these reasons, JSON seems to be a good option for our main 'meta data' representation. Additionally Slang already has a JSON system.

If it was necessary to have meta data stored in a more compressed format we could consider also supporting [BSON](https://en.wikipedia.org/wiki/BSON). Conversion between BSON and JSON can be made quickly and simply. BSON is a well known and used standard.

## Other aspects

It may be useful for a representation to hold `slang-ir` of a compilation. This would allow some future proofing of the representation, because it would allow support for newer versions of Slang and downstream compilers without distributing source. 

Detailed Explanation
--------------------

Here's where you go into the messy details related to language semantics, implementation, corner cases and gotchas, etc.
Ideally this section provides enough detail that a contributor who wasn't involved in the proposal process could implement the feature in a way that is faithful to the original.

Alternatives Considered
-----------------------

## Issues On Github 

* Support low-overhead runtime "shader cache" lookups [#595](https://github.com/shader-slang/slang/issues/595)
* Compilation id/hash [#2050](https://github.com/shader-slang/slang/issues/2050)
* Support a simple zip-based container format [#860](https://github.com/shader-slang/slang/issues/860)
 
## Discussion with Theresa Foley

```
{
    "modules": [
        { "name":"foo", "translationUnits":[...], ... }
        ...
    ]
    "configs": [
        { "name":"my-vulkan-config", "target":"spirv", "optimization":"full", ... }
        ...
    ]
    "kernels": [
         { "module:"foo", "config":"my-vulkan-config", "path":"./kernels/my-vulkan-config/foo.spv" }
        ...
    ]
}
```

```
./kernels/my-vulkan-config/foo.spv
./kernels/my-vulkan-config/foo.spv.info-stuff
...
./kernels/my-vulkan-config/bar.spv
./kernels/my-vulkan-config/bar.spv.info-stuff
...
./kernels/my-vulkan-config.config.json
```

## ...


Any important alternative designs should be listed here.
If somebody comes along and says "that proposal is neat, but you should just do X" you want to be able to show that X was considered, and give enough context on why we made the decision we did.
This section doesn't need to be defensive, or focus on which of various options is "best".
Ideally we can acknowledge that different designs are suited for different circumstances/constraints.
