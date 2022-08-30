Artifact Container Format
=========================

A proposal for a Slang language/compiler feature or system should start with a concise description of what the feature it and why it could be important.

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

Very little code is needed to support this behavior because the IExtFileArtifactRepresentation and the use of the ISlangFileSystemExt interface, mean it works using the existing mechanisms that are used.  

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
