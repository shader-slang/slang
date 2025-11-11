// slang-serialize-ast.cpp
#include "slang-serialize-ast.h"

#include "core/slang-performance-profiler.h"
#include "slang-ast-dispatch.h"
#include "slang-check.h"
#include "slang-compiler.h"
#include "slang-diagnostics.h"
#include "slang-mangle.h"
#include "slang-parser.h"
#include "slang-serialize-fossil.h"
#include "slang-serialize-riff.h"

//
#include "slang-serialize-ast.cpp.fiddle"

// By default, the declarations in a serialized AST module will be
// deserialized on-demand, in order to improve startup times.
//
// The on-demand loading logic understandably introduces more
// complexity, and it is possible that there will be debugging
// (or even deployment reasons) scenarios where it is desirable
// to be sure that all the AST nodes for a given module are
// fully deserialized by the time `readSerailizedModuleAST()`
// returns. For those cases, we provide a macro that can be
// used to force up-front loading.
//
// Note: this macro does *not* disable most of the infrastructure
// code related to on-demand loading; things like lookup on
// a `ContainerDecl` will still check for the on-demand loading
// case at runtime. All that setting this flag to `1` does is
// extend the "fixup" logic that runs when an AST node has been
// deserialized to also force deserialization of any direct
// member declarations of a `ContainerDecl` that has just been
// deserialized.
//
// The macro is being defined conditionally here, so that we
// have the option of introducing an option to control its
// value as part of configuration for the build of the compiler
// itself (if that ever becomes relevant).
//
#ifndef SLANG_DISABLE_ON_DEMAND_AST_DESERIALIZATION
#define SLANG_DISABLE_ON_DEMAND_AST_DESERIALIZATION 0
#endif

// In the case where on-demand deserialization is enabled, it
// can be helpful to know what fraction of the declarations
// from any given module end up getting deserialized (e.g.,
// at the time this comment was written, compiling a small
// `.slang` file typically causes about 17-20% of the
// top-level declarations in the core module to get deserialized.
//
// Enabling this flag causes a message to be emitted every
// time a new top-level declaration gets deserialized for *any*
// module, so it generates a lot of output and is best seen
// as just a debugging option for use when trying to reduce
// the fraction of declarations that must be deserialized.
//
#define SLANG_ENABLE_AST_DESERIALIZATION_STATS 0

FIDDLE()
namespace Slang
{

//
// The big picture here is that we will serialize all of the structures
// that make up the AST using the framework in `slang-serialize.h`,
// and the specific *implementation* of serialization from `slang-fossil.h`.
//
// There's a certain amount of work that needs to be done on a per-type basis
// to make all of the serialization magic work the way we want. In order to
// help illustrate what's going on before we grind through all the different
// types, we will start slow and define the needed pieces for a somewhat
// trivial type: `RefObject`.
//

//
// For the general-purpose serialization framework in `slang-serialize.h`, the
// main requirement is that any type that we want to serialize should have an
// available overload of `serialize()`.
//
//
// In principle, the declarations and definitions of these functions ought to
// be more closely associated with the types that they pertain to, but for now
// they are all just getting dumped here in the AST serialization logic, because
// it is currenly the only place that cares about this stuff.
//
template<typename S>
void serialize(S const&, RefObject&)
{
    // There's actually no data stored in a `RefObject`, since it only exists
    // to make reference-counting possible for other types. This function is
    // primarily useful for cases where we might codegen logic to serialize
    // a type by serializing its base class (if it has one) and then its fields.
    // If the base class is `RefObject`, we want there to be an available
    // overload of `serialize()` to handle that case.
}

//
// In addition to using the general-purpose serialization system, we are
// specifically encoding the AST using the "fossil" format defined in `slang-fossil.h`.
// This format allows us to load the serialized data into memory and easily
// navigate it without having to deserialize any of its content.
//
// There are really two modes in which fossilized data can be navigated:
//
// * As a dynamically-typed graph of nodes, where a reference to a node
//   comprises a data pointer and a layout pointer, with the layout
//   describing the type and format of the data.
//
// * As a statically-typed data structure, where code can just cast a
//   pointer to fossilized data to the type that it knows/expects it to
//   have, and then access it like Just Another C++ Type.
//
// In order to enable the second of these modes, we need to do a little
// work to define the mapping from a "live" C++ type to its fossilized
// equivalent.
//
// In cases where a live type can use an existing fossilized type as
// its representation, we can specialize the `FossilizedTypeTraits` template:
//

template<>
struct FossilizedTypeTraits<RefObject>
{
    struct FossilizedType
    {
    };
};

//
// The handling of `RefObject` was trivial, so let's cover a few more
// simple examples in detail before we move on to the rest of the things,
// where we will be using fiddle to generate a lot of the boilerplate
// code we'd otherwise be writing by hand.
//
// The `MatrixCoord` type is a fairly simple `struct` with two fields.
// While we could include this among the types we handle using fiddle,
// let's implement it by hand here, starting with the `serialize()` function:
//
template<typename S>
void serialize(S const& serializer, MatrixCoord& value)
{
    // We start with one of the `SLANG_SCOPED_SERIALIZER_*`
    // macros, which basically just handles calling
    // `ISerializerImpl::beginTuple()` and the start of our
    // scope, and `ISerializer::endTuple()` at the end.
    //
    SLANG_SCOPED_SERIALIZER_TUPLE(serializer);

    // Next, we call `serialize()` on each of the fields
    // of our type, in order. Ordinary overload resolution
    // will pick the right function to call based on the type
    // of the field itself.
    //
    serialize(serializer, value.row);
    serialize(serializer, value.col);

    // Note: this one function handles both the read and write
    // directions. For simple types like `MatrixCoord` the logic
    // for reading and writing is symettric, and writing it as
    // one function ensures that the two paths are kept in sync.
    //
    // Some of the later examples will perform different logic
    // in the read and write cases, and all of those require
    // more conscious effort by contributors to keep the two
    // paths matching.
}

//
// Writing the `serialize()` function is one piece of the picture, but
// if we want to be able to navigate a fossilized `MatrixCoord` in
// memory, we need to declare what it's fossilized version will look like.
//
// We do that here by writing an explicit specialization of `FossilizedTypeTraits`:
//
template<>
struct FossilizedTypeTraits<MatrixCoord>
{
    // The `MatrixCoord` type can't map directly to any type
    // for fossilized data, so we declare it here as a custom
    // `struct`.
    //
    struct FossilizedType
    {
        // The contents of a fossilized struct will typically
        // just be the fossilized representation of each of
        // its fields.
        //
        // We use `decltype()` to access the type of each of
        // the fields, and `Fossilized<...>` to map those
        // types to their fossilized equivalents.
        //
        // Note that this type definition must be consistent
        // with the implementation of `serialize()` above,
        // so it is important to keep the two consistent.
        // That requirement of consistency is part of why
        // it helps to generate these definitions rather
        // than author them by hand.
        //
        Fossilized<decltype(MatrixCoord::row)> row;
        Fossilized<decltype(MatrixCoord::col)> col;
    };
};

//
// In some cases we don't really want to serialize a type directly,
// and instead want to translate it to some intermediate format
// that can be serialized more conveniently.
//
// For example, the `SemanticVersion` type conceptually has multiple
// fields, but it is also designed so that it can be encoded conveniently
// as a single scalar value. We'll define our `serialize()` function
// so that it serializes that "raw" value instead:
//
template<typename S>
void serialize(S const& serializer, SemanticVersion& value)
{
    // This function is doing something a little "clever"
    // handle the fact that it might be used to either
    // *write* a `SemanticVersion` to the serialized format,
    // or to *read* one.
    //
    // In the case where we are writing, the following line
    // will copy the `value` we want to write into the
    // local variable `raw`, but if we are *reading* instead,
    // this operation doesn't so anything useful.
    //
    // The assumption being made here is that it is safe to
    // call `getRawValue()` on any `SemanticVersion`, including
    // one that has been default-constructed, because we have
    // no guarantee that the incoming `value` represents anything
    // useful or even *valid* in the case where we are reading
    // (and thus expected to overwrite `value`).
    //
    SemanticVersion::RawValue raw = value.getRawValue();

    // Depending on whether we are reading or writing, this next
    // line will either write out the value of `raw` that was
    // computed above, or it will read serialized data into `raw`,
    // and overwrite the useless value from before.
    //
    serialize(serializer, raw);

    // Finally, we overwrite the `value` by converting `raw`
    // back to a `SemanticVersion`. If we are in reading mode,
    // this will do exactly what the caller wants/expects.
    // If we are in *writing* mode, this line makes a few more
    // subtle assumptions:
    //
    // * It assumes that we can safely round-trip any `SemanticVersion`
    //   through its `RawValue` without changing its meaning.
    //
    // * It assumes that the passed-in `value` will never be a
    //   reference to read-only memory, and that in the case where
    //   there are other concurrent accesses to `value`, this write
    //   will not somehow create a difficult-to-debug data hazard
    //   (e.g., there might be an overload of `operator=` that
    //   temporarily sets the object into a state that shouldn't be
    //   obeserved).
    //
    value = SemanticVersion::fromRaw(raw);

    // In cases where a given type doesn't satsify all the assumptions
    // being made above, it is relatively simple to just split the
    // logic into distinct cases based on `isReading(serializer)` and
    // avoid all the concerns. That conditional involves a virtual
    // function call, so in cases where it can easily be avoided,
    // we prefer to do the redundant copy-in and copy-out on a local
    // variable, like in the code above.
}

//
// Given the definition of `serialize()` above, it is clear that the
// fossilized representation of `SemanticVersion` would be the same
// as whatever the fossilized representation of `SemanticVersion::RawValue`
// would be.
//
// The fossil header provides some convenient macros for defining that
// one type gets fossilized as another:
//
SLANG_DECLARE_FOSSILIZED_AS(SemanticVersion, SemanticVersion::RawValue);

//
// While in some cases we want to serialize something via an intermediate
// type that already exists (like for `SemanticVersion` and
// `SemanticVersion::RawValue` above), in other cases we need to *define*
// an intermediate type to store the data we care about in a more
// direct fashion.
//
// When serializing an AST `ModuleDecl`, there are certain pieces
// of data that are implicitly encoded in the object graph under
// that module declaration that are beneficial to make explicit
// in the serialized representation.
//
// As a concrete example, there are various declarations in the Slang
// core module that have to be "registered" with the `SharedASTBuilder`
// being used, so that they can be looked up by a well-defined tag
// (whether an integer or string) by other logic in the compiler.
// Those declarations can be found by doing a recursive search over
// the entire `Decl` hierarchy of a module, but doing such a recursive
// search would force us to load and inspect every single declaration
// in a module as part of deserialization, which would negate any
// possible benefits to supporting on-demand deserialization of those
// declarations.
//
// Thus, we define an intermediate `ASTModuleInfo` type that holds
// the pre-computed information that we want to serialize (and thus
// also represents the data that we will want to navigate in the
// serialized representation).
//
FIDDLE()
struct ASTModuleInfo
{
    FIDDLE(...)

    // We still want to serialize the original module declaration,
    // and everything it transitvely refers to.
    //
    FIDDLE() ModuleDecl* moduleDecl;

    // The intermediate type will store an explicit list of all of
    // the declarations that we need to register upon loading
    // this module (this list is expected to be empty for everything
    // other than the core module).
    //
    FIDDLE() List<Decl*> declsToRegister;

    // Another example of data that we want to store explicitly
    // rather than leave implicit in the declaration hierarchy is
    // the set of declarations exported from the module, and their
    // mangled names.
    //
    FIDDLE() OrderedDictionary<String, Decl*> mapMangledNameToDecl;
};

//
// Another case where we wnat to define an intermediate type is
// the `ContainerDeclDirectMembers` type used to encapsulate
// the list of direct members for a `ContainerDecl` along with
// the acceleration structures used to enable efficient lookup
// of those declarations.
//
FIDDLE()
struct ContainerDeclDirectMemberDeclsInfo
{
    FIDDLE(...)

    // We need to store the ordered list of declarations,
    // because many parts of the compiler need to access
    // all of the direct members of a container, and the
    // order of the direct members often matters (e.g.,
    // for layout).
    //
    FIDDLE() List<Decl*> decls;

    // One of the acceleration structures that a `ContainerDecl`
    // may build and store is a list of those entries in
    // `decls` that are marked as "transparent." This is
    // not a commonly-occuring case, used only to support
    // a few legacy features, so it is a bit wasteful to
    // store such a list on *every* container decl in the
    // serialized format, but the format itself isn't
    // currently optimized for size, so we consider this
    // fine for now.
    //
    FIDDLE() List<FossilUInt> transparentDeclIndices;


    // The other main acceleration structure that a `ContainerDecl`
    // may build and store is a dictionary to map a string name to
    // a declaration of that name (which is then the first node
    // in an internally-linked list of *all* the declarations with
    // the given name).
    //
    FIDDLE() OrderedDictionary<String, FossilUInt> mapNameToDeclIndex;
};

//
// Many types in the AST need additional context information to be able to
// read or write them properly, so the concrete serializer type being passed
// around will include an additional "context" type, that will be either
// `ASTSerialReadContext` or `ASTSerialWriteContext`, depending on the mode
// in which serialization is being performed.
//
// We could define a base class or interface with `virtual` functions for
// accessing all of the relevant context, but because we are already
// using template specialization, it is easier to just ensure that the
// relevant context types both provide the required operations.
//

//
// Now that we've covered some of the big-picture structure, and shown
// a few small examples, we will try to use fiddle to generate the code
// to handle as many of the remaining types as we can.
//
// TODO: It would be great to have more of this logic be driven by information
// that the fiddle tool scraped from the relevant declarations.
//

//
// We start with the easiest case, which is the various `enum` types that
// get stored as part of the AST.
//
#if 0 // FIDDLE TEMPLATE:
%
%local enumTypeNames = {
%   "ASTNodeType",
%   "TypeTag",
%   "BaseType",
%   "TryClauseType",
%   "DeclVisibility",
%   "BuiltinRequirementKind",
%   "ImageFormat",
%   "PreferRecomputeAttribute::SideEffectBehavior",
%   "TreatAsDifferentiableExpr::Flavor",
%   "LogicOperatorShortCircuitExpr::Flavor",
%   "RequirementWitness::Flavor",
%   "CapabilityAtom",
%   "DeclAssociationKind",
%   "TokenType",
%   "ValNodeOperandKind",
%   "SPIRVAsmOperand::Flavor",
%   "SlangLanguageVersion",
%}
%
%for _,T in ipairs(enumTypeNames) do

/// Serialize a `value` of type `$T`.
template<typename S>
void serialize(S const& serializer, $T& value)
{
    serializeEnum(serializer, value);
}

% -- The `serializeEnum()` function encodes enum values as `FossilUInt`s
% -- so we declare the fossilized representation of these types to match.
%
// Declare fossilized representation of `$T`
SLANG_DECLARE_FOSSILIZED_AS($T, FossilUInt);

%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END

//
// Next we have a few `struct` types that can be serialized just
// based on the information that the fiddle tool is able to
// scrape from their declarations.
//
// The main wrinkle here, as compared to the `enum` handling above,
// is that we will split this logic between two fiddle templates:
// one to generate forward declarations, and another to fill in
// the actual implementations.
//
// The forward declarations are needed to resolve ordering issues
// when types in the AST can transitively reference themselves
// through pointer chains. Because of the way that the `serialize()`
// approach relies on overload resolution, and the `FossilizedTypeTraits`
// approach relies on partial template specialization, it is important
// that the relevant declarations/specializations get seen before
// any use sites are encountered.
//
// TODO: Ideally we would be placing the declarations of the `serialize()`
// functions and the `FossilizedTypeTraits` specializations next to the
// declarations of the types themselves. It would be great if the scraper
// part of the fiddle tool could generate those for `FIDDLE()`-annotated
// types.
//
// The basic idea here is that for each struct type `Foo` that
// we want to serialize, we will forward-declare the `serialize()`
// function, and also forward-declare a type `Fossilized_Foo`
// that will represent a fossilized `Foo` (and we also wire it
// up so that `Fossilized<Foo>` will map to `Fossilized_Foo`).
//
// All of this can be done without ever iterating over the members
// of `Foo`, so we don't run into any ordering issues.
//
#if 0 // FIDDLE TEMPLATE:
%
% -- TODO: This declaration would ideally be `local` in Lua,
% -- but the way that fiddle currently translates the templates
% -- in a C++ file over to Lua puts each distinct template in
% -- its own nested function, which means that their `local`
% -- scopes are distinct. We should see if we can change the
% -- translation so that the code directly nested under a
% -- template like this is in the global scope.
%
%astStructTypes = {
%   Slang.QualType,
%   Slang.SPIRVAsmOperand,
%   Slang.DeclAssociation,
%   Slang.NameLoc,
%   Slang.WitnessTable,
%   Slang.SPIRVAsmInst,
%   Slang.ASTModuleInfo,
%   Slang.ContainerDeclDirectMemberDeclsInfo,
%}
%
%for _,T in ipairs(astStructTypes) do

/// Fossilized representation of a `$T`
struct Fossilized_$T;

SLANG_DECLARE_FOSSILIZED_TYPE($T, Fossilized_$T);

/// Serialize a `$T`
template<typename S>
void serialize(S const& serializer, $T& value);
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 1
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END

//
// Now we move on to the AST nodes themselves (subtypes of `NodeBase`).
//
// The handling of these is largely the same as for the struct
// types above, except that the function that handles serializing
// them is called `_serializeASTNodeContents()`, because of the
// logic that we use to handle the polymorphism of `NodeBase`.
//
#if 0 // FIDDLE TEMPLATE:
%
%astNodeClasses = Slang.NodeBase.subclasses
%
%for _,T in ipairs(astNodeClasses) do

/// Fossilized representation of a `$T`
struct Fossilized_$T;

SLANG_DECLARE_FOSSILIZED_TYPE($T, Fossilized_$T);

/// Serialize the content of a `$T`
template<typename S>
void _serializeASTNodeContents(S const& serializer, $T* value);
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 2
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END


//
// We will define two different implementations of `IASTSerializerImpl`, one for
// the writing case, and one for the reading case. The writing direction is
// the simpler one, so let's look at it first:
//

/// Context for writing a Slang AST to a serialized format.
///
/// This type only provides the contextual information needed
/// to correctly write AST-related types, and delegates the
/// lower-level serialization operations to an underlying
/// `ISerializerImpl`.
///
struct ASTSerialWriteContext : SourceLocSerialContext
{
public:
    using ASTSerializer = Serializer<Fossil::SerialWriter, ASTSerialWriteContext>;

    /// Construct a context for writing a serialized AST.
    ///
    /// * `module` is the module that is being serialized, and will be
    ///   used to detect whether declarations are part of the module,
    ///   or imported from other modules.
    ///
    /// * `sourceLocWriter` will be used to handle translation of
    ///   `SourceLoc`s into a format suitable for serialization.
    ///
    ASTSerialWriteContext(ModuleDecl* module, SerialSourceLocWriter* sourceLocWriter)
        : _module(module), _sourceLocWriter(sourceLocWriter)
    {
    }

private:
    ModuleDecl* _module = nullptr;
    SerialSourceLocWriter* _sourceLocWriter = nullptr;

public:
    //
    // For the most part, this type just implements the methods
    // of the `IASTSerializerImpl` interface, and then has some
    // support routines needed by those implementations.
    //

    void handleName(ASTSerializer const& serializer, Name*& value);
    void handleToken(ASTSerializer const& serializer, Token& value);
    void handleASTNode(ASTSerializer const& serializer, NodeBase*& node);
    void handleASTNodeContents(ASTSerializer const& serializer, NodeBase* node);
    void handleContainerDeclDirectMemberDecls(
        ASTSerializer const& serializer,
        ContainerDeclDirectMemberDecls& value);
    SerialSourceLocWriter* getSourceLocWriter() { return _sourceLocWriter; }

private:
    void _writeImportedModule(ASTSerializer const& serializer, ModuleDecl* moduleDecl);
    void _writeImportedDecl(
        ASTSerializer const& serializer,
        Decl* decl,
        ModuleDecl* importedFromModuleDecl);

    ModuleDecl* _findModuleForDecl(Decl* decl)
    {
        for (auto d = decl; d; d = d->parentDecl)
        {
            if (auto m = as<ModuleDecl>(d))
                return m;
        }
        return nullptr;
    }

    ModuleDecl* _findModuleDeclWasImportedFrom(Decl* decl)
    {
        auto declModule = _findModuleForDecl(decl);
        if (declModule == nullptr)
            return nullptr;
        if (declModule == _module)
            return nullptr;
        return declModule;
    }
};

//
// The reading direction is where things get a bit more interesting.
//
// In order to support on-demand deserialization, we need an object
// that persists across multiple deserialization requests, and that
// stores the state about what values have/haven't already been
// deserialized. Concretely, multiple requests to deserialize the
// same serialized declaration had better return the same `Decl*`.
//
// This is the place where we start concretely assuming that the
// AST will be written and read using the fossil format.
//

/// Context for on-demand AST deserialization.
///
/// This type owns the mapping from fossilized AST declarations
/// to their live `Decl*` counterparts.
///
/// A single `ASTDeserializationContext` should be created and
/// maintained for the entire duration during which fossilized
/// declarations might need to be revitalized. Using multiple
/// contexts could result in the same declaration getting turned
/// into multiple distinct `Decl*`s.
///
struct ASTSerialReadContext : public SourceLocSerialContext, public RefObject
{
public:
    using ASTSerializer = Serializer<Fossil::SerialReader, ASTSerialReadContext>;

    /// Construct an AST deserialization context.
    ///
    /// The `linkage`, `astBuilder`, and `sink` arguments must
    /// all remain valid for as long as this context will be used.
    ///
    /// The context will retain the `sourceLocReader` and the
    /// `blobHoldingSerializedData`. It is assumed that the
    /// `fossilizedModuleInfo` is a pointer into the
    /// `blobHoldingSerializedData`, so that keeping the blob
    /// alive will ensure that the pointer stays valid.
    ///
    ASTSerialReadContext(
        Linkage* linkage,
        ASTBuilder* astBuilder,
        DiagnosticSink* sink,
        SerialSourceLocReader* sourceLocReader,
        SourceLoc requestingSourceLoc,
        Fossilized<ASTModuleInfo> const* fossilizedModuleInfo,
        ISlangBlob* blobHoldingSerializedData)
        : _linkage(linkage)
        , _astBuilder(astBuilder)
        , _sink(sink)
        , _sourceLocReader(sourceLocReader)
        , _requestingSourceLoc(requestingSourceLoc)
        , _fossilizedModuleInfo(fossilizedModuleInfo)
        , _blobHoldingSerializedData(blobHoldingSerializedData)
    {
    }

    /// Translate a fossilized declaration into a live `Decl*`.
    ///
    /// If the same `fossilizedDecl` address has been passed to this
    /// operation before, it will return the same `Decl*`.
    ///
    /// Otherwise, this operation will trigger deserialization
    /// of the `fossilizedDecl` and return the result.
    ///
    /// It is assumed that the `fossilizedDecl` comes from the same
    /// serialized AST and the same data blob that were passed into
    /// the constructor for `ASTDeserializationContext`.
    ///
    Decl* readFossilizedDecl(Fossilized<Decl>* fossilizedDecl);

    /// Look up an export from the fossilized module, by its mangled name.
    ///
    /// If a matching export is found in the serialized data, returns a
    /// the corresponding declaration as if `readFossilizedDecl()` was
    /// invoked on it.
    ///
    /// If no matching export is found, returns null.
    ///
    Decl* findExportedDeclByMangledName(UnownedStringSlice const& mangledName);

private:
    Linkage* _linkage = nullptr;
    ASTBuilder* _astBuilder = nullptr;
    DiagnosticSink* _sink = nullptr;
    RefPtr<SerialSourceLocReader> _sourceLocReader = nullptr;
    SourceLoc _requestingSourceLoc;
    Fossilized<ASTModuleInfo> const* _fossilizedModuleInfo;
    ComPtr<ISlangBlob> _blobHoldingSerializedData;

    //
    // The actual cache for the mapping from fossilized declaration pointers
    // to their revitalized `Decl*`s is maintained by the `Fossil::ReadContext`.
    //

    Fossil::ReadContext _readContext;

#if SLANG_ENABLE_AST_DESERIALIZATION_STATS
    Count _deserializedTopLevelDeclCount = 0;
#endif

public:
    //
    // Much like the `ASTSerialWriter`, for the most part this
    // type just implements the `IASTSerializer` interface,
    // plus a small number of utility methods that serve those
    // implementations.
    //

    void handleName(ASTSerializer const& serializer, Name*& value);
    void handleToken(ASTSerializer const& serializer, Token& value);
    void handleASTNode(ASTSerializer const& serializer, NodeBase*& outNode);
    void handleASTNodeContents(ASTSerializer const& serializer, NodeBase* node);
    void handleContainerDeclDirectMemberDecls(
        ASTSerializer const& serializer,
        ContainerDeclDirectMemberDecls& value);
    SerialSourceLocReader* getSourceLocReader() { return _sourceLocReader; }

private:
    ModuleDecl* _readImportedModule(ASTSerializer const& serializer);
    NodeBase* _readImportedDecl(ASTSerializer const& serializer);

    void _cleanUpASTNode(NodeBase* node);
    void _assignGenericParameterIndices(GenericDecl* genericDecl);
};

//
// Earlier we generated forward declarations for all of the types
// that we'll be able to handle with fiddle, but there are still a
// large number of types that we currently have to hand-write the
// serialization logic for. We'll go over those here.
//

//
// A `Name` is basically just a string, but we need to handle
// a `Name*` as a pointer, and deal with the possibility that
// it might be null.
//
// TODO: It might be better to customize the serialization of
// `Name*` itself, so that it is handled as an optional string.
//

SLANG_DECLARE_FOSSILIZED_AS(Name, String);

template<typename S>
void serializeObject(S const& serializer, Name*& value, Name*)
{
    serializer.getContext()->handleName(serializer, value);
}

void ASTSerialWriteContext::handleName(ASTSerializer const& serializer, Name*& value)
{
    serialize(serializer, value->text);
}

void ASTSerialReadContext::handleName(ASTSerializer const& serializer, Name*& value)
{
    String text;
    serialize(serializer, text);
    value = _astBuilder->getNamePool()->getName(text);
}

//
// A `Token` is *almost* an easy type to handle, and
// the declaration for its fossilized representation
// makes it look like it should be a simple `struct`
// that we can let fiddle generate the implementation
// for:
//

template<>
struct FossilizedTypeTraits<Token>
{
    struct FossilizedType
    {
        Fossilized<decltype(Token::type)> type;
        Fossilized<decltype(Token::loc)> loc;
        Fossilized<decltype(Token::flags)> flags;
        Fossilized<String> content;
    };
};

template<typename S>
void serialize(S const& serializer, Token& value)
{
    serializer.getContext()->handleToken(serializer, value);
}

//
// The cracks start to show when we look at the logic
// for writing a `Token`:
//

void ASTSerialWriteContext::handleToken(ASTSerializer const& serializer, Token& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.type);
    serialize(serializer, value.loc);

    // The flags stored in a `Token` have one bit
    // (`TokenFlag::Name`) that we don't want
    // to have read in on the other side, because
    // it relates to some aspects of the underlying
    // in-memory representation that don't actually
    // relate to the semantic *value* we are serializing.

    TokenFlags flags = TokenFlags(value.flags & ~TokenFlag::Name);
    serialize(serializer, flags);

    // The content of a token is basically just a
    // string, but it can be encoded in different
    // ways, so we extract it here for writing.
    //
    String content = value.getContent();
    serialize(serializer, content);
}

//
// The reading logic adds yet more complexity...
//

void ASTSerialReadContext::handleToken(ASTSerializer const& serializer, Token& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.type);
    serialize(serializer, value.loc);

    serialize(serializer, value.flags);

    String content;
    serialize(serializer, content);

    // Note that we cannot just call `value.setContent(...)`
    // and pass in an `UnownedStringSlice` of `content`,
    // because the `Token` will not take ownership of its own
    // textual content.
    //
    // Instead, we need to get the text we just loaded
    // into something that the `Token` can refer info,
    // and the easiest way to accomplish that is to
    // represent the text using a `Name`.
    //
    Name* name = _astBuilder->getNamePool()->getName(content);
    value.setName(name);
}

//
// While we use fiddle to generate a lot of the code related to
// specific subclasses of `NodeBase`, the logic to serialize
// a `NodeBase*` itself needs to be special-cased by intercepting
// the `serializeObject()` customization point provided by
// the serialization system.
//
// We'll cover the implementations of `handleASTNode()` for the
// reading and writing cases later; what matters now is to
// establish this declaration before any code that tries to
// serialize any pointers to AST nodes.
//

template<typename S, typename T>
SLANG_FORCE_INLINE void serializeObject(S const& serializer, T*& value, NodeBase*)
{
    // The general-purpose serialization layer defines
    // a variant as akin to a struct, but where the
    // specific number and type of fields that get written
    // can vary from value to value, for the same type.
    //
    // The fossil encoding of a variant is always via indirection,
    // as a pointer to a memory region holding the particular
    // value, along with a pointer to the layout information
    // for that value.
    //
    // Because `NodeBase` is the base class of a polymorphic
    // class hierarchy, we treat all pointers to `NodeBase`-derived
    // types as variants for serialization purposes.
    //
    SLANG_SCOPED_SERIALIZER_VARIANT(serializer);
    serializer.getContext()->handleASTNode(serializer, reinterpret_cast<NodeBase*&>(value));
}

//
// We also intercept the `serializeObjectContents()` customization
// point, which is used to read/write most of the actual members
// of an AST node, whereas the `serializeObject()` step just deals
// with the parts that are necessary to allocate (or find) an
// object in the reading direction.
//

template<typename S>
SLANG_FORCE_INLINE void serializeObjectContents(S const& serializer, NodeBase* value, NodeBase*)
{
    serializer.getContext()->handleASTNodeContents(serializer, value);
}

//
// The handling of the members of a `ContainerDecl` is another
// complicated part of the serialization process, so we will
// define the `serialize()` implementation here, but defer its
// implementation until later.
//
// We know, however, that the members will be serialized via
// the intermediate `ContainerDeclDirectMemberDeclsInfo` type
// that was defined earlier in this file.
//

SLANG_DECLARE_FOSSILIZED_AS(ContainerDeclDirectMemberDecls, ContainerDeclDirectMemberDeclsInfo);

template<typename S>
SLANG_FORCE_INLINE void serialize(S const& serializer, ContainerDeclDirectMemberDecls& value)
{
    serializer.getContext()->handleContainerDeclDirectMemberDecls(serializer, value);
}

//
// Pointers to diagnostics (which can be referenced in attributes
// related to enabling/disabling warnings) get serialized as
// the integer diagnostic ID.
//

SLANG_DECLARE_FOSSILIZED_AS(DiagnosticInfo const*, Int32);

template<typename S>
void serializePtr(S const& serializer, DiagnosticInfo const*& value, DiagnosticInfo const*)
{
    Int32 id = 0;
    if (isWriting(serializer))
    {
        id = value->id;
        serialize(serializer, id);
    }
    else
    {
        serialize(serializer, id);
        value = getDiagnosticsLookup()->getDiagnosticById(id);
    }
}


//
// A `DeclRef<T>` is just a wrapper around a `DeclRefBase*`,
// and we'll serialize it as such.
//

template<typename S, typename T>
void serialize(S const& serializer, DeclRef<T>& value)
{
    serialize(serializer, value.declRefBase);
}

template<typename T>
struct FossilizedTypeTraits<DeclRef<T>>
{
    // TODO: This case can't be declared with `SLANG_DECLARE_FOSSILIZED_AS()`
    // because of the need for the template parameter `T`. A more advanced
    // version of that macro could also allow for template parameters,
    // but for now it is okay to just write these cases out long-form.
    //
    using FossilizedType = Fossilized<DeclRefBase*>;
};

//
// A `SyntaxClass<T>` is a wrapper around an `ASTNodeType`:
//

SLANG_DECLARE_FOSSILIZED_AS(SyntaxClass<NodeBase>, ASTNodeType);

template<typename S>
void serialize(S const& serializer, SyntaxClass<NodeBase>& value)
{
    ASTNodeType raw = ASTNodeType(0);
    if (isWriting(serializer))
    {
        raw = value.getTag();
    }
    serialize(serializer, raw);
    if (isReading(serializer))
    {
        value = SyntaxClass<NodeBase>(raw);
    }
}

//
// The `Modifiers` type is just a wrapper around the way
// that the `Modifier` type uses an internally-linked list.
//
// We serialize `Modifiers` as if they were just using
// the ordinary `List<T>` type (which maybe they should...).
//

SLANG_DECLARE_FOSSILIZED_AS(Modifiers, List<Modifier*>);

template<typename S>
void serialize(S const& serializer, Modifiers& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);

    // Because we are dealing with a list, rather
    // than a more mundane aggregate type list
    // a struct, we need our logic to distinguish
    // between the writing and reading cases.
    //
    if (isWriting(serializer))
    {
        for (auto modifier : value)
        {
            serialize(serializer, modifier);
        }
    }
    else
    {
        Modifier** link = &value.first;

        while (hasElements(serializer))
        {
            Modifier* modifier = nullptr;
            serialize(serializer, modifier);

            *link = modifier;
            link = &modifier->next;
        }
    }
}

//
// For the purposes of serialization, a `TypeExp` is just
// a wrapper around a `Type*`.
//
// (Under the hood a `TypeExp` has room to store both a
// type *expression* (an `Expr*`) and the `Type*` that
// we compute as a result of checking that type expression.
// For any AST that has passed front-end semantic checking,
// the `Type*` part is expected to be filled in, and the
// `Expr*` part is no longer relevant.)
//
// Here we use another convenience macro to declare that
// the fossilized reprsentation of a `TypeExp` is the same
// as the `TypeExpr::type` member.
//
SLANG_DECLARE_FOSSILIZED_AS_MEMBER(TypeExp, type);

template<typename S>
void serialize(S const& serializer, TypeExp& value)
{
    serialize(serializer, value.type);
}

//
// The `CandidateExtensionList` and `DeclAssociationList` types
// are simple wrappers around a single field.
//

SLANG_DECLARE_FOSSILIZED_AS_MEMBER(CandidateExtensionList, candidateExtensions);

template<typename S>
void serialize(S const& serializer, CandidateExtensionList& value)
{
    serialize(serializer, value.candidateExtensions);
}


SLANG_DECLARE_FOSSILIZED_AS_MEMBER(DeclAssociationList, associations);

template<typename S>
void serialize(S const& serializer, DeclAssociationList& value)
{
    serialize(serializer, value.associations);
}

//
// The `RequirementWitness` type is a variant, where the `m_flavor`
// field determines what data can follow.
//
// For now we will skip declaring those additional members as part
// of the fossilized representation, because we do not have any
// code that wants to navigate them directly on that representation:
//

template<>
struct FossilizedTypeTraits<RequirementWitness>
{
    struct FossilizedType
    {
        Fossilized<decltype(RequirementWitness::m_flavor)> m_flavor;
    };
};

template<typename S>
void serialize(S const& serializer, RequirementWitness& value)
{
    SLANG_SCOPED_SERIALIZER_VARIANT(serializer);
    serialize(serializer, value.m_flavor);
    switch (value.m_flavor)
    {
    case RequirementWitness::Flavor::none:
        break;

    case RequirementWitness::Flavor::declRef:
        serialize(serializer, value.m_declRef);
        break;

    case RequirementWitness::Flavor::val:
        serialize(serializer, value.m_val);
        break;

    case RequirementWitness::Flavor::witnessTable:
        serialize(serializer, value.m_obj);
        break;
    }
}

//
// The `ValNodeOperand` type, used to store the operands of
// a `Val`-derived AST node, is a variant that gets handled
// similarly to `RequirementWitness` above.
//

template<>
struct FossilizedTypeTraits<ValNodeOperand>
{
    struct FossilizedType
    {
        Fossilized<decltype(ValNodeOperand::kind)> kind;
    };
};

template<typename S>
void serialize(S const& serializer, ValNodeOperand& value)
{
    SLANG_SCOPED_SERIALIZER_VARIANT(serializer);
    serialize(serializer, value.kind);
    switch (value.kind)
    {
    case ValNodeOperandKind::ConstantValue:
        serialize(serializer, value.values.intOperand);
        break;

    case ValNodeOperandKind::ValNode:
    case ValNodeOperandKind::ASTNode:
        serialize(serializer, value.values.nodeOperand);
        break;
    }
}

//
// Now that we've covered the types that required hand-writing their
// serialization logic, we return to the types that will have their
// serialization logic generated using fiddle.
//
// We start with the ordinary types (everything other than the
// `NodeBase`-derived stuff).
//
// The code being generated here is the same sort of thing that was
// in the hand-written case for types like `MatrixCoord` way earlier
// in this file (in fact, `MatrixCoord` could be handled by this
// logic, and is only hand-written to help illustrate what's going on).
//
// WARNING: The way these declarations are currently being generated
// uses inheritance in the definitions of the `Fossilized_*` types,
// which isn't actually something we can be confident will work correctly
// across compilers. This isn't a problem right now, because there
// doesn't end up being any code that will actually use these generated
// types in the case where there is inhertance going on that would
// break C++ "standard layout" rules.
//
// TODO: If we reach a point where the use of inheritance ends up
// breaking things, then we'll have to do a fair bit more. It might
// seem like we could just turn the `: public Whatever` base into
// a `Whatever super;` field declaration, but that wouldn't give
// us the correct layout in cases where `Whatever` is an empty
// type.
//
#if 0 // FIDDLE TEMPLATE:
%for _,T in ipairs(astStructTypes) do
/// Fossilized representation of a value of type `$T`
struct Fossilized_$T
%   if T.directSuperClass then
    : public Fossilized<$(T.directSuperClass)>
%   else
    : public FossilizedRecordVal
%   end
{
%   for _,f in ipairs(T.directFields) do
    Fossilized<decltype($T::$f)> $f;
%   end
};

/// Serialize a `value` of type `$T`
template<typename S>
void serialize(S const& serializer, $T& value)
{
    SLANG_UNUSED(value);
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
%   if T.directSuperClass then
    serialize(serializer, static_cast<$(T.directSuperClass)&>(value));
%   end
%   for _,f in ipairs(T.directFields) do
    serialize(serializer, value.$f);
%   end
}
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 3
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END

//
// After the ordinary struct types come the AST node classes.
// As with the declarations, the definitions here aren't all
// that different from how the structs are being handled.
//
// Note that the big "WARNING" on the comment before the struct
// cases also applies to the inheritance here. It just turns out
// that no code (currently) wants to navigate serialized AST
// nodes in memory (so the `Fossilized_*` declarations are largely
// just there to be convenient when debugging).
//
// One wrinkle we deal with here is that the `astNodeType`
// field is not treated as part of the "content" of an AST
// node for the purposes of the `_serializeASTNodeContents()`
// functions, but it needs to be present in the `Fossilized_NodeBase`
// type declaration in order for the layout of these types to
// be correct. We handle that with a small but ugly conditional
// in the logic to define `Fossilized_*`.
//
#if 0 // FIDDLE TEMPLATE:
%for _,T in ipairs(astNodeClasses) do

/// Fossilized representation of a value of type `$T`
struct Fossilized_$T
%   if T.directSuperClass then
    : public Fossilized_$(T.directSuperClass)
%   else
    : public FossilizedVariantObj
%   end
{
%   if T == Slang.NodeBase then
    Fossilized<ASTNodeType> astNodeType;
%   end

%   for _,f in ipairs(T.directFields) do
    Fossilized<decltype($T::$f)> $f;
%   end
};

/// Serialize the contents of an AST node of type `$T`
template<typename S>
void _serializeASTNodeContents(S const& serializer, $T* value)
{
    SLANG_UNUSED(serializer);
    SLANG_UNUSED(value);
%   if T.directSuperClass then
    _serializeASTNodeContents(serializer, static_cast<$(T.directSuperClass)*>(value));
%   end
%   for _,f in ipairs(T.directFields) do
    serialize(serializer, value->$f);
%   end
}
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 4
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END

//
// Each of the `_serializeASTNodeContents()` functions handles one class in the hierarchy,
// but we need to be able to dispatch to the correct one based on the run-time type of
// a particular AST node.
//
// The `serializeASTNodeContents()` function is a wrapper around those underscore-prefixed
// functions, and dispatches to the correct one based on the type of the given node.
//

template<typename S>
void serializeASTNodeContents(S const& serializer, NodeBase* node)
{
    ASTNodeDispatcher<NodeBase, void>::dispatch(
        node,
        [&](auto n) { _serializeASTNodeContents(serializer, n); });
}

//
// At this point we can get back to the handling of reading/writing actual AST nodes.
//
// We'll start with the writing logic, because that gives a good idea of
// the overall structure, which the reading logic will need to follow:
//

void ASTSerialWriteContext::handleASTNode(ASTSerializer const& serializer, NodeBase*& node)
{
    // The first complication that needs to be handled is that when we
    // run into a `Decl*` that is being written, we need to check
    // whether it comes from an imported module (as opposed to the
    // module we are being asked to serialize).
    //
    if (auto decl = as<Decl>(node))
    {
        if (auto moduleDeclWasImportedFrom = _findModuleDeclWasImportedFrom(decl))
        {
            // If we find that the declaration is imported, then there
            // are two sub-cases that we want to handle a bit differently:
            //
            // * When the `decl` we are writing is itself a module
            //   (and thus identical to `moduleDeclWasImportedFrom`).
            //
            // * The ordinary case, where `decl` is one of the declarations
            //   contained in `moduleDeclWasImportedFrom`.
            //
            if (decl == moduleDeclWasImportedFrom)
            {
                _writeImportedModule(serializer, moduleDeclWasImportedFrom);
                return;
            }
            else
            {
                _writeImportedDecl(serializer, decl, moduleDeclWasImportedFrom);
                return;
            }
        }
    }

    // The next complication we need to deal with is that
    // for most AST nodes we will want to defer writing
    // out their contents until a later step (to avoid
    // going into an infinite recursion when there are
    // cycles in the object graph), but because of the
    // way that AST nodes derived from `Val` are
    // deduplicated as part of creation, we can't
    // defer reading their operands.
    //
    // Thus we branch here based on whether we are
    // writing a `Val`-derived node, or not.
    //
    if (auto val = as<Val>(node))
    {
        val = val->resolve();

        serialize(serializer, val->astNodeType);
        serialize(serializer, val->m_operands);
    }
    else
    {
        serialize(serializer, node->astNodeType);
        deferSerializeObjectContents(serializer, node);
    }
}

//
// In order to be able to encode the cases for imported
// modules and declarations, we get a little bit "clever"
// with the representation and store some out-of-range
// values in an `ASTNodeType` to represent these
// additional cases.
//

enum class PseudoASTNodeType
{
    None,
    ImportedModule,
    ImportedDecl,
};

// All valid `ASTNodeType`s will be non-negative integers,
// so the `PseudoASTNodeType` are encoded into an
// `ASTNodeType` as negative values that are the bitwise
// negation of their value in the `PseudoASTNodeType` enumeration.

static PseudoASTNodeType _getPseudoASTNodeType(ASTNodeType type)
{
    return Int32(type) < 0 ? PseudoASTNodeType(~Int32(type)) : PseudoASTNodeType::None;
}

static ASTNodeType _getAsASTNodeType(PseudoASTNodeType type)
{
    return ASTNodeType(~Int32(type));
}

//
// With the `PseudoASTNodeType` trickery introduced,
// it is possible to show the reading logic for
// `NodeBase`-derived types:
//

void ASTSerialReadContext::handleASTNode(ASTSerializer const& serializer, NodeBase*& outNode)
{
    // We start by reading the `ASTNodeType`, because
    // we will dispatch differently based on what
    // value we see there.
    //
    ASTNodeType typeTag = ASTNodeType::NodeBase;
    serialize(serializer, typeTag);

    // In the case where the `ASTNodeType` is actually
    // smuggling in one of our `PseudoASTNodeType`
    // values, we can delegate to the correct
    // subroutine to handle that case.
    //
    // These two cases mirror the cases for imported
    // modules and declarations in
    // `ASTSerialWriter::handleASTNode()`.
    //
    switch (_getPseudoASTNodeType(typeTag))
    {
    default:
        break;

    case PseudoASTNodeType::ImportedModule:
        outNode = _readImportedModule(serializer);
        return;

    case PseudoASTNodeType::ImportedDecl:
        outNode = _readImportedDecl(serializer);
        return;
    }

    // Next we check whether the `typeTag`
    // indicates that we are looking at a
    // subclass of `Val`, because we need
    // to handle those differently.
    //
    auto syntaxClass = SyntaxClass<NodeBase>(typeTag);
    if (syntaxClass.isSubClassOf<Val>())
    {
        // Subclasses of `Val` are deduplicated as part
        // of creation, so we need to read in their
        // operands before we can create them, rather
        // than allocating the object up front and
        // then deserializing its content into it later.

        ValNodeDesc desc;
        desc.type = syntaxClass;
        serialize(serializer, desc.operands);

        desc.init();

        auto node = _astBuilder->_getOrCreateImpl(std::move(desc));
        outNode = node;
    }
    else
    {
        // In the ordinary case, we can allocate an empty
        // shell of an AST node to represent the object,
        // and defer actually serializing the contents
        // of that object until later.

        auto node = syntaxClass.createInstance(_astBuilder);
        outNode = node;

        deferSerializeObjectContents(serializer, node);
    }
}

//
// Imported modules are serialized using one of the
// `PseudoASTNodeType` cases as its tag, and then
// store a single field with the name of the module.
//

void ASTSerialWriteContext::_writeImportedModule(
    ASTSerializer const& serializer,
    ModuleDecl* moduleDecl)
{
    ASTNodeType type = _getAsASTNodeType(PseudoASTNodeType::ImportedModule);
    auto moduleName = String(moduleDecl->module->getName());

    serialize(serializer, type);
    serialize(serializer, moduleName);
}

ModuleDecl* ASTSerialReadContext::_readImportedModule(ASTSerializer const& serializer)
{
    // In the reading direction, we need to actually
    // kick off the logic to import the module
    // that this one depends on.
    //
    // TODO: It might be cleaner if we changed up
    // the representation so that imported modules
    // get listed at the top level, as part of
    // the `ASTModuleInfo`, and thus allowing the
    // process of importing them to be handled
    // by logic that isn't deep in the guts of the
    // serialization code.
    //
    Name* moduleName = nullptr;
    serialize(serializer, moduleName);
    auto module = _linkage->findOrImportModule(moduleName, _requestingSourceLoc, _sink);
    if (!module)
    {
        if (_sink)
            _sink->diagnose(_requestingSourceLoc, Diagnostics::importFailed, moduleName);
        return nullptr;
    }
    return module->getModuleDecl();
}

//
// Imported declarations use a `PseudoASTNodeType`
// to define their type tag, and are then serialized
// like a struct that contains a poitner to the
// module that the declaration was imported from,
// and the mangled name of the specific declaration.
//

void ASTSerialWriteContext::_writeImportedDecl(
    ASTSerializer const& serializer,
    Decl* decl,
    ModuleDecl* importedFromModuleDecl)
{
    ASTNodeType type = _getAsASTNodeType(PseudoASTNodeType::ImportedDecl);
    auto mangledName = getMangledName(getCurrentASTBuilder(), decl);

    serialize(serializer, type);
    serialize(serializer, importedFromModuleDecl);
    serialize(serializer, mangledName);
}

NodeBase* ASTSerialReadContext::_readImportedDecl(ASTSerializer const& serializer)
{
    ModuleDecl* importedFromModuleDecl = nullptr;
    String mangledName;

    serialize(serializer, importedFromModuleDecl);
    serialize(serializer, mangledName);

    if (!importedFromModuleDecl)
        return nullptr;

    auto importedFromModule = importedFromModuleDecl->module;
    if (!importedFromModule)
    {
        return nullptr;
    }

    auto importedDecl =
        importedFromModule->findExportedDeclByMangledName(mangledName.getUnownedSlice());
    if (!importedDecl)
    {
        _sink->diagnose(
            SourceLoc(),
            Diagnostics::cannotResolveImportedDecl,
            mangledName,
            importedFromModule->getName());
    }
    return importedDecl;
}

//
// Handling the contents of an AST node is mostly the
// same logic between the reading and writing directions.
// The only difference is that when we are reading in
// an AST node there is some cleanup work we have to
// do after reading is complete, in order to make
// the AST node actually usable.
//

void ASTSerialWriteContext::handleASTNodeContents(ASTSerializer const& serializer, NodeBase* node)
{
    serializeASTNodeContents(serializer, node);
}

void ASTSerialReadContext::handleASTNodeContents(ASTSerializer const& serializer, NodeBase* node)
{
    serializeASTNodeContents(serializer, node);

    _cleanUpASTNode(node);
}

void ASTSerialReadContext::_cleanUpASTNode(NodeBase* node)
{
    if (auto expr = as<Expr>(node))
    {
        expr->checked = true;
    }
    else if (auto decl = as<Decl>(node))
    {
        decl->checkState = DeclCheckState::CapabilityChecked;

        if (auto genericDecl = as<GenericDecl>(node))
        {
            _assignGenericParameterIndices(genericDecl);
        }
        else if (auto syntaxDecl = as<SyntaxDecl>(node))
        {
            syntaxDecl->parseCallback = &parseSimpleSyntax;
            syntaxDecl->parseUserData = (void*)syntaxDecl->syntaxClass.getInfo();
        }
        else if (auto namespaceLikeDecl = as<NamespaceDeclBase>(node))
        {
            auto declScope = _astBuilder->create<Scope>();
            declScope->containerDecl = namespaceLikeDecl;
            namespaceLikeDecl->ownedScope = declScope;
        }

#if SLANG_ENABLE_AST_DESERIALIZATION_STATS
        if (auto moduleDecl = as<ModuleDecl>(decl->parentDecl))
        {
            auto& deserializedCount = _deserializedTopLevelDeclCount;
            deserializedCount++;

            Count totalCount = moduleDecl->getDirectMemberDeclCount();

            fprintf(
                stderr,
                "loaded %d / %d direct members of module '%s' (%f%%)\n",
                int(deserializedCount),
                int(totalCount),
                moduleDecl->getName() ? moduleDecl->getName()->text.getBuffer() : "",
                float(deserializedCount) * 100.0f / float(totalCount));
        }
#endif

        // TODO(tfoley): If we are disabling on-demand deserialization
        // for now (because of other changes that are needed before we
        // can enable it), then we will intentionally load all of the
        // direct member declarations of a container declarations
        // up-front.
#if SLANG_DISABLE_ON_DEMAND_AST_DESERIALIZATION
        if (auto containerDecl = as<ContainerDecl>(decl))
        {
            auto& directMemberDecls = containerDecl->getDirectMemberDecls();
            SLANG_UNUSED(directMemberDecls);
        }
#endif
    }
}

void ASTSerialReadContext::_assignGenericParameterIndices(GenericDecl* genericDecl)
{
    int parameterCounter = 0;
    for (auto m : genericDecl->getDirectMemberDecls())
    {
        if (auto typeParam = as<GenericTypeParamDeclBase>(m))
        {
            typeParam->parameterIndex = parameterCounter++;
        }
        else if (auto valParam = as<GenericValueParamDecl>(m))
        {
            valParam->parameterIndex = parameterCounter++;
        }
    }
}


//
//
//

template<typename K, typename V>
static void _sortByKey(List<KeyValuePair<K, V>>& array)
{
    array.sort([](KeyValuePair<K, V> const& lhs, KeyValuePair<K, V> const& rhs)
               { return lhs.key < rhs.key; });
}

static void _collectASTModuleInfo(ModuleDecl* moduleDecl, ASTModuleInfo& moduleInfo)
{
    auto module = moduleDecl->module;

    moduleInfo.moduleDecl = moduleDecl;
    collectBuiltinDeclsThatNeedRegistration(moduleDecl, moduleInfo.declsToRegister);

    // We want to store a dictionary of exported declarations
    // from the module, mapping from a mangled name to the
    // declaration with that name.
    //
    // In order to accelerate search on the reading side, we will
    // conspire to make the entries in the serialized dictionary
    // be in sorted order by their keys.
    //
    List<KeyValuePair<String, Decl*>> exportNameDeclPairs;

    auto exportCount = module->getExportedDeclCount();
    for (Index exportIndex = 0; exportIndex < exportCount; ++exportIndex)
    {
        auto exportMangledName = String(module->getExportedDeclMangledName(exportIndex));
        auto exportDecl = module->getExportedDecl(exportIndex);

        exportNameDeclPairs.add(KeyValuePair(exportMangledName, exportDecl));
    }
    _sortByKey(exportNameDeclPairs);

    for (auto& entry : exportNameDeclPairs)
    {
        moduleInfo.mapMangledNameToDecl.add(entry.key, entry.value);
    }
}

//
// The `ContainerDeclDirectMemberDecls` type is serialized via the
// intermediate type `ContainerDeclDirectMemberDeclsInfo`. We start
// by defining the logic to collect the required information:
//

static ContainerDeclDirectMemberDeclsInfo _collectContainerDeclDirectMemberDeclsInfo(
    ContainerDeclDirectMemberDecls const& decls)
{
    ContainerDeclDirectMemberDeclsInfo info;
    info.decls = decls.getDecls();

    // In order to ensure that the accelerators that we serialize
    // match with those created by the compiler front-end, we
    // will pull the data from `decls` via its public API rather
    // than try to reconstruct any of that information.
    //
    // Because the public API of `ContainerDeclDirectMemberDecls`
    // traffics in `Decl*`s but we want to serialize indices,
    // we will create a dictionary to reverse the mapping so that
    // we can serialize out indices.
    //
    Dictionary<Decl*, FossilUInt> mapDeclToIndex;
    Count declCount = info.decls.getCount();
    for (Index i = 0; i < declCount; ++i)
    {
        auto decl = info.decls[i];
        if (!decl)
            continue;

        mapDeclToIndex[decl] = FossilUInt(i);
    }

    // With our decl-to-index mapping created, filling
    // out the to-be-serialized list of transparent
    // declarations is a simple matter.
    //
    for (auto decl : decls.getTransparentDecls())
    {
        if (!decl)
            continue;

        auto found = mapDeclToIndex.tryGetValue(decl);
        SLANG_ASSERT(found);

        info.transparentDeclIndices.add(*found);
    }

    // Handling the name-to-declaration mapping is a bit
    // more complicated, simply because we want to store
    // the entries of the resulting dictionary in sorted
    // order to enable them to be looked up via a binary
    // search. Thus we start by creating a list of the
    // key-value pairs, which we will then sort.
    //
    List<KeyValuePair<String, FossilUInt>> nameIndexPairs;
    for (auto& entry : decls.getMapFromNameToLastDeclOfThatName())
    {
        auto name = entry.first;
        if (!name)
            continue;

        auto decl = entry.second;
        if (!decl)
            continue;

        auto found = mapDeclToIndex.tryGetValue(decl);
        SLANG_ASSERT(found);

        nameIndexPairs.add(KeyValuePair(name->text, *found));
    }
    _sortByKey(nameIndexPairs);

    // The `info.mapNameToDeclIndex` is stored as an `OrderedDictionary`,
    // so it will preserve the order in which we insert its entries here.
    //
    for (auto& entry : nameIndexPairs)
    {
        info.mapNameToDeclIndex.add(entry.key, entry.value);
    }

    return info;
}

void ASTSerialWriteContext::handleContainerDeclDirectMemberDecls(
    ASTSerializer const& serializer,
    ContainerDeclDirectMemberDecls& value)
{
    // Writing the members of a container declaration is
    // just a matter of collecting the information into
    // the intermediate type, and then writing *that*.

    ContainerDeclDirectMemberDeclsInfo info = _collectContainerDeclDirectMemberDeclsInfo(value);

    serialize(serializer, info);
}

void ASTSerialReadContext::handleContainerDeclDirectMemberDecls(
    ASTSerializer const& serializer,
    ContainerDeclDirectMemberDecls& value)
{
    // In the reading direction, we will intentionally
    // *not* deserialize things the usual way, because
    // we want to support deserializing only a subset
    // of the direct member declarations of a given
    // container, on-demand.

    // We start by reading a pointer to a single fossilized
    // value from the underlying `Fossil::Reader` that we
    // are using, and cast it to the type that we expect to
    // find there.
    //
    // The underlying reader was passed in as part of the
    // `serializer` parameter, but it is only typed as an
    // `ISerializerImpl`, whereas we *know* it has a more
    // specific type, which we want to make use of.
    //
    auto readerImpl = serializer.getImpl();
    auto fossilReader = static_cast<Fossil::SerialReader*>(readerImpl);
    //
    auto fossilizedInfo =
        (Fossilized<ContainerDeclDirectMemberDeclsInfo>*)fossilReader->readValPtr().get();

    // We can read specific fields out of the `fossilizedInfo`
    // without triggering full deserialization. At this point
    // we will do exactly that to read the number of direct
    // member declarations.
    //
    auto declCount = fossilizedInfo->decls.getElementCount();

    // We will set up the `ContainerDeclDirectMemberDecls` to
    // be in on-demand deserialization mode, in which it will
    // retain a pointer to this context (which is being used for
    // the entire AST module), along with a pointer to the
    // fossilized information for this specific container's
    // member declarations.
    //
    value._initForOnDemandDeserialization(this, fossilizedInfo, declCount);
}


//
// {write|read}SerializedModuleAST()
//

void writeSerializedModuleAST(
    RIFF::BuildCursor& cursor,
    ModuleDecl* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter)
{
    SLANG_PROFILE;

    // TODO: we might want to have a more careful pass here,
    // where we only encode the public declarations.

    // Rather than serialize the `ModuleDecl` directly, we instead
    // collect the information we want to serialize into an intermediate
    // `ASTModuleInfo` value, and then serialize *that*.
    //
    // This choice allows us to build up some data structures that will
    // be very useful when reading the serialized data later, and that
    // would not naturally "fall out" of serializing the module more
    // directly.

    ASTModuleInfo moduleInfo;
    _collectASTModuleInfo(moduleDecl, moduleInfo);

    // At the most basic, we are building a single "blob" of data
    // (in the sense of the `ISlangBlob` interface).
    //
    BlobBuilder blobBuilder;
    {
        // The architecture of the serialization system means that
        // we need a few steps to set up everything before we can
        // actually call `serialize()`:
        //
        // * We need an implementation of `ISerializerImpl` to do
        // the actual writing, which in this case will be a
        // `Fossil::SerialWriter`.
        //
        // * We need the additional context information that many
        // of the AST types require in their `serialize()` overloads,
        // which will be an `ASTSerialWriteContext`.
        //
        // * We need to wrap those two values up in an `ASTSerializer`
        // (which is more or less just a pair of pointers, to the two
        // values described above).
        //
        Fossil::SerialWriter writer(blobBuilder);
        ASTSerialWriteContext context(moduleDecl, sourceLocWriter);
        ASTSerialWriteContext::ASTSerializer serializer(&writer, &context);

        // Once we have our `serializer`, we can finally invoke
        // `serialize()` on the `ASTModuleInfo` to cause everything
        // to be recursively written.
        //
        serialize(serializer, moduleInfo);

        // Note that we wrapped these steps in a scope, because
        // it is the destructor for `Fossil::SerialWriter` that
        // will actually "flush" any pending serialization operations
        // and cause the full blob to be written.
    }

    // We can now grab the serialized data as a single `ISlangBlob`.
    //
    ComPtr<ISlangBlob> blob;
    blobBuilder.writeToBlob(blob.writeRef());

    // While the AST serialization system is using fossil, the
    // overall module serialization is still based on the RIFF
    // container format, so we immediately turn around and
    // add the blob we just created as a single data chunk in
    // the RIFF hierarchy.
    //
    // TODO: This step copies the entire blob. If that copy
    // operation ever becomes a performance concern, we should
    // be able to tweak things so that the `BlobBuilder` uses
    // the same memory arena that the RIFF builder is using,
    // and then employ the `RIFF::BuildCursor::addUnownedData()`
    // method to add the data without copying.
    //
    void const* data = blob->getBufferPointer();
    size_t size = blob->getBufferSize();
    cursor.addDataChunk(PropertyKeys<Module>::ASTModule, data, size);
}

//
// The reading direction is significantly more subtle than the
// writing direction, because we will be traversing some of
// the fossilized data structures without first deserializing
// them into ordinary C++ objects.
//
// In order for this code to work, we need to know that the
// fossilized layout for the types we will access directly
// (such as `ASTModuleInfo`) will exactly match what we expect.
//
// As a small safety measure, we include some static assertions
// about the key properties we expect of the fossilized `ASTModuleInfo`.
//

static_assert(sizeof(Fossilized<ASTModuleInfo>) == 12);
static_assert(offsetof(Fossilized<ASTModuleInfo>, moduleDecl) == 0);
static_assert(offsetof(Fossilized<ASTModuleInfo>, declsToRegister) == 4);
static_assert(offsetof(Fossilized<ASTModuleInfo>, mapMangledNameToDecl) == 8);

ModuleDecl* readSerializedModuleAST(
    Linkage* linkage,
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    ISlangBlob* blobHoldingSerializedData,
    RIFF::Chunk const* chunk,
    SerialSourceLocReader* sourceLocReader,
    SourceLoc requestingSourceLoc)
{
    SLANG_PROFILE;

    // We expect the `chunk` that was passed in to be a RIFF
    // data chunk (matching what was written in `writeSerializedModuleAST()`,
    // and to be proper fossil-format data.
    //
    auto dataChunk = as<RIFF::DataChunk>(chunk);
    if (!dataChunk)
    {
        SLANG_UNEXPECTED("invalid format for serialized module AST");
    }

    Fossil::AnyValPtr rootValPtr =
        Fossil::getRootValue(dataChunk->getPayload(), dataChunk->getPayloadSize());
    if (!rootValPtr)
    {
        SLANG_UNEXPECTED("invalid format for serialized module AST");
    }

    // We don't want to simply mirror the `writeSerializedModuleAST()` logic
    // here and deserialize an entire `ASTModuleInfo`. Instead, we will
    // traverse the `Fossilized<ASTModuleInfo>` directly, and extract only
    // the information we need.
    //
    // The `rootValPtr` above uses the `Fossil::AnyValPtr` type, which
    // is basically a dynamically-typed pointer to fossilized data of
    // any type, and carries around its own layout information. We could
    // in principle traverse the structure using that type by making dynamic
    // queries (and doing so would let us detect various error cases where
    // the serialized format might not match what we expect), but instead
    // we are going to simply perform an uncheckedcast on that dynamically-typed
    // pointer to get out a statically-typed pointer to what we expect to
    // find there.
    //
    Fossilized<ASTModuleInfo>* fossilizedModuleInfo = cast<Fossilized<ASTModuleInfo>>(rootValPtr);

    // We now have enough information to construct an `ASTSerialReadContext`,
    // which is the mirror to the `ASTSerialWriteContext`, but which has the
    // important difference that the `ASTSerialReadContext` is allowed to
    // persist past when this function returns. Thus we cannot allocte the
    // read context on the stack like we did for the write context, and
    // we instead allocate it as a reference-counted object.
    //
    auto sharedDecodingContext = RefPtr(new ASTSerialReadContext(
        linkage,
        astBuilder,
        sink,
        sourceLocReader,
        requestingSourceLoc,
        fossilizedModuleInfo,
        blobHoldingSerializedData));

    // The `sharedDecodingContext` will allow us to deserialize individual
    // `Decl`s from the AST one-by-one. One declaration that we *know*
    // we need right away is the actual `ModuleDecl` (since we need to
    // return it from this function).
    //
    ModuleDecl* moduleDecl =
        as<ModuleDecl>(sharedDecodingContext->readFossilizedDecl(fossilizedModuleInfo->moduleDecl));
    SLANG_ASSERT(moduleDecl);

#if SLANG_ENABLE_AST_DESERIALIZATION_STATS
    fprintf(
        stderr,
        "finished loading the `ModuleDecl` for '%s'\n",
        moduleDecl->getName()->text.getBuffer());
#endif

    // In the case where we are reading one of the builtin modules (e.g.
    // the core module), there may be declarations inside that module
    // that need to be registered with the `SharedASTBuilder`, because
    // parts of the C++ compiler code need to be able to form references
    // to those declarations.
    //
    // We will handle those here by traversing the fossilized equivalent
    // of the `ASTModuleInfo::declsToRegister` array, then deserializing
    // and registering each entry we find.
    //
    for (Fossilized<Decl>* fossilizedDecl : fossilizedModuleInfo->declsToRegister)
    {
        Decl* decl = sharedDecodingContext->readFossilizedDecl(fossilizedDecl);
        registerBuiltinDecl(astBuilder, decl);
    }

#if SLANG_ENABLE_AST_DESERIALIZATION_STATS
    fprintf(
        stderr,
        "finished registering builtins for '%s'\n",
        moduleDecl->getName()->text.getBuffer());
#endif

    //
    // At this point any further data in the serialized AST can be read
    // on-demand as needed, via the accessor methods on `ContainerDeclDirectMemberDecls`
    // and `ModuleDecl` that are implemented below.
    //

    return moduleDecl;
}

//
// A key facility that makes on-demand deserialization possible is
// the ability to read individual serialized declarations out of
// a module, just based on a pointer to their fossilized representation.
//

Decl* ASTSerialReadContext::readFossilizedDecl(Fossilized<Decl>* fossilizedDecl)
{
    // AST nodes are all fossilized as variants, which means that they
    // carrying their own layout information. We can exploit this fact
    // to get from the raw pointer that was passed in to a `Fossil::AnyValPtr`
    // that includes the layout information that a `Fossil::SerialReader`
    // needs.
    //
    Fossil::AnyValPtr contentValPtr = getVariantContentPtr(fossilizedDecl);

    // One subtle issue is that when we call `serialize()` below to read
    // a `Decl*`, the `SerialReader` wants to *read* a pointer to the
    // serialized object from its current cursor position. But what we have
    // is a pointer to the object... not a pointer to a *pointer* to the object.
    //
    // We thus tweak the `InitialStateType` used for the `SerialReader` to
    // tell it that it should treat our `contentValPtr` as if there was
    // an additional level of pointer indirection above it.
    //
    Fossil::SerialReader reader(
        _readContext,
        contentValPtr,
        Fossil::SerialReader::InitialStateType::PseudoPtr);
    ASTSerializer serializer(&reader, this);

    Decl* decl = nullptr;
    serialize(serializer, decl);
    return decl;
}

//
// We now turn our attention to the various accessors on AST types
// that need to read from the serialized data on-demand.
//
// One key design choice in the current encoding is that the
// fossilized dictionaries that we will use for lookup operations
// are written as ordinary `FossilizedDictionary<K,V>` values (which
// are ultimately just flat arrays of `K`,`V` pairs), but have their
// keys sorted before being written out.
//
// We can thus look up entries in these fossilized dictionaries
// using a binary search.
//

template<typename T>
T const* _findEntryInFossilizedDictionaryWithSortedKeys(
    FossilizedDictionary<FossilizedString, T> const& dictionary,
    UnownedStringSlice const& key)
{
    Index lo = 0;
    Index hi = dictionary.getElementCount() - 1;

    auto elements = dictionary.getBuffer();

    while (lo <= hi)
    {
        Index mid = lo + ((hi - lo) >> 1);

        auto element = elements + mid;
        int cmp = compare(element->key, key);
        if (cmp == 0)
            return &element->value;

        if (cmp < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }

    return nullptr;
}

Decl* ModuleDecl::_findSerializedDeclByMangledExportName(UnownedStringSlice const& mangledName)
{
    // Each of the accessors defined in this file should only
    // ever be invoked in the case where the corresponding
    // AST node is using on-demand deserialization.
    //
    SLANG_ASSERT(isUsingOnDemandDeserializationForExports());

    // The `context` pointer stored in the `ContainerDeclDirectMemberDecls` type is
    // a raw `RefPtr<RefObject>`, so that the definition of the `ASTSerialReadContext`
    // doesn't need to be exposed outside this file.
    //
    // In order to access the context pointer, we thus need to cast it.
    //
    auto sharedContext =
        as<ASTSerialReadContext>(_directMemberDecls.onDemandDeserialization.context);

    // The `sharedContext` has the information needed to do lookup
    // based on mangled names, so we delegate the actual work to it.
    //
    return sharedContext->findExportedDeclByMangledName(mangledName);
}

Decl* ASTSerialReadContext::findExportedDeclByMangledName(UnownedStringSlice const& mangledName)
{
    // The read context has retained a pointer to the `Fossilized<ASTModuleInfo>`,
    // which allows us to perform a lookup in the serialized `mapMangledNameToDecl`
    // without ever deserializing it.
    //
    auto found = _findEntryInFossilizedDictionaryWithSortedKeys(
        _fossilizedModuleInfo->mapMangledNameToDecl,
        mangledName);
    if (!found)
        return nullptr;

    // If the given `mangledName` does indeed map to a pointer to
    // a fossilized declaration, then we will read the declaration
    // on-demand before returing it.
    //
    // Note that if we've seen the same declaration before (whether
    // via a previous call to `findExportedDeclByMangledName()` or
    // through some other path leading to `readFossilizedDecl()`,
    // this will return the same `Decl*` that was previously
    // deserialized).
    //
    auto decl = readFossilizedDecl(*found);
    return decl;
}

Decl* ContainerDeclDirectMemberDecls::_readSerializedDeclsOfName(Name* name) const
{
    // All of these accessors on `ContainerDeclDirectMemberDecls` start with
    // a similar pattern of asserting that they are only used when on-demand
    // deserialization is active, and then casting the `void*` that is stored
    // in the AST representation over to the correct fossilized type (a type
    // that is only used/visible within this file).
    //
    SLANG_ASSERT(isUsingOnDemandDeserialization());
    auto& fossilizedInfo =
        *(Fossilized<ContainerDeclDirectMemberDeclsInfo>*)onDemandDeserialization.data;

    // TODO: It isn't clear why the compiler will sometimes perform by-name
    // lookup using a null name, but it happens and thus the code here
    // needs to be defensive against that scenario.
    //
    if (name == nullptr)
        return nullptr;

    // Once we are sure that `name` is valid, the overall logic here
    // is quite similar to `findExportedDeclByMangledName()` above:
    // we do a lookup in the serialized dictionary by binary search.
    //
    auto found = _findEntryInFossilizedDictionaryWithSortedKeys(
        fossilizedInfo.mapNameToDeclIndex,
        name->text.getUnownedSlice());
    if (!found)
        return nullptr;

    // Unlike the case for `findExportedDeclByMangledName()`, the
    // dictionary stored on a container declaration only holds indices
    // rather than pointers. The reason for this is that we want to
    // bottleneck deserialization of direct member declarations through
    // the by-index accessor, when possilbe.
    //
    // One thing to note here is that this function is being called
    // to get the list of *all* declarations with a given name, but
    // it seems to only fetch one. In practice this works fine because
    // the `_prevInContainerWithSameName` field in `Decl` is part of
    // the state that gets serialized for a `Decl`, so that loading
    // the head of the linked list will cause the rest to get
    // deserialized eagerly.
    //
    // TODO: We could avoid serializing the `_prevInContainerWithSameName`
    // field on every `Decl` (since it is almost always null), and instead
    // use a more complicated lookup structure here. E.g., the dictionary
    // entries could either refer to a single declaration by index (the
    // common case, we hope) or to a sequence of two or more indices stored
    // in some side-band structure (in the case where multiple declarations
    // have the same name). For now we are sticking with the simpler
    // representation; further complexity would need to be motivated by
    // profiling information showing there's a problem to be solved.
    //
    Index declIndex = *found;
    return getDecl(declIndex);
}

void ContainerDeclDirectMemberDecls::_readSerializedTransparentDecls() const
{
    SLANG_ASSERT(isUsingOnDemandDeserialization());
    auto& fossilizedInfo =
        *(Fossilized<ContainerDeclDirectMemberDeclsInfo>*)onDemandDeserialization.data;

    // This particular function works by filling in the `filteredListOfTransparentDecls`
    // part of the lookup accelerators. If it has been called once and put anything
    // into that array, then `filteredListOfTransparentDecls` should be used instead
    // of calling this method again. We enforce this invariant in an attempt to
    // avoid overhead that might be associated with this method.
    //
    SLANG_ASSERT(accelerators.filteredListOfTransparentDecls.getCount() == 0);

    // If this is the first time this method is being called (or, in the very common
    // corner case, when there are no transparent decls at all...) we loop over the
    // fossilized array holding the indices of the transparent members.
    //
    for (auto index : fossilizedInfo.transparentDeclIndices)
    {
        // For each index that is found, we do a by-index query for
        // the member and then add it to the list. This is another
        // case of us trying to bottleneck access to members through
        // the by-index accessor as much as possible.
        //
        auto decl = getDecl(index);
        accelerators.filteredListOfTransparentDecls.add(decl);
    }
}

Decl* ContainerDeclDirectMemberDecls::_readSerializedDeclAtIndex(Index index) const
{
    SLANG_ASSERT(isUsingOnDemandDeserialization());
    auto& fossilizedInfo =
        *(Fossilized<ContainerDeclDirectMemberDeclsInfo>*)onDemandDeserialization.data;

    //
    // It isn't visible here, but `ContainerDeclDirectMemberDecls::getDecl(index)`
    // will automatically cache the decl that we return, so that subsequent queries
    // for the same index shouldn't call this method at all (unless we end up
    // returning null, for some unexpected reason).
    //

    // The logic here is fairly simple: we directly read from the array of (fossilized)
    // declaration pointers in the (fossilized) `ContainerDeclDirectMemberDeclsInfo`,
    // to get a pointer to the (fossilized) declaration we want.
    //
    // Note that it is important that the variable here is *not* being declared
    // with `auto`. If we were to simply write `auto fossilizedDecl` then the type
    // that gets inferred would be `Fossilized<Decl*>` which is a `FossilizedPtr<...>`
    // - a 32-bit relative pointer. On systems with a 64-bit address space, it is
    // not guaranteed that a 32-bit offset is enough to refer to part of the serialized
    // AST blob (in the heap) from this local variable (on the stack).
    //
    // The two options are to use `auto&` so that we capture a *reference* to the
    // fossilized pointer (rather than try to copy it), or to declare the variable
    // as an ordinary "live" pointer.
    //
    Fossilized<Decl>* fossilizedDecl = fossilizedInfo.decls[index];

    // Once we have a pointer to the (fossilized) declaration that we want,
    // we can use the `ASTSerialReadContext` to read it on-demand. Because
    // the `context` pointer declared on the actual AST type is untyped
    // (to avoid needing to expose `ASTSerialReadContext` outside this file),
    // we need to cast the pointer before we can perform the read.
    //
    auto sharedContext = as<ASTSerialReadContext>(onDemandDeserialization.context);
    auto decl = sharedContext->readFossilizedDecl(fossilizedDecl);
    return decl;
}

} // namespace Slang
