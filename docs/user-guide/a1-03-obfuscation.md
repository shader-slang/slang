---
layout: user-guide
---

Obfuscation
===========

The Slang obfuscation feature allows developers to distribute shader code in a way where the implementation details are obscured, and so to a degree protected. For example lets say a developer has produced a novel way to render and want to protect that intellectual property. If it is possible to compile all possible uses of the shader code into SPIR-V/DXIL the developer can ship their product with those binaries without debug information. This is similar to the protection achived by shipping an executable - a determined person may with a good deal of effort work out how some algorithm in the executable works, but doing so requires a considerable amount of work, and certainly more work than reading the originating source code.

If a developer is not able to ship shader binaries then there is a problem. The developer doesn't want to ship the source code as in doing so it is relatively straight forward to see how it works or even copy the implementation. A developer could provide some level of protection by encrypting the source, but when compilation occurs it will still be necessary to decrypt and so make it available to read. A developer could obfuscate their source before shipping it. To do so they would

* Require tooling to do the obfuscation of the source
* Any source on the client that isn't obfuscated, needs to be able to call to the obfuscated code
  * Depending on how the obfuscation takes place this could be hard - remapping symbols or obfuscating on the fly on the client
  * If "public" symbols keep their original names they leak information about the implementation
* Obfuscated source is easier to read and interpret than say a binary file format, and so less protected
* How can you debug with the original source, if a crash happens on obfuscated source compiled on the clients machine
* If a failure occurs - how is it possible to report meaningful errors

Some of these issues are similar to the problems of distributing JavaScript libraries that run on client machines, but which the original authors do not want to directly make available the implementation. Some of the obfuscation solutions used in the JavaScript world are partially applicable to Slangs obfuscation solution, including [source maps](https://github.com/source-map/source-map-spec).

Obfuscation in Slang
====================

Slang provides an obfuscation feature that addresses these issues. The major parts being

* The ability to compile a module with obfuscation enabled
  * The module is a binary format made up or 
* The ability to compile regular slang code that can *link* against an obfuscated module
* Code emitted to downstream compilers contain none of the symbols from the original source
* Source map(s) to provide mappings between originating source and obfuscated source produced on the client

Enabling obfuscation can be achieved via the "-obfuscate" option. When enabled a few things will happen

* Source locations are scrambled to (blank) lines in an "empty" obfuscation source file.
* A source map is produced mapping from the (blank) lines, to the originating source locations 
* Name hints are stripped.
* If a `slang-module` is being produced, AST information will be stripped.
* The names of symbols are scrambled into hashes

The source slang emits that is passed down to downstream compilers is obfuscated, and only contains the sections of code necessary for the kernel to compile and function. 

Currently all source that is going to be compiled and linked must all have the `-obfuscate` option enabled to be able to link correctly.

When obfuscation is enabled, source locations are scrambled, but Slang will also create a [source map](https://github.com/source-map/source-map-spec), that provides the mapping from the obfuscated locations to the original source. This so called "obfuscated source map" is stored with the module. If compilation produces an an error, Slang will automatically use the obfuscated source map to display the error location in the originating source.

If the obfuscated source map isn't available, it will still display a source location if available, but the location will be in the "empty" obfuscated source file. This will appear in diagnostics as "(hex-digits)-obfuscated(line)". With this information and the source map it is possible to output the original source location. Importantly without the obfuscated source map information leakage about the original source is very limited.

It should be noted that the obfuscated source map is of key importance in hiding the information. In the example scenario of protecting intellectual property, a developer should compile the code they wish to protect with `-obfuscate` and distribute *just* the `.slang-module` file to link on the client machine. The source map file should not be distributed onto client machines. 

## Accessing Source Maps

During a compilation Slang can produce many different "artifacts". When using the obfuscated source map option to produce a `slang-module` slang will associate a source map with the mapping for that module with the module. 

With typical Slang usage, a compilation takes place and the output is a "blob" that is the output kernel. It is also possible to compile to a container, such as a zip file or a directory. The zip file can contain other artifacts than kernels including source maps.



Implementation
==============