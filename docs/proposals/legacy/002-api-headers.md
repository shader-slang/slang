
Revise Slang/GFX API Headers
============================

The public C/C++ API headers for Slang (and GFX) are in need of cleanup and refactoring for us to reach a "1.0" API.
This document attempts to document the guidelines that we will follow in such a refactor.

Status
------

In discussion.

Background
----------

The Slang API header (`slang.h`) has evolved over many years, going back as far as the "Spire" research project, which predates Slang (Spire is the reason for the `sp` prefix on functions in the C API).

At some point, we made a conscious decision to move toward a COM-based API, both because it would simplify our story around binary compatibility and because it would allow us to provide more convenient API models in cases where subtyping/inheritance is fundamental to the domain.
Unfortunately, the net result has been that we have two different APIs cluttering up the same header file (the old C one, and the newer C++/COM one), and the one that is presented *first* is actually the one we would rather users didn't reach for.
The two APIs are sometimes out of sync, with one providing services the other doesn't.

While the GFX project started later and was thus able to start using COM interfaces and a C++ API from the start, it still faces some challenges around API evolution and binary compatibility.
As support for GPU features (whether pre-existing or new) gets added, we find that the various interfaces want to grow and the various `Desc` structures want to add new fields.
Without care, neither of those is a binary-compatible change for user code.

A concern across both Slang and GFX is that we have tended to design our APIs around the *most complicated* use cases we intend to support, at the expense of the *simplest* cases.
We know that we cannot remove support for difficult cases, but it would be good to support concise code for simple use cases, and to support a "progressive disclosure" style that allows users to gradually adopt more involved API constructs as they become necessary.

Related Work
------------

There are obviously far too many C/C++ APIs and approachs to design for C/C++ APIs for us to review them all.
We will simply note a few key examples that can be relevant for comparison.

The gold standard for C/C++ APIs is ultimately plain C. Plain C is easy for most systems programmers to understand and benefits from having a well-defined ABI on almost every interesting platform. FFI systems for other languages tend to work with plain C APIs. Clarity around ABI makes it easy to know what changes/additions to a plain C API will and will not break binary compatibility. The Cg compiler API and the Vulkan GPU API are good examples of C-based APIs in the same domains as Slang and GFX, respectively. These APIs reveal some of the challenges of using plain C for large and complicated APIs:

* The lack of subtype polymorphism is a problem when a domain fundamentally has subtyping. The Cg reflection API uses a single `CGtype` type to represent all types, so that the an operation like `cgGetMatrixSize` is applicable to any type, not just matrix types. The API cannot guide a programmer toward correct usage, and must define what happens in all possible incorrect cases.

* C has no built-in model for error signalling or handling. Error codes and out-of-band values (`NULL`, `-1`) are the norm, and there are a multitude of API-specific conventions for how they are applied.

* C has no built-in model for memory and lifetime management. Most APIs either expose create/delete pairs or some kind of per-type reference-counting retain/release operations. In either case, the application developer is left to ensure that the operations are correctly invoked, often by writing their own C++ wrapper around the raw C API.

Some developers opt for a "Modern C++" philosophy where the public API of a system makes direct use of standard C++ library types where possible.
For example, strings are passed as `std::string`s, cases that need polymoprhism expose `class` hierarchies, types that benefit from reference-counted lifetime management, explicitly uses `std::shared_ptr<...>`, and errors are signaled by `throw`ing exceptions.
The Falcor API ascribes to aspects of this approach.
The biggest challenges with Modern C++ APIs are:

* Source compatibility can usually be achieved, but binary compatibility is hard to achieve even *within* a version of a system, must less across versions. The central problem is that C++ ABIs are often compiler-specific (rather than standard on a platform), and even for a single compiler like gcc or Visual Studio, the binary interface to the C++ standard library can and does break between versions.

* While C++ exceptions are a built-in error-handling scheme, they are almost universally disliked among the kinds of developers who use APIs like Slang. Enabling exceptions in most compilers adds overhead, and actually using exceptions for their intended purpose (catching and handling errors) tends to be onerous.

* Reference-counted lifetime management in Modern C++ relies on standard library types like `std::shared_ptr` - a type that is both inefficient and inconvenient. Most developers in our target demographic end up using "intrusive" reference counts (when they use reference-counting at all) because they are more efficient and convenient.

COM is first and foremost an idiomatic way of using C++ to define APIs that are reasonably convenient while also dealing with the recurring problems of typical C and C++ approaches.
COM defines rules for error handling, memory management, and interface versioning that all compatible APIs can use.
While code using COM-based APIs is often verbose, it is largely consistent across all such APIs.

A key place where COM does *not* provide a complete answer is around fine-grained "extensibility" of APIs, of the kind that commonly occurs with GPU APIs like OpenGL, D3D, and Vulkan.
Across such APIs, we see a wide variety of strategies to dealing with extensibility:

* OpenGL uses an approach where objects are typically opaque but mutable, and a large number of fine-grained operations are used to massage an object into the correct state for use. Often the fine-grained state-setting operations are all able to use a single API entry point for key-value parameter setting (e.g., `glTexParameteri`), and a new feature can be exposed simply by defining constants for new keys and/or values. When new operations are required, they need to be queried using string-based lookup of functions.

* D3D11 uses COM interfaces and "desc" structures (called "descriptors" at the time). For example, a mutable `D3D11_RASTERIZER_DESC` structure is filled in and used to create an *immutable* `ID3D11RasterizerState`. If extended features are required, a new interface like `ID3D11RasterizerState1` and/or a new descriptor type like `D3D11_RASTERIZER_DESC1` is defined. In all cases, the "desc" structure holds the union of all state that a given type supports.

* Vulkan uses "desc" structures (usually called "info" or "create info" structures), which contain a baseline set of state/fields, along with a linked list of dynamically-typed/-tagged extension structures. New functionality that only requires changes to "desc" structures can be added by defining a new tag and extension structure. New operations are added in a manner similar to OpenGL.

* D3D12 also uses COM interfaces and "desc" structures (although now officialy called "descriptions" to not overload the use of "descriptor" in descriptor tables), much like D3D11, and sometimes uses the same approach to extensibility (e.g., there are currently `ID3D12Device`, `ID3D12Device`, ... `ID2D12Device9`). In addition, D3D12 has also added two variations on Vulkan-like models for creating pipeline state (`ID3D12Device2::CreatePipelineState` and `ID3D12Device5::CreateStateObject`), using a notion of more fine-grained "subojects" that are dynamically-typed/-tagged and each have their own "desc".

It is important to note that even with the nominal flexibility that COM provides around versioning, D3D12 has opted for a more fine-grained approach when dealing with something as complicated as GPU pipeline state.

Proposed Approach
-----------------

The long/short of the proposal is:

* The primary API interface to Slang (and GFX) should be COM-based and use C++. Convenient C++ features like `enum class` are usable when they do not constrain binary compatibility.

* Extensibility and versioning (where appropriate) should make use of "desc"-style tagged structures. Each of Slang and GFX will define its own `enum class` for the space of tags, rather than us trying to coordinate across the APIs.

* We will focus on providing "shortcut" operations in the public API that allow developers to optimize for common cases and reduce the amount of boilerplate.

* We will expose a C API that wraps the COM using `inline` functions. We will attempt to make the C API idiomatic when/as possible.

* We can eventually/graduatelly provide a set of C++ wrapper/utility types that can further streamline the experience of using Slang, by hiding some of the details of COM reference counting, and "desc" structs". The utility code could also translate COM-style result codes into C++ exceptions, if we find that this is desired by some users.

Detailed Explanation
--------------------

At the end of this document there is a lengthy code block that sketches a possible outline for what the `slang.h` header *could* look like.

Questions
---------

### Will we generate all or some of the API header? If so, what will be the "ground truth" verison?

Note that Vulkan and SPIR-V benefit from having ground-truth computer-readable definitions, allowing both header files and tooling code to be generated.

### Can we actually make a reasonably idiomatic C API that wraps a COM one, or should we admit defeat and have everything look like `slang_<InterfaceName>_<methodName>(...)`?

Alternatives Considered
-----------------------

We haven't seriously considered many alternatives in detail, other than the possibility of a plain C API (which we have tried, but not been able to make work).

Appendix: A Header of Notes
---------------------------

The following code represents an sketch of a header that tries to match this proposal (and includes a lot of its own discussion/comentary).

```
/* Slang API Header (Proposed)

This file is an attempt to sketch how the API headers for both
Slang and gfx could be organized in order to provide a nice
experience for developers who belong to different camps in terms
of what they want to see.

Goals:

* Support both C and C++ access to the API, with matching types, etc.

* Able to use COM interfaces including use of inheritance/polymorphism

* When compiling as C++, it should be possible to mix-and-match both C and C++ APIs

Constraints:


Questions:

* Do we actually need to restrict to block comments for pedantic compatibility
  with old C versions? Are line comments close enough to universally-supported?

*/

#ifdef __cplusplus

/* Because of our goals above, we will actually end up with what amounts to
two copies of the C API, depend on whether or not we are compiling as C++.

We start with the C++ COM-lite API, since that is the baseline. Note that
in this proposal, everything is being defined in the `slang` namespace,
rather than first declaring many things as C types and then mapping that
over to C++.
*/

namespace slang
{
    /* Note that in this proposal, everything is being declared in the `slang`
    namespace first, rather than the old model of declaring various things
    in C and then importing them into the namespace.
    */

/* Basic Types */

    typedef int32_t Int32;
    /* typedefs for basic types, as needed ... */


/* Enumerations and Constants */
 
    /* Non-flag enumerations will use `enum class`. If we need to support clients
    using older C++ versions/compilers, we can discuss macro-based ways to try
    to work around this.

    Except in cases where there is an *extremely* compelling reason to do something
    different, all enumerations use the `int` tag type that is the default for
    `enum class`.
    */

 
    /** Severity of a diagnostic generated by the compiler.
        ...
    */
    enum class Severity
    {
        Note, /**< An informative message. */
        /* ... */
    };

    /* TODO: We need a clearly-defined policy for how to handle "flags" enumerations.

    My strong opinion is that we should generally avoid flags in public API just
    because of the tendency to run out of bits sooner or later, but I also understand
    the appeal...
    */

    struct SlangTargetFlag
    {
        enum : UInt32
        {
            DumpIR = 1 << 0,
            /* ... */
        };
    };
    typedef UInt32 SlangTargetFlags;

    /* I'm note sure about whether the `Result` type ought to be declared with `enum class`.
    It would be nice to have the extra level of type safety, but it would also make our
    `Result` incompatible with macros and template types that are intended to work with
    `HRESULT`s.
    */
    enum Result : Int32
    {
        /* I *do*  think we should go ahead and define all the cases of `Result`s
        that we expect our API to traffic in right here in the `enum`, so that users
        can easily inspect result codes in the debugger.
        */

        OK = 0,
        /* ... */
    };

/* Forward Declarations */

    /* Simple Types */
    struct UUID;
    /* ... */

    /* "Desc" Types */
    struct SessionDesc;
    /* ... */

    /* Interface Types */
    struct ISession;


/* Structure Types */

    /* Theres's not much to say for the easy case... */

    struct UUID
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t  data4[8];
    };
    /* ... */

    /* The more interesting bit is "descriptor" sorts of structures, which
    we've done a lot of back-and-forth on how best to support.

    I'm going to write up something here while also acknowledging that picking
    a good policy for how to handle this stuff is an orthogonal design choice.
    */

    enum class DescType : UInt32
    {
        None,
        SessionDesc,
        /* ... */
    };
    enum class DescTag : UInt64;

    #define SLANG_MAKE_DESC_TAG(TYPE) DescTag(UInt64(slang::DescType::TYPE) << 32 | sizeof(slang::##TYPE))

    struct SessionDesc
    {
        DescTag tag = SLANG_MAKE_DESC_TAG(SessionDesc);

        TargetDesc const*   targets = nullptr;
        Int                 targetCount = 0;
        /* ... */

        /* Note: There is some subtlety here if we use default member
        initializers here, but also want to expose these types directly
        via the C API (in cases where somebody is using the C API but
        a C++ compiler).

        The tag approach here is intended to support something akin to
        the Vulkan style, when using the C API:

            SlangSessionDesc sessionDesc = { SLANG_DESC_TAG_SESSION_DESC };
            ...

        That code will not compile under C++11, because of the default
        members initializers in `slang::SessionDesc`, but it *will* compile
        under C++14 and later.

        If we want to deal with C++11 compatiblity in that case, we can, but
        it would slightly clutter up the way we declare these things. Realistically,
        we'd just split the two types:

            struct _SessionDesc { ... data but no initialization ... };
            struct SessionDec : _SessionDesc { ... put a default constructor here ... };

        I'm not a fan of that option if we can avoid it.
        */
    };

    /* ... */

    /* *After* all the "desc" types are defined, we can actually define the enum
    for their tags (just to make life easier for users looking at things in their
    debugger.
    */
    enum class DescTag : UInt64
    {
        None = 0,
        SessionDesc = SLANG_MAKE_DESC_TAG(SessionDesc),
        /* ... */
    };

    /* Versioning: If we are in a situation where we'd like to change a type that
    has already been tagged, we should first consider just creating an additional
    "extension" desc type, to be used together with the original. By adding
    suitable convenience APIs, we can make this easy to work with.

    If we really do decide that we want a new version of a specific desc, we should
    start by doing the thing D3D does, and make a new numbered type:

        struct SessionDesc { ... the original ... };
        struct SessionDesc1 { ... the new one ... };
    
    When possible, the new type should use matching field names/types and ordering.
    Even if we are just adding fields, we should not try using inheritance (just
    because the C++ spec doesn't guarantee enough about how inheritance is implemented).

    The new structure types should get its own desc type/tag, distinct from the original.

    If we decide that we want clients to compile against the latest version of these
    types by default, we can shuffle around the names, but we need to be careful to
    *also* shuffle the `DescType` cases (so that the binary values stay the same):

        struct SessionDesc0 { ... the original ... };
        struct SessionDesc1 { ... the new one ... };
        typedef SessionDesc SesssionDesc1;

    At the point where we introduce a second version, it is probably the right time
    to enable developers to lock in to any version they choose. In the code above
    the user can always just use `SessionDesc0` or `SessionDesc1` explicitly, or they
    can just stick with `SessionDesc` in the case where they always want the latest
    at the point they compile.

    (If we wanted to get really "future-proof" we'd define every struct with the `0`
    prefix right out of the gate, and always have the `typedef` in place. I'm not conviced
    that would ever pay off.)
    
    I expect most of this to be a non-issue if we are zealous about using fine-grained
    rather than catch-all descriptors at this level of the API.

    (There's more I could talk about here, but this isn't supposed to be the topic at hand)
    */


/* Interfaces */

    /* There's an open question of how to name the `IUnknown` equivalent
    once things are all namespaced. We could use `slang::IUnknown`, but I fear
    that could lead to complications or confusion for codebases that also use
    MS-provided COM-ish APIs.
    */

    struct ISession : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(...);

        /* In order to maximize our ability to evolve the API while maintaining
        binary compatibility, I'm going to recommend the somewhat bold step
        of defaulting to making interface entry points that are "implementation
        details" rather than intended for direct use in most cases.
        */

        virtual SLANG_NO_THROW Result SLANG_MCALL _createCompileRequest(
            void const* const*  descs,
            Int                 descCount,
            UUID const&         uuid,
            void**              outObject) = 0;

        /* Instead, most users will direclty call the operations only through
        wrappers that provide conveniently type-safe behavior:
        */
        inline Result createCompileRequest(
            CompileRequestDesc const&   desc,
            ICompileRequest**           outCompileRequest)
        {
            return _createCompileRequest(
                &desc, 1, SLANG_UUID_OF(ICompileRequest),
                (void**)outCompileRequest);
        }

        /* An important property of this design is that we can easily define
        convenience overloads that take direct parameters for common cases:
        */
        inline Result createCompileRequestForPath(
            const char*         path,
            ICompileRequest**   outCompileRequest)
        {
            ...;
        }

        /* Versioning: Note that we can easily define convenience overloads
        for multiple versions of descriptor types (`CompileRequest` and
        `CompileRequest`), or for *combinations* of descriptor types:
        */
        inline Result createCompileRequest(
            CompileRequestDesc const&   requestDesc,
            ExtraFeatureDesc const*&    extraFeatureDesc,
            ICompileRequest**           outCompileRequest)
        {
            void* descs[] = { &requestDec, &extraFeatureDesc };
            return _createCompileRequest
                descs, 2, SLANG_UUID_OF(ICompileRequest),
                (void**)outCompileRequest);
        }

        /* As a final detail, we should consider whether to support overloads
        that work with either our `slang::ComPtr<T>` *or* an application's own
        smart pointer type.

        The user could override the smart pointer type by defining macros.
        The defaults would be:

        #define SLANG_SMART_PTR(TYPE) slang::ComPtr<TYPE>
        #define SLANG_SMART_PTR_WRITE_REF(PTR) ((PTR).writeRef())
        */
#ifndef SLANG_DISABLE_SMART_POINTER_OVERLOADS
        inline Result createCompileRequest(
            CompileRequestDesc const&           desc,
            SLANG_SMART_PTR(ICompileRequest)*   outCompileRequest)
        {
            return _createCompileRequest(
                &desc, 1, SLANG_UUID_OF(ICompileRequest),
                SLANG_SMART_PTR_WRITE_REF(*outCompileRequest));
        )
#endif

        /* If we ever have cases where we want to support utility/wrapper operations
        of higher complexity than what we feel comfortbale making `inline` (that is,
        stuff that might be best off in a `slang-utils` static library) we could conceivably
        handle those via judicious use of `extern`:
        */
        inline Result createCompileRequestFromJSON(
            char const* jsonBlob, //< a serialized form of the compilation state
            ICompileRequest** outCompileRequest,
        {
            extern Result slang_ISession_createCompileRequestFromJSON(
                char const*, ICompileRequest**);
            return slang_ISession_createCompileRequestFromJSON(jsonBlob, outCompileRequest);
        }
        /* I doubt we'd ever really need that kind of approach, and would always decide
        that functionality either belongs in core Slang (perhaps as a new derived interface)
        or can go as global (non-member) functions in a utility library.
        */

        /* ... */
    }

    /* Note: I'm assuming here that we continue our implicit contract in terms
    of versioning of COM interfaces:

    * Every interface is thought of as having a contract about who can *provide*
        and who can *consume* it. For many (like `ISession`) only the Slang implementation
        is supposed to provide it and only users consume it. Some callback interfaces
        go the other way, and a few (like `slang::IBlob`) need to go both directions.

    * For interfaces that Slang provides and the user consumes, we can append new
        `virtual` methods onto the end. This realistically needs a check somewhere, such
        that we fail creation of the `IGlobalSession` if the user compiled against a
        header that exposes the new method but is linking a DLL that doesn't. This is
        what the `SLANG_API_VERSION` in the original header is supposed to be for: we
        should increment it when we expand the API contract, and the global-session
        creation should fail if the application is asking for too new of a version.
    
    * For interfaces that Slang consumes, we cannot realistically add/remove anything.
        Theoretically, we could delete some of the `virtual` methods if we no longer
        expect to call them, but that could still break client code that uses `override`
        on their definitions.
    
    * We need to try very hard not to change the interface types of parameters to
        non-wrapper COM methods, even if the result should be binary-compatible. There
        are cases where it might be reasonable and "type-safe," but each and every one
        probably needs clear auditing.

    Ideally the rules we use for Slang-provided interfaces can help us avoid the
    proliferation of `IThing`, `IThing2`, `IThing3`, etc. We need to be careful about
    that in the long run, though, because we may find that it causes problems in the wild
    if software needs to interact with Slang in a context where the developer cannot
    control the version of the Slang DLL they will be using at runtime.

    In theory, we could solve that problem by letting a user pass `SLANG_API_VERSION`
    *in* to the header via a `#define`, and have us skip over any declarations introduced
    after the given version.
    */
}

/* Back outside the namespace (but still in the case where we know C++ is
supported), we can define the C-compatible API by using the C++ API
as its underlying representation.
*/

extern "C"
{

/* Basic Types */

    /* The C-API types can just be `typedef`s of the C++ ones */

    typedef slang::Int32 SlangInt32;
    /* ... */

/* Enumerations and Constants */
 
    /* For the case where we *know* a C++ compiler is being used, we
    can actually use the C++ `enum class` declarations to provide
    the enumerated types of the C API.
    */
    typedef slang::Severity SlangSeverity;

    /* We can use macros to define the C-API enum cases, while still
    preserving the type safety of the `enum class` approach.

    Note: We could also use `static const`s here, but that seems like
    overkill.
    */
    #define SLANG_SEVERITY_NONE (slang::Severity::None)
    /* ... */

/* Structure Types */

    /* For the case of providing the C API for a C++ compiler, we can
    directly use the C++ structure types in all cases. */

    typedef slang::UUID SlangUUID;
    typedef slang::SessionDesc SlangSessionDesc;
    /* ... */

/* Interfaces */

    /* Because we are compiling as C++, defining the types for the interfaces
    is easy, and we can easily pass objects between modules/files that are
    using the two version of the API without any casting:
    */
    typedef slang::ISession SlangSession;
    /* ... */

    /* In order to have a plain-C API, the user of course needs a way to
    dispatch into those interfaces.

    Note: There is a big question here of whether the API header should be
    trying to define the C API functions `inline` here or not.

    The argument for using `inline` is that it doesn't add any additional
    requirements for somebody using the C API from within C/C++, compared to
    the C++ API.

    The argument *against* is that for things like binding to other languages,
    the user would probably prefer that these operations have linkage.

    Realistically, the right thing is for the header to include both declarations
    *and* definitions, but to allow the application to conditionalize the inclusion
    of the definitions *and* enable/disable the use of `inline` for declarations/definitions.
    A user could use that control to compile their own linkable stub with C-compatible
    functions.
    */

    /* We need to provide the fully-general version of the function, for clients
    that might need it, but we probably don't want that to be the first one users
    reach for.
    */
    inline SlangResult _SlangSession_createCompileRequest(
        SlangSession*       session,
        void const* const*  descs,
        SlangInt            descCount,
        SlangUUID const*    uuid,
        void**              outObject)
    {
        return session->_createCompileRequest(descs, descCount, uuid, outObject);
    }

    /* The catch here is that the C++ API used overloading as a way
    to provide convenient wrappers around the fully-general core operations,
    and also to provide versioning support.

    We could define the same set of overloads here, with the same names, for
    use by clients who don't actually care about C compatiblity but just like
    a C-style API. That is probably worth doing.

    Otherwise, we realistically need to start defining some de-facto naming
    scheme and/or versioning for stuff in the C API. At least one wrapper
    should be "blessed" as the default one.
    */
    inline SlangResult SlangSession_createCompileRequest(
        SlangCompileRequestDesc const*  desc,
        SlangCompileRequest**           outCompileRequest)
    {
        return session->_createCompileRequest(*desc, outCompileRequest);
    }

    /* Note that we need/want to provide wrappers for *all* the operations
    on each interface, even the ones they inherit. E.g.:*/
    inline uint32_t SlangSession_addRef(
        SlangSession* session)
    { ... }
    /* The reason for this is so that a pure-C user doesn't *have* to rely on
    implicit conversion of these types to their bases (which in this path is
    made possible via C++ features, but wouldn't be available in a true pure-C
    world).
    */

    /* If/when we start to deal with versioning of either the "desc" type or
    the interface involved in such an operation, we will need to do the numeric-suffix
    thing or similar stuff to distinguish the old and new functions.

    We can probably do some work to always make the latest version (or at least
    the one we want users to be using) have the short/clean name. Binary compatibility
    shouldn't actually break so long as the signature of the new function can technically
    handle calls of the old form (since the COM-level bottleneck function won't care about
    the static types of descs - just their tags).
    */

    /* Finally, the C API level is where we should define the core factory entry
    point for creating and initializing the Slang global session (just like
    in the current header). Here we jsut generalize it for creaitng "any" global
    object, based on a UUID and a bunch of descs.
    */
    SLANG_API SlangResult slang_createObject(
        void const* const*  descs,
        Int                 descCount,
        UUID const*         uuid,
        void**              outObject);

    /* The actual global session creation is then a wrapper like everything else.
    */
    inline SlangResult SlangGlobalSession_create(
        SlangGlobalSessionDesc const*   desc,
        SlangGlobalSession**            outGlobalSession)
    {
        return slang_createObject(
            &dec, 1, SLANG_UUID_OF(slang::IGlobalSession), (void**)outGlobalSession);
    }
}

#else

/* All of the above declarations (even the C-level ones) only work if we are
compiling as C++. Thus we need a distinct strategy to define everything in the
case where we are compiling as pure C.

The basic strategy isn't that hard: we just do things the raw C way.
There will be a lot of repetition involved, but this proposal assumes we are
generating as much of the API as possible anyway.
*/

/* Basic Types */

    /* We just define the basic types direclty, without the indirection
    through the declarations in the `slang::` namespace.
    */

    typedef int32_t SlangInt32;
    /* ... */

/* Enumerations and Constants */
 
    /* Every enum in this case is a `typedef` plus an actual `enum`:
    */

    typedef int SlangSeverity;
    enum
    {
        SLANG_SEVERITY_NONE = 0,
        /* ... */
    };

    /* ... */

/* Structure Types */

    /* The simple case stays simple, just with the gross bit of
    duplicating a *lot* of what we already had in the C++ API.
    
    (There's a big design question here of whether we can/should try
    to remove as much duplication as possible in order to reduce
    boilerplate, even if it comes at the cost of clarity because of
    heavier reliance on macros, etc.)
    */

    struct SlangUUID
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t  data4[8];
    };
    /* ... */

    /* The desc-related stuff is really just a translation of the
    same basic ideas to plain C: */

    typedef SlangUInt32 SlangDescType;
    enum
    {
        SLANG_DESC_TYPE_NONE = 0,
        SLANG_DESC_TYPE_SessionDesc,
        /* ... */
    };
    typedf SlangUInt64 SlangDescTag;

    #define SLANG_MAKE_DESC_TAG(TYPE) SlangDescTag(UInt64(SLANG_DESC_TYPE_##TYPE) << 32 | sizeof(Slang##TYPE))

    struct SlangSessionDesc
    {
        SlangDescTag tag;

        SlangTargetDesc const*   targets;
        SlangInt                 targetCount;
        /* ... */
    };
    /* ... */

    #define SLANG_DESC_TAG_SESSION_DESC SLANG_MAKE_DESC_TAG(SessionDesc)
    /* ... */

/* Forward Declarations */

    typedef struct SlangSession SlangSession;
    /* ... */

/* Interfaces */

    /* There's already a lot known about how to define COM interfaces for
    consumption from C, so this is actually mostly straightforward.

    Note: these definitions would *only* be needed in the case where we
    are compiling the actual implementations of the C API functions. It
    is possible that we can/should just not bother with these, under
    the assumption that anybody who wants a true pure-C API probably wants
    a linkable "stub" library anyway, in which case we can provide that
    library ourselves, and compile it as C++.
    */

    /* TODO: The big thing I'm skipping here is setup for the UUIDs.
    I think we can provide C-compatible macros for those pretty easily,
    but exactly what that should look like is maybe more complicated. */

    struct SlangSession
    {
        /* The long/short is that we define a pointer field to a struct
        of function pointers, which matches the expected C++ virtual
        function table layout.
        */

        struct
        {
            /* Note: methods from all base interfaces need to go here... */

            SLANG_NO_THROW SlangResult SLANG_MCALL (*_createCompileRequest)(
                    SlangSession*       session,
                    void const* const*  descs,
                    SlangInt            descCount,
                    SlangUUID const*    uuid,
                    void**              outObject);

            /* ... */

        } * vtbl;
    };
    /* ... */

    /* With the core type declarations out of the way, the actual functions
    that forward to it are easy enough:
    */
    inline SlangResult _SlangSession_createCompileRequest(
        SlangSession*       session,
        void const* const*  descs,
        SlangInt            descCount,
        SlangUUID const*    uuid,
        void**              outObject)
    {
        /* The only interesting complications here are the `->vtbl`
        and the need to pass `session` explicitly. We could probably
        macro away the difference if we don't want to have distinct
        C-API-compiled-via-C++ and C-API-compiled-via-C cases.
        */
        return session->vtbl->_createCompileRequest(
            session, descs, descCount, uuid, outObject);
    }

    /* The declarations of the global session creation stuff are almost
    identical, so there's no real need to dpulicate it here.
    */

    /* For the true pure-C users, we probably want to provide convenience
    functions and/or macros to enable the casts that should be statically
    possible.*/
    inline SlangUnknown* SlangSession_asUnknown(SlangSession* session)
    {
        return (SlangUnknown*) session;
    }
    /* ... */


/*

Okay, so that's the basic idea of the proposal for how to expose our API(s).

I realize this didn't get into the actual details of type hierarchies or what
the actual "desc" types need to be for Slang and gfx. The focus here was much
more on the syntactic side of things, in terms of how we can define our API
so that both C and C++ are usable and can be freely intermixed within a codebase.

*/

/* There's probably an entire additional document that could be written about
utility/wrapper stuff to make the interfaces nicer for C++ users. Some examples
follow:

We could consider having a hierarchy of wrapper smart-pointer types that codify the
reference-counting policies without the user having to really think about `ComPtr<T>` stuff:

    struct Unknown
    {
    public:
        // typical stuff...


    protected:
        slang::IUnknown* _ptr = nullptr;
    }

    struct Session : Unknown
    {
    public:
        ISession* get() const { return (ISession*)_ptr; }    
        operator ISession*() const { return get(); }    

        Result createCompileRequest(
            CompileRequestDesc const& desc,
            CompileRequest* outCompileRequest)
        { ... }
    }

Another thing to consider is whether any of our COM-ish wrappers should allow for
use of exceptions instead of `Result`s:

    struct ISession : ...
    {
        ...

#if SLANG_ENABLE_SMART_PTR
        ...

    #if SLANG_ENABLE_EXCEPTIONS
        SLANG_SMART_PTR(ICompileRequest) createCompileRequest(
            CompileRequestDesc const& desc)
        {
            SLANG_SMART_PTR(ICompileReqest) compileRequest;
            SLANG_THROW_IF_FAIL(_createCompileRequest(
                &desc, 1, SLANG_UUID_OF(IComileRequest), comileRequest.writeRef()));
            return compileRequest;
        }

        ...
    #endif
#endif
    }

Both for the sake of C API and especialy for gfx (both C and C++), we should consider
defining some coarse-grained aggregate desc types as utilities:

    struct SimpleRasterizationPipelineStateDesc
    {
        // sub-descs for all the relevant pieces:
        //
        PipelineProgramDec program;
        DepthStencilDesc depthStencil;
        MultisampleDesc multisample;
        PrimitiveTopologyDesc primitiveTopology;
        NVAPIDesc nvapi;
        // ...

        // "fluent"-style setters for all the relevant pieces:

        SimpleRasterizationPipelineStateDesc& setEnableDepthTest(bool value)
        {
            markDepthStencilDescUsed();
            depthStencil.enableDepthTest = value;
            return *this;
        }

        // ...

        // This is also the logical granularity to provide things like
        // List<T> members for attachments, etc. rather than just pointer-and-count:

        private: List<AttachmentDesc> colorAttachments;
        public: AttachmentDesc& addColorAttachement();

        // There should also be convenience constructors common cases
        // (especially relevant for things like textures).

        // In the simplest implementation strategy, we keep a bitmask for which
        // of the sub-descs have actually beem used (either requested by the user,
        // or set to non-default values):
        //
        enum class SubDesc { Program, DepthStencil, ... Count };
        uint32_t usedSubDescs = 0;
        void markSubDescUsed(SubDesc d)
        {
            uint32_t bit = 1 << int(d);
            if(usedSubDesc & bit) return;

            usedSubDescs |= bit;
            updatePointers();
        }

        // We then maintain a compacted array of all the sub-descriptors needed
        // to form the combined state for passing along to the lower-level API.
        //
        void* subDescs[int(SubDesc::Count)];
        int subDescCount = 0;

        void updatePointers()
        {
            subDescCount = 0;
            if(usedSubDescs & (1 << int(Program)))
            {
                subDescs[subDescCount++] = &program;
            }
            /// ...
        }
    };

While the implementation of this monolithic desc types would not necessarily be pretty,
it would enable users who want the benefits of the "one big struct" appraoch to get
what they seem to want.

The next step down this road is to take these aggregate desc types and turn them into
actual API objects for the purposes of the C API, so that users can more conveniently
create stuff:

    GFXRasterizationPipelineStateBuilder* GFXDevice_beginCreateRasterizationPipelineState(
        GFXDevice* device);

    void GFXRasterizationPipelineStateBuilder_setEnableDepthTest(
        GFXRasterizationPipelineStateBuilder*   builder,
        bool                                    enable);

    // Note: frees the given `builder`, so user doesn't have to do it manually
    GFXPipelineState* GFXRasterizationPipelineStateBuilder_create(
        GFXRasterizationPipelineStateBuilder*   builder);

Obviously the function names are very verbose there, but they could probably be cleaned
up a lot if we want to go down this route. Certainly, if we decide that C API users are
not going to be inclined to use a lot of fine-grained descs, this starts to seem like
an increasingly attractive way to go.
*/

#endif
```