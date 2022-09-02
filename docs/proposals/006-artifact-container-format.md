Shader Container Format
=======================

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

* Interchangeable use of static shader cache/slang compilation/combination of the two
  * Implies compilation needs to be initiated in some way that is compatible with shader cache keying
* Ability to store compilations as they are produced

Needs to be able relate and group products such that with suitable keys, it is relatively fast and easy to find appropriate results.

It's importance/relevance

* Provides a way to represent complex compilation results 
* Could be used to support an open standard around 'shader cache'
* Provide a standard 'shader cache' system that can be used for Slang tooling and customers 
* Supports Slang tooling and language features

## Use

There are several kinds of usage scenario

* A runtime shader cache
* A runtime shader cache with persistence
* A capture of compilations
* A baked persistent cache - must also work without shader source
* A baked persistent cache, that is obfuscated

A runtime shader cache has the following characteristics:

* Can works with mechanisms that do not require any user control (such as naming). Ie purely inputs/options can define a 'key'.
* It is okay to have keys/naming that are not human understandable/readable. 
* The source is available - such that hashes based on source contents can be produced. 
* It does not matter if hashes/keys are static between runs. 
* It is not important that a future version would be compatible with a previous version or vice versa.
* Could be all in memory.
* May need mechanism/s to limit the working set
* Generated source can be made to work, because it is possible to hash generated source

At the other end of the spectrum a baked persistent cache

* Probably wants user control over naming 
* Doesn't have access to source so can't use that as part of a hash
* Probably doesn't have access to dependencies
* Having some indirection between a request and a result is a useful feature
* Ideally can be manipulated and altered without significant tooling
* Generated source may need to be identified in some other way than the source itself

It should be possible to serialize out a 'runtime shader cache' into the same format as used for persistent cache. It may be harder to use such a cache without Slang tooling, because the mapping from compilation options to keys will probably not be simple.

Status
------

## Gfx

There is a run time shader cache that is implemented in gfx.

There is some work around a file system backed shader cache in gfx.

## Artifact System

The Artifact provides a mechanism to transport source/compile results through the Slang compiler. It already supports most of the different items that need to be stored.

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

In order to access a file system via artifact, is as simple as adding a modification to the default handler to load the container, and to implement `expandChildren`, which will allow traversal of the container. In general this works in a 'lazy' manner. Children are not expanded, unless requested, and files not decompressed unless required. The system also provides a caching mechanism such that a representation, such as uncompressed blob, can be associated with the artifact.

Very little code is needed to support this behavior because the IExtFileArtifactRepresentation and the use of the ISlangFileSystemExt interface, mean it can work using the existing mechanisms.

It is a desired feature of the container format that it can be represented as 'file system', and have the option of being human readable where appropriate. Doing so allows

* Third party to develop tools/formats that suit their specific purposes
* Allows different containers to used
* Is generally simple to understand
* Allows editing and manipulating of contents using pre-existing extensive and cross platform tools
* Is a simple basis

This documents is, at least in part,  about how to structure the file system to represent a 'shader cache' like scenario. 

Incorporating the 'shader container' into the Artifact system will require a suitable Payload type. It may be acceptable to use `ArtifactPayload::CompileResults`. The IArtifactHandler will need to know how to interpret the contents. This will need to occur lazily at the `expandChildren` level. This will create IArtifacts for the children that some aspects are lazily evaluated, and others are interpreted at the expansion. For example setting up the ArtifactDesc will need to happen at expansion.

Background
==========

The following long section provides background discussion on a variety of topics. Jump to the [Proposed Approach](#proposed-approach) to describe what is actually being suggested in conclusion.

To enumerate the major challenges

* How to generate a key for the runtime scenario
* How to produce keys for the persistent scenario - implies user control, and human readability
* How to represent compilation in a composable 'nameable' way 
* How to produce options from a named combination

The mechanism for producing keys in the runtime scenario could be used to check if an entry in the cache is out of date.

A compilation can be configured in many ways. Including

* The source including source injection
* Pre-processor defines
* Compile options - optimization, debug information, include paths, libraries 
* Specialization types and values
* Target and target specific features API/tools/operating system
* Specific version of slang and/or downstream compilers
* Pipeline aspects
* Slang components

In general we probably don't want to use the combination of source and/or the above options as a 'key'. Such a key would be hard and slow to produce. It would not be something that could be created and used easily by an application. Moreover it is commonly useful to be able to name results such that the actual products can be changed and have things still work. 

Background: Hashing source
==========================

Hashing source is something that is needed for runtime cache scenario, as it is necessary to generate a key purely from 'input' which source is part of. It can also be used in the persistent scenario, in order to validate if everything is in sync. That sync checking might perhaps only be performed when source and other resources are available. 

The fastest/simplest way to hash source, is to take the blob and hash that. Unfortunately there are several issues 

* Ignores dependencies - if this source includes another file the hash will also need to depend on that transitively
* Hash changes with line end character encoding
* Hash is sensitivve to white space changes in general 

A way to work around whitespace issues would be to use a tokenizer, or a 'simplified' tokenizer that only handles the necessary special cases. An example special case would be white space in a string is always important. Such a solution does not require an AST or rely on a specific tokenizer. A hash could be made of concatenation of all of the lexemes with white space inserted between.

Another approach would be to hash each "token" as produced. Doing so doesn't require memory allocation for the concatenation. You could special case short strings or single chars, and hash longer strings.

## Dependencies

Its not enough to rely on hashing of input source, because `#include` or other resource references, such as modules or libraries may be involved. 

If we are are relying on dependencies specified at least in part by `#include`, it implies the preprocessor be executed. This could be used for other languages such as C/C++. Some care would need to be taken because *some* includes will probably not be located by our preprocessor, such as system include paths in C++. For the purpose of hashing, an implementation could ignore `#includes` that cannot be resolved. This may work for some scenarios - but doesn't work in general because symbols defined in unfound includes might cause other includes. Thus this could lead to other dependencies not being found, or being assumed when they weren't possible.

In practice whilst not being perfect it may work well enough to be broadly usable. 

## AST

A hash could be performed via the AST. This assumes

1) You can produce an AST for the input source - this is not generally true as source could be CUDA, C++, etc 
2) The AST would have to be produced post preprocessing - because prior to preprocessing it may not be valid source
3) If 3rd parties are supposed to be able to produce a hash it requires their implementing a Slang lexer/parser in general
4) Depending on how the AST is used, may not be stable between versions 

Another disadvantage around using the AST is that it requires the extra work and space for parsing. 

Using the AST does allow using pre-existing Slang code. It is probably more resilient to structure changes. It would also provide slang specific information more simply - such as imports.

## Slang lexer

If we wanted to use Slang lexer it would imply the hash process would 

1) Load the file
2) Lex the file
3) Preprocess the file (to get dependencies). Throw these tokens away (we want the hash of just the source)
4) Hash the original lex of the files tokens
5) Dependencies would require hashing and combining

For hashing Slang language and probably HLSL we can use the Slang preprocessor tokenizer, and hash the tokens (actually probably just the token text). 

Using the Slang lexer/preprocessor may work reasonably for other languages such as C++/C/HLSL/GLSL. It does imply a reliance on a fairly large amount of slang source. 

## Simplified Lexer

We may want to use some simple lexer. A problem with using a lexer at all is that it adds a great amount of complexity to a stand alone implementation. The simplified lexer would

* Simplify white space - much we can strip
* Honor string representations (we can't strip whitespace)
* Honor identifiers
* We may want some special cases around operators and the like
* Honor `#include` (but ignore preprocessor behavior in general)
* Ignore comments

We need to handle `#include` such that we have dependencies. This can lead to dependencies that aren't required in actual compilation.

We need to honor some language specific features - such as say `import` in Slang. 

Such an implementation would be significantly simpler, and more broadly applicable than Slang lexer/parser etc. Writing an implementation would determine how complex - but would seem to be at a minimum 100s of lines of code.

We can provide source for an implementation. We could also provide a shared library that made the functionality available via COM interface. This may help many usage scenarios, but we would want to limit the complexity as much as possible.

## Generated Source

Generated source can be part of a hash if the source is available. As touched on there are scenarios where generated source may not be available.

We could side step the issues around source generation if we push that problem back onto users. If they are using code generation, the system could require providing a string that uniquely identifies the generation that is being used. This perhaps being a requirement for a persistent cache. For a temporary runtime cache, we can allow hash generation from source.  

Background: Hashing Stability
=============================

Ideally a hashing mechanism can be resilient to unimportant changes. The previous section described some approaches for changes in source. The other area of significant complexity is around options. If options are defined as JSON (or some other 'bag of values') hashing can be performed relatively easily with a few rules. If the representation is such that if a value is not set, the default is used, it is resilient to changes of options that are explicitly set. 

When the hashing is on some native representation this isn't quite so simple, as a typical has function will include all fields. A field value, default or not will alter the hash. Therefore adding or removing a field will necessarily change the hash.

One way around this would be to use a hashing regime that only altered the hash if the values are not default.

```C++

Hash calcHash(const Options& options)
{
    const Options defaultOptions;

    Hash = ...;
    if (option.someOption != defaultOption.someValue)
    {
        hash = combineHash(hash, option.someOption.getHash());
    }
    // ...
}
```

This could perhaps be simplified with some macro magic.

```

// Another 

struct HashCalculator<T>
{
    template <typename FIELD>
    void hashIfDifferent(const field& f, const T& defaultValue)
    {
        if (value != defaultValue)
        {
            hash = combineHash(hash, value.getHash());
        }
    }
    
    const T defaultValue;
    const T* value;
    Hash hash;
};

Hash calcHash(const Options& options)
{
    HashCalculator calc;
    const Options defaultOptions;

    calc.hashIfDifferent(options.someOption, defaultOptions.someOption);
    // ...
    Hash = ...;
    
}
```

This is a little more clumsy, but if we wanted to use a final native representation, it is workable.

Note that the ordering of hashing is also important for stability.

Background: Key Naming
======================

The container could be seen as a glorified key value store, with the key identifying a kernel and associated data. 

Much of the difficulty here is how to define the key. If it's a combination of the 'inputs' it would be huge and complicated. If it's a hash, then it can be short, but not human readable, and without considerable care not stable to small or irrelevant changes.

For a runtime cache type scenario, the instability and lack of human readability of the key probably doesn't matter too much. It probably is a consideration how slow and complicated it is to produce the key. 

For any cache that is persistent how naming occurs probably is important. Because

* Our 'options' aren't going to make much sense with other compilers (if we want the standard to be more broadly applicable)
* The options we have will not remain static
* Having an indirection is useful from an application development and shipping perspective
* That the *name* perhaps doesn't always have to indicate every aspect of a compilation from the point of view of the application

One idea touched on in this document is to move 'naming' into a user space problem. That compilations are defined by the combination of 'named' options. In order to produce a shader cache name we have a concatination of names. The order can be user specified. Order could also break down into "directory" hierarchy as necessary.

Some options will need to be part of some order. This is perhaps all a little abstract so as an example

```JSON
{
    // Configuration

    "configuration" : {
    
        "debug" : {
            "group" : "configuration",
            "optimization" : "0",
            "debug-info" : true,
            "defines" : [ "-DDEBUG=1" ]
        },
        "release" : {
            "optimization" : "2",
            "debug-info" : true,
            "defines" : [ "-DRELEASE=1" ]
        },
        "full-release" : {
            "optimization" : "3",
            "debug-info" : false,
            "defines" : [ "-DRELEASE=1", "-DFULL_RELEASE=1" ]
        }
    },
    
    // Target
    "target" : { 
        "vk" : {
        }   
        "d3d12" : {
        }
        
        "cpu" : {
        }
    },
    
    // Stage
    "stage" : {
        "compute" : { 
        },
    },
    
    combinations : [
        {
            key : [ "vk", "compute", ["release", "full-release"] ],
            options : 
            {
                "optimization" : 1
            }
        }
    ]
}
``` 

The combination in this manner doesn't quite work, because some combinations may imply different options. The "combinations" section tries to address this by providing a way to 'override' behavior. This could of course be achieved with a call back mechanism. We may also want to have options that don't appear in the key, allowing 'overriding' behavior without needing a 'combinations' section. The implication is that when used in the application when looking up only the 'named' configuration is needed. 

This whole mechanism provides a way of specifying a compilation by a series of names, that can produce a unique human readable key. It is under user control, but the mechanism on how the combination takes place is at least as a default defined within an implementation.

It may be necessary to define options by tool chain. Doing so would mean the names can group together what might be quite different options on different compilers. Having options defined in JSON means that the mechanisms described here can be used for other tooling. If the desire is to have some more broadly applicable 'shader cache' representation this is desirable. 

If it is necessary obfuscate the contents, it would be possible to put the human readable key though a hash, and then the hash can be used for lookup. 

## Container Location

We could consider the contents of the container as 'flat' with files for each of the keys. There could be several files related to a key if we use file association mechanism (as opposed to a single manifest).

Whilst this works it probably isn't particularly great from an organizational point of view. It might be more convenient if we can use the directory hierarchy if at least optionally. For example putting all the kernels for a target together...

```
/target/name-entryPoint
```    
    
Or 

```
/target/name/entryPoint-generated-hash
```    
    
Where 'generated-hash' was the hash for generated source code.    
    
Perhaps this information would be configured in a JSON file for the repository. 

What happens if we want to obfuscate? We could make the whole path obfuscated. We could in the configuration describe which parts of the name will be obfuscated, such that it's okay to see there are different 'target' names.

## Default names    
    
When originally discussed, the idea was that all options can be named, and thus any combination is just a combination of names. That combination produces the key. 

Whilst this works, it might make sense to allow 'meta' names for common types. The things that will typically change for a fixed set of options would be 

* The input translation unit source
* The target (in the Slang sense)
* The entryPoint/stage (can we say the entry point name implies the stage?)

We could have pseudo names for these commonly changed values. If there are multiple input source files for a translation unit, we could key on the first. 

We could also have some 'names' that are built in. For example default configuration names such as 'debug' and 'release'. They can be changed in part of configuration but have some default meaning. That options can perhaps override the defaults. 

Using the pseudo name idea might mean it is possible to produce reasonable default names. Moreover we can still use the hashing mechanism to either report a validation issue, or trigger recompilation when everything needed to do as much is available.

## Target

A target can be quite a complicated thing to represent. Artifact has

* 'Kind' - executable, library, object code, shared library
* 'Payload' - SPIR-V, DIXL, DXBC, Host code, Universal, x86_64 etc...
  * Version
* 'Style' - Kernel, host, unknown

This doesn't take into account a specific 'platform', where that could vary a kernel depending on the specific features of the platform. There are different versions of SPIR-V and there are different extensions. 

This doesn't cover the breadth though because for CPU targets there is additionally

* Operating system - including operating system version
* Tool chain - Compiler 

Making this part of the filename could lead to very long filenames. The more detailed information could be made available in JSON associated files. 

This section doesn't provide a specific plan on how to encapsulate the subtlety around a 'target'. Again how this is named is probably something that is controllable in user space, but there are some reasonable defaults when it is not defined. 
    
Background: Describing Options
==============================

We need some way to describe options for compilation. The most 'obvious' way would be something like the IDownstreamCompiler interface and associated types

```C++
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

For this to work we need a mapping from 'options' to the cached result (if there is one). There are problem around this, because

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

This specifying of options in this way is tied fairly tightly to the Slang API. We can generalize the named options by allowing more than one named option set.

## Bag of Named Options

Perhaps identification is something that is largely in user space for the persistent scenario. You could imagine a bag of 'options', that are typically named. Then the output name is the concatenation of the names. If an option set isn't named it doesn't get included. Perhaps the order of the naming defines the precedence.

This 'bag of options' would need some way to know the order the names would be combined. This could be achieved with another parameter or option that describes name ordering. Defining the ordering could be achieved if different types of options are grouped, by specifying the group. The ordering would only be significant for named items that will be concatenated. The ordering of the options could define the order of precedence of application.

Problems: 

How to combine all of these options to compile? 
How to define what options are set? Working at the level of a struct doesn't work if you want to override a single option.
The grouping - how does it actually work? It might require specifying what group a set of options is in.

An advantage to this approach is that policy of how naming works as a user space problem. It is also powerful in that it allows control on compilation that has some independence from the name.

We could have some options that are named, but do not appear as part of the name/path within the container. The purpose of this is to allow customization of a compilation, without that customization necessarily appearing withing the application code. The container could store group of named options that is used, such that it is possible to recreate the compilation or perhaps to detect there is a difference. 

### JSON options

One way of dealing with the 'bag of options' issue would be to just make the runtime json options representation, describe options. Merging JSON at a most basic level is straight forward. For certain options it may make sense to have them describe adding, merging or replacing. We could add this control via adding a key prefix.

```JSON
{
    "includePaths" : ["somePath", "another/path"],
    "someValue" : 10,
    "someEnum" : enumValue,
    "someFlags" : 12
}      
```

As an example

```JSON
{
    "+includePaths" : ["yet/another"],
    "intValue" : 20,
    "-someValue" : null,
    "+someFlags" : 1
}
```

When merged produces

```JSON
{
    "includePaths" : ["somePath", "another/path", "yet/another"],
    "someEnum" : enumValue,
    "someFlags" : 13,
    "intValue" : 20
}      
```

It's perhaps also worth pointing out that using JSON as the representation provides a level of compatibility. Things that are not understood can be ignored. It is human readable and understandable. We only need to convert the final JSON into the options that are then finally processed.
  
One nice property of a JSON representation is that it is potentially the same for processing and hashing.   
 
### Producing a hash from JSON options
    
One approach would be to just hash the JSON if that is the representation. We might want a pass to filter out to just known fields and perhaps some other sanity processing. 

* Filtering  
* Ordering - the order of fields is generally not the order we want to combine. One option would be to order by key in alphabetical order.
* Handling values that can have multiple representations (if we allow an enum as int or text, we need to hash with one or ther other)
* Duplicate handling 

Alternatively the JSON could be converted into a native representation and that hashed. The problem with this is that without a lot of care, the hash will not be stable with respect to small changes in the native representation.

Another advantage of using JSON for hash production, is that it is something that could be performed fairly easily in user space.    

Two issues remain significant with this approach

* Filtering - how?
* Handling multiple representations for values

Filtering is not trivial - it's not a question of just specifying what fields are valid, because doing so requires context. In essence it is necessary to describe types, and then describe where in a hierarchy a type is used. 

I guess this could be achieved with... JSON. For example

```
{
    "types": 
    {
        "SomeType":
        {
            "kind" : "struct",
            "derivesFrom": "..."
            fields: {
                [ "name", "type", "default"]
            }
        },
        "MainOptions": 
        {
            "..."
        }
    },
    "structure" : 
    {
        "MainOptions"
    }
}
```

When traversing we use the 'structure' to work out where a type is used. 

This is workable, but adds additional significant complexity.

The issue around different representations could also use the information in the description to convert into some canonical form. 

The structure could potentially generated via reflection information.

## Native bag of options

Options could be represented via an internal struct on which a hash can be performed. 

Input can be described as "deltas" to the current options. The final options is the combination of all the deltas - which would produce the final options structure for use. The hash of all of the options is the hash of the final structure.

How are the deltas described?

The in memory representation is not trivial in that if we want to add a struct to a list we would need a way to describe this.

Whilst in the runtime the 'field' could be uniquely identified an offset, within a file format representation it would need to be by something that works across targets, and resistant to change in contents. 

## Slangs Component System

Slang has a component system that can be used for combining options to produce a compilation. An argument can be made that it should be part of the hashing representation, as it is part of compilation.

If combination is at the level of components, then as long as components are serializable, we can represent a compilation by a collection of components. Has several derived interfaces...

* IEntryPoint
* ITypeConformance 
* IModule

Can be constructed into composites, through `createCompositeComponentType`, which describes aspects of the combination takes place.

If the components were serializable (as say as JSON), we could describe a compilation as combination of components. If components are named, a concatenation of names could name a compilation.

It doesn't appear as if there is a way to more finely control the application of component types. For example if there was a desire to change the optimization option, it would appear to remain part of the ICompileRequest (it's not part of a component). This implies this mechanism as it stands whilst allowing composition, doesn't provide the more nuanced composition. Additional component types could perhaps be added which would add such control.

Perhaps having components is not necessary as part of the representation, as 'component' system is a mechanism for achieving a 'bag of options' and so we can get the same effect by using that mechanism without components.
    
Background: Describing Options
==============================

The 'naming' options idea implies that options and ways of combining options can be stored within the configuration for a container. Perhaps there is additionally a runtime API that allows creation of deltas. 
    
## 'Bag of named options' 

Perhaps identification is something that is largely in user space for the persistent scenario. You could imagine a bag of 'options', that are typically named. Then the output name is the concatenation of the names. If an option set isn't named it doesn't get included. Perhaps the order of the naming defines the precedence.

This 'bag of options' would need some way to know the order the names would be combined. This could be achieved with another parameter or option that describes name ordering. Defining the ordering could be achieved if different types of options are grouped, by specifying the group. The ordering would only be significant for named items that will be concatenated. The ordering of the options could define the order of precedence of application.

Problems: 

How to combine all of these options to compile? 
How to define what options are set? Working at the level of a struct doesn't work if you want to override a single option.
The grouping - how does it actually work? It might require specifying what group a set of options is in.

An advantage to this approach is that policy of how naming works as a user space problem. It is also powerful in that it allows control on compilation that has some independence from the name.

### JSON options

One way of dealing with the 'bag of options' issue would be to just make the runtime JSON options representation, describe options. Merging JSON at a most basic level is straight forward. For certain options it may make sense to have them describe adding, merging or replacing. We could add this control via adding a key prefix.

```JSON
{
    "includePaths" : ["somePath", "another/path"],
    "someValue" : 10,
    "someEnum" : enumValue,
    "someFlags" : 12
}      
```

As an example

```JSON
{
    "+includePaths" : ["yet/another"],
    "intValue" : 20,
    "-someValue" : null,
    "+someFlags" : 1
}
```

When merged produces

```JSON
{
    "includePaths" : ["somePath", "another/path", "yet/another"],
    "someEnum" : enumValue,
    "someFlags" : 13,
    "intValue" : 20
}      
```

It's perhaps also worth pointing out that using JSON as the representation provides a level of compatibility. Things that are not understood can be ignored. It is human readable and understandable. We only need to convert the final JSON into the options that are then finally processed.
  
One nice property of a JSON representation is that it is potentially the same for processing and hashing.   
 
### Producing a hash from JSON options
    
One approach would be to just hash the JSON if that is the representation. We might want a pass to filter out to just known fields and perhaps some other sanity processing. 

* Filtering  
* Ordering - the order of fields is generally not the order we want to combine. One option would be to order by key in alphabetical order.
* Handling values that can have multiple representations (if we allow an enum as int or text, we need to hash with one or ther other)
* Duplicate handling 

Alternatively the JSON could be converted into a native representation and that hashed. The problem with this is that without a lot of care, the hash will not be stable with respect to small changes in the native representation.

Another advantage of using JSON for hash production, is that it is something that could be performed fairly easily in user space.    

Two issues remain significant with this approach

* Filtering - how?
* Handling multiple representations for values

Filtering is not trivial - it's not a question of just specifying what fields are valid, because doing so requires context. In essence it is necessary to describe types, and then describe where in a hierarchy a type is used. 

I guess this could be achieved with... JSON. For example

```
{
    "types": {
        "SomeType":
        {
            "kind" : "struct",
            "derivesFrom": "..."
            fields: {
                [ "name", "type", "default"]
            }
        },
        "MainOptions": 
        {
            "..."
        }
    },
    "structure" : 
    {
        "MainOptions"
    }
}
```

When traversing we use the 'structure' to work out where a type is used. 

This is workable, but adds additional significant complexity.

The issue around different representations could also use the information in the description to convert into some canonical form. 

The structure could potentially generated via reflection information.

## Native bag of options

Options could be represented via an internal struct on which a hash can be performed. 

Input can be described as "deltas" to the current options. The final options is the combination of all the deltas - which would produce the final options structure for use. The hash of all of the options is the hash of the final structure.

How are the deltas described?

The in memory representation is not trivial in that if we want to add a struct to a list we would need a way to describe this.

Whilst in the runtime the 'field' could be uniquely identified an offset, within a file format representation it would need to be by something that works across targets, and resistant to change in contents. That implies it should be a name.

If we ensure that all types involved in options as JSON serializable via reflection, this does provide a way for code to traffic between and manipulate the native types.

How do we add a structure to a list?

```JSON
{
    "+listField", { "structField" : 10, "anotherField" : 20 }
} 
```

The problem perhaps is how to implement this in native code? It looks like it is workable with the functionality already available in RttiUtil.

## Slangs Component System

Slang has a component system that can be used for combining options to produce a compilation. An argument can be made that it should be part of the hashing representation, as it is part of compilation.

If combination is at the level of components, then as long as components are serializable, we can represent a compilation by a collection of components. Has several derived interfaces...

* IEntryPoint
* ITypeConformance 
* IModule

Can be constructed into composites, through `createCompositeComponentType`, which describes aspects of the combination takes place.

If the components were serializable (as say as JSON), we could describe a compilation as combination of components. If components are named, a concatenation of names could name a compilation.

It doesn't appear as if there is a way to more finely control the application of component types. For example if there was a desire to change the optimization option, it would appear to remain part of the ICompileRequest (it's not part of a component). This implies this mechanism as it stands whilst allowing composition, doesn't provide the more nuanced composition. Additional component types could perhaps be added which would add such control.

Perhaps having components is not necessary as part of the representation, as 'component' system is a mechanism for achieving a 'bag of options' and so we can get the same effect by using that mechanism without components.

Discussion: Container 
=====================
    
## Manifest or association

A typical container will contain kernels - in effect blobs. The blobs themselves, or the blob names are not going to be sufficient to express the amount of information that is necessary to meet the goals laid out at the start of this document. Some extra information may be user supplied. Some extra information might be user based to know how to classify different kernels. Therefore it is necessary to have some system to handle this metadata. 

As previously discussed the underlying container format is a file system. Some limited information could be infered from the filename. For example a .spv extension file is probably SPIR-V blob. For more rich meta data describing a kernel something more is needed. Two possible approaches could be to have a 'manifest' that described the contents of the container. Another approach would to have a file associated with the kernel that describes it's contents.

Single Manifest Pros

* Single file describes contents
* Probably faster to load and use
* Reduces the amount of extra files
* Everything describing how the contents is to be interpreted is all in one place

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

Given that we want the format to represent a file system, and that we would want it to be easy and intuitive how to manipulate the representation, using a single manifest is probably ruled out. It remains to be seen which is preferable in practice, but it seems likely that using 'associated files' is probably the way to go.

## How to represent data

As previously discussed, unless there is a very compelling reason not to we want to use representations that are open standards and easy to use. We also need such representations to be resilient to changes. It is important that file formats can be human readable or easily changeable into something that is human readable. For these reasons, JSON seems to be a good option for our main 'meta data' representation. Additionally Slang already has a JSON system.

If it was necessary to have meta data stored in a more compressed format we could consider also supporting [BSON](https://en.wikipedia.org/wiki/BSON). Conversion between BSON and JSON can be made quickly and simply. BSON is a well known and used standard.    
    
Discussion: Container Layout
============================

We probably want

* Global configuration information - describe how names map to contents
* Configuration that is compiler specific
  * The format could support configuration for different compilers
* Use 'associated' file style for additional information to a result 

```
config
config/global.json
config/slang.json
config/dxc.json
source/
source/some-source.slang
source/some-header.h
```

The `source` path holds all the unique source used during a compilation. This is the 'deduped' representation. Any include hierarchy is lost. Names are generated such that they remain the same as the original where possible, but are made unique if not. The 'dependency' file for a compilation specifies how the source as included maps to the source held in the source directory. The source held in the repository like this provides a way to repeat compilations from source, but isn't the same as the source hierarchy for compilation and is typically a subset. 

`config/global.json` holds configuration that applies to the whole of the container. In particular how names map to the container contents - say the use of directories, or naming concat order.
`config/slang.json` holds how names map to option configuration that is specific to Slang

We may want to have some config that applies to all different compilers. 

We may want to use the 'name' mechanism for some options, but commonly changing items such as the translation unit source name, entry point name can be passed directly (and used as part of the name).

Let's say we have a config that consists of `target`, `configuration`. And we use the source name and entry point directly. We could have a configuration that expressed this as a location 

```JSON
{
    "keyPath" : "$(target)/$(filename)/$(entry-point)-$(configuration)"
}
```

Lets say we compile `thing.slang`, with entry point 'computeMain' and options

```
target: vk
configuration: release
```

We end up with 

```
vk/thing/computeMain-release.spv
vk/thing/computeMain-release.spv-info.json
vk/thing/computeMain-release.spv-diagnostics.json
vk/thing/computeMain-release.spv-layout.json
vk/thing/computeMain-release.spv-dependency.json
```

`-info.json` holds the detailed information about what is in spv to identify the artifact, but also for the 'system' in general. Including 

`-dependency.json` is a mapping of source 'names' as part of compilation to the file

* Artifact type
* The combination of options (which might be *more* than in the path)
* Hashes/other extra information

Other items associated with the main 'artifact' - typically stored as 'associated' in an artifact, are optional and could be deleted. 

Having an extension, on the associated types is perhaps not necessary. Doing so makes it more clear what items are from a usability point of view. If this data can be represented in multiple ways - say JSON and BSON it also makes clear which it is.

Discussion: Interface
=====================

There are perhaps two ends of the spectrum of how an interface might work. One one end would be the interface is a 'slang like' compiler interface, with the the most extreme version being it *is* the Slang compiler interface. Having this interface like this means

* There is direct access to all options
* If your application already uses the Slang interface, it can just be switched out for the cache
* Has full knowledge of the compilation - making identification unambiguous, and trivially allowing fallback to actually doing a compilation
* Trivially supports more unusual aspects of the API such as the component system
* There is no (or very little) new API, the shader container API *is* the Slang API

It also means

* The API has a very large surface area
* It works at the detail of the API
* Does not provide an application level indirection to some more meaningful naming/identification
* It is tied to the Slang compiler - so can't be seen as an interface to 'shader container's more generally
* Naming will almost certainly need to include a hash 
* The hash will be hard to produce independently (it will be hard to calculate just anyway)

More significantly

* Higher requirement for source
  * Could store hashes of source seen
    * If there is source injection does this even make sense?
  * Could store modules as Slang IR
* For generated source it requires the source
* All source does in general need to be hashed - as paths do not indicate uniqueness
* How is this obfuscated? The amount of information needed is *all of the settings*.

At the other end of the spectrum the interface could be akin to passing in a set of user configurable parameter "names", that identify the input 'options'. The most extreme form might look something like....

```
class IShaderContainer
{
    Result getArtifact(const char*const* configNames, Count configNamesCount, const Options& options, IArtifact** outArtifact);
    Result getOrCreateArtifact(const char*const* configNames, Count configNamesCount, const Options& options, IArtifact** outArtifact);
};
```

Q: Perhaps we don't return an Artifact, because the IArtifact interface is not simple enough. Maybe it returns a blob and an ArtifactDesc? 
Q: We could simplify the IArtifact interface by moving to IArtifactContainer. Perhaps we should do this just for this reason?
Q: Is there a way to access other information - diagnostics for example? With IArtifact that can be returned as associated data. We don't want to create by default probably.
Q: If we wanted to associate 'user data' with a result how do we do that? It could just be JSON stored in the `-info`?
Q: We could have a JSON like interface for arbitrary data?

The combination of the 'configNames' produce the key/paths within the container. 

It would probably be desirable to be able to create 'configNames' through an API. This would have to be Slang specific, and not part of this interface. The config system could be passed into the construction of the container. Doing so might contain all the information to map names to how to compile something. 

This interface may be a little too abstract, and perhaps should have parameters for common types of controls.

As previously touched on it may be useful to pass in configuration that is *not* part of the key name to override compilation behavior. 

This style means

* Naming is trivial
* Hashing is often not necessary
* Issues such as 'generated source' are pushed to user space
* Is user configurable 
* Main interface is very simple and small
* Can be used with other compilers - because the interface is not tied to Slang in any way
* Implies the container format itself can be used trivially 
* Human/application centered
* Hashing of source/options is still possible, for a variety of purposes, but is not a *requirement* as it doesn't identify a compilation. 
  * Meaning a simpler/less stable hashing might be fine

With this style is it implied that the identification of a unique combination is a user space problem. For example that the source is static in general, and if not generated source identification is a user space problem. It's perhaps important to note that mechanisms previously discussed - such as hashing the source can still be useful and used. The hashing of source could be used to identify in a development environment that a recompilation is required. Or an edit of source could be made, and a single command could update all contents that is applicable automatically. These are more advanced features, and are not necessary for a user space implementation, which typically do not require the capability.

More problematically

* Doesn't provide a runtime cache that 'just works' for example just using the slang API
* Needs to provide a way given a combination of config names to produce the appropriate settings
  * If it is just a delivery mechanism this isn't a requirement
* Probably needs both an API and 'config' mechanisms to describe options
* The indirection may lose some control

All things considered, based on the goals of the effort it seems to make more sense to have an interface that is in the named config style. Because

* It allows trivial 3rd party implementation
* It works with other compilers - (important if it's to work as some kind of standard)
* Provides an easy to understand mapping from input to the contents of the cache
* Can use more advanced features (like source hashing) if desired

How config options are described or combined may be somewhat complicated, but is not necessary to use the system, and allows different compilers to implement however is appropriate.

Discussion : Deduping source
============================

When compiling shaders, typically much of the source is shared. Unfortunately it is not generally possible to just save 'used source', because some source can be generated on demand. One way this is already performed by users is to use a specialized include handler, that will inject the necessary code. 

It is not generally possible therefore to identify source by path, or unique identity (as used by the slang file system interface). 

It is also the case that compilations can be performed where the source is passed by contents, and the name is not set, or not unique. 

The `slang-repro` system already handles these cases, and outputs a map from the input path to the potentially 'uniquified' name within the repro.

You could imagine a container holding a folder of source that is shared between all the kernels. In general it would additionally require a map of each kernel that would map names to uniqified files.

In the `slang-repro` mechanism the source is actually stored in a 'flat' manner, with the actual looked up paths stored within a map for the compilation. It would be preferable if the source could be stored in a hierarchy similar to the file system it originates. This would be possible for source that are on a file system, but would in general lead to deeper and more complex hierarchies contained in container. 

Including source, provides a way to distribute a 'compilation' much like the `slang-repro` file. It may also be useful such that a shader could be recompiled on a target. This could be for many reasons - allowing support for future platforms, allowing recompilation to improve performance or allowing compilation to happen on client machines for rare scenarios on demand.

We may want to have tooling such that directories of source can be specified and are added to the deduplicated source library.

We may also want to have configuration information that describes how the contents maps to search paths. It might be useful to have only the differences for lookup stored for a compilation, and some or perhaps multiple configuration files that describe the common cases.

Discussion : Artifact With Runtime Interface
============================================

It should be noted *by design* `IArtifactContainer`s children is *not* a mechanism that automatically updates some underlying representation, such as files on the file system. Once a IArtifactContainer has been expanded, it allows for manipulation of the children (for example adding and removing). The typical way to produce a zip from an artifact hierarchy would be to call a function that writes it out as such. This is not something that happens incrementally. 

For an in memory caching scenario this choice works well. We can update the artifact hierarchy as needed and all is good.

In terms of just saving off the whole container - this is also fine as we can have a function given a hierarchy that saves off the contents into a ISlangMutableFileSystem, such that it's on the file system or compressed. 

If we want the representation to be *synced* to some backing store this presents some problems. It seems this most logically happens as part of the compilation interface implementation. The Artifact system doesn't need to know anything about such issues directly.

Once a compilation is complete, an implementation could save the result in Artifact hierarchy and write out a representation to disk from that part of the hierarchy. For some file systems doing this on demand is probably not a great idea. For example the Zip file system does not free memory directly when a file is deleted. Perhaps as part of the interface there needs to be a way to 'flush' cached data to backing store. Lastly there could be a mechanism to write out the changes (or the new archive).

Discussion : Other
==================

It would be a useful feature to have tooling where it is possible to

## Generating the container

* Generate updated kernels automatically offline
  * For example when a source file changed
  * For example when a config file changed
  * Just force rebuilding the whole container
* Specify the combinations that are wanted in some offline manner
  * Perhaps compiling in parallel
  * Perhaps noticing aspects such that work can be shared 

## Obfuscation

* Most simply could be using a hash of a 'key'. 
* Or perhaps if the desire is to obfuscate at the application level, a hash of the *names* as input could be used

## Stripping containers

At a minimum there needs to be mechanisms to be able to strip out information that is not needed for use on a target. 

There probably also additionally needs to be a way to specify items such that names, such as type names, source names, entry point names, compile options and so forth are not trivially contained in the format, as their existence could leak sensitive information about the specifics of a compilation.

## Indexing

No optimized indexed scheme is described as part of this proposal. 

Indexing is probably something that happens at the 'runtime interface' level. The index can be built up using the contents of the file system.

No attempt at an index is made as part of the container, unless later we find scenarios where this is important. Not having an index means that the file system structure itself describes it's contents, and allows manipulation of the containers contents, without manipulation of an index or some other tooling.

## Slang IR

It may be useful for a representation to hold `slang-ir` of a compilation. This would allow some future proofing of the representation, because it would allow support for newer versions of Slang and downstream compilers without distributing source. 

Related Work
============

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
    

<a id="proposed-approach"></a>
Proposed Approach
=================

Based on the goals described in the introduction, the proposed approach is

* Use a collection of named options to describe a compilation
  * Requires a mechanism to provide combining
  * Have some additional symbols (such as + field name prefix as described elsewhere) to describe how options should be applied
* Meaning of names can be described within configuration and through an API
  * API might be compiler specific
* Some names contribute to the key, whilst others do not
  * Non inclusion in key allows customization for a specific result without a key change
* Some options within a configuration can use standard names, others will require being compiler specific
* Probably easiest to use a native representation for combining
  * Using the collection of names approach makes hash stability and hashes in general less important
* Use JSON/BSON as the format for configuration files
  * Possible to have some options defined that *aren't* part of the key name
  * The actual combination can be stored along with products, such that the combination can be recreated, or an inconsistency detected
* Use JSON to native conversion to produce native types that can then be combined
* Can have some standard ways to generate names for standard scenarios
  * Such as using input source name as part of key
* Use associated files (not a global manifest), to allow easy manipulation/tooling
* Source will in general be deduped, with a compilation describing where it's source originated
  * This is similar to how repro files work
* Some names will be automatically available by default 
* Ideally a 'non configured' (or default configured) cache can work for common usage scenarios.  
  
For the runtime cache scenario this all still works. If an application wants a runtime cache that is memory based that works transparently (ie just through the use of Slang API), this is of course possible and it's output can be made compatible with the format. It will be fragile to Slang API changes, and probably not usable outside of the Slang ecosystem.   
  
Alternatives Considered
-----------------------

Discussed elsewhere.

## Issues On Github 

* Support low-overhead runtime "shader cache" lookups [#595](https://github.com/shader-slang/slang/issues/595)
* Compilation id/hash [#2050](https://github.com/shader-slang/slang/issues/2050)
* Support a simple zip-based container format [#860](https://github.com/shader-slang/slang/issues/860)
 
