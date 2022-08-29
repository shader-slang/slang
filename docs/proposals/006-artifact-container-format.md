Artifact Container Format
========================

This document provides a starting point for a larger feature proposal.
The sections in it are suggested, but can be removed if they don't make sense for a chosen feature.

The first section should provide a concise description of **what** the feature is and, if possible, **why** it is important.

A proposal for a Slang language/compiler feature or system should start with a concise description of what the feature it and why it could be important.

This proposal is for a file hierarchy based structure that can be used to represent compile results and more generally a 'shader cache'. Ideally would feature

* Does not require an extensive code base to implement
* Flexible and customizable for specific use cases
* Possible to produce a simple fast implementation 
* Use simple and open standards where appropriate

It's importance

* Provides a way to represent complex compilation results 
* Could be used to support an open standard around 'shader cache'
* Provide a standard 'shader cache' system that can be used for Slang tooling and customers 
* Supports Slang tooling and language features

Status
------

The Artifact system exists and provides a mechanism to transport source/compile results through the Slang compiler. Artifact has support for "containers". An artifact container is an artifact that can contain other artifacts. Support for different 'file system' style container formats is also implemented. The currently supported underlying container formats supported are

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

The related work section should show examples of how other languages, compilers, etc. have solved the same or related problems. Even if there are no direct precedents for what is being proposed, there should ideally be some points of comparison for where ideas sprang from.

Proposed Approach
-----------------

Explain the idea in enough detail that a reader can concretely know what you are proposing to do. Anybody who is just going to *use* the resulting feature/system should be able to read this and get an accurate idea of what that experience will be like.

Detailed Explanation
--------------------

Here's where you go into the messy details related to language semantics, implementation, corner cases and gotchas, etc.
Ideally this section provides enough detail that a contributor who wasn't involved in the proposal process could implement the feature in a way that is faithful to the original.

Alternatives Considered
-----------------------

Any important alternative designs should be listed here.
If somebody comes along and says "that proposal is neat, but you should just do X" you want to be able to show that X was considered, and give enough context on why we made the decision we did.
This section doesn't need to be defensive, or focus on which of various options is "best".
Ideally we can acknowledge that different designs are suited for different circumstances/constraints.
