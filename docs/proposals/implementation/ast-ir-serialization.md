Improving AST and IR (De)Serialization
======================================

Background
----------

Slang supports serialization of modules after front-end compilation.
A serialized module contains a combination of AST-level and IR-level information.

The AST-level information is primarily concerned with the `Decl` hierarchy of the module, and is used for semantic checking of other modules that `import` it.
The serialized AST needs to record information about types, as well as functions/methods and their signatures.
In principle, there is no need for function *bodies* to be encoded, and any non-`public` declarations could also be stripped.

The IR-level information is primarily concerned with encoding the generated Slang IR code for the `public` symbols in the module, so that they can be linked with other modules that might reference those symbols by their mangled name.
The serialized IR for a module does *not* encode information sufficient for semantic checking of modules that `import` it.

Currently, deserialization of the AST or IR for a module is an all-or-nothing operation.
Either the entire `Decl` hierarchy of the AST is deserialized and turned into in-memory C++ objects, or none of it is.
Similarly, we can either construct the `IRInst` hierarchy for an entire module, or none of it.

Releases of the Slang compiler typically included a serialized form of the core module, and the runtime cost of deserializing this module has proven to be a problem for users of the compiler.
Because parts of the Slang compiler are not fully thread-safe/reentrant, the core module must be deserialized for each "global session," so that deserialization cost is incurred per-thread in scenarios with thread pools.
Even in single-threaded scenarios, the deserialization step adds significantly to the startup time for the compiler, making single-file compiles less efficient than compiling large batches of files in a single process.

Overview of Proposed Solution
-----------------------------

The long/short of the proposed solution is to enable lazy/on-demand deserialization of both the AST and IR information for a module.

Enabling on-demand deserialization requires defining a new on-disk representation for serialized AST and IR modules.
Defining entirely new formats for serialization, implementing them, and then switching over to them will be a large task on its own.
It is recommended that we retain *both* the old and new implementations of serialization in the codebase, until we are confident that we are ready to "flip the switch" and exclusively support the new one.

In addition to changes related to the new serialized formats, there will of course be changes required to the C++ code that interacts with previously-compiled modules.
This document will attempt to highlight the critical places where logic will need to change for both the AST and IR.

IR
---

We will start by discussing the IR case, because it is simpler than the AST case.
The way the compiler works with Slang IR was designed with the assumption that on-demand deserialization would eventually be desired, so there are (hopefully) fewer thorny issues.

### Intercept During IR Linking

We expect that the primary (and perhaps only) point where on-demand deserialization will be needed is when running IR-level linking.

IR linking takes as input a collection of one or more IR modules, and information to identify one or more public/exported symbols (typically shader entry points) for which target code is desired.
The linker creates a *fresh* `IRModule` for the linked result, and clones/copies IR instructions from the input modules over to the output using a superficially-simple process:

1. Given an instruction in an input module to be copied over, use an `IRBuilder` on the output module to create a deep copy of that instruction and its children.

2. Whenever an instruction being copied over references another top-level instruction local to the same input module (that is, one without a linkage decoration), either construct a deep copy of the referenced instruction in the output module, or find and re-use a copy that was made previously.

3. Whenever an instruction being copied over references a top-level instruction that might be resolved to come from another module (that is, one with a linkage decoration), use the mangled name on the linkage decoration to search *all* of the input modules for candidate instructions that match. Use some fancy logic to pick one of them (the details aren't relevant at this exact moment) and then copy the chosen instruction over, more or less starting at (1) above.

A key observation is that nothing about these steps actually *cares* if the input module is realized in-memory as a bunch of `IRInst`s, or just as serialized data.
Furthermore, there are only two cases where a top-level instruction in an input module might need to be copied over to the output module:

* When it is referenced by another instruction inside the same module
* When it is referenced from another module by its mangled name

So, the long/short of the proposed changes to the C++ code for the IR is to make it so that the input to IR linking is a collection of one or more **serialized** IR modules, and the linker only deserializes the specific top-level instructions that need to be copied over to the output.
Effectively, the linker is just deserializing instructions from the serialized input modules *into* the output module.

### On-Disk Encoding

This document will sketch one *possible* on-disk encoding for an IR module that supports on-demand deserialization, but it is not intended to take away the freedom for the implementer to make other choices.

We propose that the serialized IR should use a section-based format where the entire file can be memory-mapped in and then the byte range of individual sections can be found without any actual deserialization or copying of data.

Two of the sections in the file will contain:

* The raw bytes representing serialized IR instruction trees
* An array of *top-level instruction entries*

Top-level instruction entries can be referenced by index (a *top-level instruction index*).
Each entry specifies the range of bytes in the "raw bytes" section above that encodes that instruction and its operands/children.
We propose that index 0 be used as a special case to indicate a null or missing instruction; the entry at that index should not be used for real data.
The "raw bytes" encoding of a given top-level instruction and its tree of children need *not* support random access.

When encoding the operands of an instruction in the "raw bytes" section, there must be a way to determine whether an operand refers to another instruction in the same top-level-instruction tree, or refers to *another* top-level instruction tree.
Operands that refer to other top-level instruction trees will store the top-level instruction index, so that the matching entry can be found easily.

When performing on-demand deserialization, application code can easily maintain a map from the top-level instructio index to the corresponding `IRInst*` to cache and re-use deserialized instructions.
It can even use a flat array of `IRInst*` allocated based on the number of entries, for simplicity.

Along with the above sections, the serialized format should contain:

* A *string table* which stores the raw data for any strings in the IR module (including mangled symbol names) and allows strings to be referenced by a simple *string table index*
* A hash-table or other acceleration structure that maps mangled names (as string table indices) to top-level instruction indices.

> Note: One detail being swept under the rug a bit here is what to do when a module has multiple top-level instructions with the *same* mangled name.
> So long as we retain that flexibility (which we may not need in the presence of `target_switch`), the acceleration structure might have to map from a mangled name to a *list* of top-level instruction indices.

If we decide that storing a serialized hash-table adds too much complexity, we can instead store a flat array of pairs (string-table-index, top-level-instruction-index) sorted by the content of those strings, and then do a binary search.
Whether we use hashing or a binary search, it would be ideal if looking up a top-level instruction by mangled name did not require deserializing the acceleration structure.

> Note: Another small detail here is that the serialized format being proposed does not clearly distinguish cases (2) and (3) in the deserialization/linking process described above.
> More or less, the linker would find an operand that references another top-level instruction in the same module, and would then deserialize it on-demand from that module, along the lines of case (2).
> If, after deserialization, we find that the instruction has a linkage decoration, we can jump to step (3) and scan for instructions in *other* modules that match on name.


AST
---

### Intercept During Lookup and Enumeration

There are many more parts of the compiler that touch AST structures that *might* be in a partially-deserialized state, so there are far more contact points that will have to be discovered and handled.

At the most basic, the proposal is to change `ContainerDecl` so that it supports having only a subset of its child declarations (aka "members") loaded at a time.
The two main ways that the child declarations are accessed are:

* Enumeration of *all* members (or all members of a given class), which involves iterating over the `ContainerDecl::members` array.

* Lookup of members matching a specific name, which involves using the `ContainerDecl::memberDictionary` dictionary.

Currently the `memberDictionary` field is private, and has access go through methods that check whether the dictionary needs to be rebuilt.
The `members` field should also be made private, so that we can carefully intercept any code that wants to enumerate all members of a declaration.

We should probably also make the `memberDictionary` field map from a name to the *index* of a declaration in `members`, instead of directly to a `Decl*`.

> Note: We're ignoring the `ContainerDecl::transparentMembers` field here, but it does need to be taken into account in the actual implementation.

There is already a field `ContainerDecl::dictionaryLastCount` that is used to encode some state related to the status of the `memberDictionary` field.
We can update the representation used by that field so that it supports four cases instead of the current three:

* If `count == members.getCount()`, then the `members` and `memberDictionary` fields are accurate and ready to use for enumeration and lookup, respectively.

* Otherwise, if `count >= 0`, then the `members` array is accurate, and the `memberDictionary` includes the first `count` members, but not those at or after `count` in the array.

* If `count == -1`, then the `members` array is accurate, but the the `memberDictionary` is invalid, and needs to be recreated from scratch.

* If `count == -2`, then we are in a new case where the declaration is being lazily loaded from some external source.

In the new "lazy-loading" case, any entries in the `memberDictionary` will be accurate, but the absence of an entry for a given `Name*` does *not* guarantee that the declaration has no children matching that name.
The `members` array will either be empty, or will be correctly-sized for the number of children that the declaration has.
The entries in `members` may be null, however, if the corresponding child declaration has not been deserialized.

We will need to attach a pointer to information related to lazy-loading to the `ContainerDecl`.
The simplest approach would be to add a field to `ContainerDecl`, but we could also consider using a custom `Modifier` if we are concerned about bloat.

#### Enumeration

If some piece of code wants to enumerate all members of a given `ContainerDecl` that is in the lazy-loading mode, then we will need to:

* Allocate a correctly-sized `members` array, if one has not already been created.

* Walk through each child-declaration index and on-demand load the child declaration at that index (if its entry in `members` is null)

This is a relatively simple answer, and it is likely that the biggest problems will arise around code that is currently enumerating all members of a container but that we would rather *didn't* (e.g., code that enumerates all `extension`s).

One potential cleanup/improvement would be to create a unique `Name*` for each kind of symbol that has no name of its own.
E.g., each `init` declaration could be treated as-if its name was `$init`, and so on for `$subscript`, `$extension`, etc.
That change would mean that enumerating all child declarations of certain specific classes is equivalent to *looking up* child declarations of a given name.
We should consider making such a change if/when we see that code is enumerating all declarations and forcing full deserialization where it wasn't needed.

#### Lookup

The main place where the `ContainerDecl::memberDictionary` field needs to be accessed is during name lookup.
When looking up a name in a `ContainerDecl` that is in lazy-loading mode, the process would be:

* If the lookup finds a valid index in `memberDictionary`, then that is the index of the first child declaration with that name (and the others can be found via the `Decl::nextInContainerWithSameName` field).

* We could potentially use a sentinel value like a `-1` index in `memberDictionary` to indicate that there are definitely no members of that name.

* Otherwise, we will need to inspect the serialized representation of the given `ContainerDecl` to see if there are not-yet-deserialized members matching that name.

In that last case, we either find that there were *no* matching members in the serialized data, in which case we could stash a sentinel value in `memberDictionary`, or we find that there *were* one or more members, in which case we should deserialize those members into the `members` array, stash the indices of the first one into `membersDictionary` and then return the resulting (deserialized) members.

### On-Disk Encoding

Similar to what is proposed for the IR, we propose to use a section-based format for AST serialization, and that two of the key sections should be:

* An array of *AST entries* each referenced by an *AST entry index*
* A section for the raw data of each serialized AST entry

As in the IR case, we propose that an AST entry index of zero be used to represent a null or missing entry.

Like the IR, the AST has an underlying design where each node has some number of *children*, which it owns, and also some number of *operands*, which it references.
Despite the similarity, the AST structure is more complicated than the IR structure for a few reasons:

* The operands of one AST node might reference AST nodes from outside the same top-level declaration, but that are not *themselves* top-level declarations (they might be a child of a child of a top-level declaration).

* While much of the AST structure is made of `Decl`s, there can also be references to `Type`s and `DeclRef`s, etc. Some of the uniformity of the IR ("everything is an `IRInst`") is missing.

These complications lead to two big consequences for the encoding:

* The array of *AST entries* will not just contain the entries for top-level `Decl`s. It needs to contain an entry for each `Decl` that might be referenced from elsewhere in the AST. For simplicity, it will probably contain *all* `Decl`s that are not explicitly stripped as part of producing the serialized AST.

* The array won't even consist of just `Decl`s. It will also need to have entries for things like `DeclRef`s and `Type`s that can also be referenced as operands of AST nodes.

As a stab at a simple representation, each AST entry should include:

* A *tag* that defines the subclass of the node (more or less like the tags we use on AST nodes at runtime)

* A range of bytes in the raw data that holds the serialized representation of that node (e.g., its operands)

An entry for a `ContainerDecl` should include (whether directly or encoded in the raw data...)

* A contiguous range of AST entry indices that represent the direct children of the node, in declaration order (the order they'd appear in `ContainerDecl::members`)

* A contiguous range of AST entry indices that represent all the descendents of the node

Index `1` in the entry array should probably represent the entire module, and thus establish a root for the entire `Decl` hierarchy and associated ranges.
We should require that a parent declaration is always listed before its children.

Given the above representation, there is no need to explicitly encode the parent of a `Decl`.
Given an AST entry index for a `Decl`, we can find its parent by recursing through the hierarchy starting at the root, and doing a binary search at each hierarchy level to find the (unique) child declaration at that level which contains that index in its range of descendents.

When there is a request to on-demand deserialize a `Decl` based on its AST entry index, we would need to first deserialize each of its ancestors, up the hierarchy.
That on-demand deserialization of the ancestors can follow the flow given above for recursively walking the hierarchy to find which declaration at each level contains the given index.

In order to support lookup of members of a declaration by name, we propose the following:

* A string table for storing all the strings used in the AST (including names), so that each can be references by a string table index

* A lookup structure that maps each string table index to a list of *all* `Decl`s in the module that have that name, in sorted order.

As in the IR case, the lookup structure could be something like a hash table, or it could simply be an array of key-value pairs where the keys are sorted by their string values.

Given a parent decl `P`, and a name for a member we want to look up in it, the procedure would basically be:

* Use the lookup structure to find the (sorted) list of AST entry indices for all `Decl`s with the given name

* Use binary searches on that sorted list to find the subset of indices that are within the range that represent child declarations of `P`.

> Note: One key detail being glossed over here is when lookup needs to traverse through the "bases" of a type.
> The `InheritanceDecl`s that represent the bases of a type are very similar to "transparent" declarations.
> If we want to support lazy lookup of members of type declarations like `struct`s, we would need to be able to eagerly deserialize the `InheritanceDecl` members, without also deserializing all the others.
> This might be a good use case for the idea pitched above, of giving all unnnamed declarations of a given category a single synthetic name (e.g., `$base`), so that they can be queried by ordinary by-name lookup, which would trigger on-demand deserialization.

It would be possible to store individual lookup structures as part of the serialized data for each `ContainerDecl`, but the approach given here sacrifices some possible efficiency in the lookup step for the sake of storing less data on each AST node.

Shared Stuff
------------

Some bits of the implementation described here are the same, or have large overlap, between the IR and AST case.
This section describes some points about implementation that could apply to both.

### String Tables

It seems reasonable to encode string tables the same way for both IR and AST (and any other parts of Slang that would like to serialize lots of strings).

As with other structures described above, a string table should probably be split into two pieces:

* An array of *entries*, one for each string in the table
* A range of raw bytes for the data associated with each entry

As with other structures, we recommend leaving index 0 unused, to represent a null or absent string (as something distinct from an *empty* string).

In practice, the string tables created by a compiler like Slang (and *especially* any string tables that might include things like mangled names) will contain many strings with common prefixes. In order to optimize for this case, we can store strings in a structure inspired by a "suffix tree."

First we collect all of the strings that need to be stored.
Then we perform a lexicogrpahic sort on them.
Then for each string table entry, we store:

* The size, in bytes, of the *prefix* it shares with the preceding string in the table
* The byte range in the "raw data" area for the data of the string that comes after that prefix

### Abbreviations

As discussed above, both the IR and AST can be described in simplified terms as a hierarchy of nodes, where each node comprises:

* An opcode/tag
* Zero or more *operands*, which are references to other nodes in the hierarchy
* Zero or more *children*, which are uniquely owned by this node

> Note: The Slang IR representation sticks very zealously to this model, but the AST is a lot more loose as a byproduct of starting as a purely ad hoc C++ class hierarchy.
> Ideally we should be able to serialize the AST *as if* it had a more uniform structure than it really has, but we might also want to do refactoring passes on the in-memory representation of the AST to make it more uniform.

A serialized module will typically contain a large number of nodes, and there will often be a high degree of redundancy between nodes.
That redundancy can be exploited to reduce the size of each node.

The basic idea is that rather than explicitly encoding the opcode/tag and number of children, each serialized node would encode an index into a table of *abbreviations* stored as a section of the serialized data.
This idea is loosely based on how abbreviations work in DWARF debug information, although greatly simplified.

Each entry in the abbreviation table would store:

* The opcode/tag used by all nodes that are defined with this abbreviation
* (Optionally) zero or more operands that are used as a prefix on the operands of nodes using this abbreviation
* The number of additional operands to read for each node
* (Optionally) some information on the number of children, or on structurally-identical children that all nodes created from this abbreviation should share as a prefix (e.g., a set of IR decorations)

When deserializing a node, code would read its abbreviation index first, and then look up the corresponding abbreviation to both find important information about the node, and also to drive deserialization of the rest of its data (e.g., by determining how many operands to read before reading in children).

In cases where the low-level serialization uses things like variable-length encodings for integers, the abbreviations can be sorted so that the most-frequently used abbreviations have the lowest indices, and thus take the fewest bits/bytes to encode.
