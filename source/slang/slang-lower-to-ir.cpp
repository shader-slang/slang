// lower.cpp
#include "slang-lower-to-ir.h"

#include "../../slang.h"

#include "slang-check.h"
#include "slang-ir.h"
#include "slang-ir-constexpr.h"
#include "slang-ir-dce.h"
#include "slang-ir-diff-call.h"
#include "slang-ir-diff-jvp.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-missing-return.h"
#include "slang-ir-sccp.h"
#include "slang-ir-ssa.h"
#include "slang-ir-strip.h"
#include "slang-ir-validate.h"
#include "slang-ir-string-hash.h"
#include "slang-ir-clone.h"
#include "slang-ir-lower-error-handling.h"

#include "slang-mangle.h"
#include "slang-type-layout.h"
#include "slang-visitor.h"

namespace Slang
{

// This file implements lowering of the Slang AST to a simpler SSA
// intermediate representation.
//
// IR is generated in a context (`IRGenContext`), which tracks the current
// location in the IR where code should be emitted (e.g., what basic
// block to add instructions to). Lowering a statement will emit some
// number of instructions to the context, and possibly change the
// insertion point (because of control flow).
//
// When lowering an expression we have a more interesting challenge, for
// two main reasons:
//
// 1. There might be types that are representible in the AST, but which
//    we don't want to support natively in the IR. An example is a `struct`
//    type with both ordinary and resource-type members; we might want to
//    split values with such a type into distinct values during lowering.
//
// 2. We need to handle the difference between l-value and r-value expressions,
//    and in particular the fact that HLSL/Slang supports complicated sorts
//    of l-values (e.g., `someVector.zxy` is an l-value, even though it can't
//    be represented by a single pointer), and also allows l-values to appear
//    in multiple contexts (not just the left-hand side of assignment, but
//    also as an argument to match an `out` or `in out` parameter).
//
// Our solution to both of these problems is the same. Rather than having
// the lowering of an expression return a single IR-level value (`IRInst*`),
// we have it return a more complex type (`LoweredValInfo`) which can represent
// a wider range of conceptual "values" which might correspond to multiple IR-level
// values, and/or represent a pointer to an l-value rather than the r-value itself.

// We want to keep the representation of a `LoweringValInfo` relatively light
// - right now it is just a single pointer plus a "tag" to distinguish the cases.
//
// This means that cases that can't fit in a single pointer need a heap allocation
// to store their payload. For simplicity we represent all of these with a class
// hierarchy:
//
struct ExtendedValueInfo : RefObject
{};

// This case is used to indicate a value that is a reference
// to an AST-level subscript declaration.
//
struct SubscriptInfo : ExtendedValueInfo
{
    DeclRef<SubscriptDecl> declRef;
};

// Some cases of `ExtendedValueInfo` need to
// recursively contain `LoweredValInfo`s, and
// so we forward declare them here and fill
// them in later.
//
struct BoundStorageInfo;
struct BoundMemberInfo;
struct SwizzledLValueInfo;
struct CopiedValInfo;
struct ExtractedExistentialValInfo;

// This type is our core representation of lowered values.
// In the simple case, it just wraps an `IRInst*`.
// More complex cases, representing l-values or aggregate
// values are also supported.
struct LoweredValInfo
{
    // Which of the cases of value are we looking at?
    enum class Flavor
    {
        // No value (akin to a null pointer)
        None,

        // A simple IR value
        Simple,

        // An l-value represented as an IR
        // pointer to the value
        Ptr,

        // A member declaration bound to a particular `this` value
        BoundMember,

        // A reference to an AST-level subscript operation
        Subscript,

        // An AST-level subscript operation bound to a particular
        // object and arguments.
        BoundStorage,

        // The result of applying swizzling to an l-value
        SwizzledLValue,

        // The value extracted from an opened existential
        ExtractedExistential,
    };

    union
    {
        IRInst*            val;
        ExtendedValueInfo*  ext;
    };
    Flavor flavor;

    LoweredValInfo()
    {
        flavor = Flavor::None;
        val = nullptr;
    }

    LoweredValInfo(IRType* t)
    {
        flavor = Flavor::Simple;
        val = t;
    }

    static LoweredValInfo simple(IRInst* v)
    {
        LoweredValInfo info;
        info.flavor = Flavor::Simple;
        info.val = v;
        return info;
    }

    static LoweredValInfo ptr(IRInst* v)
    {
        LoweredValInfo info;
        info.flavor = Flavor::Ptr;
        info.val = v;
        return info;
    }

    static LoweredValInfo boundMember(
        BoundMemberInfo*    boundMemberInfo);

    BoundMemberInfo* getBoundMemberInfo()
    {
        SLANG_ASSERT(flavor == Flavor::BoundMember);
        return (BoundMemberInfo*)ext;
    }

    static LoweredValInfo subscript(
        SubscriptInfo* subscriptInfo);

    SubscriptInfo* getSubscriptInfo()
    {
        SLANG_ASSERT(flavor == Flavor::Subscript);
        return (SubscriptInfo*)ext;
    }

    static LoweredValInfo boundStorage(
        BoundStorageInfo* boundStorageInfo);

    BoundStorageInfo* getBoundStorageInfo()
    {
        SLANG_ASSERT(flavor == Flavor::BoundStorage);
        return (BoundStorageInfo*)ext;
    }

    static LoweredValInfo swizzledLValue(
        SwizzledLValueInfo* extInfo);

    SwizzledLValueInfo* getSwizzledLValueInfo()
    {
        SLANG_ASSERT(flavor == Flavor::SwizzledLValue);
        return (SwizzledLValueInfo*)ext;
    }

    static LoweredValInfo extractedExistential(
        ExtractedExistentialValInfo* extInfo);

    ExtractedExistentialValInfo* getExtractedExistentialValInfo()
    {
        SLANG_ASSERT(flavor == Flavor::ExtractedExistential);
        return (ExtractedExistentialValInfo*)ext;
    }
};

// This case is used to indicate a reference to an AST-level
// operation that accesses abstract storage.
//
// This could be an invocation of a `subscript` declaration,
// with argument representing an index or indices:
//
//     RWStructuredBuffer<Foo> gBuffer;
//     ... gBuffer[someIndex] ...
//
// the expression `gBuffer[someIndex]` will be lowered to
// a value that references `RWStructureBuffer<Foo>::operator[]`
// with arguments `(gBuffer, someIndex)`.
//
// This could also be an reference to a `property` declaration,
// with no arguments:
//
//      struct Sphere { property radius : int { get { ... } } }
//      Sphere sphere;
//      ... sphere.radius ...
//
// the expression `sphere.radius` will be lowered to a value
// that references `Sphere::radius` with arguments `(sphere)`.
//
// Such a value can be an l-value, and depending on the context
// where it is used, can lower into a call to either the getter
// or setter operations of the storage.
//
struct BoundStorageInfo : ExtendedValueInfo
{
        /// The declaration of the abstract storage (subscript or property)
    DeclRef<ContainerDecl>  declRef;

        /// The IR-level type of the stored value
    IRType*                 type;

        /// The base value/object on which storage is being accessed
    LoweredValInfo          base;

        /// Additional arguments required to reify a reference to the storage
    List<IRInst*>           additionalArgs;
};


// Represents some declaration bound to a particular
// object. For example, if we had `obj.f` where `f`
// is a member function, we'd use a `BoundMemberInfo`
// to represnet this.
//
// Note: This case is largely avoided by special-casing
// in the handling of calls (like `obj.f(arg)`), but
// it is being left here as an example of what we might
// need/want to do in the long term.
struct BoundMemberInfo : ExtendedValueInfo
{
    // The base object
    LoweredValInfo  base;

    // The (AST-level) declaration reference.
    DeclRef<Decl> declRef;

    // The type of this value
    IRType* type;
};

// Represents the result of a swizzle operation in
// an l-value context. A swizzle without duplicate
// elements is allowed as an l-value, even if the
// element are non-contiguous (`.xz`) or out of
// order (`.zxy`).
//
struct SwizzledLValueInfo : ExtendedValueInfo
{
    // The type of the expression.
    IRType*         type;

    // The base expression (this should be an l-value)
    LoweredValInfo  base;

    // The number of elements in the swizzle
    UInt            elementCount;

    // THe indices for the elements being swizzled
    UInt            elementIndices[4];
};

// Represents the results of extractng a value of
// some (statically unknown) concrete type from
// an existential, in an l-value context.
//
struct ExtractedExistentialValInfo : ExtendedValueInfo
{
    // The extracted value
    IRInst* extractedVal;

    // The original existential value
    LoweredValInfo existentialVal;

    // The type of `existentialVal`
    IRType* existentialType;

    // The IR witness table for the conformance of
    // the type of `extractedVal` to `existentialType`
    //
    IRInst* witnessTable;
};

LoweredValInfo LoweredValInfo::boundMember(
    BoundMemberInfo*    boundMemberInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::BoundMember;
    info.ext = boundMemberInfo;
    return info;
}

LoweredValInfo LoweredValInfo::subscript(
    SubscriptInfo* subscriptInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::Subscript;
    info.ext = subscriptInfo;
    return info;
}

LoweredValInfo LoweredValInfo::boundStorage(
    BoundStorageInfo* boundStorageInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::BoundStorage;
    info.ext = boundStorageInfo;
    return info;
}

LoweredValInfo LoweredValInfo::swizzledLValue(
        SwizzledLValueInfo* extInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::SwizzledLValue;
    info.ext = extInfo;
    return info;
}

LoweredValInfo LoweredValInfo::extractedExistential(
    ExtractedExistentialValInfo* extInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::ExtractedExistential;
    info.ext = extInfo;
    return info;
}


// An "environment" for mapping AST declarations to IR values.
//
// This is required because in some cases we might lower the
// same AST declaration to the IR multiple times (e.g., when
// a generic transitively contains multiple functions, we
// will emit a distinct IR generic for each function, with
// its own copies of the generic parameters).
//
struct IRGenEnv
{
    // Map an AST-level declaration to the IR-level value that represents it.
    Dictionary<Decl*, LoweredValInfo> mapDeclToValue;

    // The next outer env around this one
    IRGenEnv*   outer = nullptr;
};

struct SharedIRGenContext
{
    SharedIRGenContext(
        Session*        session,
        DiagnosticSink* sink,
        bool obfuscateCode, 
        ModuleDecl*     mainModuleDecl = nullptr)
        : m_session(session)
        , m_sink(sink)
        , m_obfuscateCode(obfuscateCode)
        , m_mainModuleDecl(mainModuleDecl)
    {}

    Session*        m_session = nullptr;
    DiagnosticSink* m_sink = nullptr;
    bool            m_obfuscateCode = false;
    ModuleDecl*     m_mainModuleDecl = nullptr;

    // The "global" environment for mapping declarations to their IR values.
    IRGenEnv globalEnv;

    // Map an AST-level declaration of an interface
    // requirement to the IR-level "key" that
    // is used to fetch that requirement from a
    // witness table.
    Dictionary<Decl*, IRStructKey*> interfaceRequirementKeys;

    // Arrays we keep around strictly for memory-management purposes:

    // Any extended values created during lowering need
    // to be cleaned up after the fact. We don't try
    // to reference-count these along the way because
    // they need to get stored into a `union` inside `LoweredValInfo`
    List<RefPtr<ExtendedValueInfo>> extValues;

    // Map from an AST-level statement that can be
    // used as the target of a `break` or `continue`
    // to the appropriate basic block to jump to.
    Dictionary<Stmt*, IRBlock*> breakLabels;
    Dictionary<Stmt*, IRBlock*> continueLabels;

    // List of all string literals used in user code, regardless
    // of how they were used (i.e., whether or not they were hashed).
    //
    // This does *not* collect:
    // * String literals that were only used for attributes/modifiers in
    //   the user's code (e.g., `"compute"` in `[shader("compute")]`)
    // * Any IR string literals constructed for the purpose of decorations,
    //   reflection, or other meta-data that did not appear as a literal
    //   in the source code.
    //
    List<IRInst*> m_stringLiterals;
};


struct IRGenContext
{
    ASTBuilder* astBuilder;

    // Shared state for the IR generation process
    SharedIRGenContext* shared;

    // environment for mapping AST decls to IR values
    IRGenEnv*   env;

    // IR builder to use when building code under this context
    IRBuilder* irBuilder;

    // The value to use for any `this` expressions
    // that appear in the current context.
    //
    // TODO: If we ever allow nesting of (non-static)
    // types, then we may need to support references
    // to an "outer `this`", and this representation
    // might be insufficient.
    LoweredValInfo thisVal;

    // The IRType value to lower into for `ThisType`.
    IRInst* thisType = nullptr;

    // The IR witness value to use for `ThisType`
    IRInst* thisTypeWitness = nullptr;

    explicit IRGenContext(SharedIRGenContext* inShared, ASTBuilder* inAstBuilder)
        : shared(inShared)
        , astBuilder(inAstBuilder)
        , env(&inShared->globalEnv)
        , irBuilder(nullptr)
    {}

    Session* getSession()
    {
        return shared->m_session;
    }

    DiagnosticSink* getSink()
    {
        return shared->m_sink;
    }

    ModuleDecl* getMainModuleDecl()
    {
        return shared->m_mainModuleDecl;
    }

    LoweredValInfo* findLoweredDecl(Decl* decl)
    {
        IRGenEnv* envToFindIn = env;
        while (envToFindIn)
        {
            if (auto rs = envToFindIn->mapDeclToValue.TryGetValue(decl))
                return rs;
            envToFindIn = envToFindIn->outer;
        }
        return nullptr;
    }
};

void setGlobalValue(SharedIRGenContext* sharedContext, Decl* decl, LoweredValInfo value)
{
    sharedContext->globalEnv.mapDeclToValue[decl] = value;
}

void setGlobalValue(IRGenContext* context, Decl* decl, LoweredValInfo value)
{
    setGlobalValue(context->shared, decl, value);
}

void setValue(IRGenContext* context, Decl* decl, LoweredValInfo value)
{
    context->env->mapDeclToValue[decl] = value;
}

ModuleDecl* findModuleDecl(Decl* decl)
{
    for (auto dd = decl; dd; dd = dd->parentDecl)
    {
        if (auto moduleDecl = as<ModuleDecl>(dd))
            return moduleDecl;
    }
    return nullptr;
}

bool isFromStdLib(Decl* decl)
{
    for (auto dd = decl; dd; dd = dd->parentDecl)
    {
        if (dd->hasModifier<FromStdLibModifier>())
            return true;
    }
    return false;
}

bool isImportedDecl(IRGenContext* context, Decl* decl)
{
    // If the declaration has the extern attribute then it must be imported
    // from another module
    //
    // The [__extern] attribute is a very special case feature (aka "a hack") that allows a symbol to be declared
    // as if it is part of the current module for AST purposes, but then expects to be imported from another IR module.
    // For that linkage to work, both the exporting and importing modules must have the same name (which would
    // usually indicate that they are the same module).
    //
    // Note that in practice for matching during linking uses the fully qualified name - including module name.
    // Thus using extern __attribute isn't useful for symbols that are imported via `import`, only symbols
    // that notionally come from the same module but are split into separate compilations (as can be done with -module-name)
    if (decl->findModifier<ExternAttribute>())
    {
        return true;
    }

    ModuleDecl* moduleDecl = findModuleDecl(decl);
    if (!moduleDecl)
        return false;

    // HACK: don't treat standard library code as
    // being imported for right now, just because
    // we don't load its IR in the same way as
    // for other imports.
    //
    // TODO: Fix this the right way, by having standard
    // library declarations have IR modules that we link
    // in via the normal means.
    if (isFromStdLib(decl))
        return false;

    if (moduleDecl != context->getMainModuleDecl())
        return true;

    return false;
}

    /// Is `decl` a function that should be force-inlined early in compilation (before linking)?
static bool isForceInlineEarly(Decl* decl)
{
    if(decl->hasModifier<UnsafeForceInlineEarlyAttribute>())
        return true;

    return false;
}

    /// Should the given `decl` nested in `parentDecl` be treated as a static rather than instance declaration?
bool isEffectivelyStatic(
    Decl*           decl,
    ContainerDecl*  parentDecl);

bool isStdLibMemberFuncDecl(
    Decl*   decl);

// Ensure that a version of the given declaration has been emitted to the IR
LoweredValInfo ensureDecl(
    IRGenContext*   context,
    Decl*           decl);

// Emit code as needed to construct a reference to the given declaration with
// any needed specializations in place.
LoweredValInfo emitDeclRef(
    IRGenContext*   context,
    DeclRef<Decl>   declRef,
    IRType*         type);

IRInst* getSimpleVal(IRGenContext* context, LoweredValInfo lowered);

int32_t getIntrinsicOp(
    Decl*                   decl,
    IntrinsicOpModifier*    intrinsicOpMod)
{
    int32_t op = intrinsicOpMod->op;
    if(op != 0)
        return op;

    // No specified modifier? Then we need to look it up
    // based on the name of the declaration...

    auto name = decl->getName();
    auto nameText = getUnownedStringSliceText(name);

    IROp irOp = findIROp(nameText);
    SLANG_ASSERT(irOp != kIROp_Invalid);
    SLANG_ASSERT(int32_t(irOp) >= 0);
    return int32_t(irOp);
}

struct TryClauseEnvironment
{
    TryClauseType clauseType = TryClauseType::None;
    IRBlock* catchBlock = nullptr;
};

// Given a `LoweredValInfo` for something callable, along with a
// bunch of arguments, emit an appropriate call to it.
LoweredValInfo emitCallToVal(
    IRGenContext*   context,
    IRType*         type,
    LoweredValInfo  funcVal,
    UInt            argCount,
    IRInst* const*  args,
    const TryClauseEnvironment& tryEnv)
{
    auto builder = context->irBuilder;
    switch (funcVal.flavor)
    {
    case LoweredValInfo::Flavor::None:
        SLANG_UNEXPECTED("null function");
    default:
        switch (tryEnv.clauseType)
        {
        case TryClauseType::None:
            return LoweredValInfo::simple(
                builder->emitCallInst(type, getSimpleVal(context, funcVal), argCount, args));

        case TryClauseType::Standard:
            {
                auto callee = getSimpleVal(context, funcVal);
                auto succBlock = builder->createBlock();
                auto failBlock = builder->createBlock();
                auto funcType = as<IRFuncType>(callee->getDataType());
                auto throwAttr = funcType->findAttr<IRFuncThrowTypeAttr>();
                assert(throwAttr);
                auto voidType = builder->getVoidType();
                builder->emitTryCallInst(voidType, succBlock, failBlock, callee, argCount, args);
                builder->insertBlock(failBlock);
                auto errParam = builder->emitParam(throwAttr->getErrorType());
                builder->emitThrow(errParam);
                builder->insertBlock(succBlock);
                auto value = builder->emitParam(type);
                return LoweredValInfo::simple(value);
            }
            break;
        default:
            SLANG_UNIMPLEMENTED_X("emitCallToVal(tryClauseType)");
        }
    }
}

LoweredValInfo lowerRValueExpr(
    IRGenContext*   context,
    Expr*           expr);

IRType* lowerType(
    IRGenContext*   context,
    Type*           type);

static IRType* lowerType(
    IRGenContext*   context,
    QualType const& type)
{
    return lowerType(context, type.type);
}

// Given a `DeclRef` for something callable, along with a bunch of
// arguments, emit an appropriate call to it.
LoweredValInfo emitCallToDeclRef(
    IRGenContext*   context,
    IRType*         type,
    DeclRef<Decl>   funcDeclRef,
    IRType*         funcType,
    UInt            argCount,
    IRInst* const*  args,
    const TryClauseEnvironment& tryEnv)
{
    SLANG_ASSERT(funcType);

    auto builder = context->irBuilder;

    auto funcDecl = funcDeclRef.getDecl();
    if(auto intrinsicOpModifier = funcDecl->findModifier<IntrinsicOpModifier>())
    {
        // The intrinsic op maps to a single IR instruction,
        // so we will emit an instruction with the chosen
        // opcode, and the arguments to the call as its operands.
        //
        auto intrinsicOp = getIntrinsicOp(funcDecl, intrinsicOpModifier);
        return LoweredValInfo::simple(builder->emitIntrinsicInst(
            type,
            IROp(intrinsicOp),
            argCount,
            args));
    }

    if( auto ctorDeclRef = funcDeclRef.as<ConstructorDecl>() )
    {
        if(!ctorDeclRef.getDecl()->body && isFromStdLib(ctorDeclRef.decl))
        {
            // HACK: For legacy reasons, all of the built-in initializers
            // in the standard library are declared without proper
            // intrinsic-op modifiers, so we will assume that an
            // initializer without a body should map to `kIROp_Construct`.
            //
            // TODO: We should make all the initializers in the
            // standard library have either a body or a proper
            // intrinsic-op modifier.
            //
            // TODO: We should eliminate `kIROp_Construct` from the
            // IR completely, in favor of more detailed/specific ops
            // that cover the cases we actually care about.
            //
            return LoweredValInfo::simple(builder->emitConstructorInst(type, argCount, args));
        }
    }

    // Fallback case is to emit an actual call.
    //
    LoweredValInfo funcVal = emitDeclRef(context, funcDeclRef, funcType);
    return emitCallToVal(context, type, funcVal, argCount, args, tryEnv);
}

LoweredValInfo emitCallToDeclRef(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<Decl>           funcDeclRef,
    IRType*                 funcType,
    List<IRInst*> const&    args,
    const TryClauseEnvironment& tryEnv)
{
    return emitCallToDeclRef(
        context,
        type,
        funcDeclRef,
        funcType,
        args.getCount(),
        args.getBuffer(),
        tryEnv);
}

    /// Emit a call to the given `accessorDeclRef`.
    ///
    /// The `base` value represents the object on which the accessor is being invoked.
    /// The `args` represent any additional arguments to the accessor. This could be
    /// because we are invoking a subscript accessor (so the args include any index value(s)),
    /// and/or because we are invoking a setter (so that the args include the new value
    /// to be set).
    ///
static LoweredValInfo _emitCallToAccessor(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<AccessorDecl>   accessorDeclRef,
    LoweredValInfo          base,
    UInt                    argCount,
    IRInst* const*          args);

static LoweredValInfo _emitCallToAccessor(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<AccessorDecl>   accessorDeclRef,
    LoweredValInfo          base,
    List<IRInst*> const&    args)
{
    return _emitCallToAccessor(context, type, accessorDeclRef, base, args.getCount(), args.getBuffer());
}

    /// Lower a reference to abstract storage (a property or subscript).
    ///
    /// The given `storageDeclRef` is being accessed on some `base` value,
    /// to yield a value of some expected `type`. The additional `args`
    /// are only needed in the case of a subscript declaration (for
    /// a property, `argCount` should be zero).
    ///
    /// In the case where there is only a `get` accessor, this function
    /// will go ahead and invoke it to produce a value here and now.
    /// Otherwise, it will produce an abstract `LoweredValInfo` that
    /// encapsulates the reference to the storage so that downstream
    /// code can decide which accessor(s) to invoke.
    ///
static LoweredValInfo lowerStorageReference(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<ContainerDecl>  storageDeclRef,
    LoweredValInfo          base,
    UInt                    argCount,
    IRInst* const*          args)
{
    DeclRef<GetterDecl> getterDeclRef;
    bool justAGetter = true;
    for (auto accessorDeclRef : getMembersOfType<AccessorDecl>(storageDeclRef, MemberFilterStyle::Instance))
    {
        // We want to track whether this storage has any accessors other than
        // `get` (assuming that everything except `get` can be used for setting...).

        if (auto foundGetterDeclRef = accessorDeclRef.as<GetterDecl>())
        {
            // We found a getter.
            getterDeclRef = foundGetterDeclRef;
        }
        else
        {
            // There was something other than a getter, so we can't
            // invoke an accessor just now.
            justAGetter = false;
        }
    }

    if (!justAGetter || !getterDeclRef)
    {
        // We can't perform an actual call right now, because
        // this expression might appear in an r-value or l-value
        // position (or *both* if it is being passed as an argument
        // for an `in out` parameter!).
        //
        // Instead, we will construct a special-case value to
        // represent the latent access operation (abstractly
        // this is a reference to a storage location).

        // The abstract storage location will need to include
        // all the arguments being passed in the case of a subscript operation.

        RefPtr<BoundStorageInfo> boundStorage = new BoundStorageInfo();
        boundStorage->declRef = storageDeclRef;
        boundStorage->type = type;
        boundStorage->base = base;
        boundStorage->additionalArgs.addRange(args, argCount);

        context->shared->extValues.add(boundStorage);

        return LoweredValInfo::boundStorage(boundStorage);
    }

    return _emitCallToAccessor(context, type, getterDeclRef, base, argCount, args);
}

IRInst* getFieldKey(
    IRGenContext*   context,
    DeclRef<Decl>   field)
{
    return getSimpleVal(context, emitDeclRef(context, field, context->irBuilder->getKeyType()));
}

LoweredValInfo extractField(
    IRGenContext*   context,
    IRType*         fieldType,
    LoweredValInfo  base,
    DeclRef<Decl>   field)
{
    IRBuilder* builder = context->irBuilder;

    switch (base.flavor)
    {
    default:
        {
            IRInst* irBase = getSimpleVal(context, base);
            return LoweredValInfo::simple(
                builder->emitFieldExtract(
                    fieldType,
                    irBase,
                    getFieldKey(context, field)));
        }
        break;

    case LoweredValInfo::Flavor::BoundMember:
    case LoweredValInfo::Flavor::BoundStorage:
        {
            // The base value is one that is trying to defer a get-vs-set
            // decision, so we will need to do the same.

            RefPtr<BoundMemberInfo> boundMemberInfo = new BoundMemberInfo();
            boundMemberInfo->type = fieldType;
            boundMemberInfo->base = base;
            boundMemberInfo->declRef = field;

            context->shared->extValues.add(boundMemberInfo);
            return LoweredValInfo::boundMember(boundMemberInfo);
        }
        break;

    case LoweredValInfo::Flavor::Ptr:
        {
            // We are "extracting" a field from an lvalue address,
            // which means we should just compute an lvalue
            // representing the field address.
            IRInst* irBasePtr = base.val;
            return LoweredValInfo::ptr(
                builder->emitFieldAddress(
                    builder->getPtrType(fieldType),
                    irBasePtr,
                    getFieldKey(context, field)));
        }
        break;
    }
}



LoweredValInfo materialize(
    IRGenContext*   context,
    LoweredValInfo  lowered)
{
    auto builder = context->irBuilder;

top:
    switch(lowered.flavor)
    {
    case LoweredValInfo::Flavor::None:
    case LoweredValInfo::Flavor::Simple:
    case LoweredValInfo::Flavor::Ptr:
        return lowered;

    case LoweredValInfo::Flavor::BoundStorage:
        {
            auto boundStorageInfo = lowered.getBoundStorageInfo();

            // We are being asked to extract a value from a subscript call
            // (e.g., `base[index]`). We will first check if the subscript
            // declared a getter and use that if possible, and then fall
            // back to a `ref` accessor if one is defined.
            //
            // (Picking the `get` over the `ref` accessor simplifies things
            // in case the `get` operation has a natural translation for
            // a target, while the general `ref` case does not...)

            auto getters = getMembersOfType<GetterDecl>(boundStorageInfo->declRef, MemberFilterStyle::Instance);
            if (getters.getCount())
            {
                auto getter = *getters.begin();
                lowered = _emitCallToAccessor(
                    context,
                    boundStorageInfo->type,
                    getter,
                    boundStorageInfo->base,
                    boundStorageInfo->additionalArgs);
                goto top;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(boundStorageInfo->declRef, MemberFilterStyle::Instance);
            if(refAccessors.getCount())
            {
                auto refAccessor = *refAccessors.begin();

                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(boundStorageInfo->type);

                LoweredValInfo refVal = _emitCallToAccessor(
                    context,
                    ptrType,
                    refAccessor,
                    boundStorageInfo->base,
                    boundStorageInfo->additionalArgs);

                // The result from the call needs to be implicitly dereferenced,
                // so that it can work as an l-value of the desired result type.
                lowered = LoweredValInfo::ptr(getSimpleVal(context, refVal));

                goto top;
            }

            SLANG_UNEXPECTED("subscript had no getter");
            UNREACHABLE_RETURN(LoweredValInfo());
        }
        break;

    case LoweredValInfo::Flavor::BoundMember:
        {
            auto boundMemberInfo = lowered.getBoundMemberInfo();
            auto base = materialize(context, boundMemberInfo->base);

            auto declRef = boundMemberInfo->declRef;
            if( auto fieldDeclRef = declRef.as<VarDecl>() )
            {
                lowered = extractField(context, boundMemberInfo->type, base, fieldDeclRef);
                goto top;
            }
            else
            {

                SLANG_UNEXPECTED("unexpected member flavor");
                UNREACHABLE_RETURN(LoweredValInfo());
            }
        }
        break;

    case LoweredValInfo::Flavor::SwizzledLValue:
        {
            auto swizzleInfo = lowered.getSwizzledLValueInfo();

            return LoweredValInfo::simple(builder->emitSwizzle(
                swizzleInfo->type,
                getSimpleVal(context, swizzleInfo->base),
                swizzleInfo->elementCount,
                swizzleInfo->elementIndices));
        }

    case LoweredValInfo::Flavor::ExtractedExistential:
        {
            auto info = lowered.getExtractedExistentialValInfo();

            return LoweredValInfo::simple(info->extractedVal);
        }

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

}

IRInst* getSimpleVal(IRGenContext* context, LoweredValInfo lowered)
{
    auto builder = context->irBuilder;

    // First, try to eliminate any "bound" operations along the chain,
    // so that we are dealing with an ordinary value, or an l-value pointer.
    lowered = materialize(context, lowered);

    switch(lowered.flavor)
    {
    case LoweredValInfo::Flavor::None:
        return nullptr;

    case LoweredValInfo::Flavor::Simple:
        return lowered.val;

    case LoweredValInfo::Flavor::Ptr:
        return builder->emitLoad(lowered.val);

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        UNREACHABLE_RETURN(nullptr);
    }
}

LoweredValInfo lowerVal(
    IRGenContext*   context,
    Val*            val);

IRInst* lowerSimpleVal(
    IRGenContext*   context,
    Val*            val)
{
    auto lowered = lowerVal(context, val);
    return getSimpleVal(context, lowered);
}

LoweredValInfo lowerLValueExpr(
    IRGenContext*   context,
    Expr*           expr);

void assign(
    IRGenContext*           context,
    LoweredValInfo const&   left,
    LoweredValInfo const&   right);

IRInst* getAddress(
    IRGenContext*           context,
    LoweredValInfo const&   inVal,
    SourceLoc               diagnosticLocation);

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt);

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    DeclBase*       decl);

IRType* getIntType(
    IRGenContext* context)
{
    return context->irBuilder->getBasicType(BaseType::Int);
}

static IRGeneric* getOuterGeneric(IRInst* gv)
{
    auto parentBlock = as<IRBlock>(gv->getParent());
    if (!parentBlock) return nullptr;

    auto parentGeneric = as<IRGeneric>(parentBlock->getParent());
    return parentGeneric;
}

static void addLinkageDecoration(
    IRGenContext*               context,
    IRInst*                     inInst,
    Decl*                       decl,
    UnownedStringSlice const&   mangledName)
{
    // If the instruction is nested inside one or more generics,
    // then the mangled name should really apply to the outer-most
    // generic, and not the declaration nested inside.

    auto builder = context->irBuilder;

    IRInst* inst = inInst;
    while (auto outerGeneric = getOuterGeneric(inst))
    {
        inst = outerGeneric;
    }

    if (isImportedDecl(context, decl))
    {
        builder->addImportDecoration(inst, mangledName);
    }
    else
    {
        builder->addExportDecoration(inst, mangledName);
    }
    if (decl->findModifier<PublicModifier>())
    {
        builder->addPublicDecoration(inst);
        builder->addKeepAliveDecoration(inst);
    }
    if (decl->findModifier<HLSLExportModifier>())
    {
        builder->addHLSLExportDecoration(inst);
        builder->addKeepAliveDecoration(inst);
    }
    if (decl->findModifier<ExternCppModifier>())
    {
        builder->addExternCppDecoration(inst, mangledName);
    }
    if (decl->findModifier<JVPDerivativeModifier>())
    {
        builder->addJVPDerivativeMarkerDecoration(inst);
    }
    if (as<InterfaceDecl>(decl->parentDecl) &&
        decl->parentDecl->hasModifier<ComInterfaceAttribute>())
    {
        builder->addExternCppDecoration(inst, decl->getName()->text.getUnownedSlice());
    }
    if (auto dllImportModifier = decl->findModifier<DllImportAttribute>())
    {
        auto libraryName = dllImportModifier->modulePath;
        builder->addDllImportDecoration(inst, libraryName.getUnownedSlice(), decl->getName()->text.getUnownedSlice());
    }
}

static void addLinkageDecoration(
    IRGenContext*               context,
    IRInst*                     inst,
    Decl*                       decl)
{
     String mangledName = getMangledName(context->astBuilder, decl);

     if (context->shared->m_obfuscateCode)
     {
         mangledName = getHashedName(mangledName.getUnownedSlice());
     }

    addLinkageDecoration(context, inst, decl, mangledName.getUnownedSlice());
}

IRStructKey* getInterfaceRequirementKey(
    IRGenContext*   context,
    Decl*           requirementDecl)
{
    // TODO: this special case logic can be removed if we also clean up `doesGenericSignatureMatchRequirement`
    // Currently `doesGenericSignatureMatchRequirement` will use the inner func decl as the key
    // in AST WitnessTable. Therefore we need to match this behavior by always using the inner
    // decl as the requirement key.
    if (auto genericDecl = as<GenericDecl>(requirementDecl))
        return getInterfaceRequirementKey(context, genericDecl->inner);
    IRStructKey* requirementKey = nullptr;
    if(context->shared->interfaceRequirementKeys.TryGetValue(requirementDecl, requirementKey))
    {
        return requirementKey;
    }

    IRBuilder builderStorage = *context->irBuilder;
    auto builder = &builderStorage;

    builder->setInsertInto(builder->getModule());

    // Construct a key to serve as the representation of
    // this requirement in the IR, and to allow lookup
    // into the declaration.
    requirementKey = builder->createStructKey();

    addLinkageDecoration(context, requirementKey, requirementDecl);

    context->shared->interfaceRequirementKeys.Add(requirementDecl, requirementKey);

    return requirementKey;
}

void getGenericTypeConformances(IRGenContext* context, ShortList<IRType*>& supTypes, Decl* genericParamDecl)
{
    auto parent = genericParamDecl->parentDecl;
    if (parent)
    {
        for (auto typeConstraint : parent->getMembersOfType<GenericTypeConstraintDecl>())
        {
            if (auto declRefType = as<DeclRefType>(typeConstraint->sub.type))
            {
                if (declRefType->declRef.decl == genericParamDecl)
                {
                    supTypes.add(lowerType(context, typeConstraint->getSup().type));
                }
            }
        }
    }
}

//

struct ValLoweringVisitor : ValVisitor<ValLoweringVisitor, LoweredValInfo, LoweredValInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    LoweredValInfo visitVal(Val* /*val*/)
    {
        SLANG_UNIMPLEMENTED_X("value lowering");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitGenericParamIntVal(GenericParamIntVal* val)
    {
        return emitDeclRef(context, val->declRef,
            lowerType(context, getType(context->astBuilder, val->declRef)));
    }

    LoweredValInfo visitDeclaredSubtypeWitness(DeclaredSubtypeWitness* val)
    {
        return emitDeclRef(context, val->declRef,
            context->irBuilder->getWitnessTableType(
                lowerType(context, val->sup)));
    }

    LoweredValInfo visitTransitiveSubtypeWitness(
        TransitiveSubtypeWitness* val)
    {
        // The base (subToMid) will turn into a value with
        // witness-table type.
        IRInst* baseWitnessTable = lowerSimpleVal(context, val->subToMid);

        // The next step should map to an interface requirement
        // that is itself an interface conformance, so the result
        // of lowering this value should be a "key" that we can
        // use to look up a witness table.
        //
        // TODO: There are some ugly cases here if `midToSup` is allowed
        // to be an arbitrary witness, rather than just a declared one,
        // and we probably need to change the logic here so that we
        // instead think in terms of applying a subtype witness to
        // either a value or a witness table, to perform the appropriate
        // casting/lookup logic.
        //
        // For now we rely on the fact that the front-end doesn't
        // produce transitive witnesses in shapes that will cuase us
        // problems here.
        //
        IRInst* requirementKey = lowerSimpleVal(context, val->midToSup);

        return LoweredValInfo::simple(getBuilder()->emitLookupInterfaceMethodInst(
            getBuilder()->getWitnessTableType(lowerType(context, val->sup)),
            baseWitnessTable,
            requirementKey));
    }

    LoweredValInfo visitTaggedUnionSubtypeWitness(
        TaggedUnionSubtypeWitness* val)
    {
        // The sub-type in this case is a tagged union `A | B | ...`,
        // and the witness holds an array of witnesses showing that each
        // "case" (`A`, `B`, etc.) is a subtype of the super-type.

        // We will start by getting the IR-level representation of the
        // sub type (the tagged union type).
        //
        auto irTaggedUnionType = lowerType(context, val->sub);

        // We can turn each of those per-case witnesses into a witness
        // table value:
        //
        auto caseCount = val->caseWitnesses.getCount();
        List<IRInst*> caseWitnessTables;
        for( auto caseWitness : val->caseWitnesses )
        {
            auto caseWitnessTable = lowerSimpleVal(context, caseWitness);
            caseWitnessTables.add(caseWitnessTable);
        }

        // Now we need to synthesize a witness table for the tagged union
        // value, showing how it can implement all of the requirements
        // of the super type by delegating to the appropriate implementation
        // on a per-case basis.
        //
        // We will assume here that the super-type is an interface, and it
        // will be left to the front-end to ensure this property.
        //
        auto supDeclRefType = as<DeclRefType>(val->sup);
        if(!supDeclRefType)
        {
            SLANG_UNEXPECTED("super-type not a decl-ref type when generating tagged union witness table");
            UNREACHABLE_RETURN(LoweredValInfo());
        }
        auto supInterfaceDeclRef = supDeclRefType->declRef.as<InterfaceDecl>();
        if( !supInterfaceDeclRef )
        {
            SLANG_UNEXPECTED("super-type not an interface type when generating tagged union witness table");
            UNREACHABLE_RETURN(LoweredValInfo());
        }

        auto subType = lowerType(context, val->sub);
        auto irWitnessTableBaseType = lowerType(context, supDeclRefType);
        auto irWitnessTable = getBuilder()->createWitnessTable(irWitnessTableBaseType, subType);

        // Now we will iterate over the requirements (members) of the
        // interface and try to synthesize an appropriate value for each.
        //
        for( auto reqDeclRef : getMembers(supInterfaceDeclRef) )
        {
            // TODO: if there are any members we shouldn't process as a requirement,
            // then we should detect and skip them here.
            //

            // Every interface requirement will have a unique key that is used
            // when looking up the requirement in a concrete witness table.
            //
            auto irReqKey = getInterfaceRequirementKey(context, reqDeclRef.getDecl());

            // We expect that each of the witness tables in `caseWitnessTables`
            // will have an entry to match these keys. However, we may not
            // have a concrete `IRWitnessTable` for each of the case types, either
            // because they are a specialization of a generic (so that the witness
            // table reference is a `specialize` instruction at this point), or
            // they are a type external to this module (so that we have a declaration
            // rather than a definition of the witness table).

            // Our task is to create an IR value that can satisfy the interface
            // requirement for the tagged union type, by appropriately delegating
            // to the implementations of the same requirement in the case types.
            //
            IRInst* irSatisfyingVal = nullptr;



            if(auto callableDeclRef = reqDeclRef.as<CallableDecl>())
            {
                // We have something callable, so we need to synthesize
                // a function to satisfy it.
                //
                auto irFunc = getBuilder()->createFunc();
                irSatisfyingVal = irFunc;

                IRBuilder subBuilderStorage(getBuilder()->getSharedBuilder());
                auto subBuilder = &subBuilderStorage;
                subBuilder->setInsertInto(irFunc);

                // We will start by setting up the function parameters,
                // which live in the entry block of the IR function.
                //
                auto entryBlock = subBuilder->emitBlock();
                subBuilder->setInsertInto(entryBlock);

                // Create a `this` parameter of the tagged-union type.
                //
                // TODO: need to handle the `[mutating]` case here...
                //
                auto irThisType = irTaggedUnionType;
                auto irThisParam = subBuilder->emitParam(irThisType);

                List<IRType*> irParamTypes;
                irParamTypes.add(irThisType);

                // Create the remaining parameters of the callable,
                // using a decl-ref specialized to the tagged union
                // type (so that things like associated types are
                // mapped to the correct witness value).
                //
                List<IRParam*> irParams;
                for( auto paramDeclRef : getMembersOfType<ParamDecl>(callableDeclRef) )
                {
                    // TODO: need to handle `out` and `in out` here. Over all
                    // there is a lot of duplication here with the existing logic
                    // for emitting the signature of a `CallableDecl`, and we should
                    // try to re-use that if at all possible.
                    //
                    auto irParamType = lowerType(context, getType(context->astBuilder, paramDeclRef));
                    auto irParam = subBuilder->emitParam(irParamType);

                    irParams.add(irParam);
                    irParamTypes.add(irParamType);
                }

                auto irResultType = lowerType(context, getResultType(context->astBuilder, callableDeclRef));

                auto irFuncType = subBuilder->getFuncType(
                    irParamTypes,
                    irResultType);
                irFunc->setFullType(irFuncType);

                // The first thing our function needs to do is extract the tag
                // from the incoming `this` parameter.
                //
                auto irTagVal = subBuilder->emitExtractTaggedUnionTag(irThisParam);

                // Next we want to emit a `switch` on the tag value, but before we
                // do that we need to generate the code for each of the cases so that
                // our `switch` has somewhere to branch to.
                //
                List<IRInst*> switchCaseOperands;

                IRBlock* defaultLabel = nullptr;

                for( Index ii = 0; ii < caseCount; ++ii )
                {
                    auto caseTag = subBuilder->getIntValue(irTagVal->getDataType(), ii);

                    subBuilder->setInsertInto(irFunc);
                    auto caseLabel = subBuilder->emitBlock();

                    if(!defaultLabel)
                        defaultLabel = caseLabel;

                    switchCaseOperands.add(caseTag);
                    switchCaseOperands.add(caseLabel);

                    subBuilder->setInsertInto(caseLabel);

                    // We need to look up the satisfying value for this interface
                    // requirement on the witness table of the particular case value.
                    //
                    // We already have the witness table, and the requirement key is
                    // just `irReqKey`.
                    //
                    auto caseWitnessTable = caseWitnessTables[ii];

                    // The subtle bit here is determining the type we expect the
                    // satisfying value to have, since that depends on the actual
                    // type that is satisfying the requirement.
                    //
                    IRType* caseResultType = irResultType;
                    IRType* caseFuncType = nullptr;
                    auto caseFunc = subBuilder->emitLookupInterfaceMethodInst(
                        caseFuncType,
                        caseWitnessTable,
                        irReqKey);

                    // We are going to emit a `call` to the satisfying value
                    // for the case type, so we will collect the arguments for that call.
                    //
                    List<IRInst*> caseArgs;

                    // The `this` argument to the call will need to represent the
                    // appropriate field of our tagged union.
                    //
                    IRType* caseThisType = (IRType*) irTaggedUnionType->getOperand(ii);
                    auto caseThisArg = subBuilder->emitExtractTaggedUnionPayload(
                        caseThisType,
                        irThisParam, caseTag);
                    caseArgs.add(caseThisArg);

                    // The remaining arguments to the call will just be forwarded from
                    // the parameters of the wrapper function.
                    //
                    // TODO: This would need to change if/when we started allowing `This` type
                    // or associated-type parameters to be used at call sites where a tagged
                    // union is used.
                    //
                    for( auto param : irParams )
                    {
                        caseArgs.add(param);
                    }

                    auto caseCall = subBuilder->emitCallInst(caseResultType, caseFunc, caseArgs);

                    if( as<IRVoidType>(irResultType->getDataType()) )
                    {
                        subBuilder->emitReturn();
                    }
                    else
                    {
                        subBuilder->emitReturn(caseCall);
                    }
                }

                // We will create a block to represent the supposedly-unreachable
                // code that will run if no `case` matches.
                //
                subBuilder->setInsertInto(irFunc);
                auto invalidLabel = subBuilder->emitBlock();
                subBuilder->setInsertInto(invalidLabel);
                subBuilder->emitUnreachable();

                if(!defaultLabel) defaultLabel = invalidLabel;

                // Now we have enough information to go back and emit the `switch` instruction
                // into the entry block.
                subBuilder->setInsertInto(entryBlock);
                subBuilder->emitSwitch(
                    irTagVal,       // value to `switch` on
                    invalidLabel,   // `break` label (block after the `switch` statement ends)
                    defaultLabel,   // `default` label (where to go if no `case` matches)
                    switchCaseOperands.getCount(),
                    switchCaseOperands.getBuffer());
            }
            else
            {
                // TODO: We need to handle other cases of interface requirements.
                SLANG_UNEXPECTED("unexpceted interface requirement when generating tagged union witness table");
                UNREACHABLE_RETURN(LoweredValInfo());
            }

            // Once we've generating a value to satisfying the requirement, we install
            // it into the witness table for our tagged-union type.
            //
            getBuilder()->createWitnessTableEntry(irWitnessTable, irReqKey, irSatisfyingVal);
        }
        return LoweredValInfo::simple(irWitnessTable);
    }

    LoweredValInfo visitDynamicSubtypeWitness(DynamicSubtypeWitness * /*val*/)
    {
        return LoweredValInfo::simple(nullptr);
    }

    LoweredValInfo visitThisTypeSubtypeWitness(ThisTypeSubtypeWitness* val)
    {
        SLANG_UNUSED(val);
        return LoweredValInfo::simple(context->thisTypeWitness);
    }

    LoweredValInfo visitConjunctionSubtypeWitness(ConjunctionSubtypeWitness* val)
    {
        // A witness `T : L & R` for a conformance of `T` to a conjunction of
        // types `L` and `R` will be lowered as a tuple of two witnesses: one
        // for `T : L` and one for `T : R`. Luckily, those two conformances
        // are exactly what the `ConjunctionSubtypeWitness` stores, so we just
        // need to lower them individually and make a tuple.
        //
        auto left   = lowerSimpleVal(context, val->leftWitness);
        auto right  = lowerSimpleVal(context, val->rightWitness);
        return LoweredValInfo::simple(getBuilder()->emitMakeTuple(left, right));
    }

    LoweredValInfo visitExtractFromConjunctionSubtypeWitness(ExtractFromConjunctionSubtypeWitness* val)
    {
        auto builder = getBuilder();

        // We know from `visitConjunctionSubtypeWitness` that a witness for a relationship
        // like `T : L & R` will be a tuple `(w_l, w_r)` where `w_l` is a witness
        // for `T : L` and `w_r` will be a witness for `T : R`.
        //
        // An `ExtractFromConjunctionSubtypeWitness` represents the intention to
        // extract one of those two sub-witnesses. It directly stores the original
        // witness that `T : L & R`, so lower that first and expect it to be
        // a value of tuple type.
        //
        auto conjunctionWitness = lowerSimpleVal(context, val->conunctionWitness);
        auto conjunctionTupleType = as<IRTupleType>(conjunctionWitness->getDataType());
        SLANG_ASSERT(conjunctionTupleType);

        // The `ExtractFromConjunctionSubtypeWitness` also stores the index of
        // the witness/supertype we want in the conjunction `L & R`.
        //
        auto indexInConjunction = val->indexInConjunction;

        // We want to extract the appropriate element from the tuple based on
        // the index, but to know the type of the result we need to look up
        // the element type that corresponds to that index.
        //
        // TODO: `IRTupleType` should really have `getElementCount()` and
        // `getElementType(index)` accessors.
        //
        auto elementType = (IRType*) conjunctionTupleType->getOperand(indexInConjunction);

        // With the information we've extracted above, we now just need to
        // extract the appropriate element from the `(w_l, w_r)` tuple of
        // witnesses, and we will have our desired result.
        //
        return LoweredValInfo::simple(builder->emitGetTupleElement(
            elementType,
            conjunctionWitness,
            indexInConjunction));
    }


    LoweredValInfo visitConstantIntVal(ConstantIntVal* val)
    {
        // TODO: it is a bit messy here that the `ConstantIntVal` representation
        // has no notion of a *type* associated with the value...

        auto type = getIntType(context);
        return LoweredValInfo::simple(getBuilder()->getIntValue(type, val->value));
    }

    IRFuncType* visitFuncType(FuncType* type)
    {
        IRType* resultType = lowerType(context, type->getResultType());
        UInt paramCount = type->getParamCount();
        List<IRType*> paramTypes;
        for (UInt pp = 0; pp < paramCount; ++pp)
        {
            paramTypes.add(lowerType(context, type->getParamType(pp)));
        }
        if (type->getErrorType()->equals(context->astBuilder->getBottomType()))
        {
            return getBuilder()->getFuncType(
                paramCount,
                paramTypes.getBuffer(),
                resultType);
        }
        else
        {
            auto errorType = lowerType(context, type->getErrorType());
            auto irThrowFuncTypeAttribute =
                getBuilder()->getAttr(kIROp_FuncThrowTypeAttr, 1, (IRInst**)&errorType);
            return getBuilder()->getFuncType(
                paramCount, paramTypes.getBuffer(), resultType, irThrowFuncTypeAttribute);
        }
    }

    IRType* visitPtrType(PtrType* type)
    {
        IRType* valueType = lowerType(context, type->getValueType());
        return getBuilder()->getPtrType(valueType);
    }

    IRType* visitDeclRefType(DeclRefType* type)
    {
        auto declRef = type->declRef;
        auto decl = declRef.getDecl();

        // Check for types with teh `__intrinsic_type` modifier.
        if(decl->findModifier<IntrinsicTypeModifier>())
        {
            return lowerSimpleIntrinsicType(type);
        }


        return (IRType*) getSimpleVal(
            context,
            emitDeclRef(context, declRef,
                context->irBuilder->getTypeKind()));
    }

    IRType* visitNamedExpressionType(NamedExpressionType* type)
    {
        return (IRType*)getSimpleVal(context, dispatchType(type->getCanonicalType()));
    }

    IRType* visitBasicExpressionType(BasicExpressionType* type)
    {
        return getBuilder()->getBasicType(
            type->baseType);
    }

    IRType* visitVectorExpressionType(VectorExpressionType* type)
    {
        auto elementType = lowerType(context, type->elementType);
        auto elementCount = lowerSimpleVal(context, type->elementCount);

        return getBuilder()->getVectorType(
            elementType,
            elementCount);
    }

    IRType* visitMatrixExpressionType(MatrixExpressionType* type)
    {
        auto elementType = lowerType(context, type->getElementType());
        auto rowCount = lowerSimpleVal(context, type->getRowCount());
        auto columnCount = lowerSimpleVal(context, type->getColumnCount());

        return getBuilder()->getMatrixType(
            elementType,
            rowCount,
            columnCount);
    }

    IRType* visitArrayExpressionType(ArrayExpressionType* type)
    {
        auto elementType = lowerType(context, type->baseType);
        if (type->arrayLength)
        {
            auto elementCount = lowerSimpleVal(context, type->arrayLength);
            return getBuilder()->getArrayType(
                elementType,
                elementCount);
        }
        else
        {
            return getBuilder()->getUnsizedArrayType(
                elementType);
        }
    }

    // Lower substitution args and collect them into a list of IR operands.
    void _collectSubstitutionArgs(List<IRInst*>& operands, Substitutions* subst)
    {
        if (!subst) return;
        _collectSubstitutionArgs(operands, subst->outer);
        if (auto genSubst = as<GenericSubstitution>(subst))
        {
            for (auto arg : genSubst->args)
            {
                operands.add(lowerVal(context, arg).val);
            }
        }
    }
    // Lower a type where the type declaration being referenced is assumed
    // to be an intrinsic type, which can thus be lowered to a simple IR
    // type with the appropriate opcode.
    IRType* lowerSimpleIntrinsicType(DeclRefType* type)
    {
        auto intrinsicTypeModifier = type->declRef.getDecl()->findModifier<IntrinsicTypeModifier>();
        SLANG_ASSERT(intrinsicTypeModifier);
        IROp op = IROp(intrinsicTypeModifier->irOp);
        List<IRInst*> operands;
        // If there are any substitutions attached to the declRef,
        // add them as operands of the IR type.
        _collectSubstitutionArgs(operands, type->declRef.substitutions.substitutions);
        return getBuilder()->getType(
            op,
            static_cast<UInt>(operands.getCount()),
            operands.getBuffer());
    }

    // Lower a type where the type declaration being referenced is assumed
    // to be an intrinsic type with a single generic type parameter, and
    // which can thus be lowered to a simple IR type with the appropriate opcode.
    IRType* lowerGenericIntrinsicType(DeclRefType* type, Type* elementType)
    {
        auto intrinsicTypeModifier = type->declRef.getDecl()->findModifier<IntrinsicTypeModifier>();
        SLANG_ASSERT(intrinsicTypeModifier);
        IROp op = IROp(intrinsicTypeModifier->irOp);
        IRInst* irElementType = lowerType(context, elementType);
        return getBuilder()->getType(
            op,
            1,
            &irElementType);
    }

    IRType* lowerGenericIntrinsicType(DeclRefType* type, Type* elementType, IntVal* count)
    {
        auto intrinsicTypeModifier = type->declRef.getDecl()->findModifier<IntrinsicTypeModifier>();
        SLANG_ASSERT(intrinsicTypeModifier);
        IROp op = IROp(intrinsicTypeModifier->irOp);
        IRInst* irElementType = lowerType(context, elementType);

        IRInst* irCount = lowerSimpleVal(context, count);

        IRInst* const operands[2] =
        {
            irElementType,
            irCount,
        };

        return getBuilder()->getType(
            op,
            SLANG_COUNT_OF(operands),
            operands);
    }

    IRType* visitResourceType(ResourceType* type)
    {
        return lowerGenericIntrinsicType(type, type->elementType);
    }

    IRType* visitSamplerStateType(SamplerStateType* type)
    {
        return lowerSimpleIntrinsicType(type);
    }

    IRType* visitBuiltinGenericType(BuiltinGenericType* type)
    {
        return lowerGenericIntrinsicType(type, type->elementType);
    }

    IRType* visitUntypedBufferResourceType(UntypedBufferResourceType* type)
    {
        return lowerSimpleIntrinsicType(type);
    }

    IRType* visitHLSLPatchType(HLSLPatchType* type)
    {
        Type* elementType = type->getElementType();
        IntVal* count = type->getElementCount();

        return lowerGenericIntrinsicType(type, elementType, count);
    }

    IRType* visitExtractExistentialType(ExtractExistentialType* type)
    {
        auto declRef = type->declRef;
        auto existentialType = lowerType(context, getType(context->astBuilder, declRef));
        IRInst* existentialVal = getSimpleVal(context, emitDeclRef(context, declRef, existentialType));
        return getBuilder()->emitExtractExistentialType(existentialVal);
    }

    LoweredValInfo visitExtractExistentialSubtypeWitness(ExtractExistentialSubtypeWitness* witness)
    {
        auto declRef = witness->declRef;
        auto existentialType = lowerType(context, getType(context->astBuilder, declRef));
        IRInst* existentialVal = getSimpleVal(context, emitDeclRef(context, declRef, existentialType));
        return LoweredValInfo::simple(getBuilder()->emitExtractExistentialWitnessTable(existentialVal));
    }

    LoweredValInfo visitTaggedUnionType(TaggedUnionType* type)
    {
        // A tagged union type will lower into an IR `union` over the cases,
        // along with an IR `struct` with a field for the union and a tag.
        // (Note: we are placing the tag after the payload to avoid padding
        // in the case where the payload is more aligned than the tag)
        //
        // TODO: should we be lowering directly like this, or have
        // an IR-level representation of tagged unions?
        //

        List<IRType*> irCaseTypes;
        for(auto caseType : type->caseTypes)
        {
            auto irCaseType = lowerType(context, caseType);
            irCaseTypes.add(irCaseType);
        }

        auto irType = getBuilder()->getTaggedUnionType(irCaseTypes);
        if(!irType->findDecoration<IRLinkageDecoration>())
        {
            // We need a way for later passes to attach layout information
            // to this type, so we will give it a mangled name here.
            //
            getBuilder()->addExportDecoration(
                irType,
                getMangledTypeName(context->astBuilder, type).getUnownedSlice());
        }
        return LoweredValInfo::simple(irType);
    }

    LoweredValInfo visitExistentialSpecializedType(ExistentialSpecializedType* type)
    {
        auto irBaseType = lowerType(context, type->baseType);

        List<IRInst*> slotArgs;
        for(auto arg : type->args)
        {
            auto irArgVal = lowerSimpleVal(context, arg.val);
            slotArgs.add(irArgVal);

            if(auto witness = arg.witness)
            {
                auto irArgWitness = lowerSimpleVal(context, witness);
                slotArgs.add(irArgWitness);
            }
        }

        auto irType = getBuilder()->getBindExistentialsType(irBaseType, slotArgs.getCount(), slotArgs.getBuffer());
        return LoweredValInfo::simple(irType);
    }

    LoweredValInfo visitThisType(ThisType* type)
    {
        // A `This` type in an interface decl should lower to `IRThisType`,
        // while `This` type in a concrete `struct` should lower to the `struct` type
        // itself. A `This` type reference in a concrete type is already translated to that
        // type in semantics checking in this setting.
        // If we see `This` type here, we are dealing with `This` inside an interface decl.
        // Therefore, `context->thisType` should have been set to `IRThisType`
        // in `visitInterfaceDecl`, and we can just use that value here.
        //
        if (context->thisType != nullptr)
            return LoweredValInfo::simple(context->thisType);
        return emitDeclRef(context, type->interfaceDeclRef, getBuilder()->getTypeKind());
    }

    LoweredValInfo visitAndType(AndType* type)
    {
        auto left = lowerType(context, type->left);
        auto right = lowerType(context, type->right);

        auto irType = getBuilder()->getConjunctionType(left, right);
        return LoweredValInfo::simple(irType);
    }

    LoweredValInfo visitModifiedType(ModifiedType* astType)
    {
        IRType* irBase = lowerType(context, astType->base);

        List<IRAttr*> irAttrs;
        for(auto astModifier : astType->modifiers)
        {
            IRAttr* irAttr = (IRAttr*) lowerSimpleVal(context, astModifier);
            if(irAttr)
                irAttrs.add(irAttr);
        }

        auto irType = getBuilder()->getAttributedType(irBase, irAttrs);
        return LoweredValInfo::simple(irType);
    }

    LoweredValInfo visitUNormModifierVal(UNormModifierVal* astVal)
    {
        SLANG_UNUSED(astVal);
        return LoweredValInfo::simple(getBuilder()->getAttr(kIROp_UNormAttr));
    }

    LoweredValInfo visitSNormModifierVal(SNormModifierVal* astVal)
    {
        SLANG_UNUSED(astVal);
        return LoweredValInfo::simple(getBuilder()->getAttr(kIROp_SNormAttr));
    }

    // We do not expect to encounter the following types in ASTs that have
    // passed front-end semantic checking.
#define UNEXPECTED_CASE(NAME) IRType* visit##NAME(NAME*) { SLANG_UNEXPECTED(#NAME); UNREACHABLE_RETURN(nullptr); }
    UNEXPECTED_CASE(GenericDeclRefType)
    UNEXPECTED_CASE(TypeType)
    UNEXPECTED_CASE(ErrorType)
    UNEXPECTED_CASE(InitializerListType)
    UNEXPECTED_CASE(OverloadGroupType)
    UNEXPECTED_CASE(NamespaceType)
#undef UNEXPECTED_CASE
};

LoweredValInfo lowerVal(
    IRGenContext*   context,
    Val*            val)
{
    ValLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(val);
}

IRType* lowerType(
    IRGenContext*   context,
    Type*           type)
{
    ValLoweringVisitor visitor;
    visitor.context = context;
    return (IRType*) getSimpleVal(context, visitor.dispatchType(type));
}

void addVarDecorations(
    IRGenContext*   context,
    IRInst*         inst,
    Decl*           decl)
{
    auto builder = context->irBuilder;
    for(Modifier* mod : decl->modifiers)
    {
        if(as<HLSLNoInterpolationModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::NoInterpolation);
        }
        else if(as<PerVertexModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::PerVertex);
        }
        else if(as<HLSLNoPerspectiveModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::NoPerspective);
        }
        else if(as<HLSLLinearModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::Linear);
        }
        else if(as<HLSLSampleModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::Sample);
        }
        else if(as<HLSLCentroidModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::Centroid);
        }
        else if(auto rayPayloadAttr = as<VulkanRayPayloadAttribute>(mod))
        {
            builder->addVulkanRayPayloadDecoration(inst, rayPayloadAttr->location);
        }
        else if(auto callablePayloadAttr = as<VulkanCallablePayloadAttribute>(mod))
        {
            builder->addVulkanCallablePayloadDecoration(inst, callablePayloadAttr->location);
        }
        else if(as<VulkanHitAttributesAttribute>(mod))
        {
            builder->addSimpleDecoration<IRVulkanHitAttributesDecoration>(inst);
        }
        else if(as<GloballyCoherentModifier>(mod))
        {
            builder->addSimpleDecoration<IRGloballyCoherentDecoration>(inst);
        }
        else if(as<PreciseModifier>(mod))
        {
            builder->addSimpleDecoration<IRPreciseDecoration>(inst);
        }
        else if(auto formatAttr = as<FormatAttribute>(mod))
        {
            builder->addFormatDecoration(inst, formatAttr->format);
        }

        // TODO: what are other modifiers we need to propagate through?
    }
}

/// If `decl` has a modifier that should turn into a
/// rate qualifier, then apply it to `inst`.
void maybeSetRate(
    IRGenContext*   context,
    IRInst*         inst,
    Decl*           decl)
{
    auto builder = context->irBuilder;

    if (decl->hasModifier<HLSLGroupSharedModifier>())
    {
        inst->setFullType(builder->getRateQualifiedType(
            builder->getGroupSharedRate(),
            inst->getFullType()));
    }
    else if (decl->hasModifier<ActualGlobalModifier>())
    {
        inst->setFullType(builder->getRateQualifiedType(
            builder->getActualGlobalRate(),
            inst->getFullType()));
    }
}

static String getNameForNameHint(
    IRGenContext*   context,
    Decl*           decl)
{
    // We will use a bit of an ad hoc convention here for now.

    Name* leafName = decl->getName();

    // Handle custom name for a global parameter group (e.g., a `cbuffer`)
    if(auto reflectionNameModifier = decl->findModifier<ParameterGroupReflectionName>())
    {
        leafName = reflectionNameModifier->nameAndLoc.name;
    }

    // There is no point in trying to provide a name hint for something with no name,
    // or with an empty name
    if(!leafName)
        return String();
    if(leafName->text.getLength() == 0)
        return String();


    if(auto varDecl = as<VarDeclBase>(decl))
    {
        // For an ordinary local variable, global variable,
        // parameter, or field, we will just use the name
        // as declared, and now work in anything from
        // its parent declaration(s).
        //
        // TODO: consider whether global/static variables should
        // follow different rules.
        //
        return leafName->text;
    }

    // For other cases of declaration, we want to consider
    // merging its name with the name of its parent declaration.
    auto parentDecl = decl->parentDecl;

    // Skip past a generic parent, if we are a declaration nested in a generic.
    if(auto genericParentDecl = as<GenericDecl>(parentDecl))
        parentDecl = genericParentDecl->parentDecl;

    // A `ModuleDecl` can have a name too, but in the common case
    // we don't want to generate name hints that include the module
    // name, simply because they would lead to every global symbol
    // getting a much longer name.
    //
    // TODO: We should probably include the module name for symbols
    // being `import`ed, and not for symbols being compiled directly
    // (those coming from a module that had no name given to it).
    //
    // For now we skip past a `ModuleDecl` parent.
    //
    if(auto moduleParentDecl = as<ModuleDecl>(parentDecl))
        parentDecl = moduleParentDecl->parentDecl;

    if(!parentDecl)
    {
        return leafName->text;
    }

    auto parentName = getNameForNameHint(context, parentDecl);
    if(parentName.getLength() == 0)
    {
        return leafName->text;
    }

    // We will now construct a new `Name` to use as the hint,
    // combining the name of the parent and the leaf declaration.

    StringBuilder sb;
    sb.append(parentName);
    sb.append(".");
    sb.append(leafName->text);

    return sb.ProduceString();
}

/// Try to add an appropriate name hint to the instruction,
/// that can be used for back-end code emission or debug info.
static void addNameHint(
    IRGenContext*   context,
    IRInst*         inst,
    Decl*           decl)
{
    String name = getNameForNameHint(context, decl);
    if(name.getLength() == 0)
        return;
    context->irBuilder->addNameHintDecoration(inst, name.getUnownedSlice());
}

/// Add a name hint based on a fixed string.
static void addNameHint(
    IRGenContext*   context,
    IRInst*         inst,
    char const*     text)
{
    if (context->shared->m_obfuscateCode)
    {
        return;
    }

    context->irBuilder->addNameHintDecoration(inst, UnownedTerminatedStringSlice(text));
}

LoweredValInfo createVar(
    IRGenContext*   context,
    IRType*         type,
    Decl*           decl = nullptr)
{
    auto builder = context->irBuilder;
    auto irAlloc = builder->emitVar(type);

    if (decl)
    {
        maybeSetRate(context, irAlloc, decl);

        addVarDecorations(context, irAlloc, decl);

        builder->addHighLevelDeclDecoration(irAlloc, decl);

        addNameHint(context, irAlloc, decl);
    }

    return LoweredValInfo::ptr(irAlloc);
}

// When we try to turn a `LoweredValInfo` into an address of some temporary storage,
// we can either do it "aggressively" or not (what we'll call the "default" behavior,
// although it isn't strictly more common).
//
// The case that this is mostly there to address is when somebody writes an operation
// like:
//
//      foo[a] = b;
//
// In that case, we might as well just use the `set` accessor if there is one, rather
// than complicate things. However, in more complex cases like:
//
//      foo[a].x = b;
//
// there is no way to satisfy the semantics of the code the user wrote (in terms of
// only writing one vector component, and not a full vector) by using the `set`
// accessor, and we need to be "aggressive" in turning the lvalue `foo[a]` into
// an address.
//
// TODO: realistically IR lowering is too early to be binding to this choice,
// because different accessors might be supported on different targets.
//
enum class TryGetAddressMode
{
    Default,
    Aggressive,
};

/// Try to coerce `inVal` into a `LoweredValInfo::ptr()` with a simple address.
LoweredValInfo tryGetAddress(
    IRGenContext* context,
    LoweredValInfo const& inVal,
    TryGetAddressMode       mode);

    /// Add a single `in` argument value to a list of arguments
void addInArg(
    IRGenContext*   context,
    List<IRInst*>*  ioArgs,
    LoweredValInfo  argVal)
{
    auto& args = *ioArgs;
    switch( argVal.flavor )
    {
    case LoweredValInfo::Flavor::Simple:
    case LoweredValInfo::Flavor::Ptr:
    case LoweredValInfo::Flavor::SwizzledLValue:
    case LoweredValInfo::Flavor::BoundStorage:
    case LoweredValInfo::Flavor::BoundMember:
    case LoweredValInfo::Flavor::ExtractedExistential:
        args.add(getSimpleVal(context, argVal));
        break;

    default:
        SLANG_UNIMPLEMENTED_X("addInArg case");
        break;
    }
}

// After a call to a function with `out` or `in out`
// parameters, we may need to copy data back into
// the l-value locations used for output arguments.
//
// During lowering of the argument list, we build
// up a list of these "fixup" assignments that need
// to be performed.
struct OutArgumentFixup
{
    LoweredValInfo dst;
    LoweredValInfo src;
};

    /// Apply any fixups that have been created for `out` and `inout` arguments.
static void applyOutArgumentFixups(
    IRGenContext*                   context,
    List<OutArgumentFixup> const&   fixups)
{
    for (auto fixup : fixups)
    {
        assign(context, fixup.dst, fixup.src);
    }
}

    /// Add one argument value to the argument list for a call being constructed
void addArg(
    IRGenContext*           context,
    List<IRInst*>*          ioArgs,         //< The argument list being built
    List<OutArgumentFixup>* ioFixups,       //< "Fixup" logic to apply for `out` or `inout` arguments
    LoweredValInfo          argVal,         //< The lowered value of the argument to add
    IRType*                 paramType,      //< The type of the corresponding parameter
    ParameterDirection      paramDirection, //< The direction of the parameter (`in`, `out`, etc.)
    SourceLoc               loc)            //< A location to use if we need to report an error
{
    switch(paramDirection)
    {
    case kParameterDirection_Ref:
        {
            // According to our "calling convention" we need to
            // pass a pointer into the callee. Unlike the case for
            // `out` and `inout` below, it is never valid to do
            // copy-in/copy-out for a `ref` parameter, so we just
            // pass in the actual pointer.
            //
            IRInst* argPtr = getAddress(context, argVal, loc);
            addInArg(context, ioArgs, LoweredValInfo::simple(argPtr));
        }
        break;

    case kParameterDirection_Out:
    case kParameterDirection_InOut:
        {
            // According to our "calling convention" we need to
            // pass a pointer into the callee.
            //
            // Ideally we would like to just pass the address of
            // `loweredArg`, and when that it possible we will do so.
            // It may happen, though, that `loweredArg` is not an
            // addressable l-value (e.g., it is `foo.xyz`, so that
            // the bytes of the l-value are not contiguous).
            //
            LoweredValInfo argPtr = tryGetAddress(context, argVal, TryGetAddressMode::Default);
            if(argPtr.flavor == LoweredValInfo::Flavor::Ptr)
            {
                addInArg(context, ioArgs, LoweredValInfo::simple(argPtr.val));
            }
            else
            {
                // If the value is not one that could yield a simple l-value
                // then we need to convert it into a temporary
                //
                LoweredValInfo tempVar = createVar(context, paramType);

                // If the parameter is `in out` or `inout`, then we need
                // to ensure that we pass in the original value stored
                // in the argument, which we accomplish by assigning
                // from the l-value to our temp.
                //
                if (paramDirection == kParameterDirection_InOut)
                {
                    assign(context, tempVar, argVal);
                }

                // Now we can pass the address of the temporary variable
                // to the callee as the actual argument for the `in out`
                SLANG_ASSERT(tempVar.flavor == LoweredValInfo::Flavor::Ptr);
                IRInst* tempPtr = getAddress(context, tempVar, loc);
                addInArg(context, ioArgs, LoweredValInfo::simple(tempPtr));

                // Finally, after the call we will need
                // to copy in the other direction: from our
                // temp back to the original l-value.
                OutArgumentFixup fixup;
                fixup.src = tempVar;
                fixup.dst = argVal;

                (*ioFixups).add(fixup);
            }
        }
        break;

    default:
        addInArg(context, ioArgs, argVal);
        break;
    }
}

    /// Add argument(s) corresponding to one parameter to a call
    ///
    /// The `argExpr` is the AST-level expression being passed as an argument to the call.
    /// The `paramType` and `paramDirection` represent what is known about the receiving
    /// parameter of the callee (e.g., if the parameter `in`, `inout`, etc.).
    /// The `ioArgs` array receives the IR-level argument(s) that are added for the given
    /// argument expression.
    /// The `ioFixups` array receives any "fixup" code that needs to be run *after* the
    /// call completes (e.g., to move from a scratch variable used for an `inout` argument back
    /// into the original location).
    ///
void addCallArgsForParam(
    IRGenContext*           context,
    IRType*                 paramType,
    ParameterDirection      paramDirection,
    Expr*                   argExpr,
    List<IRInst*>*          ioArgs,
    List<OutArgumentFixup>* ioFixups)
{
    switch(paramDirection)
    {
    case kParameterDirection_Ref:
    case kParameterDirection_Out:
    case kParameterDirection_InOut:
        {
            LoweredValInfo loweredArg = lowerLValueExpr(context, argExpr);
            addArg(context, ioArgs, ioFixups, loweredArg, paramType, paramDirection, argExpr->loc);
        }
        break;

    default:
        {
            LoweredValInfo loweredArg = lowerRValueExpr(context, argExpr);
            addInArg(context, ioArgs, loweredArg);
        }
        break;
    }
}


//

    /// Compute the direction for a parameter based on its declaration
ParameterDirection getParameterDirection(VarDeclBase* paramDecl)
{
    if( paramDecl->hasModifier<RefModifier>() )
    {
        // The AST specified `ref`:
        return kParameterDirection_Ref;
    }
    if( paramDecl->hasModifier<InOutModifier>() )
    {
        // The AST specified `inout`:
        return kParameterDirection_InOut;
    }
    if (paramDecl->hasModifier<OutModifier>())
    {
        // We saw an `out` modifier, so now we need
        // to check if there was a paired `in`.
        if(paramDecl->hasModifier<InModifier>())
            return kParameterDirection_InOut;
        else
            return kParameterDirection_Out;
    }
    else
    {
        // No direction modifier, or just `in`:
        return kParameterDirection_In;
    }
}

    /// Compute the direction for a `this` parameter based on the declaration of its parent function
    ///
    /// If the given declaration doesn't care about the direction of a `this` parameter, then
    /// it will return the provided `defaultDirection` instead.
    ///
ParameterDirection getThisParamDirection(Decl* parentDecl, ParameterDirection defaultDirection)
{
    // Applications can opt in to a mutable `this` parameter,
    // by applying the `[mutating]` attribute to their
    // declaration.
    //
    if( parentDecl->hasModifier<MutatingAttribute>() )
    {
        return kParameterDirection_InOut;
    }

    // A `set` accessor on a property or subscript declaration
    // defaults to a mutable `this` parameter, but the programmer
    // can opt out of this behavior using `[nonmutating]`
    //
    if( parentDecl->hasModifier<NonmutatingAttribute>() )
    {
        return kParameterDirection_In;
    }
    else if( as<SetterDecl>(parentDecl) )
    {
        return kParameterDirection_InOut;
    }

    // Declarations that represent abstract storage (a property
    // or subscript) do not want to dictate anything about
    // the direction of an outer `this` parameter, since that
    // should be determined by their inner accessors.
    //
    if( as<PropertyDecl>(parentDecl) )
    {
        return defaultDirection;
    }
    if( as<SubscriptDecl>(parentDecl) )
    {
        return defaultDirection;
    }

    // A parent generic declaration should not change the
    // mutating-ness of the inner declaration.
    //
    if( as<GenericDecl>(parentDecl) )
    {
        return defaultDirection;
    }

    // For now we make any `this` parameter default to `in`.
    //
    return kParameterDirection_In;
}

DeclRef<Decl> createDefaultSpecializedDeclRefImpl(IRGenContext* context, Decl* decl)
{
    DeclRef<Decl> declRef;
    declRef.decl = decl;
    declRef.substitutions = createDefaultSubstitutions(context->astBuilder, decl);
    return declRef;
}
//
// The client should actually call the templated wrapper, to preserve type information.
template<typename D>
DeclRef<D> createDefaultSpecializedDeclRef(IRGenContext* context, D* decl)
{
    DeclRef<Decl> declRef = createDefaultSpecializedDeclRefImpl(context, decl);
    return declRef.as<D>();
}

static Type* _findReplacementThisParamType(
    IRGenContext*   context,
    DeclRef<Decl>   parentDeclRef)
{
    if( auto extensionDeclRef = parentDeclRef.as<ExtensionDecl>() )
    {
        auto targetType = getTargetType(context->astBuilder, extensionDeclRef);
        if(auto targetDeclRefType = as<DeclRefType>(targetType))
        {
            if(auto replacementType = _findReplacementThisParamType(context, targetDeclRefType->declRef))
                return replacementType;
        }
        return targetType;
    }

    if (auto interfaceDeclRef = parentDeclRef.as<InterfaceDecl>())
    {
        auto thisType = context->astBuilder->create<ThisType>();
        thisType->interfaceDeclRef = interfaceDeclRef;
        return thisType;
    }

    return nullptr;
}

    /// Get the type of the `this` parameter introduced by `parentDeclRef`, or null.
    ///
    /// E.g., if `parentDeclRef` is a `struct` declaration, then this will
    /// return the type of that `struct`.
    ///
    /// If this function is called on a declaration that does not itself directly
    /// introduce a notion of `this`, then null will be returned. Note that this
    /// includes things like function declarations themselves, which inherit the
    /// definition of `this` from their parent/outer declaration.
    ///
Type* getThisParamTypeForContainer(
    IRGenContext*   context,
    DeclRef<Decl>   parentDeclRef)
{
    if(auto replacementType = _findReplacementThisParamType(context, parentDeclRef))
        return replacementType;

    if( auto aggTypeDeclRef = parentDeclRef.as<AggTypeDecl>() )
    {
        return DeclRefType::create(context->astBuilder, aggTypeDeclRef);
    }

    return nullptr;
}

Type* getThisParamTypeForCallable(
    IRGenContext*   context,
    DeclRef<Decl>   callableDeclRef)
{
    auto parentDeclRef = callableDeclRef.getParent();

    if(auto subscriptDeclRef = parentDeclRef.as<SubscriptDecl>())
        parentDeclRef = subscriptDeclRef.getParent();

    if(auto genericDeclRef = parentDeclRef.as<GenericDecl>())
        parentDeclRef = genericDeclRef.getParent();

    return getThisParamTypeForContainer(context, parentDeclRef);
}

// When lowering something callable (most commonly a function declaration),
// we need to construct an appropriate parameter list for the IR function
// that folds in any contributions from both the declaration itself *and*
// its parent declaration(s).
//
// For example, given code like:
//
//     struct Foo { int bar(float y) { ... } };
//
// we need to generate IR-level code something like:
//
//     func Foo_bar(Foo this, float y) -> int;
//
// that is, the `this` parameter has become explicit.
//
// The same applies to generic parameters, and these
// should apply even if the nested declaration is `static`:
//
//     struct Foo<T> { static int bar(T y) { ... } };
//
// becomes:
//
//     func Foo_bar<T>(T y) -> int;
//
// In order to implement this, we are going to do a recursive
// walk over a declaration and its parents, collecting separate
// lists of ordinary and generic parameters that will need
// to be included in the final declaration's parameter list.
//
// When doing code generation for an ordinary value parameter,
// we mostly care about its type, and then also its "direction"
// (`in`, `out`, `in out`). We sometimes need acess to the
// original declaration so that we can inspect it for meta-data,
// but in some cases there is no such declaration (e.g., a `this`
// parameter doesn't get an explicit declaration in the AST).
// To handle this we break out the relevant data into derived
// structures:
//
struct IRLoweringParameterInfo
{
    // This AST-level type of the parameter
    Type*        type = nullptr;

    // The direction (`in` vs `out` vs `in out`)
    ParameterDirection  direction;

    // The variable/parameter declaration for
    // this parameter (if any)
    VarDeclBase*        decl = nullptr;

    // Is this the representation of a `this` parameter?
    bool                isThisParam = false;
};
//
// We need a way to be able to create a `IRLoweringParameterInfo` given the declaration
// of a parameter:
//
IRLoweringParameterInfo getParameterInfo(
    IRGenContext*               context,
    DeclRef<VarDeclBase> const& paramDecl)
{
    IRLoweringParameterInfo info;

    info.type = getType(context->astBuilder, paramDecl);
    info.decl = paramDecl;
    info.direction = getParameterDirection(paramDecl);
    info.isThisParam = false;
    return info;
}
//

// Here's the declaration for the type to hold the lists:
struct ParameterLists
{
    List<IRLoweringParameterInfo> params;
};
//
// Because there might be a `static` declaration somewhere
// along the lines, we need to be careful to prohibit adding
// non-generic parameters in some cases.
enum ParameterListCollectMode
{
    // Collect everything: ordinary and generic parameters.
    kParameterListCollectMode_Default,


    // Only collect generic parameters.
    kParameterListCollectMode_Static,
};
//
// We also need to be able to detect whether a declaration is
// either explicitly or implicitly treated as `static`:
ParameterListCollectMode getModeForCollectingParentParameters(
    Decl*           decl,
    ContainerDecl*  parentDecl)
{
    // If we have a `static` parameter, then it is obvious
    // that we should use the `static` mode
    if(isEffectivelyStatic(decl, parentDecl))
        return kParameterListCollectMode_Static;

    // Otherwise, let's default to collecting everything
    return kParameterListCollectMode_Default;
}
//
// When dealing with a member function, we need to be able to add the `this`
// parameter for the enclosing type:
//
void addThisParameter(
    ParameterDirection  direction,
    Type*               type,
    ParameterLists*     ioParameterLists)
{
    IRLoweringParameterInfo info;
    info.type = type;
    info.decl = nullptr;
    info.direction = direction;
    info.isThisParam = true;

    ioParameterLists->params.add(info);
}
//
// And here is our function that will do the recursive walk:
void collectParameterLists(
    IRGenContext*               context,
    DeclRef<Decl> const&        declRef,
    ParameterLists*             ioParameterLists,
    ParameterListCollectMode    mode,
    ParameterDirection          thisParamDirection)
{
    // The parameters introduced by any "parent" declarations
    // will need to come first, so we'll deal with that
    // logic here.
    if( auto parentDeclRef = declRef.getParent() )
    {
        // Compute the mode to use when collecting parameters from
        // the outer declaration. The most important question here
        // is whether parameters of the outer declaration should
        // also count as parameters of the inner declaration.
        ParameterListCollectMode innerMode = getModeForCollectingParentParameters(declRef, parentDeclRef);

        // Don't down-grade our `static`-ness along the chain.
        if(innerMode < mode)
            innerMode = mode;

        ParameterDirection innerThisParamDirection = getThisParamDirection(declRef, thisParamDirection);


        // Now collect any parameters from the parent declaration itself
        collectParameterLists(context, parentDeclRef, ioParameterLists, innerMode, innerThisParamDirection);

        // We also need to consider whether the inner declaration needs to have a `this`
        // parameter corresponding to the outer declaration.
        if( innerMode != kParameterListCollectMode_Static )
        {
            auto thisType = getThisParamTypeForContainer(context, parentDeclRef);
            if(thisType)
            {
                addThisParameter(innerThisParamDirection, thisType, ioParameterLists);
            }
        }
    }

    // Once we've added any parameters based on parent declarations,
    // we can see if this declaration itself introduces parameters.
    //
    if( auto callableDeclRef = declRef.as<CallableDecl>() )
    {
        // Don't collect parameters from the outer scope if
        // we are in a `static` context.
        if( mode == kParameterListCollectMode_Default )
        {
            for( auto paramDeclRef : getParameters(callableDeclRef) )
            {
                ioParameterLists->params.add(getParameterInfo(context, paramDeclRef));
            }
        }
    }
}

bool isConstExprVar(Decl* decl)
{
    if( decl->hasModifier<ConstExprModifier>() )
    {
        return true;
    }
    else if(decl->hasModifier<HLSLStaticModifier>() && decl->hasModifier<ConstModifier>())
    {
        return true;
    }

    return false;
}


IRType* maybeGetConstExprType(
    IRBuilder*      builder,
    IRType*         type,
    Decl*           decl)
{
    if(isConstExprVar(decl))
    {
        return builder->getRateQualifiedType(
            builder->getConstExprRate(),
            type);
    }

    return type;
}


struct FuncDeclBaseTypeInfo
{
    IRType*         type;
    IRType*         resultType;
    ParameterLists  parameterLists;
    List<IRType*>   paramTypes;
};

void _lowerFuncDeclBaseTypeInfo(
    IRGenContext*               context,
    DeclRef<FunctionDeclBase>   declRef,
    FuncDeclBaseTypeInfo&       outInfo)
{
    auto builder = context->irBuilder;

    // Collect the parameter lists we will use for our new function.
    auto& parameterLists = outInfo.parameterLists;
    collectParameterLists(
        context,
        declRef,
        &parameterLists, kParameterListCollectMode_Default, kParameterDirection_In);

    auto& paramTypes = outInfo.paramTypes;

    for( auto paramInfo : parameterLists.params )
    {
        IRType* irParamType = lowerType(context, paramInfo.type);

        switch( paramInfo.direction )
        {
        case kParameterDirection_In:
            // Simple case of a by-value input parameter.
            break;

        // If the parameter is declared `out` or `inout`,
        // then we will represent it with a pointer type in
        // the IR, but we will use a specialized pointer
        // type that encodes the parameter direction information.
        case kParameterDirection_Out:
            irParamType = builder->getOutType(irParamType);
            break;
        case kParameterDirection_InOut:
            irParamType = builder->getInOutType(irParamType);
            break;
        case kParameterDirection_Ref:
            irParamType = builder->getRefType(irParamType);
            break;

        default:
            SLANG_UNEXPECTED("unknown parameter direction");
            break;
        }

        // If the parameter was explicitly marked as being a compile-time
        // constant (`constexpr`), then attach that information to its
        // IR-level type explicitly.
        if( paramInfo.decl )
        {
            irParamType = maybeGetConstExprType(builder, irParamType, paramInfo.decl);
        }

        if (paramInfo.decl && paramInfo.decl->hasModifier<HLSLGroupSharedModifier>())
        {
            irParamType = builder->getRateQualifiedType(builder->getGroupSharedRate(), irParamType);
        }

        paramTypes.add(irParamType);
    }

    auto& irResultType = outInfo.resultType;
    irResultType = lowerType(context, getResultType(context->astBuilder, declRef));

    if (auto setterDeclRef = declRef.as<SetterDecl>())
    {
        // A `set` accessor always returns `void`
        //
        // TODO: We should handle this by making the result
        // type of a `set` accessor be represented accurately
        // at the AST level (ditto for the `ref` case below).
        //
        irResultType = builder->getVoidType();
    }

    if( auto refAccessorDeclRef = declRef.as<RefAccessorDecl>() )
    {
        // A `ref` accessor needs to return a *pointer* to the value
        // being accessed, rather than a simple value.
        irResultType = builder->getPtrType(irResultType);
    }
  
    if (!getErrorCodeType(context->astBuilder, declRef)->equals(context->astBuilder->getBottomType()))
    {
        auto errorType = lowerType(context, getErrorCodeType(context->astBuilder, declRef));
        IRAttr* throwTypeAttr = nullptr;
        throwTypeAttr = builder->getAttr(kIROp_FuncThrowTypeAttr, 1, (IRInst**)&errorType);
        outInfo.type = builder->getFuncType(
            paramTypes.getCount(), paramTypes.getBuffer(), irResultType, throwTypeAttr);
    }
    else
    {
        outInfo.type =
            builder->getFuncType(paramTypes.getCount(), paramTypes.getBuffer(), irResultType);
    }
}

static LoweredValInfo _emitCallToAccessor(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<AccessorDecl>   accessorDeclRef,
    LoweredValInfo          base,
    UInt                    argCount,
    IRInst* const*          args)
{
    FuncDeclBaseTypeInfo info;
    _lowerFuncDeclBaseTypeInfo(context, accessorDeclRef, info);

    List<IRInst*> allArgs;

    List<OutArgumentFixup> fixups;
    if(base.flavor != LoweredValInfo::Flavor::None)
    {
        SLANG_ASSERT(info.parameterLists.params.getCount() >= 1);
        SLANG_ASSERT(info.parameterLists.params[0].isThisParam);

        auto thisParam = info.parameterLists.params[0];
        auto thisParamType = lowerType(context, thisParam.type);

        addArg(context, &allArgs, &fixups, base, thisParamType, thisParam.direction, SourceLoc());
    }

    allArgs.addRange(args, argCount);

    LoweredValInfo result = emitCallToDeclRef(
        context,
        type,
        accessorDeclRef,
        info.type,
        allArgs.getCount(),
        allArgs.getBuffer(),
        TryClauseEnvironment());

    applyOutArgumentFixups(context, fixups);

    return result;
}

//

template<typename Derived>
struct ExprLoweringVisitorBase : ExprVisitor<Derived, LoweredValInfo>
{
    static bool isLValueContext() { return Derived::_isLValueContext(); }

    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }
    ASTBuilder* getASTBuilder() { return context->astBuilder; }

    // Lower an expression that should have the same l-value-ness
    // as the visitor itself.
    LoweredValInfo lowerSubExpr(Expr* expr)
    {
        IRBuilderSourceLocRAII sourceLocInfo(getBuilder(), expr->loc);
        return this->dispatch(expr);
    }

    LoweredValInfo lowerSubExpr(Expr* expr, IRGenContext* subContext)
    {
        IRBuilderSourceLocRAII sourceLocInfo(getBuilder(), expr->loc);
        Derived d;
        d.context = subContext;
        return d.dispatch(expr);
    }

    LoweredValInfo visitIncompleteExpr(IncompleteExpr*)
    {
        SLANG_UNEXPECTED("a valid ast should not contain an IncompleteExpr.");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitVarExpr(VarExpr* expr)
    {
        LoweredValInfo info = emitDeclRef(
            context,
            expr->declRef,
            lowerType(context, expr->type));
        return info;
    }

    // Emit IR to denote the forward-mode derivative
    // of the inner func-expr. This will be resolved 
    // to a concrete function during the derivative 
    // pass.
    LoweredValInfo visitJVPDifferentiateExpr(JVPDifferentiateExpr* expr)
    {
        auto baseVal = lowerSubExpr(expr->baseFunction);
        SLANG_ASSERT(baseVal.flavor == LoweredValInfo::Flavor::Simple);

        return LoweredValInfo::simple(
            getBuilder()->emitJVPDifferentiateInst(
                lowerType(context, expr->type),
                baseVal.val));
    }

    LoweredValInfo visitOverloadedExpr(OverloadedExpr* /*expr*/)
    {
        SLANG_UNEXPECTED("overloaded expressions should not occur in checked AST");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitOverloadedExpr2(OverloadedExpr2* /*expr*/)
    {
        SLANG_UNEXPECTED("overloaded expressions should not occur in checked AST");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitIndexExpr(IndexExpr* expr)
    {
        auto type = lowerType(context, expr->type);
        auto baseVal = lowerSubExpr(expr->baseExpression);
        auto indexVal = getSimpleVal(context, lowerRValueExpr(context, expr->indexExpression));

        return subscriptValue(type, baseVal, indexVal);
    }

    LoweredValInfo visitThisExpr(ThisExpr* /*expr*/)
    {
        return context->thisVal;
    }

    LoweredValInfo visitMemberExpr(MemberExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);

        auto baseExpr = expr->baseExpression;
        baseExpr = maybeIgnoreCastToInterface(baseExpr);
        auto loweredBase = lowerSubExpr(baseExpr);

        auto declRef = expr->declRef;
        if (auto fieldDeclRef = declRef.as<VarDecl>())
        {
            // Okay, easy enough: we have a reference to a field of a struct type...
            return extractField(loweredType, loweredBase, fieldDeclRef);
        }
        else if (auto callableDeclRef = declRef.as<CallableDecl>())
        {
            RefPtr<BoundMemberInfo> boundMemberInfo = new BoundMemberInfo();
            boundMemberInfo->type = nullptr;
            boundMemberInfo->base = loweredBase;
            boundMemberInfo->declRef = callableDeclRef;
            return LoweredValInfo::boundMember(boundMemberInfo);
        }
        else if(auto propertyDeclRef = declRef.as<PropertyDecl>())
        {
            // A reference to a property is a special case, because
            // we must translate the reference to the property
            // into a reference to one of its accessors.
            //
            return lowerStorageReference(context, loweredType, propertyDeclRef, loweredBase, 0, nullptr);
        }

        SLANG_UNIMPLEMENTED_X("codegen for member expression");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    // We will always lower a dereference expression (`*ptr`)
    // as an l-value, since that is the easiest way to handle it.
    LoweredValInfo visitDerefExpr(DerefExpr* expr)
    {
        auto loweredBase = lowerRValueExpr(context, expr->base);

        // TODO: handle tupel-type for `base`

        // The type of the lowered base must by some kind of pointer,
        // in order for a dereference to make senese, so we just
        // need to extract the value type from that pointer here.
        //
        IRInst* loweredBaseVal = getSimpleVal(context, loweredBase);
        IRType* loweredBaseType = loweredBaseVal->getDataType();

        if (as<IRPointerLikeType>(loweredBaseType)
            || as<IRPtrTypeBase>(loweredBaseType))
        {
            // Note that we do *not* perform an actual `load` operation
            // here, but rather just use the pointer value to construct
            // an appropriate `LoweredValInfo` representing the underlying
            // dereference.
            //
            // This is important so that an expression like `&((*foo).bar)`
            // (which is desugared from `&foo->bar`) can be handled; such
            // an expression does *not* perform a dereference at runtime,
            // and is just a bit of pointer math.
            //
            return LoweredValInfo::ptr(loweredBaseVal);
        }
        else
        {
            SLANG_UNIMPLEMENTED_X("codegen for deref expression");
            UNREACHABLE_RETURN(LoweredValInfo());
        }
    }

    LoweredValInfo visitParenExpr(ParenExpr* expr)
    {
        return lowerSubExpr(expr->base);
    }

    LoweredValInfo getSimpleDefaultVal(IRType* type)
    {
        if(auto basicType = as<IRBasicType>(type))
        {
            switch( basicType->getBaseType() )
            {
            default:
                SLANG_UNEXPECTED("missing case for getting IR default value");
                UNREACHABLE_RETURN(LoweredValInfo());
                break;

            case BaseType::Bool:
                return LoweredValInfo::simple(getBuilder()->getBoolValue(false));

            case BaseType::Int8:
            case BaseType::Int16:
            case BaseType::Int:
            case BaseType::Int64:
            case BaseType::UInt8:
            case BaseType::UInt16:
            case BaseType::UInt:
            case BaseType::UInt64:
                return LoweredValInfo::simple(getBuilder()->getIntValue(type, 0));

            case BaseType::Half:
            case BaseType::Float:
            case BaseType::Double:
                return LoweredValInfo::simple(getBuilder()->getFloatValue(type, 0.0));
            }
        }

        SLANG_UNEXPECTED("missing case for getting IR default value");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo getDefaultVal(Type* type)
    {
        auto irType = lowerType(context, type);
        if (auto basicType = as<BasicExpressionType>(type))
        {
            return getSimpleDefaultVal(irType);
        }
        else if (auto vectorType = as<VectorExpressionType>(type))
        {
            UInt elementCount = (UInt) getIntVal(vectorType->elementCount);

            auto irDefaultValue = getSimpleVal(context, getDefaultVal(vectorType->elementType));

            List<IRInst*> args;
            for(UInt ee = 0; ee < elementCount; ++ee)
            {
                args.add(irDefaultValue);
            }
            return LoweredValInfo::simple(
                getBuilder()->emitMakeVector(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto matrixType = as<MatrixExpressionType>(type))
        {
            UInt rowCount = (UInt) getIntVal(matrixType->getRowCount());

            auto rowType = matrixType->getRowType();

            auto irDefaultValue = getSimpleVal(context, getDefaultVal(rowType));

            List<IRInst*> args;
            for(UInt rr = 0; rr < rowCount; ++rr)
            {
                args.add(irDefaultValue);
            }
            return LoweredValInfo::simple(
                getBuilder()->emitMakeMatrix(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto arrayType = as<ArrayExpressionType>(type))
        {
            UInt elementCount = (UInt) getIntVal(arrayType->arrayLength);

            auto irDefaultElement = getSimpleVal(context, getDefaultVal(arrayType->baseType));

            List<IRInst*> args;
            for(UInt ee = 0; ee < elementCount; ++ee)
            {
                args.add(irDefaultElement);
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeArray(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto declRefType = as<DeclRefType>(type))
        {
            DeclRef<Decl> declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
            {
                List<IRInst*> args;

                if (auto structTypeDeclRef = aggTypeDeclRef.as<StructDecl>())
                {
                    if (auto baseStructType = findBaseStructType(getASTBuilder(), structTypeDeclRef))
                    {
                        auto irBaseVal = getSimpleVal(context, getDefaultVal(baseStructType));
                        args.add(irBaseVal);
                    }
                }

                for (auto ff : getMembersOfType<VarDecl>(aggTypeDeclRef, MemberFilterStyle::Instance))
                {
                    auto irFieldVal = getSimpleVal(context, getDefaultVal(ff));
                    args.add(irFieldVal);
                }

                return LoweredValInfo::simple(
                    getBuilder()->emitMakeStruct(irType, args.getCount(), args.getBuffer()));
            }
        }

        SLANG_UNEXPECTED("unexpected type when creating default value");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo getDefaultVal(DeclRef<VarDeclBase> decl)
    {
        if(auto initExpr = decl.getDecl()->initExpr)
        {
            return lowerRValueExpr(context, initExpr);
        }
        else
        {
            Type* type = decl.substitute(getASTBuilder(), decl.getDecl()->type);
            SLANG_ASSERT(type);
            return getDefaultVal(type);
        }
    }

    LoweredValInfo visitInitializerListExpr(InitializerListExpr* expr)
    {
        // Allocate a temporary of the given type
        auto type = expr->type;
        IRType* irType = lowerType(context, type);
        List<IRInst*> args;

        UInt argCount = expr->args.getCount();

        // If the initializer list was empty, then the user was
        // asking for default initialization, which should apply
        // to (almost) any type.
        //
        if(argCount == 0)
        {
            return getDefaultVal(type.type);
        }

        // Now for each argument in the initializer list,
        // fill in the appropriate field of the result
        if (auto arrayType = as<ArrayExpressionType>(type))
        {
            UInt elementCount = (UInt) getIntVal(arrayType->arrayLength);

            for (UInt ee = 0; ee < argCount; ++ee)
            {
                auto argExpr = expr->args[ee];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.add(getSimpleVal(context, argVal));
            }
            if(elementCount > argCount)
            {
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(arrayType->baseType));
                for(UInt ee = argCount; ee < elementCount; ++ee)
                {
                    args.add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeArray(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto vectorType = as<VectorExpressionType>(type))
        {
            UInt elementCount = (UInt) getIntVal(vectorType->elementCount);

            for (UInt ee = 0; ee < argCount; ++ee)
            {
                auto argExpr = expr->args[ee];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.add(getSimpleVal(context, argVal));
            }
            if(elementCount > argCount)
            {
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(vectorType->elementType));
                for(UInt ee = argCount; ee < elementCount; ++ee)
                {
                    args.add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeVector(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto matrixType = as<MatrixExpressionType>(type))
        {
            UInt rowCount = (UInt) getIntVal(matrixType->getRowCount());

            for (UInt rr = 0; rr < argCount; ++rr)
            {
                auto argExpr = expr->args[rr];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.add(getSimpleVal(context, argVal));
            }
            if(rowCount > argCount)
            {
                auto rowType = matrixType->getRowType();
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(rowType));

                for(UInt rr = argCount; rr < rowCount; ++rr)
                {
                    args.add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeMatrix(irType, args.getCount(), args.getBuffer()));
        }
        else if (auto declRefType = as<DeclRefType>(type))
        {
            DeclRef<Decl> declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
            {
                UInt argCounter = 0;

                // If the type is a structure type that inherits from another
                // structure type, then we need to treat the base type as
                // an implicit first field.
                //
                if(auto structTypeDeclRef = aggTypeDeclRef.as<StructDecl>())
                {
                    if (auto baseStructType = findBaseStructType(getASTBuilder(), structTypeDeclRef))
                    {
                        UInt argIndex = argCounter++;
                        if (argIndex < argCount)
                        {
                            auto argExpr = expr->args[argIndex];
                            LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                            args.add(getSimpleVal(context, argVal));
                        }
                        else
                        {
                            auto irDefaultValue = getSimpleVal(context, getDefaultVal(baseStructType));
                            args.add(irDefaultValue);
                        }
                    }
                }

                for (auto ff : getMembersOfType<VarDecl>(aggTypeDeclRef, MemberFilterStyle::Instance))
                {
                    UInt argIndex = argCounter++;
                    if (argIndex < argCount)
                    {
                        auto argExpr = expr->args[argIndex];
                        LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                        args.add(getSimpleVal(context, argVal));
                    }
                    else
                    {
                        auto irDefaultValue = getSimpleVal(context, getDefaultVal(ff));
                        args.add(irDefaultValue);
                    }
                }

                return LoweredValInfo::simple(
                    getBuilder()->emitMakeStruct(irType, args.getCount(), args.getBuffer()));
            }
        }

        // If none of the above cases matched, then we had better
        // have zero arguments in the initializer list, in which
        // case we are just looking for default initialization.
        //
        SLANG_UNEXPECTED("unhandled case for initializer list codegen");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitBoolLiteralExpr(BoolLiteralExpr* expr)
    {
        return LoweredValInfo::simple(context->irBuilder->getBoolValue(expr->value));
    }

    LoweredValInfo visitNullPtrLiteralExpr(NullPtrLiteralExpr*)
    {
        return LoweredValInfo::simple(context->irBuilder->getPtrValue(nullptr));
    }

    LoweredValInfo visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
    {
        auto type = lowerType(context, expr->type);
        return LoweredValInfo::simple(context->irBuilder->getIntValue(type, expr->value));
    }

    LoweredValInfo visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr)
    {
        auto type = lowerType(context, expr->type);
        return LoweredValInfo::simple(context->irBuilder->getFloatValue(type, expr->value));
    }

    LoweredValInfo visitStringLiteralExpr(StringLiteralExpr* expr)
    {
        auto irLit = context->irBuilder->getStringValue(expr->value.getUnownedSlice());
        context->shared->m_stringLiterals.add(irLit);
        return LoweredValInfo::simple(irLit);
    }

    LoweredValInfo visitAggTypeCtorExpr(AggTypeCtorExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("codegen for aggregate type constructor expression");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    void _lowerSubstitutionArg(IRGenContext* subContext, GenericSubstitution* subst, Decl* paramDecl, Index argIndex)
    {
        SLANG_ASSERT(argIndex < subst->args.getCount());
        auto argVal = lowerVal(subContext, subst->args[argIndex]);
        setValue(subContext, paramDecl, argVal);
    }

    void _lowerSubstitutionEnv(IRGenContext* subContext, Substitutions* subst)
    {
        if(!subst) return;
        _lowerSubstitutionEnv(subContext, subst->outer);

        if (auto genSubst = as<GenericSubstitution>(subst))
        {
            auto genDecl = genSubst->genericDecl;

            Index argCounter = 0;
            for( auto memberDecl: genDecl->members )
            {
                if(auto typeParamDecl = as<GenericTypeParamDecl>(memberDecl) )
                {
                    _lowerSubstitutionArg(subContext, genSubst, typeParamDecl, argCounter++);
                }
                else if( auto valParamDecl = as<GenericValueParamDecl>(memberDecl) )
                {
                    _lowerSubstitutionArg(subContext, genSubst, valParamDecl, argCounter++);
                }
            }
            for( auto memberDecl: genDecl->members )
            {
                if(auto constraintDecl = as<GenericTypeConstraintDecl>(memberDecl) )
                {
                    _lowerSubstitutionArg(subContext, genSubst, constraintDecl, argCounter++);
                }
            }
        }
        // TODO: also need to handle this-type substitution here?
    }

        /// Create IR instructions for an argument at a call site, based on
        /// AST-level expressions plus function signature information.
        ///
        /// The `funcType` parameter is always required, and specifies the types
        /// of all the parameters. The `funcDeclRef` parameter is only required
        /// if there are parameter positions for which the matching argument is
        /// absent.
        ///
    void addDirectCallArgs(
        InvokeExpr*             expr,
        Index                   argIndex,
        IRType*                 paramType,
        ParameterDirection      paramDirection,
        DeclRef<ParamDecl>      paramDeclRef,
        List<IRInst*>*          ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        Count argCount = expr->arguments.getCount();
        if (argIndex < argCount)
        {
            auto argExpr = expr->arguments[argIndex];
            addCallArgsForParam(context, paramType, paramDirection, argExpr, ioArgs, ioFixups);
        }
        else
        {
            // We have run out of arguments supplied at the call site,
            // but there are still parameters remaining. This must mean
            // that these parameters have default argument expressions
            // associated with them.
            //
            // Currently we simply extract the initial-value expression
            // from the parameter declaration and then lower it in
            // the context of the caller.
            //
            // Note that the expression could involve subsitutions because
            // in the general case it could depend on the generic parameters
            // used the specialize the callee. For now we do not handle that
            // case, and simply ignore generic arguments.
            //
            SubstExpr<Expr> argExpr = getInitExpr(getASTBuilder(), paramDeclRef);
            SLANG_ASSERT(argExpr);

            IRGenEnv subEnvStorage;
            IRGenEnv* subEnv = &subEnvStorage;
            subEnv->outer = context->env;

            IRGenContext subContextStorage = *context;
            IRGenContext* subContext = &subContextStorage;
            subContext->env = subEnv;

            _lowerSubstitutionEnv(subContext, argExpr.getSubsts());

            addCallArgsForParam(subContext, paramType, paramDirection, argExpr.getExpr(), ioArgs, ioFixups);

            // TODO: The approach we are taking here to default arguments
            // is simplistic, and has consequences for the front-end as
            // well as binary serialization of modules.
            //
            // We could consider some more refined approaches where, e.g.,
            // functions with default arguments generate multiple IR-level
            // functions, that compute and provide the default values.
            //
            // Alternatively, each parameter with defaults could be generated
            // into its own callable function that provides the default value,
            // so that calling modules can call into a pre-generated function.
            //
            // Each of these options involves trade-offs, and we need to
            // make a conscious decision at some point.

            // Assert that such an expression must have been present.
        }
    }

    void addDirectCallArgs(
        InvokeExpr*             expr,
        FuncType*               funcType,
        List<IRInst*>*          ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        Count argCount = expr->arguments.getCount();
        SLANG_ASSERT(argCount == static_cast<Count>(funcType->getParamCount()));

        for(Index i = 0; i < argCount; ++i)
        {
            IRType* paramType = lowerType(context, funcType->getParamType(i));
            ParameterDirection paramDirection = funcType->getParamDirection(i);
            addDirectCallArgs(expr, i, paramType, paramDirection, DeclRef<ParamDecl>(), ioArgs, ioFixups);
        }
    }


    void addDirectCallArgs(
        InvokeExpr*             expr,
        DeclRef<CallableDecl>   funcDeclRef,
        List<IRInst*>*          ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        Count argCounter = 0;
        for (auto paramDeclRef : getMembersOfType<ParamDecl>(funcDeclRef))
        {
            auto paramDecl = paramDeclRef.getDecl();
            IRType* paramType = lowerType(context, getType(getASTBuilder(), paramDeclRef));
            auto paramDirection = getParameterDirection(paramDecl);

            Index argIndex = argCounter++;
            addDirectCallArgs(expr, argIndex, paramType, paramDirection, paramDeclRef, ioArgs, ioFixups);
        }
    }

    // Add arguments that appeared directly in an argument list
    // to the list of argument values for a call.
    void addDirectCallArgs(
        InvokeExpr*             expr,
        DeclRef<Decl>           funcDeclRef,
        List<IRInst*>*         ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        if (auto callableDeclRef = funcDeclRef.as<CallableDecl>())
        {
            addDirectCallArgs(expr, callableDeclRef, ioArgs, ioFixups);
        }
        else
        {
            SLANG_UNEXPECTED("callee was not a callable decl");
        }
    }

    void addFuncBaseArgs(
        LoweredValInfo funcVal,
        List<IRInst*>* ioArgs)
    {
        switch (funcVal.flavor)
        {
        default:
            return;
        }
    }

    struct ResolvedCallInfo
    {
        DeclRef<Decl>   funcDeclRef;
        Expr*           baseExpr = nullptr;
    };

    // Try to resolve a the function expression for a call
    // into a reference to a specific declaration, along
    // with some contextual information about the declaration
    // we are calling.
    bool tryResolveDeclRefForCall(
        Expr*        funcExpr,
        ResolvedCallInfo*   outInfo)
    {
        // TODO: unwrap any "identity" expressions that might
        // be wrapping the callee.

        // First look to see if the expression references a
        // declaration at all.
        auto declRefExpr = as<DeclRefExpr>(funcExpr);
        if(!declRefExpr)
            return false;

        // A little bit of future proofing here: if we ever
        // allow higher-order functions, then we might be
        // calling through a variable/field that has a function
        // type, but is not itself a function.
        // In such a case we should be careful to not statically
        // resolve things.
        //
        if(auto callableDecl = as<CallableDecl>(declRefExpr->declRef.getDecl()))
        {
            // Okay, the declaration is directly callable, so we can continue.
        }
        else
        {
            // The callee declaration isn't itself a callable (it must have
            // a function type, though).
            return false;
        }

        // Now we can look at the specific kinds of declaration references,
        // and try to tease them apart.
        if (auto memberFuncExpr = as<MemberExpr>(funcExpr))
        {
            outInfo->funcDeclRef = memberFuncExpr->declRef;
            outInfo->baseExpr = memberFuncExpr->baseExpression;
            return true;
        }
        else if (auto staticMemberFuncExpr = as<StaticMemberExpr>(funcExpr))
        {
            outInfo->funcDeclRef = staticMemberFuncExpr->declRef;
            return true;
        }
        else if (auto varExpr = as<VarExpr>(funcExpr))
        {
            outInfo->funcDeclRef = varExpr->declRef;
            return true;
        }
        else
        {
            // Seems to be a case of declaration-reference we don't know about.
            SLANG_UNEXPECTED("unknown declaration reference kind");
            //return false;
        }
    }

        /// Return `expr` with any outer casts to interface types stripped away
    Expr* maybeIgnoreCastToInterface(Expr* expr)
    {
        auto e = expr;
        while( auto castExpr = as<CastToSuperTypeExpr>(e) )
        {
            if(auto declRefType = as<DeclRefType>(e->type))
            {
                if(declRefType->declRef.as<InterfaceDecl>())
                {
                    e = castExpr->valueArg;
                    continue;
                }
            }
            else if( auto andType = as<AndType>(e->type) )
            {
                // TODO: We might eventually need to tell the difference
                // between conjunctions of interfaces and conjunctions
                // that might include non-interface types.
                //
                // For now we assume that any case to a conjunction
                // is effectively a cast to an interface type.
                //
                e = castExpr->valueArg;
                continue;
            }
            break;
        }
        return e;
    }

    LoweredValInfo visitInvokeExpr(InvokeExpr* expr)
    {
        return visitInvokeExprImpl(expr, TryClauseEnvironment());
    }

    LoweredValInfo visitInvokeExprImpl(InvokeExpr* expr, const TryClauseEnvironment& tryEnv)
    {
        auto type = lowerType(context, expr->type);

        // We are going to look at the syntactic form of
        // the "function" expression, so that we can avoid
        // a lot of complexity that would come from lowering
        // it as a general expression first, and then trying
        // to apply it. For example, given `obj.f(a,b)` we
        // will try to detect that we are trying to compute
        // something like `ObjType::f(obj, a, b)` (in pseudo-code),
        // rather than trying to construct a meaningful
        // intermediate value for `obj.f` first.
        //
        // Note that this doe not preclude having support
        // for directly generating code from `obj.f` - it
        // just may be that such usage is more complicated.

        // Along the way, we may end up collecting additional
        // arguments that will be part of the call.
        List<IRInst*> irArgs;

        // We will also collect "fixup" actions that need
        // to be performed after the call, in order to
        // copy the final values for `out` parameters
        // back to their arguments.
        List<OutArgumentFixup> argFixups;

        auto funcExpr = expr->functionExpr;
        ResolvedCallInfo resolvedInfo;
        if (tryResolveDeclRefForCall(funcExpr, &resolvedInfo))
        {
            // In this case we know exactly what declaration we
            // are going to call, and so we can resolve things
            // appropriately.
            auto funcDeclRef = resolvedInfo.funcDeclRef;
            auto baseExpr = resolvedInfo.baseExpr;

            // If the thing being invoked is a subscript operation,
            // then we need to handle multiple extra details
            // that don't arise for other kinds of calls.
            //
            // TODO: subscript operations probably deserve to
            // be handled on their own path for this reason...
            //
            if (auto subscriptDeclRef = funcDeclRef.template as<SubscriptDecl>())
            {
                // A reference to a subscript declaration is a special case,
                // because it is not possible to call a subscript directly;
                // we must call one of its accessors.
                //
                auto loweredBase = lowerSubExpr(baseExpr);
                addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
                auto result = lowerStorageReference(context, type, subscriptDeclRef, loweredBase, irArgs.getCount(), irArgs.getBuffer());

                // TODO: Applying the fixups for arguments to the subscript at this point
                // won't technically be correct, since the call to the subscript may
                // not have occured at this point.
                //
                // It seems like we need to either:
                //
                // * Capture the arguments to the subscript as `LoweredValInfo` instead of `IRInst*`
                //   so that we can deal with everything related to fixups around the actual call
                //   site.
                //
                // OR
                //
                // * Handle everything to do with "fixups" differently, by treating them as deferred
                // actions that gert queued up on the context itself and then flushed at certain
                // well-defined points, so that we don't have to be as careful around them.
                //
                // OR
                //
                // * Switch to a more "destination-driven" approach to code generation, where we
                // can determine on entry to the lowering of a sub-expression whether it will be
                // used for read, write, or read/write, and resolve things like the choice of
                // accessor at that point instead.
                //
                applyOutArgumentFixups(context, argFixups);
                return result;
            }

            // First comes the `this` argument if we are calling
            // a member function:
            if (baseExpr)
            {
                // The base expression might be an "upcast" to a base interface, in
                // which case we don't want to emit the result of the cast, but instead
                // the source.
                //
                baseExpr = maybeIgnoreCastToInterface(baseExpr);

                auto thisType = getThisParamTypeForCallable(context, funcDeclRef);
                auto irThisType = lowerType(context, thisType);
                addCallArgsForParam(
                    context,
                    irThisType,
                    getThisParamDirection(funcDeclRef.getDecl(), kParameterDirection_In),
                    baseExpr,
                    &irArgs,
                    &argFixups);
            }

            // Then we have the "direct" arguments to the call.
            // These may include `out` and `inout` arguments that
            // require "fixup" work on the other side.
            //
            auto funcType = lowerType(context, funcExpr->type);
            addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
            auto result = emitCallToDeclRef(
                context,
                type,
                funcDeclRef,
                funcType,
                irArgs,
                tryEnv);
            applyOutArgumentFixups(context, argFixups);
            return result;
        }
        else if(auto funcType = as<FuncType>(expr->functionExpr->type))
        {
            auto funcVal = lowerRValueExpr(context, expr->functionExpr);
            addDirectCallArgs(expr, funcType, &irArgs, &argFixups);

            auto result = emitCallToVal(context, type, funcVal, irArgs.getCount(), irArgs.getBuffer(), tryEnv);

            applyOutArgumentFixups(context, argFixups);
            return result;
        }
        

        // TODO: In this case we should be emitting code for the callee as
        // an ordinary expression, then emitting the arguments according
        // to the type information on the callee (e.g., which parameters
        // are `out` or `inout`, and then finally emitting the `call`
        // instruction.
        //
        // We don't currently have the case of emitting arguments according
        // to function type info (instead of declaration info), and really
        // this case can't occur unless we start adding first-class functions
        // to the source language.
        //
        // For now we just bail out with an error.
        //
        SLANG_UNEXPECTED("could not resolve target declaration for call");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

        /// Emit code for a `try` invoke.
    LoweredValInfo visitTryExpr(TryExpr* expr)
    {
        auto invokeExpr = as<InvokeExpr>(expr->base);
        assert(invokeExpr);
        TryClauseEnvironment tryEnv;
        tryEnv.clauseType = expr->tryClauseType;
        return visitInvokeExprImpl(invokeExpr, tryEnv);
    }

        /// Emit code to cast `value` to a concrete `superType` (e.g., a `struct`).
        ///
        /// The `subTypeWitness` is expected to witness the sub-type relationship
        /// by naming a field (or chain of fields) that leads from the type of
        /// `value` to the field that stores its members for `superType`.
        ///
    LoweredValInfo emitCastToConcreteSuperTypeRec(
        LoweredValInfo const&   value,
        IRType*                 superType,
        Val*                    subTypeWitness)
    {
        if( auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subTypeWitness) )
        {
            return extractField(superType, value, declaredSubtypeWitness->declRef);
        }
        else
        {
            SLANG_ASSERT(!"unhandled");
            return nullptr;
        }
    }

    LoweredValInfo visitCastToSuperTypeExpr(
        CastToSuperTypeExpr* expr)
    {
        auto superType = lowerType(context, expr->type);
        auto value = lowerRValueExpr(context, expr->valueArg);

        // The actual operation that we need to perform here
        // depends on the kind of subtype relationship we
        // are making use of.
        //
        // The first important case is when the super type is
        // an interface type, such that casting from a concrete
        // value to that type creates a value of existential
        // type that binds together the concrete value and the
        // witness table that represents the subtype relationship.
        //
        if( auto declRefType = as<DeclRefType>(expr->type) )
        {
            auto declRef = declRefType->declRef;
            if( auto interfaceDeclRef = declRef.as<InterfaceDecl>() )
            {
                // We have an expression that is "up-casting" some concrete value
                // to an existential type (aka interface type), using a subtype witness
                // (which will lower as a witness table) to show that the conversion
                // is valid.
                //
                auto witnessTable = lowerSimpleVal(context, expr->witnessArg);

                // At the IR level, this will become a `makeExistential` instruction,
                // which collects the above information into a single IR-level value.
                // A dynamic CPU implementation of Slang might encode an existential
                // as a "fat pointer" representation, which includes a pointer to
                // data for the concrete value, plus a pointer to the witness table.
                //
                // Note: if/when Slang supports more general existential types, such
                // as compositions of interface (e.g., `IReadable & IWritable`), then
                // we should probably extend the AST and IR mechanism here to accept
                // a sequence of witness tables.
                //
                auto concreteValue = getSimpleVal(context, value);
                auto existentialValue = getBuilder()->emitMakeExistential(
                    superType,
                    concreteValue,
                    witnessTable);
                return LoweredValInfo::simple(existentialValue);
            }
            else if( auto structDeclRef = declRef.as<StructDecl>() )
            {
                // We are up-casting to a concrete `struct` super-type,
                // such that the witness will represent a field of the super-type
                // that is stored in instances of the sub-type (or a chain
                // of such fields for a transitive witness).
                //
                return emitCastToConcreteSuperTypeRec(value, superType, expr->witnessArg);
            }
        }

        SLANG_UNEXPECTED("unexpected case of subtype relationship");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitModifierCastExpr(
        ModifierCastExpr* expr)
    {
        return this->dispatch(expr->valueArg);
    }

    LoweredValInfo subscriptValue(
        IRType*         type,
        LoweredValInfo  baseVal,
        IRInst*         indexVal)
    {
        auto builder = getBuilder();

        // The `tryGetAddress` operation will take a complex value representation
        // and try to turn it into a single pointer, if possible.
        //
        baseVal = tryGetAddress(context, baseVal, TryGetAddressMode::Aggressive);

        // The `materialize` operation should ensure that we only have to deal
        // with the small number of base cases for lowered value representations.
        //
        baseVal = materialize(context, baseVal);

        switch (baseVal.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
            return LoweredValInfo::simple(
                builder->emitElementExtract(
                    type,
                    getSimpleVal(context, baseVal),
                    indexVal));

        case LoweredValInfo::Flavor::Ptr:
            return LoweredValInfo::ptr(
                builder->emitElementAddress(
                    context->irBuilder->getPtrType(type),
                    baseVal.val,
                    indexVal));

        default:
            SLANG_UNIMPLEMENTED_X("subscript expr");
            UNREACHABLE_RETURN(LoweredValInfo());
        }

    }

    LoweredValInfo extractField(
        IRType*         fieldType,
        LoweredValInfo  base,
        DeclRef<Decl>   field)
    {
        return Slang::extractField(context, fieldType, base, field);
    }

    LoweredValInfo visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        return emitDeclRef(context, expr->declRef,
            lowerType(context, expr->type));
    }

    LoweredValInfo visitGenericAppExpr(GenericAppExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("generic application expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitSharedTypeExpr(SharedTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("shared type expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitTaggedUnionTypeExpr(TaggedUnionTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("tagged union type expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitThisTypeExpr(ThisTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("this-type expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitAndTypeExpr(AndTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("'&' type expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitModifiedTypeExpr(ModifiedTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("type expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitAssocTypeDecl(AssocTypeDecl* decl)
    {
        SLANG_UNIMPLEMENTED_X("associatedtype expression during code generation");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitAssignExpr(AssignExpr* expr)
    {
        // Because our representation of lowered "values"
        // can encompass l-values explicitly, we can
        // lower assignment easily. We just lower the left-
        // and right-hand sides, and then perform an assignment
        // based on the resulting values.
        //
        auto leftVal = lowerLValueExpr(context, expr->left);
        auto rightVal = lowerRValueExpr(context, expr->right);
        assign(context, leftVal, rightVal);

        // The result value of the assignment expression is
        // the value of the left-hand side (and it is expected
        // to be an l-value).
        return leftVal;
    }

    LoweredValInfo visitLetExpr(LetExpr* expr)
    {
        // Note: The semantics here are annoyingly subtle.
        //
        // If `expr->decl->initExpr` is an l-value, then we will set things up
        // so that `expr->decl` is bound as an *alias* for that l-value.
        //
        // Otherwise, `expr->decl` will simply be bound to the r-value.
        //
        // The first case is necessary to make `maybeMoveTemp` operations that
        // produce l-value results work correctly, but seems slippery.
        //
        // TODO: We should probably have two AST node types to cover the two
        // different use cases of `LetExpr`: the definitely-immutable case that
        // actually behaves like a `let`, and this other mutable-alias case that
        // feels kind of messy and gross.

        auto initVal = lowerLValueExpr(context, expr->decl->initExpr);
        setGlobalValue(context, expr->decl, initVal);
        auto bodyVal = lowerSubExpr(expr->body);
        return bodyVal;
    }

    LoweredValInfo visitExtractExistentialValueExpr(ExtractExistentialValueExpr* expr)
    {
        // We are being asked to extract the value from an existential, which
        // is itself a single IR op. However, we also need to handle the case
        // where `expr` might be used as an l-value, in which case we need
        // additional information to allow any mutations through the extracted
        // value to be written back.

        auto existentialType = lowerType(context, getType(getASTBuilder(), expr->declRef));
        auto existentialVal = emitDeclRef(context, expr->declRef, existentialType);

        // Note that we make a *copy* of the existential value that is definitely
        // a simple r-value. This ensures that all the `extractExistential*()` operations
        // below work on the same consistent IR value.
        //
        auto existentialValCopy = getSimpleVal(context, existentialVal);

        auto openedType = lowerType(context, expr->type);

        auto extractedVal = getBuilder()->emitExtractExistentialValue(
            openedType, existentialValCopy);

        if(!isLValueContext())
        {
            // If we are in an r-value context, we can directly use the `extractExistentialValue`
            // instruction as the result, and life is simple.
            //
            return LoweredValInfo::simple(extractedVal);
        }

        // In an l-value context, we need to track the information necessary so that
        // if a new/modified value of `openedType` was produced, we could write it
        // back into the original `existentialVal`'s location.
        //
        // The write-back is actually pretty simple: it is just a `makeExisential` op.
        // In order to be able to emit that op later, we need to track the operands
        // that it would use. The first operand would be the new concrete value (which
        // would implicitly encode the concrete type via its IR type) while the second
        // is the witness table for the conformance to the existential.
        //
        // Note: We are assuming/requiring here that any value "written back" must have
        // the exact same concrete type as `extractedVal`, so taht it can use the same
        // IR witness table. The front-end should be enforcing that constraint, and we
        // have no way to check or enforce it at this point.

        auto witnessTable = getBuilder()->emitExtractExistentialWitnessTable(existentialValCopy);

        RefPtr<ExtractedExistentialValInfo> info = new ExtractedExistentialValInfo();
        info->extractedVal = extractedVal;
        info->existentialVal = existentialVal;
        info->existentialType = existentialType;
        info->witnessTable = witnessTable;

        context->shared->extValues.add(info);
        return LoweredValInfo::extractedExistential(info);
    }
};

struct LValueExprLoweringVisitor : ExprLoweringVisitorBase<LValueExprLoweringVisitor>
{
    static bool _isLValueContext() { return true; }

    // When visiting a swizzle expression in an l-value context,
    // we need to construct a "swizzled l-value."
    LoweredValInfo visitMatrixSwizzleExpr(MatrixSwizzleExpr*)
    {
        SLANG_UNIMPLEMENTED_X("matrix swizzle lvalue case");
    }

    // When visiting a swizzle expression in an l-value context,
    // we need to construct a "sizzled l-value."
    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        auto irType = lowerType(context, expr->type);
        auto loweredBase = lowerRValueExpr(context, expr->base);

        RefPtr<SwizzledLValueInfo> swizzledLValue = new SwizzledLValueInfo();
        swizzledLValue->type = irType;

        UInt elementCount = (UInt)expr->elementCount;
        swizzledLValue->elementCount = elementCount;

        // As a small optimization, we will detect if the base expression
        // has also lowered into a swizzle and only return a single
        // swizzle instead of nested swizzles.
        //
        // E.g., if we have input like `foo[i].zw.y` we should optimize it
        // down to just `foo[i].w`.
        //
        if(loweredBase.flavor == LoweredValInfo::Flavor::SwizzledLValue)
        {
            auto baseSwizzleInfo = loweredBase.getSwizzledLValueInfo();

            // Our new swizzle will use the same base expression (e.g.,
            // `foo[i]` in our example above), but will need to remap
            // the swizzle indices it uses.
            //
            swizzledLValue->base = baseSwizzleInfo->base;
            for (UInt ii = 0; ii < elementCount; ++ii)
            {
                // First we get the swizzle element of the "outer" swizzle,
                // as it was written by the user. In our running example of
                // `foo[i].zw.y` this is the `y` element reference.
                //
                UInt originalElementIndex = UInt(expr->elementIndices[ii]);

                // Next we will use that original element index to figure
                // out which of the elements of the original swizzle this
                // should map to.
                //
                // In our example, `y` means index 1, and so we fetch
                // element 1 from the inner swizzle sequence `zw`, to get `w`.
                //
                SLANG_ASSERT(originalElementIndex < baseSwizzleInfo->elementCount);
                UInt remappedElementIndex = baseSwizzleInfo->elementIndices[originalElementIndex];

                swizzledLValue->elementIndices[ii] = remappedElementIndex;
            }
        }
        else
        {
            // In the default case, we can just copy the indices being
            // used for the swizzle over directly from the expression,
            // and use the base as-is.
            //
            swizzledLValue->base = loweredBase;
            for (UInt ii = 0; ii < elementCount; ++ii)
            {
                swizzledLValue->elementIndices[ii] = (UInt) expr->elementIndices[ii];
            }
        }

        context->shared->extValues.add(swizzledLValue);
        return LoweredValInfo::swizzledLValue(swizzledLValue);
    }
};

struct RValueExprLoweringVisitor : ExprLoweringVisitorBase<RValueExprLoweringVisitor>
{
    static bool _isLValueContext() { return false; }

    // A matrix swizzle in an r-value context can save time by just
    // emitting the matrix swizzle instructions directly.
    LoweredValInfo visitMatrixSwizzleExpr(MatrixSwizzleExpr* expr)
    {
        auto resultType = lowerType(context, expr->type);
        auto base = lowerSubExpr(expr->base);
        auto matType = as<MatrixExpressionType>(expr->base->type.type);
        if (!matType)
            SLANG_UNEXPECTED("Expected a matrix type in matrix swizzle");
        auto subscript2 = lowerType(context, matType->getElementType());
        auto subscript1 = lowerType(context, matType->getRowType());

        auto builder = getBuilder();

        auto irIntType = getIntType(context);

        UInt elementCount = (UInt)expr->elementCount;
        IRInst* irExtracts[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            auto index1 = builder->getIntValue(
                irIntType,
                (IRIntegerValue)expr->elementCoords[ii].row);
            auto index2 = builder->getIntValue(
                irIntType,
                (IRIntegerValue)expr->elementCoords[ii].col);
            // First index expression
            auto irExtract1 = subscriptValue(
                subscript1,
                base,
                index1);
            // Second index expression
            irExtracts[ii] = getSimpleVal(context, subscriptValue(
                subscript2,
                irExtract1,
                index2));
        }
        auto irVector = builder->emitMakeVector(
            resultType,
            elementCount,
            irExtracts
        );

        return LoweredValInfo::simple(irVector);
    }

    // A swizzle in an r-value context can save time by just
    // emitting the swizzle instructions directly.
    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        auto irType = lowerType(context, expr->type);
        auto irBase = getSimpleVal(context, lowerRValueExpr(context, expr->base));

        auto builder = getBuilder();

        auto irIntType = getIntType(context);

        UInt elementCount = (UInt)expr->elementCount;
        IRInst* irElementIndices[4];
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            irElementIndices[ii] = builder->getIntValue(
                irIntType,
                (IRIntegerValue)expr->elementIndices[ii]);
        }

        auto irSwizzle = builder->emitSwizzle(
            irType,
            irBase,
            elementCount,
            &irElementIndices[0]);

        return LoweredValInfo::simple(irSwizzle);
    }
};

LoweredValInfo lowerLValueExpr(
    IRGenContext*   context,
    Expr*           expr)
{
    IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, expr->loc);

    LValueExprLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(expr);
}

LoweredValInfo lowerRValueExpr(
    IRGenContext*   context,
    Expr*           expr)
{
    IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, expr->loc);

    RValueExprLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(expr);
}

struct StmtLoweringVisitor : StmtVisitor<StmtLoweringVisitor>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    void visitEmptyStmt(EmptyStmt*)
    {
        // Nothing to do.
    }

    void visitUnparsedStmt(UnparsedStmt*)
    {
        SLANG_UNEXPECTED("UnparsedStmt not supported by IR");
    }

    void visitCaseStmtBase(CaseStmtBase*)
    {
        SLANG_UNEXPECTED("`case` or `default` not under `switch`");
    }

    void visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        // The user is asking us to emit code for the loop
        // body for each value in the given integer range.
        // For now, we will handle this by repeatedly lowering
        // the body statement, with the loop variable bound
        // to a different integer literal value each time.
        //
        // TODO: eventually we might handle this as just an
        // ordinary loop, with an `[unroll]` attribute on
        // it that we would respect.

        auto rangeBeginVal = getIntVal(stmt->rangeBeginVal);
        auto rangeEndVal = getIntVal(stmt->rangeEndVal);

        if (rangeBeginVal >= rangeEndVal)
            return;

        auto varDecl = stmt->varDecl;
        auto varType = lowerType(context, varDecl->type);

        IRGenEnv subEnvStorage;
        IRGenEnv* subEnv = &subEnvStorage;
        subEnv->outer = context->env;

        IRGenContext subContextStorage = *context;
        IRGenContext* subContext = &subContextStorage;
        subContext->env = subEnv;



        for (IntegerLiteralValue ii = rangeBeginVal; ii < rangeEndVal; ++ii)
        {
            auto constVal = getBuilder()->getIntValue(
                varType,
                ii);

            subEnv->mapDeclToValue[varDecl] = LoweredValInfo::simple(constVal);

            lowerStmt(subContext, stmt->body);
        }
    }

    // Create a basic block in the current function,
    // so that it can be used for a label.
    IRBlock* createBlock()
    {
        return getBuilder()->createBlock();
    }

    /// Does the given block have a terminator?
    bool isBlockTerminated(IRBlock* block)
    {
        return block->getTerminator() != nullptr;
    }

    /// Emit a branch to the target block if the current
    /// block being inserted into is not already terminated.
    void emitBranchIfNeeded(IRBlock* targetBlock)
    {
        auto builder = getBuilder();
        auto currentBlock = builder->getBlock();

        // Don't emit if there is no current block.
        if(!currentBlock)
            return;

        // Don't emit if the block already has a terminator.
        if(isBlockTerminated(currentBlock))
            return;

        // The block is unterminated, so cap it off with
        // a terminator that branches to the target.
        builder->emitBranch(targetBlock);
    }

    /// Insert a block at the current location (ending
    /// the previous block with an unconditional jump
    /// if needed).
    void insertBlock(IRBlock* block)
    {
        auto builder = getBuilder();

        auto prevBlock = builder->getBlock();
        auto parentFunc = prevBlock ? prevBlock->getParent() : builder->getFunc();

        // If the previous block doesn't already have
        // a terminator instruction, then be sure to
        // emit a branch to the new block.
        emitBranchIfNeeded(block);

        // Add the new block to the function we are building,
        // and setit as the block we will be inserting into.
        parentFunc->addBlock(block);
        builder->setInsertInto(block);
    }

    // Start a new block at the current location.
    // This is just the composition of `createBlock`
    // and `insertBlock`.
    IRBlock* startBlock()
    {
        auto block = createBlock();
        insertBlock(block);
        return block;
    }

    /// Start a new block if there isn't a current
    /// block that we can append to.
    ///
    /// The `stmt` parameter is the statement we
    /// are about to emit.
    void startBlockIfNeeded(Stmt* stmt)
    {
        auto builder = getBuilder();
        auto currentBlock = builder->getBlock();

        // If there is a current block and it hasn't
        // been terminated, then we can just use that.
        if(currentBlock && !isBlockTerminated(currentBlock))
        {
            return;
        }

        // We are about to emit code *after* a terminator
        // instruction, and there is no label to allow
        // branching into this code, so whatever we are
        // about to emit is going to be unreachable.
        //
        // Let's diagnose that here just to help the user.
        //
        // TODO: We might want to have a more robust check
        // for unreachable code based on IR analysis instead,
        // at which point we'd probably disable this check.
        //
        context->getSink()->diagnose(stmt, Diagnostics::unreachableCode);

        startBlock();
    }

    void visitIfStmt(IfStmt* stmt)
    {
        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        auto condExpr = stmt->predicate;
        auto thenStmt = stmt->positiveStatement;
        auto elseStmt = stmt->negativeStatement;

        auto irCond = getSimpleVal(context,
            lowerRValueExpr(context, condExpr));

        if (elseStmt)
        {
            auto thenBlock = createBlock();
            auto elseBlock = createBlock();
            auto afterBlock = createBlock();

            builder->emitIfElse(irCond, thenBlock, elseBlock, afterBlock);

            insertBlock(thenBlock);
            lowerStmt(context, thenStmt);
            emitBranchIfNeeded(afterBlock);

            insertBlock(elseBlock);
            lowerStmt(context, elseStmt);

            insertBlock(afterBlock);
        }
        else
        {
            auto thenBlock = createBlock();
            auto afterBlock = createBlock();

            builder->emitIf(irCond, thenBlock, afterBlock);

            insertBlock(thenBlock);
            lowerStmt(context, thenStmt);

            insertBlock(afterBlock);
        }
    }

    void addLoopDecorations(
        IRInst* inst,
        Stmt*   stmt)
    {
        if( stmt->findModifier<UnrollAttribute>() )
        {
            getBuilder()->addLoopControlDecoration(inst, kIRLoopControl_Unroll);
        }
        else if( stmt->findModifier<LoopAttribute>() )
        {
            getBuilder()->addLoopControlDecoration(inst, kIRLoopControl_Loop);
        }
        // TODO: handle other cases here
    }

    void visitForStmt(ForStmt* stmt)
    {
        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        // The initializer clause for the statement
        // can always safetly be emitted to the current block.
        if (auto initStmt = stmt->initialStatement)
        {
            lowerStmt(context, initStmt);
        }

        // We will create blocks for the various places
        // we need to jump to inside the control flow,
        // including the blocks that will be referenced
        // by `continue` or `break` statements.
        auto loopHead = createBlock();
        auto bodyLabel = createBlock();
        auto breakLabel = createBlock();
        auto continueLabel = createBlock();

        // Register the `break` and `continue` labels so
        // that we can find them for nested statements.
        context->shared->breakLabels.Add(stmt, breakLabel);
        context->shared->continueLabels.Add(stmt, continueLabel);

        // Emit the branch that will start out loop,
        // and then insert the block for the head.

        auto loopInst = builder->emitLoop(
            loopHead,
            breakLabel,
            continueLabel);

        addLoopDecorations(loopInst, stmt);

        insertBlock(loopHead);

        // Now that we are within the header block, we
        // want to emit the expression for the loop condition:
        if (auto condExpr = stmt->predicateExpression)
        {
            auto irCondition = getSimpleVal(context,
                lowerRValueExpr(context, stmt->predicateExpression));

            // Now we want to `break` if the loop condition is false.
            builder->emitLoopTest(
                irCondition,
                bodyLabel,
                breakLabel);
        }

        // Emit the body of the loop
        insertBlock(bodyLabel);
        lowerStmt(context, stmt->statement);

        // Insert the `continue` block
        insertBlock(continueLabel);
        if (auto incrExpr = stmt->sideEffectExpression)
        {
            lowerRValueExpr(context, incrExpr);
        }

        // At the end of the body we need to jump back to the top.
        emitBranchIfNeeded(loopHead);

        // Finally we insert the label that a `break` will jump to
        insertBlock(breakLabel);
    }

    void visitWhileStmt(WhileStmt* stmt)
    {
        // Generating IR for `while` statement is similar to a
        // `for` statement, but without a lot of the complications.

        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        // We will create blocks for the various places
        // we need to jump to inside the control flow,
        // including the blocks that will be referenced
        // by `continue` or `break` statements.
        auto loopHead = createBlock();
        auto bodyLabel = createBlock();
        auto breakLabel = createBlock();

        // A `continue` inside a `while` loop always
        // jumps to the head of hte loop.
        auto continueLabel = loopHead;

        // Register the `break` and `continue` labels so
        // that we can find them for nested statements.
        context->shared->breakLabels.Add(stmt, breakLabel);
        context->shared->continueLabels.Add(stmt, continueLabel);

        // Emit the branch that will start out loop,
        // and then insert the block for the head.

        auto loopInst = builder->emitLoop(
            loopHead,
            breakLabel,
            continueLabel);

        addLoopDecorations(loopInst, stmt);

        insertBlock(loopHead);

        // Now that we are within the header block, we
        // want to emit the expression for the loop condition:
        if (auto condExpr = stmt->predicate)
        {
            auto irCondition = getSimpleVal(context,
                lowerRValueExpr(context, condExpr));

            // Now we want to `break` if the loop condition is false.
            builder->emitLoopTest(
                irCondition,
                bodyLabel,
                breakLabel);
        }

        // Emit the body of the loop
        insertBlock(bodyLabel);
        lowerStmt(context, stmt->statement);

        // At the end of the body we need to jump back to the top.
        emitBranchIfNeeded(loopHead);

        // Finally we insert the label that a `break` will jump to
        insertBlock(breakLabel);
    }

    void visitDoWhileStmt(DoWhileStmt* stmt)
    {
        // Generating IR for `do {...} while` statement is similar to a
        // `while` statement, just with the test in a different place

        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        // We will create blocks for the various places
        // we need to jump to inside the control flow,
        // including the blocks that will be referenced
        // by `continue` or `break` statements.
        auto loopHead = createBlock();
        auto testLabel = createBlock();
        auto breakLabel = createBlock();

        // A `continue` inside a `do { ... } while ( ... )` loop always
        // jumps to the loop test.
        auto continueLabel = testLabel;

        // Register the `break` and `continue` labels so
        // that we can find them for nested statements.
        context->shared->breakLabels.Add(stmt, breakLabel);
        context->shared->continueLabels.Add(stmt, continueLabel);

        // Emit the branch that will start out loop,
        // and then insert the block for the head.

        auto loopInst = builder->emitLoop(
            loopHead,
            breakLabel,
            continueLabel);

        addLoopDecorations(loopInst, stmt);

        insertBlock(loopHead);

        // Emit the body of the loop
        lowerStmt(context, stmt->statement);

        insertBlock(testLabel);

        // Now that we are within the header block, we
        // want to emit the expression for the loop condition:
        if (auto condExpr = stmt->predicate)
        {
            auto irCondition = getSimpleVal(context,
                lowerRValueExpr(context, condExpr));

            // Now we want to `break` if the loop condition is false,
            // otherwise we will jump back to the head of the loop.
            builder->emitLoopTest(
                irCondition,
                loopHead,
                breakLabel);
        }

        // Finally we insert the label that a `break` will jump to
        insertBlock(breakLabel);
    }

    void visitGpuForeachStmt(GpuForeachStmt* stmt)
    {
        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        auto device = getSimpleVal(context, lowerRValueExpr(context, stmt->device));
        auto gridDims = getSimpleVal(context, lowerRValueExpr(context, stmt->gridDims));

        List<IRInst*> irArgs;
        if (auto callExpr = as<InvokeExpr>(stmt->kernelCall))
        {
            irArgs.add(device);
            irArgs.add(gridDims);
            auto fref = getSimpleVal(context, lowerRValueExpr(context, callExpr->functionExpr));
            irArgs.add(fref);
            for (auto arg : callExpr->arguments)
            {
                // if a reference to dispatchThreadID, don't emit
                if (auto declRefExpr = as<DeclRefExpr>(arg))
                {
                    if (declRefExpr->declRef.getDecl() == stmt->dispatchThreadID)
                    {
                        continue;
                    }
                }
                auto irArg = getSimpleVal(context, lowerRValueExpr(context, arg));
                irArgs.add(irArg);
            }
        }
        else
        {
            SLANG_UNEXPECTED("GPUForeach parsing produced an invalid result");
        }

        builder->emitGpuForeach(irArgs);
        return;
    }

    void visitExpressionStmt(ExpressionStmt* stmt)
    {
        startBlockIfNeeded(stmt);

        // The statement evaluates an expression
        // (for side effects, one assumes) and then
        // discards the result. As such, we simply
        // lower the expression, and don't use
        // the result.
        //
        // Note that we lower using the l-value path,
        // so that an expression statement that names
        // a location (but doesn't load from it)
        // will not actually emit a load.
        lowerLValueExpr(context, stmt->expression);
    }

    void visitDeclStmt(DeclStmt* stmt)
    {
        startBlockIfNeeded(stmt);

        // For now, we lower a declaration directly
        // into the current context.
        //
        // TODO: We may want to consider whether
        // nested type/function declarations should
        // be lowered into the global scope during
        // IR generation, or whether they should
        // be lifted later (pushing capture analysis
        // down to the IR).
        //
        lowerDecl(context, stmt->decl);
    }

    void visitSeqStmt(SeqStmt* stmt)
    {
        // To lower a sequence of statements,
        // just lower each in order
        for (auto ss : stmt->stmts)
        {
            lowerStmt(context, ss);
        }
    }

    void visitBlockStmt(BlockStmt* stmt)
    {
        // To lower a block (scope) statement,
        // just lower its body. The IR doesn't
        // need to reflect the scoping of the AST.
        lowerStmt(context, stmt->body);
    }

    void visitReturnStmt(ReturnStmt* stmt)
    {
        startBlockIfNeeded(stmt);

        // A `return` statement turns into a `return` instruction,
        // but we have two kinds of `return`: one for returning
        // a (non-`void`) value, and one for returning "no value"
        // (which effectively returns a value of type `void`).
        //
        if( auto expr = stmt->expression )
        {
            // If the AST `return` statement had an expression, then we
            // need to lower it to the IR at this point, both to
            // compute its value and (in case we are returning a
            // `void`-typed expression) to execute its side effects.
            //
            auto loweredExpr = lowerRValueExpr(context, expr);

            // If the AST `return` statement was returning a non-`void`
            // value, then we need to emit an IR `return` of that value.
            //
            if(!expr->type.type->equals(context->astBuilder->getVoidType()))
            {
                getBuilder()->emitReturn(getSimpleVal(context, loweredExpr));
            }
            else
            {
                // If the type of the value returned was `void`, then
                // we don't want to emit an IR-level `return` with a value,
                // because that could trip up some of our back-end.
                //
                // TODO: We should eventually have only a single IR-level
                // `return` operation that always takes a value (including
                // values of type `void`), and then treat an AST `return;`
                // as equivalent to something like `return void();`.
                //
                getBuilder()->emitReturn();
            }
        }
        else
        {
            // If we hit this case, then the AST `return` was a `return;`
            // with no value, which can only occur in a function with
            // a `void` result type.
            //
            getBuilder()->emitReturn();
        }
    }

    void visitDiscardStmt(DiscardStmt* stmt)
    {
        startBlockIfNeeded(stmt);
        getBuilder()->emitDiscard();
    }

    void visitBreakStmt(BreakStmt* stmt)
    {
        startBlockIfNeeded(stmt);

        // Semantic checking is responsible for finding
        // the statement taht this `break` breaks out of
        auto parentStmt = stmt->parentStmt;
        SLANG_ASSERT(parentStmt);

        // We just need to look up the basic block that
        // corresponds to the break label for that statement,
        // and then emit an instruction to jump to it.
        IRBlock* targetBlock = nullptr;
        context->shared->breakLabels.TryGetValue(parentStmt, targetBlock);
        SLANG_ASSERT(targetBlock);
        getBuilder()->emitBreak(targetBlock);
    }

    void visitContinueStmt(ContinueStmt* stmt)
    {
        startBlockIfNeeded(stmt);

        // Semantic checking is responsible for finding
        // the loop that this `continue` statement continues
        auto parentStmt = stmt->parentStmt;
        SLANG_ASSERT(parentStmt);


        // We just need to look up the basic block that
        // corresponds to the continue label for that statement,
        // and then emit an instruction to jump to it.
        IRBlock* targetBlock = nullptr;
        context->shared->continueLabels.TryGetValue(parentStmt, targetBlock);
        SLANG_ASSERT(targetBlock);
        getBuilder()->emitContinue(targetBlock);
    }

    // Lowering a `switch` statement can get pretty involved,
    // so we need to track a bit of extra data:
    struct SwitchStmtInfo
    {
        // The block that will be made to contain the `switch` statement
        IRBlock* initialBlock = nullptr;

        // The label for the `default` case, if any.
        IRBlock*    defaultLabel = nullptr;

        // The label of the current "active" case block.
        IRBlock*    currentCaseLabel = nullptr;

        // Has anything been emitted to the current "active" case block?
        bool anythingEmittedToCurrentCaseBlock = false;

        // The collected (value, label) pairs for
        // all the `case` statements.
        List<IRInst*>  cases;
    };

    // We need a label to use for a `case` or `default` statement,
    // so either create one here, or re-use the current one if
    // that is okay.
    IRBlock* getLabelForCase(SwitchStmtInfo* info)
    {
        // Look at the "current" label we are working with.
        auto currentCaseLabel = info->currentCaseLabel;

        // If there is a current block, and it is empty,
        // then it is still a viable target (we are in
        // a case of "trivial fall-through" from the previous
        // block).
        if(currentCaseLabel && !info->anythingEmittedToCurrentCaseBlock)
        {
            return currentCaseLabel;
        }

        // Othwerise, we need to start a new block and use that.
        IRBlock* newCaseLabel = createBlock();

        // Note: if the previous block failed
        // to end with a `break`, then inserting
        // this block will append an unconditional
        // branch to the end of it that will target
        // this block.
        insertBlock(newCaseLabel);

        info->currentCaseLabel = newCaseLabel;
        info->anythingEmittedToCurrentCaseBlock = false;
        return newCaseLabel;
    }

    bool hasSwitchCases(Stmt* inStmt)
    {
        Stmt* stmt = inStmt;
        // Unwrap any surrounding `{ ... }` so we can look
        // at the statement inside.
        while (auto blockStmt = as<BlockStmt>(stmt))
        {
            stmt = blockStmt->body;
            continue;
        }

        if (auto seqStmt = as<SeqStmt>(stmt))
        {
            // Walk through the children looking for cases
            for (auto childStmt : seqStmt->stmts)
            {
                if (hasSwitchCases(childStmt))
                {
                    return true;
                }
            }
        }
        else if (auto caseStmt = as<CaseStmt>(stmt))
        {
            return true;
        }
        else if (auto defaultStmt = as<DefaultStmt>(stmt))
        {
            // A 'default:' is a kind of case. 
            return true;
        }

        return false;
    }

    // Given a statement that appears as (or in) the body
    // of a `switch` statement
    void lowerSwitchCases(Stmt* inStmt, SwitchStmtInfo* info)
    {
        // TODO: in the general case (e.g., if we were going
        // to eventual lower to an unstructured format like LLVM),
        // the Right Way to handle C-style `switch` statements
        // is just to emit the body directly as "normal" statements,
        // and then treat `case` and `default` as special statements
        // that start a new block and register a label with the
        // enclosing `switch`.
        //
        // For now we will assume that any `case` and `default`
        // statements need to be directly nested under the `switch`,
        // and so we can find them with a simpler walk.

        Stmt* stmt = inStmt;

        // Unwrap any surrounding `{ ... }` so we can look
        // at the statement inside.
        while(auto blockStmt = as<BlockStmt>(stmt))
        {
            stmt = blockStmt->body;
            continue;
        }

        if(auto seqStmt = as<SeqStmt>(stmt))
        {
            // Walk through teh children and process each.
            for(auto childStmt : seqStmt->stmts)
            {
                lowerSwitchCases(childStmt, info);
            }
        }
        else if(auto caseStmt = as<CaseStmt>(stmt))
        {
            // A full `case` statement has a value we need
            // to test against. It is expected to be a
            // compile-time constant, so we will emit
            // it like an expression here, and then hope
            // for the best.
            //
            // TODO: figure out something cleaner.

            // Actually, one gotcha is that if we ever allow non-constant
            // expressions here (or anything that requires instructions
            // to be emitted to yield its value), then those instructions
            // need to go into an appropriate block.

            IRGenContext subContext = *context;
            IRBuilder subBuilder = *getBuilder();
            subBuilder.setInsertInto(info->initialBlock);
            subContext.irBuilder = &subBuilder;
            auto caseVal = getSimpleVal(context, lowerRValueExpr(&subContext, caseStmt->expr));

            // Figure out where we are branching to.
            auto label = getLabelForCase(info);

            // Add this `case` to the list for the enclosing `switch`.
            info->cases.add(caseVal);
            info->cases.add(label);
        }
        else if(auto defaultStmt = as<DefaultStmt>(stmt))
        {
            auto label = getLabelForCase(info);

            // We expect to only find a single `default` stmt.
            SLANG_ASSERT(!info->defaultLabel);

            info->defaultLabel = label;
        }
        else if(auto emptyStmt = as<EmptyStmt>(stmt))
        {
            // Special-case empty statements so they don't
            // mess up our "trivial fall-through" optimization.
        }
        else
        {
            // We have an ordinary statement, that needs to get
            // emitted to the current case block.
            if(!info->currentCaseLabel)
            {
                // It possible in full C/C++ to have statements
                // before the first `case`. Usually these are
                // unreachable, unless they start with a label.
                //
                // We'll ignore them here, figuring they are
                // dead. If we ever add `LabelStmt` then we'd
                // need to emit these statements to a dummy
                // block just in case.
            }
            else
            {
                // Emit the code to our current case block,
                // and record that we've done so.
                lowerStmt(context, stmt);
                info->anythingEmittedToCurrentCaseBlock = true;
            }
        }
    }

    void visitSwitchStmt(SwitchStmt* stmt)
    {
        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        // Given a statement:
        //
        //      switch( CONDITION )
        //      {
        //      case V0:
        //          S0;
        //          break;
        //
        //      case V1:
        //      default:
        //          S1;
        //          break;
        //      }
        //
        // we want to generate IR like:
        //
        //      let %c = <CONDITION>;
        //      switch %c,          // value to switch on
        //          %breakLabel,    // join point (and break target)
        //          %s1,            // default label
        //          %v0,            // first case value
        //          %s0,            // first case label
        //          %v1,            // second case value
        //          %s1             // second case label
        //  s0:
        //      <S0>
        //      break %breakLabel
        //  s1:
        //      <S1>
        //      break %breakLabel
        //  breakLabel:
        //

        // First emit code to compute the condition:
        auto conditionVal = getSimpleVal(context, lowerRValueExpr(context, stmt->condition));

        // Check for any cases or default. 
        if (!hasSwitchCases(stmt->body))
        {
            // If we don't have any case/default then nothing inside switch can be executed (other than condition)
            // so we are done.
            return;
        }

        // Remember the initial block so that we can add to it
        // after we've collected all the `case`s
        auto initialBlock = builder->getBlock();

        // Next, create a block to use as the target for any `break` statements
        auto breakLabel = createBlock();

        // Register the `break` label so
        // that we can find it for nested statements.
        context->shared->breakLabels.Add(stmt, breakLabel);

        builder->setInsertInto(initialBlock->getParent());

        // Iterate over the body of the statement, looking
        // for `case` or `default` statements:
        SwitchStmtInfo info;
        info.initialBlock = initialBlock;
        info.defaultLabel = nullptr;
        lowerSwitchCases(stmt->body, &info);

        // TODO: once we've discovered the cases, we should
        // be able to make a quick pass over the list and eliminate
        // any cases that have the exact same label as the `default`
        // case, since these don't actually need to be represented.

        // If the current block (the end of the last
        // `case`) is not terminated, then terminate with a
        // `break` operation.
        //
        // Double check that we aren't in the initial
        // block, so we don't get tripped up on an
        // empty `switch`.
        auto curBlock = builder->getBlock();
        if(curBlock != initialBlock)
        {
            // Is the block already terminated?
            if(!curBlock->getTerminator())
            {
                // Not terminated, so add one.
                builder->emitBreak(breakLabel);
            }
        }

        // If there was no `default` statement, then the
        // default case will just branch directly to the end.
        auto defaultLabel = info.defaultLabel ? info.defaultLabel : breakLabel;

        // Now that we've collected the cases, we are
        // prepared to emit the `switch` instruction
        // itself.
        builder->setInsertInto(initialBlock);
        builder->emitSwitch(
            conditionVal,
            breakLabel,
            defaultLabel,
            info.cases.getCount(),
            info.cases.getBuffer());

        // Finally we insert the label that a `break` will jump to
        // (and that control flow will fall through to otherwise).
        // This is the block that subsequent code will go into.
        insertBlock(breakLabel);
        context->shared->breakLabels.Remove(stmt);
    }
};

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt)
{
    IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, stmt->loc);

    StmtLoweringVisitor visitor;
    visitor.context = context;

    try
    {
        visitor.dispatch(stmt);
    }
    // Don't emit any context message for an explicit `AbortCompilationException`
    // because it should only happen when an error is already emitted.
    catch(const AbortCompilationException&) { throw; }
    catch(...)
    {
        context->getSink()->noteInternalErrorLoc(stmt->loc);
        throw;
    }
}

/// Create and return a mutable temporary initialized with `val`
static LoweredValInfo moveIntoMutableTemp(
    IRGenContext*           context,
    LoweredValInfo const&   val)
{
    IRInst* irVal = getSimpleVal(context, val);
    auto type = irVal->getDataType();
    auto var = createVar(context, type);

    assign(context, var, LoweredValInfo::simple(irVal));
    return var;
}

LoweredValInfo tryGetAddress(
    IRGenContext*           context,
    LoweredValInfo const&   inVal,
    TryGetAddressMode       mode)
{
    LoweredValInfo val = inVal;

    switch(val.flavor)
    {
    case LoweredValInfo::Flavor::Ptr:
        // The `Ptr` case means that we already have an IR value with
        // the address of our value. Easy!
        return val;

    case LoweredValInfo::Flavor::BoundStorage:
        {
            // If we are are trying to turn a subscript operation like `buffer[index]`
            // into a pointer, then we need to find a `ref` accessor declared
            // as part of the subscript operation being referenced.
            //
            auto subscriptInfo = val.getBoundStorageInfo();

            // We don't want to immediately bind to a `ref` accessor if there is
            // a `set` accessor available, unless we are in an "aggressive" mode
            // where we really want/need a pointer to be able to make progress.
            //
            if(mode != TryGetAddressMode::Aggressive
                && getMembersOfType<SetterDecl>(subscriptInfo->declRef, MemberFilterStyle::Instance).isNonEmpty())
            {
                // There is a setter that we should consider using,
                // so don't go and aggressively collapse things just yet.
                return val;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(subscriptInfo->declRef, MemberFilterStyle::Instance);
            if(refAccessors.isNonEmpty())
            {
                auto refAccessor = *refAccessors.begin();

                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(subscriptInfo->type);

                LoweredValInfo refVal = _emitCallToAccessor(
                    context,
                    ptrType,
                    refAccessor,
                    subscriptInfo->base,
                    subscriptInfo->additionalArgs);

                // The result from the call should be a pointer, and it
                // is the address that we wanted in the first place.
                return LoweredValInfo::ptr(getSimpleVal(context, refVal));
            }

            // Otherwise, there was no `ref` accessor, and so it is not possible
            // to materialize this location into a pointer for whatever purpose
            // we have in mind (e.g., passing it to an atomic operation).
        }
        break;

    case LoweredValInfo::Flavor::BoundMember:
        {
            auto boundMemberInfo = val.getBoundMemberInfo();

            // If we hit this case, then it means that we have a reference
            // to a single field in something, but for whatever reason the
            // higher-level logic was not able to turn it into a pointer
            // already (maybe the base value for the field reference is
            // a `BoundStorage`, etc.).
            //
            // We need to read the entire base value out, modify the field
            // we care about, and then write it back.

            auto declRef = boundMemberInfo->declRef;
            if( auto fieldDeclRef = declRef.as<VarDecl>() )
            {
                auto baseVal = boundMemberInfo->base;
                auto basePtr = tryGetAddress(context, baseVal, TryGetAddressMode::Aggressive);

                return extractField(context, boundMemberInfo->type, basePtr, fieldDeclRef);
            }

        }
        break;

    case LoweredValInfo::Flavor::SwizzledLValue:
        {
            auto originalSwizzleInfo = val.getSwizzledLValueInfo();
            auto originalBase = originalSwizzleInfo->base;

            UInt elementCount = originalSwizzleInfo->elementCount;

            auto newBase = tryGetAddress(context, originalBase, TryGetAddressMode::Aggressive);
            RefPtr<SwizzledLValueInfo> newSwizzleInfo = new SwizzledLValueInfo();
            context->shared->extValues.add(newSwizzleInfo);

            newSwizzleInfo->base = newBase;
            newSwizzleInfo->type = originalSwizzleInfo->type;
            newSwizzleInfo->elementCount = elementCount;
            for(UInt ee = 0; ee < elementCount; ++ee)
                newSwizzleInfo->elementIndices[ee] = originalSwizzleInfo->elementIndices[ee];

            return LoweredValInfo::swizzledLValue(newSwizzleInfo);
        }
        break;

    // TODO: are there other cases we need to handled here?

    default:
        break;
    }

    // If none of the special cases above applied, then we werent' able to make
    // this value into a pointer, and we should just return it as-is.
    return val;
}

IRInst* getAddress(
    IRGenContext*           context,
    LoweredValInfo const&   inVal,
    SourceLoc               diagnosticLocation)
{
    LoweredValInfo val = tryGetAddress(context, inVal, TryGetAddressMode::Aggressive);

    if( val.flavor == LoweredValInfo::Flavor::Ptr )
    {
        return val.val;
    }

    context->getSink()->diagnose(diagnosticLocation, Diagnostics::invalidLValueForRefParameter);
    return nullptr;
}

void assign(
    IRGenContext*           context,
    LoweredValInfo const&   inLeft,
    LoweredValInfo const&   inRight)
{
    LoweredValInfo left = inLeft;
    LoweredValInfo right = inRight;

    // Before doing the case analysis on the shape of the `left` value,
    // we might as well go ahead and see if we can coerce it into
    // a simple pointer, since that would make our life a lot easier
    // when handling complex cases.
    //
    left = tryGetAddress(context, left, TryGetAddressMode::Default);

    auto builder = context->irBuilder;

top:
    switch (left.flavor)
    {
    case LoweredValInfo::Flavor::Ptr:
        {
            // The `left` value is just a pointer, so we can emit
            // a store to it directly.
            //
            builder->emitStore(
                left.val,
                getSimpleVal(context, right));
        }
        break;

    case LoweredValInfo::Flavor::SwizzledLValue:
        {
            // The `left` value is of the form `<base>.<swizzleElements>`.
            // How we will handle this depends on what `base` looks like:
            auto swizzleInfo = left.getSwizzledLValueInfo();
            auto loweredBase = swizzleInfo->base;

            // Note that the call to `tryGetAddress` at the start should
            // ensure that `loweredBase` has been simplified as much as
            // possible (e.g., if it is possible to turn it into a
            // `LoweredValInfo::ptr()` then that will have been done).

            switch( loweredBase.flavor )
            {
            default:
                {
                    // Our fallback position is to lower via a temporary, e.g.:
                    //
                    //      float4 tmp = <base>;
                    //      tmp.xyz = float3(...);
                    //      <base> = tmp;
                    //

                    // Load from the base value
                    IRInst* irLeftVal = getSimpleVal(context, loweredBase);

                    // Extract a simple value for the right-hand side
                    IRInst* irRightVal = getSimpleVal(context, right);

                    // Apply the swizzle
                    IRInst* irSwizzled = builder->emitSwizzleSet(
                        irLeftVal->getDataType(),
                        irLeftVal,
                        irRightVal,
                        swizzleInfo->elementCount,
                        swizzleInfo->elementIndices);

                    // And finally, store the value back where we got it.
                    //
                    // Note: this is effectively a recursive call to
                    // `assign()`, so we do a simple tail-recursive call here.
                    left = loweredBase;
                    right = LoweredValInfo::simple(irSwizzled);
                    goto top;
                }
                break;

            case LoweredValInfo::Flavor::Ptr:
                {
                    // We are writing through a pointer, which might be
                    // pointing into a UAV or other memory resource, so
                    // we can't introduce use a temporary like the case
                    // above, because then we would read and write bytes
                    // that are not strictly required for the store.
                    //
                    // Note that the messy case of a "swizzle of a swizzle"
                    // was handled already in lowering of a `SwizzleExpr`,
                    // so that we don't need to deal with that case here.
                    //
                    // TODO: we may need to consider whether there is
                    // enough value in a masked store like this to keep
                    // it around, in comparison to a simpler model where
                    // we simply form a pointer to each of the vector
                    // elements and write to them individually.
                    //
                    // TODO: we might also consider just special-casing
                    // single-element swizzles so that the common case
                    // can turn into a simple `store` instead of a
                    // `swizzledStore`.
                    //
                    IRInst* irRightVal = getSimpleVal(context, right);
                    builder->emitSwizzledStore(
                        loweredBase.val,
                        irRightVal,
                        swizzleInfo->elementCount,
                        swizzleInfo->elementIndices);
                }
                break;
            }
        }
        break;

    case LoweredValInfo::Flavor::BoundStorage:
        {
            // The `left` value refers to a subscript operation on
            // a resource type, bound to particular arguments, e.g.:
            // `someStructuredBuffer[index]`.
            //
            // When storing to such a value, we need to emit a call
            // to the appropriate builtin "setter" accessor, if there
            // is one, and then fall back to a `ref` accessor if
            // there is no setter.
            //
            auto subscriptInfo = left.getBoundStorageInfo();

            // Search for an appropriate "setter" declaration
            auto setters = getMembersOfType<SetterDecl>(subscriptInfo->declRef, MemberFilterStyle::Instance);
            if (setters.isNonEmpty())
            {
                auto setter = *setters.begin();

                auto allArgs = subscriptInfo->additionalArgs;

                // Note: here we are assuming that all setters take
                // the new-value parameter as an `in` rather than
                // as any kind of reference.
                //
                // TODO: If we add support for something like `const&`
                // for input parameters, we might have to deal with
                // that here.
                //
                addInArg(context, &allArgs, right);

                _emitCallToAccessor(
                    context,
                    builder->getVoidType(),
                    setter,
                    subscriptInfo->base,
                    allArgs);
                return;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(subscriptInfo->declRef, MemberFilterStyle::Instance);
            if(refAccessors.isNonEmpty())
            {
                auto refAccessor = *refAccessors.begin();

                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(subscriptInfo->type);

                LoweredValInfo refVal = _emitCallToAccessor(
                    context,
                    ptrType,
                    refAccessor,
                    subscriptInfo->base,
                    subscriptInfo->additionalArgs);

                // The result from the call needs to be implicitly dereferenced,
                // so that it can work as an l-value of the desired result type.
                left = LoweredValInfo::ptr(getSimpleVal(context, refVal));

                // Tail-recursively attempt assignment again on the new l-value.
                goto top;
            }

            // No setter found? Then we have an error!
            SLANG_UNEXPECTED("no setter found");
            break;
        }
        break;

    case LoweredValInfo::Flavor::BoundMember:
        {
            auto boundMemberInfo = left.getBoundMemberInfo();

            // If we hit this case, then it means that we are trying to set
            // a single field in someting that is not atomically set-able.
            // (e.g., an element of a value where the `subscript` operation
            // has `get` and `set` but not a `ref` accessor).
            //
            // We need to read the entire base value out, modify the field
            // we care about, and then write it back.

            auto declRef = boundMemberInfo->declRef;
            if( auto fieldDeclRef = declRef.as<VarDecl>() )
            {
                // materialize the base value and move it into
                // a mutable temporary if needed
                auto baseVal = boundMemberInfo->base;
                auto tempVal = moveIntoMutableTemp(context, baseVal);

                // extract the field l-value out of the temporary
                auto tempFieldVal = extractField(context, boundMemberInfo->type, tempVal, fieldDeclRef);

                // assign to the field of the temporary l-value
                assign(context, tempFieldVal, right);

                // write back the modified temporary to the base l-value
                assign(context, baseVal, tempVal);

                return;
            }
            else
            {
                SLANG_UNEXPECTED("handled member flavor");
            }

        }
        break;

    case LoweredValInfo::Flavor::ExtractedExistential:
        {
            // The `left` value is the result of opening an existential.
            //
            auto leftInfo = left.getExtractedExistentialValInfo();
            auto existentialVal = leftInfo->existentialVal;

            // The actual desitnation we need to store into is the
            // existential value itself.
            //
            left = existentialVal;

            // The `right` value must be of the same concrete type as
            // the opened value, but the new destination is of the
            // original existential type, so we need to wrap it up
            // appropriately.
            //
            right = LoweredValInfo::simple(builder->emitMakeExistential(
                leftInfo->existentialType,
                getSimpleVal(context, right),
                leftInfo->witnessTable));

            goto top;
        }
        break;

    default:
        SLANG_UNIMPLEMENTED_X("assignment");
        break;
    }
}

struct DeclLoweringVisitor : DeclVisitor<DeclLoweringVisitor, LoweredValInfo>
{
    IRGenContext*   context;

    DiagnosticSink* getSink() { return context->getSink(); }

    IRBuilder* getBuilder()
    {
        return context->irBuilder;
    }

    LoweredValInfo visitDeclBase(DeclBase* /*decl*/)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitDecl(Decl* /*decl*/)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitExtensionDecl(ExtensionDecl* decl)
    {
        for (auto & member : decl->members)
            ensureDecl(context, member);
        return LoweredValInfo();
    }

#define IGNORED_CASE(NAME) \
    LoweredValInfo visit##NAME(NAME*) { return LoweredValInfo(); }

    IGNORED_CASE(ImportDecl)
    IGNORED_CASE(UsingDecl)
    IGNORED_CASE(EmptyDecl)
    IGNORED_CASE(SyntaxDecl)
    IGNORED_CASE(AttributeDecl)
    IGNORED_CASE(NamespaceDecl)

#undef IGNORED_CASE

    LoweredValInfo visitTypeDefDecl(TypeDefDecl* decl)
    {
        // A type alias declaration may be generic, if it is
        // nested under a generic type/function/etc.
        //
        NestedContext nested(this);
        auto subBuilder = nested.getBuilder();
        auto subContext = nested.getContext();
        IRGeneric* outerGeneric = emitOuterGenerics(subContext, decl, decl);

        // TODO: if a type alias declaration can have linkage,
        // we will need to lower it to some kind of global
        // value in the IR so that we can attach a name to it.
        //
        // For now, we can only attach a name *if* the type
        // alias is somehow generic.
        if(outerGeneric)
        {
            addLinkageDecoration(context, outerGeneric, decl);
        }

        auto type = lowerType(subContext, decl->type.type);

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, type, outerGeneric));
    }

    LoweredValInfo visitGenericTypeParamDecl(GenericTypeParamDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitGenericTypeConstraintDecl(GenericTypeConstraintDecl* decl)
    {
        // This might be a type constraint on an associated type,
        // in which case it should lower as the key for that
        // interface requirement.
        if(auto assocTypeDecl = as<AssocTypeDecl>(decl->parentDecl))
        {
            // TODO: might need extra steps if we ever allow
            // generic associated types.


            if(auto interfaceDecl = as<InterfaceDecl>(assocTypeDecl->parentDecl))
            {
                // Okay, this seems to be an interface rquirement, and
                // we should lower it as such.
                return LoweredValInfo::simple(getInterfaceRequirementKey(decl));
            }
        }

        if(auto globalGenericParamDecl = as<GlobalGenericParamDecl>(decl->parentDecl))
        {
            // This is a constraint on a global generic type parameters,
            // and so it should lower as a parameter of its own.
            auto supType = lowerType(context, decl->getSup().type);
            auto inst = getBuilder()->emitGlobalGenericWitnessTableParam(supType);
            addLinkageDecoration(context, inst, decl);
            return LoweredValInfo::simple(inst);
        }

        // Otherwise we really don't expect to see a type constraint
        // declaration like this during lowering, because a generic
        // should have set up a parameter for any constraints as
        // part of being lowered.

        SLANG_UNEXPECTED("generic type constraint during lowering");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitGlobalGenericParamDecl(GlobalGenericParamDecl* decl)
    {
        auto inst = getBuilder()->emitGlobalGenericTypeParam();
        addLinkageDecoration(context, inst, decl);
        return LoweredValInfo::simple(inst);
    }

    LoweredValInfo visitGlobalGenericValueParamDecl(GlobalGenericValueParamDecl* decl)
    {
        auto builder = getBuilder();
        auto type = lowerType(context, decl->type);
        auto inst = builder->emitGlobalGenericParam(type);
        addLinkageDecoration(context, inst, decl);
        return LoweredValInfo::simple(inst);
    }

    bool isPublicType(Type* type)
    {
        // TODO(JS):
        // Not clear how should handle HLSLExportModifier here. 
        // In the HLSL spec 'export' is only applicable to functions. So for now we ignore.

        if (auto declRefType = as<DeclRefType>(type))
        {
            if (declRefType->declRef.getDecl()->findModifier<PublicModifier>())
                return true;
        }
        return false;
    }

    void lowerWitnessTable(
        IRGenContext*                               subContext,
        WitnessTable*                               astWitnessTable,
        IRWitnessTable*                             irWitnessTable,
        Dictionary<WitnessTable*, IRWitnessTable*>  mapASTToIRWitnessTable)
    {
        auto subBuilder = subContext->irBuilder;

        for(auto entry : astWitnessTable->requirementList)
        {
            auto requiredMemberDecl = entry.Key;
            auto satisfyingWitness = entry.Value;

            auto irRequirementKey = getInterfaceRequirementKey(requiredMemberDecl);
            IRInst* irSatisfyingVal = nullptr;

            switch(satisfyingWitness.getFlavor())
            {
            case RequirementWitness::Flavor::declRef:
                {
                    auto satisfyingDeclRef = satisfyingWitness.getDeclRef();
                    irSatisfyingVal = getSimpleVal(subContext,
                        emitDeclRef(subContext, satisfyingDeclRef,
                        // TODO: we need to know what type to plug in here...
                        nullptr));
                }
                break;

            case RequirementWitness::Flavor::val:
                {
                    auto satisfyingVal = satisfyingWitness.getVal();
                    irSatisfyingVal = lowerSimpleVal(subContext, satisfyingVal);
                }
                break;

            case RequirementWitness::Flavor::witnessTable:
                {
                    auto astReqWitnessTable = satisfyingWitness.getWitnessTable();
                    IRWitnessTable* irSatisfyingWitnessTable = nullptr;
                    if(!mapASTToIRWitnessTable.TryGetValue(astReqWitnessTable, irSatisfyingWitnessTable))
                    {
                        // Need to construct a sub-witness-table
                        auto irWitnessTableBaseType = lowerType(subContext, astReqWitnessTable->baseType);
                        irSatisfyingWitnessTable = subBuilder->createWitnessTable(irWitnessTableBaseType, irWitnessTable->getConcreteType());
                        auto mangledName = getMangledNameForConformanceWitness(
                            subContext->astBuilder,
                            astReqWitnessTable->witnessedType,
                            astReqWitnessTable->baseType);
                        subBuilder->addExportDecoration(irSatisfyingWitnessTable, mangledName.getUnownedSlice());
                        if (isPublicType(astReqWitnessTable->witnessedType))
                        {
                            subBuilder->addPublicDecoration(irSatisfyingWitnessTable);
                            subBuilder->addKeepAliveDecoration(irSatisfyingWitnessTable);
                        }

                        // Recursively lower the sub-table.
                        lowerWitnessTable(
                            subContext,
                            astReqWitnessTable,
                            irSatisfyingWitnessTable,
                            mapASTToIRWitnessTable);

                        irSatisfyingWitnessTable->moveToEnd();
                    }
                    irSatisfyingVal = irSatisfyingWitnessTable;
                }
                break;

            default:
                SLANG_UNEXPECTED("handled requirement witness case");
                break;
            }


            subBuilder->createWitnessTableEntry(
                irWitnessTable,
                irRequirementKey,
                irSatisfyingVal);
        }
    }

    LoweredValInfo visitInheritanceDecl(InheritanceDecl* inheritanceDecl)
    {
        // An inheritance clause inside of an `interface`
        // declaration should not give rise to a witness
        // table, because it represents something the
        // interface requires, and not what it provides.
        //
        auto parentDecl = inheritanceDecl->parentDecl;
        if (auto parentInterfaceDecl = as<InterfaceDecl>(parentDecl))
        {
            return LoweredValInfo::simple(getInterfaceRequirementKey(inheritanceDecl));
        }
        //
        // We also need to cover the case where an `extension`
        // declaration is being used to add a conformance to
        // an existing `interface`:
        //
        if(auto parentExtensionDecl = as<ExtensionDecl>(parentDecl))
        {
            auto targetType = parentExtensionDecl->targetType;
            if(auto targetDeclRefType = as<DeclRefType>(targetType))
            {
                if(auto targetInterfaceDeclRef = targetDeclRefType->declRef.as<InterfaceDecl>())
                {
                    return LoweredValInfo::simple(getInterfaceRequirementKey(inheritanceDecl));
                }
            }
        }

        // Find the type that is doing the inheriting.
        // Under normal circumstances it is the type declaration that
        // is the parent for the inheritance declaration, but if
        // the inheritance declaration is on an `extension` declaration,
        // then we need to identify the type being extended.
        //
        Type* subType = nullptr;
        if (auto extParentDecl = as<ExtensionDecl>(parentDecl))
        {
            subType = extParentDecl->targetType.type;
        }
        else
        {
            subType = DeclRefType::create(context->astBuilder, makeDeclRef(parentDecl));
        }

        // What is the super-type that we have declared we inherit from?
        Type* superType = inheritanceDecl->base.type;

        if(auto superDeclRefType = as<DeclRefType>(superType))
        {
            if( superDeclRefType->declRef.as<StructDecl>() || superDeclRefType->declRef.as<ClassDecl>() )
            {
                // TODO: the witness that a type inherits from a `struct`
                // type should probably be a key that will be used for
                // a field that holds the base type...
                //
                auto irKey = getBuilder()->createStructKey();
                addLinkageDecoration(context, irKey, inheritanceDecl);
                auto keyVal = LoweredValInfo::simple(irKey);
                setGlobalValue(context, inheritanceDecl, keyVal);
                return keyVal;
            }
        }

        // Construct the mangled name for the witness table, which depends
        // on the type that is conforming, and the type that it conforms to.
        //
        // TODO: This approach doesn't really make sense for generic `extension` conformances.
        auto mangledName = getMangledNameForConformanceWitness(context->astBuilder, subType, superType);

        // A witness table may need to be generic, if the outer
        // declaration (either a type declaration or an `extension`)
        // is generic.
        //
        NestedContext nested(this);
        auto subBuilder = nested.getBuilder();
        auto subContext = nested.getContext();
        auto outerGeneric = emitOuterGenerics(subContext, inheritanceDecl, inheritanceDecl);

        // Lower the super-type to force its declaration to be lowered.
        //
        // Note: we are using the "sub-context" here because the
        // type being inherited from could reference generic parameters,
        // and we need those parameters to lower as references to
        // the parameters of our IR-level generic.
        //
        auto irWitnessTableBaseType = lowerType(subContext, superType);

        // Create the IR-level witness table
        auto irWitnessTable = subBuilder->createWitnessTable(irWitnessTableBaseType, nullptr);

        // Register the value now, rather than later, to avoid any possible infinite recursion.
        setGlobalValue(context, inheritanceDecl, LoweredValInfo::simple(irWitnessTable));

        auto irSubType = lowerType(subContext, subType);
        irWitnessTable->setOperand(0, irSubType);

        addLinkageDecoration(context, irWitnessTable, inheritanceDecl, mangledName.getUnownedSlice());

        // TODO(JS):
        // Not clear what to do here around HLSLExportModifier. 
        // In HLSL it only (currently) applies to functions, so perhaps do nothing is reasonable.
        
        if (parentDecl->findModifier<PublicModifier>())
        {
            subBuilder->addPublicDecoration(irWitnessTable);
            subBuilder->addKeepAliveDecoration(irWitnessTable);
        }


        // Make sure that all the entries in the witness table have been filled in,
        // including any cases where there are sub-witness-tables for conformances
        Dictionary<WitnessTable*, IRWitnessTable*> mapASTToIRWitnessTable;
        lowerWitnessTable(
            subContext,
            inheritanceDecl->witnessTable,
            irWitnessTable,
            mapASTToIRWitnessTable);

        irWitnessTable->moveToEnd();

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irWitnessTable, outerGeneric));
    }

    LoweredValInfo visitDeclGroup(DeclGroup* declGroup)
    {
        // To lower a group of declarations, we just
        // lower each one individually.
        //
        for (auto decl : declGroup->decls)
        {
            IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, decl->loc);

            // Note: I am directly invoking `dispatch` here,
            // instead of `ensureDecl` just to try and
            // make sure that we don't accidentally
            // emit things to an outer context.
            //
            // TODO: make sure that can't happen anyway.
            dispatch(decl);
        }

        return LoweredValInfo();
    }

    LoweredValInfo visitStorageDeclCommon(ContainerDecl* decl)
    {
        // A subscript operation may encompass one or more
        // accessors, and these are what should actually
        // get lowered (they are effectively functions).

        for (auto accessor : decl->getMembersOfType<AccessorDecl>())
        {
            if (accessor->hasModifier<IntrinsicOpModifier>())
                continue;

            ensureDecl(context, accessor);
        }

        // The subscript declaration itself won't correspond
        // to anything in the lowered program, so we don't
        // bother creating a representation here.
        //
        // Note: We may want to have a specific lowered value
        // that can represent the combination of callables
        // that make up the subscript operation.
        return LoweredValInfo();
    }

    LoweredValInfo visitSubscriptDecl(SubscriptDecl* decl)
    {
        return visitStorageDeclCommon(decl);
    }

    LoweredValInfo visitPropertyDecl(PropertyDecl* decl)
    {
        return visitStorageDeclCommon(decl);
    }

    bool isGlobalVarDecl(VarDecl* decl)
    {
        auto parent = decl->parentDecl;
        if (as<NamespaceDeclBase>(parent))
        {
            // Variable declared at global/namespace scope? -> Global.
            return true;
        }
        else if(as<AggTypeDeclBase>(parent))
        {
            if(decl->hasModifier<HLSLStaticModifier>())
            {
                // A `static` member variable is effectively global.
                return true;
            }
        }

        return false;
    }

    bool isMemberVarDecl(VarDecl* decl)
    {
        auto parent = decl->parentDecl;
        if (as<AggTypeDecl>(parent))
        {
            // A variable declared inside of an aggregate type declaration is a member.
            return true;
        }

        return false;
    }

    LoweredValInfo lowerGlobalShaderParam(VarDecl* decl)
    {
        IRType* paramType = lowerType(context, decl->getType());

        auto builder = getBuilder();

        auto irParam = builder->createGlobalParam(paramType);
        auto paramVal = LoweredValInfo::simple(irParam);

        addLinkageDecoration(context, irParam, decl);
        addNameHint(context, irParam, decl);
        maybeSetRate(context, irParam, decl);
        addVarDecorations(context, irParam, decl);

        if (decl)
        {
            builder->addHighLevelDeclDecoration(irParam, decl);
        }

        addTargetIntrinsicDecorations(irParam, decl);

        // A global variable's SSA value is a *pointer* to
        // the underlying storage.
        setGlobalValue(context, decl, paramVal);

        irParam->moveToEnd();

        return paramVal;
    }

    LoweredValInfo lowerConstantDeclCommon(VarDeclBase* decl) 
    {
        // It's constant, so shoul dhave this modifier
        SLANG_ASSERT(decl->hasModifier<ConstModifier>());

        NestedContext nested(this);
        auto subBuilder = nested.getBuilder();
        auto subContext = nested.getContext();
        IRGeneric* outerGeneric = emitOuterGenerics(subContext, decl, decl);

        // TODO(JS): Is this right? 
        //
        // If we *are* in a generic, then outputting this in the (current) generic scope would be correct.
        // If we *aren't* we want to go the level above for insertion
        //
        // Just inserting into the parent doesn't work with a generic that holds a function that has a static const
        // variable.
        // 
        // This tries to match the behavior of previous `lowerFunctionStaticConstVarDecl` functionality
        if (!outerGeneric && isFunctionStaticVarDecl(decl))
        {
            // We need to insert the constant at a level above
            // the function being emitted. This will usually
            // be the global scope, but it might be an outer
            // generic if we are lowering a generic function.

            subBuilder->setInsertInto(subBuilder->getFunc()->getParent());
        }

        auto initExpr = decl->initExpr;

        // We want to be able to support cases where a global constant is defined in
        // another module and we should not bind to its value at (front-end) compile
        // time. We handle this by adding a level of indirection where a global constant
        // is represented as an IR node with zero or one operands. In the zero-operand
        // case the node represents a global constant with an unknown value (perhaps
        // an imported constant), while in the one-operand case the operand gives us
        // the concrete value to use for the constant.
        //
        // Using a level of indirection also gives us a well-defined place to attach
        // annotation information like name hints, since otherwise two constants
        // with the same value would map to identical IR nodes.
        //
        // TODO: For now we detect whether or not to include the value operand based on
        // whether we see an initial-value expression in the AST declaration, but
        // eventually we might base this on whether or not the value should be accessible
        // to the module we are lowering.

        IRInst* irConstant = nullptr;
        if(!initExpr)
        {
            // If we don't know the value we want to use, then we just create
            // a global constant IR node with the right type.
            //
            auto irType = lowerType(subContext, decl->getType());
            irConstant = subBuilder->emitGlobalConstant(irType);
        }
        else
        {
            // We lower the value expression directly, which yields a
            // global instruction to represent the value. There is
            // no guarantee that this instruction is unique (e.g.,
            // if we have two different constants definitions both
            // with the value `5`, then we might have only a single
            // instruction to represent `5`.
            //
            auto irInitVal = getSimpleVal(subContext, lowerRValueExpr(subContext, initExpr));

            // We construct a distinct IR instruction to represent the
            // constant itself, with the value as an operand.
            //
            irConstant = subBuilder->emitGlobalConstant(
                irInitVal->getFullType(),
                irInitVal);
        }

        // All of the attributes/decorations we can attach
        // belong on the IR constant node.
        //

        addLinkageDecoration(context, irConstant, decl);
        
        addNameHint(context, irConstant, decl);
        addVarDecorations(context, irConstant, decl);

        getBuilder()->addHighLevelDeclDecoration(irConstant, decl);

        // Finish of generic

        auto loweredValue = LoweredValInfo::simple(finishOuterGenerics(subBuilder, irConstant, outerGeneric));

        // Register the value that was emitted as the value
        // for any references to the constant from elsewhere
        // in the code.
        //
        setGlobalValue(context, decl, loweredValue);

        return loweredValue;
    }

    LoweredValInfo lowerGlobalConstantDecl(VarDecl* decl)
    {
        return lowerConstantDeclCommon(decl);
    }

    LoweredValInfo lowerGlobalVarDecl(VarDecl* decl)
    {
        // A non-`static` global is actually a shader parameter in HLSL.
        //
        // TODO: We should probably make that case distinct at the AST
        // level as well, since global shader parameters are fairly
        // different from global variables.
        //
        if(isGlobalShaderParameter(decl))
        {
            return lowerGlobalShaderParam(decl);
        }

        // A `static const` global is actually a compile-time constant.
        //
        if (decl->hasModifier<HLSLStaticModifier>() && decl->hasModifier<ConstModifier>())
        {
            return lowerGlobalConstantDecl(decl);
        }

        IRType* varType = lowerType(context, decl->getType());

        auto builder = getBuilder();

        // TODO(JS): Do we create something derived from IRGlobalVar? Or do we use 
        // a decoration to identify an *actual* global?

        IRGlobalValueWithCode* irGlobal = builder->createGlobalVar(varType);
        LoweredValInfo globalVal = LoweredValInfo::ptr(irGlobal);

        addLinkageDecoration(context, irGlobal, decl);
        addNameHint(context, irGlobal, decl);

        maybeSetRate(context, irGlobal, decl);

        addVarDecorations(context, irGlobal, decl);

        if (decl)
        {
            builder->addHighLevelDeclDecoration(irGlobal, decl);
        }

        // A global variable's SSA value is a *pointer* to
        // the underlying storage.
        setGlobalValue(context, decl, globalVal);

        if (isImportedDecl(decl))
        {
            // Always emit imported declarations as declarations,
            // and not definitions.
        }
        else if( auto initExpr = decl->initExpr )
        {
            IRBuilder subBuilderStorage = *getBuilder();
            IRBuilder* subBuilder = &subBuilderStorage;

            subBuilder->setInsertInto(irGlobal);

            IRGenContext subContextStorage = *context;
            IRGenContext* subContext = &subContextStorage;

            subContext->irBuilder = subBuilder;

            // TODO: set up a parent IR decl to put the instructions into

            IRBlock* entryBlock = subBuilder->emitBlock();
            subBuilder->setInsertInto(entryBlock);

            LoweredValInfo initVal = lowerLValueExpr(subContext, initExpr);
            subContext->irBuilder->emitReturn(getSimpleVal(subContext, initVal));
        }

        irGlobal->moveToEnd();

        return globalVal;
    }

    bool isFunctionStaticVarDecl(VarDeclBase* decl)
    {
        // Only a variable marked `static` can be static.
        if(!decl->findModifier<HLSLStaticModifier>())
            return false;

        // The immediate parent of a function-scope variable
        // declaration will be a `ScopeDecl`.
        //
        // TODO: right now the parent links for scopes are *not*
        // set correctly, so we can't just scan up and look
        // for a function in the parent chain...
        auto parent = decl->parentDecl;
        if( as<ScopeDecl>(parent) )
        {
            return true;
        }

        return false;
    }

    struct NestedContext
    {
        IRGenEnv        subEnvStorage;
        IRBuilder       subBuilderStorage;
        IRGenContext    subContextStorage;

        NestedContext(DeclLoweringVisitor* outer)
            : subBuilderStorage(*outer->getBuilder())
            , subContextStorage(*outer->context)
        {
            auto outerContext = outer->context;

            subEnvStorage.outer = outerContext->env;

            subContextStorage.irBuilder = &subBuilderStorage;
            subContextStorage.env = &subEnvStorage;

            subContextStorage.thisType = outerContext->thisType;
            subContextStorage.thisTypeWitness = outerContext->thisTypeWitness;
        }

        IRBuilder* getBuilder() { return &subBuilderStorage; }
        IRGenContext* getContext() { return &subContextStorage; }
    };

    LoweredValInfo lowerFunctionStaticConstVarDecl(
        VarDeclBase* decl)
    {
        return lowerConstantDeclCommon(decl);
    }

    LoweredValInfo lowerFunctionStaticVarDecl(
        VarDeclBase*    decl)
    {
        // We know the variable is `static`, but it might also be `const.
        if(decl->hasModifier<ConstModifier>())
            return lowerFunctionStaticConstVarDecl(decl);

        // A function-scope `static` variable is effectively a global,
        // and a simple solution here would be to try to emit this
        // variable directly into the global scope.
        //
        // The one major wrinkle we need to deal with is the way that
        // a function-scope `static` variable could be nested under
        // a generic, leading to the situation that different instances
        // of that same generic would need distinct storage for that
        // variable declaration.
        //
        // We will handle that constraint by carefully nesting the
        // IR global variable under the parent of its containing
        // function.
        //
        auto parent = getBuilder()->getInsertLoc().getParent();
        if(auto block = as<IRBlock>(parent))
            parent = block->getParent();

        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContext();
        subBuilder->setInsertBefore(parent);

        IRType* subVarType = lowerType(subContext, decl->getType());
        IRGlobalValueWithCode* irGlobal = subBuilder->createGlobalVar(subVarType);
        addVarDecorations(subContext, irGlobal, decl);

        addNameHint(context, irGlobal, decl);
        maybeSetRate(context, irGlobal, decl);

        subBuilder->addHighLevelDeclDecoration(irGlobal, decl);

        LoweredValInfo globalVal = LoweredValInfo::ptr(irGlobal);
        setValue(context, decl, globalVal);

        // A `static` variable with an initializer needs special handling,
        // at least if the initializer isn't a compile-time constant.
        if( auto initExpr = decl->initExpr )
        {
            // We must create another global `bool isInitialized = false`
            // to represent whether we've initialized this before.
            // Then emit code like:
            //
            //      if(!isInitialized) { <globalVal> = <initExpr>; isInitialized = true; }
            //
            // TODO: we could conceivably optimize this by detecting
            // when the `initExpr` lowers to just a reference to a constant,
            // and then either deleting the extra code structure there,
            // or not generating it in the first place. That is a bit
            // more complexity than I'm ready for at the moment.
            //
            auto boolBuilder = subBuilder;

            auto irBoolType = boolBuilder->getBoolType();
            auto irBool = boolBuilder->createGlobalVar(irBoolType);
            boolBuilder->setInsertInto(irBool);
            boolBuilder->setInsertInto(boolBuilder->createBlock());
            boolBuilder->emitReturn(boolBuilder->getBoolValue(false));

            auto boolVal = LoweredValInfo::ptr(irBool);


            // Okay, with our global Boolean created, we can move on to
            // generating the code we actually care about, back in the original function.

            auto builder = getBuilder();

            auto initBlock = builder->createBlock();
            auto afterBlock = builder->createBlock();

            builder->emitIfElse(getSimpleVal(context, boolVal), afterBlock, initBlock, afterBlock);

            builder->insertBlock(initBlock);
            LoweredValInfo initVal = lowerLValueExpr(context, initExpr);
            assign(context, globalVal, initVal);
            assign(context, boolVal, LoweredValInfo::simple(builder->getBoolValue(true)));
            builder->emitBranch(afterBlock);

            builder->insertBlock(afterBlock);
        }

        return globalVal;
    }

    LoweredValInfo visitGenericValueParamDecl(GenericValueParamDecl* decl)
    {
        return emitDeclRef(context, makeDeclRef(decl),
            lowerType(context, decl->type));
    }

    LoweredValInfo visitVarDecl(VarDecl* decl)
    {
        // Detect global (or effectively global) variables
        // and handle them differently.
        if (isGlobalVarDecl(decl))
        {
            return lowerGlobalVarDecl(decl);
        }

        if(isFunctionStaticVarDecl(decl))
        {
            return lowerFunctionStaticVarDecl(decl);
        }

        if(isMemberVarDecl(decl))
        {
            return lowerMemberVarDecl(decl);
        }

        // A user-defined variable declaration will usually turn into
        // an `alloca` operation for the variable's storage,
        // plus some code to initialize it and then store to the variable.

        IRType* varType = lowerType(context, decl->getType());

        // As a special case, an immutable local variable with an
        // initializer can just lower to the SSA value of its initializer.
        //
        if(as<LetDecl>(decl))
        {
            if(auto initExpr = decl->initExpr)
            {
                auto initVal = lowerRValueExpr(context, initExpr);
                initVal = LoweredValInfo::simple(getSimpleVal(context, initVal));
                setGlobalValue(context, decl, initVal);
                return initVal;
            }
        }


        LoweredValInfo varVal = createVar(context, varType, decl);

        if( auto initExpr = decl->initExpr )
        {
            auto initVal = lowerRValueExpr(context, initExpr);

            assign(context, varVal, initVal);
        }

        setGlobalValue(context, decl, varVal);

        return varVal;
    }

    IRStructKey* getInterfaceRequirementKey(Decl* requirementDecl)
    {
        return Slang::getInterfaceRequirementKey(context, requirementDecl);
    }

    LoweredValInfo visitAssocTypeDecl(AssocTypeDecl* decl)
    {
        SLANG_ASSERT(decl->parentDecl != nullptr);
        ShortList<IRInterfaceType*> constraintInterfaces;
        for (auto constraintDecl : decl->getMembersOfType<GenericTypeConstraintDecl>())
        {
            auto baseType = lowerType(context, constraintDecl->sup.type);
            SLANG_ASSERT(baseType && baseType->getOp() == kIROp_InterfaceType);
            constraintInterfaces.add((IRInterfaceType*)baseType);
        }
        auto assocType = context->irBuilder->getAssociatedType(
            constraintInterfaces.getArrayView().arrayView);
        setValue(context, decl, assocType);
        return LoweredValInfo::simple(assocType);
    }

    LoweredValInfo visitInterfaceDecl(InterfaceDecl* decl)
    {
        // The members of an interface will turn into the keys that will
        // be used for lookup operations into witness
        // tables that promise conformance to the interface.
        //
        // TODO: we don't handle the case here of an interface
        // with concrete/default implementations for any
        // of its members.
        //
        // TODO: If we want to support using an interface as
        // an existential type, then we might need to emit
        // a witness table for the interface type's conformance
        // to its own interface.
        //
        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContext();

        // Emit any generics that should wrap the actual type.
        auto outerGeneric = emitOuterGenerics(subContext, decl, decl);

        // First, compute the number of requirement entries that will be included in this
        // interface type.
        UInt operandCount = 0;
        for (auto requirementDecl : decl->members)
        {
            operandCount++;
            // As a special case, any type constraints placed
            // on an associated type will *also* need to be turned
            // into requirement keys for this interface.
            if (auto associatedTypeDecl = as<AssocTypeDecl>(requirementDecl))
            {
                operandCount += associatedTypeDecl->getMembersOfType<TypeConstraintDecl>().getCount();
            }
        }

        // Allocate an IRInterfaceType with the `operandCount` operands.
        IRInterfaceType* irInterface = subBuilder->createInterfaceType(operandCount, nullptr);

        // Add `irInterface` to decl mapping now to prevent cyclic lowering.
        setValue(subContext, decl, LoweredValInfo::simple(irInterface));

        // Setup subContext for proper lowering `ThisType`, associated types and
        // the interface decl's self reference.
        auto thisType = getBuilder()->getThisType(irInterface);
        subContext->thisType = thisType;

        // TODO: Need to add an appropriate stand-in witness here.
        subContext->thisTypeWitness = nullptr;

        // Lower associated types first, so they can be referred to when lowering functions.
        for (auto assocTypeDecl : decl->getMembersOfType<AssocTypeDecl>())
        {
            ensureDecl(subContext, assocTypeDecl);
        }

        UInt entryIndex = 0;

        for (auto requirementDecl : decl->members)
        {
            auto entry = subBuilder->createInterfaceRequirementEntry(
                getInterfaceRequirementKey(requirementDecl),
                nullptr);
            if (auto inheritance = as<InheritanceDecl>(requirementDecl))
            {
                auto irBaseType = lowerType(context, inheritance->base.type);
                auto irWitnessTableType = subBuilder->getWitnessTableType(irBaseType);
                entry->setRequirementVal(irWitnessTableType);
            }
            else
            {
                IRInst* requirementVal = ensureDecl(subContext, requirementDecl).val;
                if (requirementVal)
                {
                    switch (requirementVal->getOp())
                    {
                    case kIROp_Func:
                    case kIROp_Generic:
                        {
                            // Remove lowered `IRFunc`s since we only care about
                            // function types.
                            auto reqType = requirementVal->getFullType();
                            entry->setRequirementVal(reqType);
                            break;
                        }
                    default:
                        entry->setRequirementVal(requirementVal);
                        break;
                    }
                }
            }
            irInterface->setOperand(entryIndex, entry);
            entryIndex++;
            // Add addtional requirements for type constraints placed
            // on an associated types.
            if (auto associatedTypeDecl = as<AssocTypeDecl>(requirementDecl))
            {
                for (auto constraintDecl : associatedTypeDecl->getMembersOfType<TypeConstraintDecl>())
                {
                    auto constraintKey = getInterfaceRequirementKey(constraintDecl);
                    auto constraintInterfaceType =
                        lowerType(context, constraintDecl->getSup().type);
                    auto witnessTableType =
                        getBuilder()->getWitnessTableType(constraintInterfaceType);

                    auto constraintEntry = subBuilder->createInterfaceRequirementEntry(constraintKey,
                            witnessTableType);
                    irInterface->setOperand(entryIndex, constraintEntry);
                    entryIndex++;

                    setValue(context, constraintDecl, LoweredValInfo::simple(constraintEntry));
                }
            }
            else
            {
                // Add lowered requirement entry to current decl mapping to prevent
                // the function requirements from being lowered again when we get to
                // `ensureAllDeclsRec`.
                setValue(context, requirementDecl, LoweredValInfo::simple(entry));
            }
        }

        addNameHint(context, irInterface, decl);
        addLinkageDecoration(context, irInterface, decl);
        if (auto anyValueSizeAttr = decl->findModifier<AnyValueSizeAttribute>())
        {
            subBuilder->addAnyValueSizeDecoration(irInterface, anyValueSizeAttr->size);
        }
        if (auto comInterfaceAttr = decl->findModifier<ComInterfaceAttribute>())
        {
            subBuilder->addComInterfaceDecoration(irInterface);
        }
        if (auto builtinAttr = decl->findModifier<BuiltinAttribute>())
        {
            subBuilder->addBuiltinDecoration(irInterface);
        }
        subBuilder->setInsertInto(irInterface);
        // TODO: are there any interface members that should be
        // nested inside the interface type itself?

        irInterface->moveToEnd();

        addTargetIntrinsicDecorations(irInterface, decl);

        auto finalVal = finishOuterGenerics(subBuilder, irInterface, outerGeneric);
        return LoweredValInfo::simple(finalVal);
    }

    LoweredValInfo visitEnumCaseDecl(EnumCaseDecl* decl)
    {
        // A case within an `enum` decl will lower to a value
        // of the `enum`'s "tag" type.
        //
        // TODO: a bit more work will be needed if we allow for
        // enum cases that have payloads, because then we need
        // a function that constructs the value given arguments.
        //
        NestedContext nestedContext(this);
        auto subContext = nestedContext.getContext();

        // Emit any generics that should wrap the actual type.
        emitOuterGenerics(subContext, decl, decl);

        return lowerRValueExpr(subContext, decl->tagExpr);
    }

    LoweredValInfo visitEnumDecl(EnumDecl* decl)
    {
        // Given a declaration of a type, we need to make sure
        // to output "witness tables" for any interfaces this
        // type has declared conformance to.
        for( auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>() )
        {
            ensureDecl(context, inheritanceDecl);
        }

        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContext();
        auto outerGeneric = emitOuterGenerics(subContext, decl, decl);

        // An `enum` declaration will currently lower directly to its "tag"
        // type, so that any references to the `enum` become referenes to
        // the tag type instead.
        //
        // TODO: if we ever support `enum` types with payloads, we would
        // need to make the `enum` lower to some kind of custom "tagged union"
        // type.

        IRType* loweredTagType = lowerType(subContext, decl->tagType);

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, loweredTagType, outerGeneric));
    }

    LoweredValInfo visitAggTypeDecl(AggTypeDecl* decl)
    {
        // Don't generate an IR `struct` for intrinsic types
        if(decl->findModifier<IntrinsicTypeModifier>() || decl->findModifier<BuiltinTypeModifier>())
        {
            return LoweredValInfo();
        }

        if (as<AssocTypeDecl>(decl))
        {
            SLANG_UNREACHABLE("associatedtype should have been handled by visitAssocTypeDecl.");
        }

        // TODO(JS): 
        // Not clear what to do around HLSLExportModifier. 
        // The HLSL spec says it only applies to functions, so we ignore for now.

        const bool isPublicType = decl->findModifier<PublicModifier>() != nullptr;

        // Given a declaration of a type, we need to make sure
        // to output "witness tables" for any interfaces this
        // type has declared conformance to.
        for( auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>() )
        {
            ensureDecl(context, inheritanceDecl);
        }

        // We are going to create nested IR building state
        // to use when emitting the members of the type.
        //
        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContext();

        // Emit any generics that should wrap the actual type.
        auto outerGeneric = emitOuterGenerics(subContext, decl, decl);

        IRType* irAggType = nullptr;
        if (as<StructDecl>(decl))
        {
            irAggType = subBuilder->createStructType();
        }
        else if (as<ClassDecl>(decl))
        {
            irAggType = subBuilder->createClassType();
        }
        else
        {
            getSink()->diagnose(decl->loc, Diagnostics::unimplemented, "lower unknown AggType to IR");
            return LoweredValInfo::simple(subBuilder->getVoidType());
        }

        addNameHint(context, irAggType, decl);
        addLinkageDecoration(context, irAggType, decl);

        if( auto payloadAttribute = decl->findModifier<PayloadAttribute>() )
        {
            subBuilder->addDecoration(irAggType, kIROp_PayloadDecoration);
        }

        subBuilder->setInsertInto(irAggType);

        // A `struct` that inherits from another `struct` must start
        // with a member for the direct base type.
        //
        for( auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>() )
        {
            if (isPublicType)
                ensureDecl(context, inheritanceDecl);
            auto superType = inheritanceDecl->base;
            if(auto superDeclRefType = as<DeclRefType>(superType))
            {
                if (superDeclRefType->declRef.as<StructDecl>() ||
                    superDeclRefType->declRef.as<ClassDecl>())
                {
                    auto superKey = (IRStructKey*) getSimpleVal(context, ensureDecl(context, inheritanceDecl));
                    auto irSuperType = lowerType(context, superType.type);
                    subBuilder->createStructField(
                        irAggType,
                        superKey,
                        irSuperType);
                }
            }
        }


        for (auto fieldDecl : decl->getMembersOfType<VarDeclBase>())
        {
            if (fieldDecl->hasModifier<HLSLStaticModifier>())
            {
                // A `static` field is actually a global variable,
                // and we should emit it as such.
                ensureDecl(context, fieldDecl);
                continue;
            }

            // Each ordinary field will need to turn into a struct "key"
            // that is used for fetching the field.
            IRInst* fieldKeyInst = getSimpleVal(context,
                ensureDecl(context, fieldDecl));
            auto fieldKey = as<IRStructKey>(fieldKeyInst);
            SLANG_ASSERT(fieldKey);

            // Note: we lower the type of the field in the "sub"
            // context, so that any generic parameters that were
            // set up for the type can be referenced by the field type.
            IRType* fieldType = lowerType(
                subContext,
                fieldDecl->getType());

            // Then, the parent `struct` instruction itself will have
            // a "field" instruction.
            subBuilder->createStructField(
                irAggType,
                fieldKey,
                fieldType);
        }

        // There may be members not handled by the above logic (e.g.,
        // member functions), but we will not immediately force them
        // to be emitted here, so as not to risk a circular dependency.
        //
        // Instead we will force emission of all children of aggregate
        // type declarations later, from the top-level emit logic.

        irAggType->moveToEnd();
        addTargetIntrinsicDecorations(irAggType, decl);

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irAggType, outerGeneric));
    }

    void lowerRayPayloadAccessModifier(IRInst* inst, RayPayloadAccessSemantic* semantic, IROp op)
    {
        auto builder = getBuilder();

        List<IRInst*> operands;
        for(auto stageNameToken : semantic->stageNameTokens)
        {
            IRInst* stageName = builder->getStringValue(stageNameToken.getContent());
            operands.add(stageName);
        }

        builder->addDecoration(inst, op, operands.getBuffer(), operands.getCount());
    }

    LoweredValInfo lowerMemberVarDecl(VarDecl* fieldDecl)
    {
        // Each field declaration in the AST translates into
        // a "key" that can be used to extract field values
        // from instances of struct types that contain the field.
        //
        // It is correct to say struct *types* because a `struct`
        // nested under a generic can be used to realize a number
        // of different concrete types, but all of these types
        // will use the same space of keys.

        auto builder = getBuilder();
        auto irFieldKey = builder->createStructKey();
        addNameHint(context, irFieldKey, fieldDecl);

        addVarDecorations(context, irFieldKey, fieldDecl);

        addLinkageDecoration(context, irFieldKey, fieldDecl);

        if (auto semanticModifier = fieldDecl->findModifier<HLSLSimpleSemantic>())
        {
            builder->addSemanticDecoration(irFieldKey, semanticModifier->name.getName()->text.getUnownedSlice());
        }

        if( auto readModifier = fieldDecl->findModifier<RayPayloadReadSemantic>() )
        {
            lowerRayPayloadAccessModifier(irFieldKey, readModifier, kIROp_StageReadAccessDecoration);
        }
        if( auto writeModifier = fieldDecl->findModifier<RayPayloadWriteSemantic>())
        {
            lowerRayPayloadAccessModifier(irFieldKey, writeModifier, kIROp_StageWriteAccessDecoration);
        }

        // We allow a field to be marked as a target intrinsic,
        // so that we can override its mangled name in the
        // output for the chosen target.
        addTargetIntrinsicDecorations(irFieldKey, fieldDecl);


        return LoweredValInfo::simple(irFieldKey);
    }





    bool isImportedDecl(Decl* decl)
    {
        return Slang::isImportedDecl(context, decl);
    }

    IRType* maybeGetConstExprType(IRType* type, Decl* decl)
    {
        return Slang::maybeGetConstExprType(getBuilder(), type, decl);
    }

        /// Emit appropriate generic parameters for a constraint, and return the value of that constraint.
        ///
        /// The `supType` paramete represents the super-type that a parameter is constrained to.
    IRInst* emitGenericConstraintValue(
        IRGenContext*               subContext,
        GenericTypeConstraintDecl*  constraintDecl,
        IRType*                     supType)
    {
        auto subBuilder = subContext->irBuilder;

        // There are two cases we care about here.
        //
        if(auto andType = as<IRConjunctionType>(supType))
        {
            // The non-trivial case is when the constraint on a generic parameter
            // was of the form `T : A & B`. In this case, we really want to
            // emit the function with parameters for each of the two independent
            // constraints `T : A` and `T : B`.
            //
            // We will loop over the "cases" of the conjunction (since
            // the `IRConunctionType` can support more than just binary
            // conjunctions) and recursively add constraints for each.
            //
            List<IRInst*> caseVals;
            auto caseCount = andType->getCaseCount();
            for(Int i = 0; i < caseCount; ++i)
            {
                auto caseType = andType->getCaseType(i);
                auto caseVal = emitGenericConstraintValue(subContext, constraintDecl, caseType);
                caseVals.add(caseVal);
            }

            return subBuilder->emitMakeTuple(caseVals);
        }
        else
        {
            // The case case is any other type being used as the constraint.
            //
            // The constraint will then map to a single generic parameter passing
            // a witness table for conformance to the given `supType`.
            //
            auto param = subBuilder->emitParam(subBuilder->getWitnessTableType(supType));
            addNameHint(context, param, constraintDecl);

            // In order to support some of the "any-value" work in dynamic dispatch
            // we have to attach the interface that was used as a constraint onto the
            // type that is being constrained (which we expect to be a generic type
            // parameter).
            //
            // TODO: It feels a bit gross to be doing this here; perhaps the front-end
            // should handle propgation of value-size information from constraints
            // back to generic parameters?
            //
            if (auto declRefType = as<DeclRefType>(constraintDecl->sub.type))
            {
                auto typeParamDeclVal = subContext->findLoweredDecl(declRefType->declRef.decl);
                SLANG_ASSERT(typeParamDeclVal && typeParamDeclVal->val);
                subBuilder->addTypeConstraintDecoration(typeParamDeclVal->val, supType);
            }

            return param;
        }
    }

    void emitGenericConstraintDecl(
        IRGenContext*               subContext,
        GenericTypeConstraintDecl*  constraintDecl)
    {
        auto supType = lowerType(context, constraintDecl->sup.type);
        auto value = emitGenericConstraintValue(subContext, constraintDecl, supType);
        setValue(subContext, constraintDecl, LoweredValInfo::simple(value));
    }

    IRGeneric* emitOuterGeneric(
        IRGenContext*   subContext,
        GenericDecl*    genericDecl,
        Decl*           leafDecl)
    {
        auto subBuilder = subContext->irBuilder;

        // Of course, a generic might itself be nested inside of other generics...
        emitOuterGenerics(subContext, genericDecl, leafDecl);

        // We need to create an IR generic

        auto irGeneric = subBuilder->emitGeneric();
        subBuilder->setInsertInto(irGeneric);

        auto irBlock = subBuilder->emitBlock();
        subBuilder->setInsertInto(irBlock);

        // Now emit any parameters of the generic
        //
        // First we start with type and value parameters,
        // in the order they were declared.
        for (auto member : genericDecl->members)
        {
            if (auto typeParamDecl = as<GenericTypeParamDecl>(member))
            {
                // TODO: use a `TypeKind` to represent the
                // classifier of the parameter.
                auto param = subBuilder->emitParam(subBuilder->getTypeType());
                addNameHint(context, param, typeParamDecl);
                setValue(subContext, typeParamDecl, LoweredValInfo::simple(param));
            }
            else if (auto valDecl = as<GenericValueParamDecl>(member))
            {
                auto paramType = lowerType(subContext, valDecl->getType());
                auto param = subBuilder->emitParam(paramType);
                addNameHint(context, param, valDecl);
                setValue(subContext, valDecl, LoweredValInfo::simple(param));
            }
        }
        // Then we emit constraint parameters, again in
        // declaration order.
        for (auto member : genericDecl->members)
        {
            if (auto constraintDecl = as<GenericTypeConstraintDecl>(member))
            {
                emitGenericConstraintDecl(subContext, constraintDecl);
            }
        }

        return irGeneric;
    }

    IRGeneric* emitOuterInterfaceGeneric(
        IRGenContext*   subContext,
        ContainerDecl*  parentDecl,
        DeclRefType*    interfaceType,
        Decl*           leafDecl)
    {
        auto subBuilder = subContext->irBuilder;

        // Of course, a generic might itself be nested inside of other generics...
        emitOuterGenerics(subContext, parentDecl, leafDecl);

        // We need to create an IR generic

        auto irGeneric = subBuilder->emitGeneric();
        subBuilder->setInsertInto(irGeneric);

        auto irBlock = subBuilder->emitBlock();
        subBuilder->setInsertInto(irBlock);

        // The generic needs two parameters: one to represent the
        // `ThisType`, and one to represent a witness that the
        // `ThisType` conforms to the interface itself.
        //
        auto irThisTypeParam = subBuilder->emitParam(subBuilder->getTypeType());

        auto irInterfaceType = lowerType(context, interfaceType);
        auto irWitnessTableParam = subBuilder->emitParam(subBuilder->getWitnessTableType(irInterfaceType));
        subBuilder->addTypeConstraintDecoration(irThisTypeParam, irInterfaceType);

        // Now we need to wire up the IR parameters
        // we created to be used as the `ThisType` in
        // the body of the code.
        //
        subContext->thisType = irThisTypeParam;
        subContext->thisTypeWitness = irWitnessTableParam;

        return irGeneric;
    }

    // If the given `decl` is enclosed in any generic declarations, then
    // emit IR-level generics to represent them.
    // The `leafDecl` represents the inner-most declaration we are actually
    // trying to emit, which is the one that should receive the mangled name.
    //
    IRGeneric* emitOuterGenerics(IRGenContext* subContext, Decl* decl, Decl* leafDecl)
    {
        for(auto pp = decl->parentDecl; pp; pp = pp->parentDecl)
        {
            if(auto genericAncestor = as<GenericDecl>(pp))
            {
                return emitOuterGeneric(subContext, genericAncestor, leafDecl);
            }

            // We introduce IR generics in one other case, where the input
            // code wasn't visibly using generics: when a concrete member
            // is defined on an interface type. In that case, the resulting
            // definition needs to be generic on a parameter to represent
            // the `ThisType` of the interface.
            //
            if(auto extensionAncestor = as<ExtensionDecl>(pp))
            {
                if(auto targetDeclRefType = as<DeclRefType>(extensionAncestor->targetType))
                {
                    if(auto interfaceDeclRef = targetDeclRefType->declRef.as<InterfaceDecl>())
                    {
                        return emitOuterInterfaceGeneric(subContext, extensionAncestor, targetDeclRefType, leafDecl);
                    }
                }
            }
        }

        return nullptr;
    }

    static bool isChildOf(IRInst* child, IRInst* parent)
    {
        while (child && child->getParent() != parent)
            child = child->getParent();
        return child != nullptr;
    }
    static void markInstsToClone(HashSet<IRInst*>& valuesToClone, IRInst* parentBlock, IRInst* value)
    {
        if (!isChildOf(value, parentBlock))
            return;
        if (valuesToClone.Add(value))
        {
            for (UInt i = 0; i < value->getOperandCount(); i++)
            {
                auto operand = value->getOperand(i);
                markInstsToClone(valuesToClone, parentBlock, operand);
            }
        }
        for (auto child : value->getChildren())
            markInstsToClone(valuesToClone, parentBlock, child);
        auto parent = parentBlock->getParent();
        while (parent && parent != parentBlock)
        {
            valuesToClone.Add(parent);
            parent = parent->getParent();
        }
    }

    // If any generic declarations have been created by `emitOuterGenerics`,
    // then finish them off by emitting `return` instructions for the
    // values that they should produce.
    //
    // Return the outer-most generic (if there is one), or the original
    // value (if there were no generics), which should be the IR-level
    // representation of the original declaration.
    //
    IRInst* finishOuterGenerics(
        IRBuilder*  subBuilder,
        IRInst*     val,
        IRGeneric*  parentGeneric)
    {
        IRInst* v = val;

        IRInst* returnType = v->getFullType();

        while (parentGeneric)
        {
            // Create a universal type in `outterBlock` that will be used
            // as the type of this generic inst. The return value of the
            // generic inst will have a specialized type.
            // For example, if we have a generic function
            // g0 = generic<T> { return f: T->int }
            // The type for `g0` should be:
            // g0Type = generic<T1> { return IRFuncType{T1->int} }
            // with `g0Type`, we can rewrite `g0` into:
            // ```
            //    g0 : g0Type = generic<T>
            //    {
            //       ftype = specialize(g0Type, T);
            //       return f : ftype;
            //    }
            // ```
            IRBuilder typeBuilder(subBuilder->getSharedBuilder());
            IRCloneEnv cloneEnv = {};
            if (returnType)
            {
                HashSet<IRInst*> valuesToClone;
                markInstsToClone(valuesToClone, parentGeneric->getFirstBlock(), returnType);
                // For Function Types, we always clone all generic parameters regardless of whether
                // the generic parameter appears in the function signature or not.
                if (returnType->getOp() == kIROp_FuncType)
                {
                    for (auto genericParam : parentGeneric->getParams())
                    {
                        markInstsToClone(valuesToClone, parentGeneric->getFirstBlock(), genericParam);
                    }
                }
                if (valuesToClone.Count() == 0)
                {
                    // If the new generic has no parameters, set
                    // the generic inst's type to just `returnType`.
                    parentGeneric->setFullType((IRType*)returnType);
                }
                else
                {
                    // In the general case, we need to construct a separate
                    // generic value for the return type, and set the generic's type
                    // to the newly construct generic value.
                    typeBuilder.setInsertBefore(parentGeneric);
                    auto typeGeneric = typeBuilder.emitGeneric();
                    typeBuilder.setInsertInto(typeGeneric);
                    typeBuilder.emitBlock();
                    
                    for (auto child : parentGeneric->getFirstBlock()->getChildren())
                    {
                        if (valuesToClone.Contains(child))
                        {
                            cloneInst(&cloneEnv, &typeBuilder, child);
                        }
                    }
                    IRInst* clonedReturnType = nullptr;
                    cloneEnv.mapOldValToNew.TryGetValue(returnType, clonedReturnType);
                    if (clonedReturnType)
                    {
                        // If the type has explicit dependency on generic parameters, use
                        // the cloned type.
                        typeBuilder.emitReturn(clonedReturnType);
                    }
                    else
                    {
                        // Otherwise just use the original type value directly.
                        typeBuilder.emitReturn(returnType);
                    }
                    parentGeneric->setFullType((IRType*)typeGeneric);
                    returnType = typeGeneric;
                }
            }

            subBuilder->setInsertInto(parentGeneric->getFirstBlock());
#if 0
            // TODO: we cannot enable this right now as it breaks too many existing code
            // that is assuming a generic function type is `IRFuncType` rather than `IRSpecialize`.
            if (v->getFullType() != returnType)
            {
                // We need to rewrite the type of the return value as
                // `specialize(returnType, ...)`.
                SLANG_ASSERT(returnType->getOp() == kIROp_Generic);
                auto oldType = v->getFullType();
                SLANG_ASSERT(isChildOf(oldType, parentGeneric->getFirstBlock()));

                List<IRInst*> specializeArgs;
                for (auto param : parentGeneric->getParams())
                {
                    IRInst* arg = nullptr;
                    if (cloneEnv.mapOldValToNew.TryGetValue(param, arg))
                    {
                        specializeArgs.add(arg);
                    }
                }
                auto specializedType = subBuilder->emitSpecializeInst(
                    subBuilder->getTypeKind(),
                    returnType,
                    (UInt)specializeArgs.getCount(),
                    specializeArgs.getBuffer());
                oldType->replaceUsesWith(specializedType);
            }
#endif
            subBuilder->emitReturn(v);
            parentGeneric->moveToEnd();

            // There might be more outer generics,
            // so we need to loop until we run out.
            v = parentGeneric;
            auto parentBlock = as<IRBlock>(v->getParent());
            if (!parentBlock) break;

            parentGeneric = as<IRGeneric>(parentBlock->getParent());
            if (!parentGeneric) break;

        }
        return v;
    }

    // Attach target-intrinsic decorations to an instruction,
    // based on modifiers on an AST declaration.
    void addTargetIntrinsicDecorations(
        IRInst* irInst,
        Decl*   decl)
    {
        auto builder = getBuilder();

        for (auto targetMod : decl->getModifiersOfType<TargetIntrinsicModifier>())
        {
            String definition;
            auto definitionToken = targetMod->definitionToken;
            if (definitionToken.type == TokenType::StringLiteral)
            {
                definition = getStringLiteralTokenValue(definitionToken);
            }
            else if(definitionToken.type == TokenType::Identifier)
            {
                definition = definitionToken.getContent();
            }
            else
            {
                if( isStdLibMemberFuncDecl(decl) )
                {
                    // We will mark member functions by appending a `.` to the
                    // start of their name.
                    //
                    definition.append(".");
                }

                definition.append(decl->getName()->text);
            }

            UnownedStringSlice targetName;
            auto& targetToken = targetMod->targetToken;
            if( targetToken.type != TokenType::Unknown )
            {
                targetName = targetToken.getContent();
            }

            CapabilitySet targetCaps;
            if( targetName.getLength() == 0 )
            {
                targetCaps = CapabilitySet::makeEmpty();
            }
            else
            {
                CapabilityAtom targetCap = findCapabilityAtom(targetName);
                SLANG_ASSERT(targetCap != CapabilityAtom::Invalid);
                targetCaps = CapabilitySet(targetCap);
            }

            builder->addTargetIntrinsicDecoration(irInst, targetCaps, definition.getUnownedSlice());
        }

        if(auto nvapiMod = decl->findModifier<NVAPIMagicModifier>())
        {
            builder->addNVAPIMagicDecoration(irInst, decl->getName()->text.getUnownedSlice());
        }
    }

        /// Is `decl` a member function (or effectively a member function) when considered as a stdlib declaration?
    bool isStdLibMemberFuncDecl(
        Decl*   inDecl)
    {
        auto decl = as<CallableDecl>(inDecl);
        if(!decl)
            return false;

        // Constructors aren't really member functions, insofar
        // as they aren't called with a `this` parameter.
        //
        // TODO: We may also want to exclude `static` functions
        // here for the same reason, but this routine is only
        // used for the stdlib, where we don't currently have
        // any `static` member functions to worry about.
        //
        if(as<ConstructorDecl>(decl))
            return false;

        auto dd = decl->parentDecl;
        for(;;)
        {
            if(auto genericDecl = as<GenericDecl>(dd))
            {
                dd = genericDecl->parentDecl;
                continue;
            }

            if( auto subscriptDecl = as<SubscriptDecl>(dd) )
            {
                dd = subscriptDecl->parentDecl;
            }

            break;
        }

        // Note: the use of `AggTypeDeclBase` here instead of just
        // `AggTypeDecl` means that we consider a declaration that
        // is under a `struct` *or* an `extension` to be a member
        // function for our purposes.
        //
        if(as<AggTypeDeclBase>(dd))
            return true;

        return false;
    }

        /// Add a "catch-all" decoration for a stdlib function if it would be needed
    void addCatchAllIntrinsicDecorationIfNeeded(
        IRInst*             irInst,
        FunctionDeclBase*   decl)
    {
        // We don't need an intrinsic decoration on a function that has a body,
        // since the body can be used as the "catch-all" case.
        //
        if(decl->body)
            return;

        // Only standard library declarations should get any kind of catch-all
        // treatment by default. Declarations in user case are responsible
        // for marking things as target intrinsics if they want to go down
        // that (unsupported) route.
        //
        if(!isFromStdLib(decl))
            return;

        // No need to worry about functions that lower to intrinsic IR opcodes
        // (or pseudo-ops).
        //
        if(decl->findModifier<IntrinsicOpModifier>())
            return;

        // We also don't need an intrinsic decoration if the function already
        // had a catch-all case on one of its target overloads.
        //
        for( auto f = decl->primaryDecl; f; f = f->nextDecl )
        {
            for(auto targetMod : f->getModifiersOfType<TargetIntrinsicModifier>())
            {
                // If we find a catch-all case (marked as either *no* target
                // token or an empty target name), then we should bail out.
                //
                if(targetMod->targetToken.type == TokenType::Unknown)
                    return;
                else if(!targetMod->targetToken.hasContent())
                    return;
            }
        }

        String definition;
        
        // If we have a member function, then we want the default intrinsic
        // definition to reflect this fact so that we can emit it correctly
        // (the assumption is that a catch-all definition of a member function
        // is itself implemented as a member function).
        //
        if( isStdLibMemberFuncDecl(decl) )
        {
            // We will mark member functions by appending a `.` to the
            // start of their name.
            //
            definition.append(".");
        }

        // We want to output the name of the declaration,
        // but in some cases the actual `decl` that has
        // to be emitted is not the one with the name.
        //
        // In particular, an accessor declaration (e.g.,
        // a `get`ter` in a subscript or property) doesn't
        // have a name, but its parent should.
        //
        Decl* declForName = decl;
        if(auto accessorDecl = as<AccessorDecl>(decl))
            declForName = decl->parentDecl;

        definition.append(getText(declForName->getName()));

        getBuilder()->addTargetIntrinsicDecoration(irInst, CapabilitySet::makeEmpty(), definition.getUnownedSlice());
    }

    void addParamNameHint(IRInst* inst, IRLoweringParameterInfo const& info)
    {
        if(auto decl = info.decl)
        {
            addNameHint(context, inst, decl);
        }
        else if( info.isThisParam )
        {
            addNameHint(context, inst, "this");
        }
    }

    IRIntLit* _getIntLitFromAttribute(IRBuilder* builder, Attribute* attrib)
    {
        attrib->args.getCount();
        SLANG_ASSERT(attrib->args.getCount() ==1);
        Expr* expr = attrib->args[0];
        auto intLitExpr = as<IntegerLiteralExpr>(expr);
        SLANG_ASSERT(intLitExpr);
        return as<IRIntLit>(builder->getIntValue(builder->getIntType(), intLitExpr->value));
    }

    IRStringLit* _getStringLitFromAttribute(IRBuilder* builder, Attribute* attrib)
    {
        attrib->args.getCount();
        SLANG_ASSERT(attrib->args.getCount() == 1);
        Expr* expr = attrib->args[0];

        auto stringLitExpr = as<StringLiteralExpr>(expr);
        SLANG_ASSERT(stringLitExpr);
        return as<IRStringLit>(builder->getStringValue(stringLitExpr->value.getUnownedSlice()));
    }

    IRInst* lowerFuncType(FunctionDeclBase* decl)
    {
        NestedContext nestedContextFuncType(this);
        auto funcTypeBuilder = nestedContextFuncType.getBuilder();
        auto funcTypeContext = nestedContextFuncType.getContext();

        auto outerGenerics = emitOuterGenerics(funcTypeContext, decl, decl);

        FuncDeclBaseTypeInfo info;
        _lowerFuncDeclBaseTypeInfo(
            funcTypeContext,
            createDefaultSpecializedDeclRef(funcTypeContext, decl),
            info);

        auto irFuncType = info.type;

        return finishOuterGenerics(funcTypeBuilder, irFuncType, outerGenerics);
    }

    bool isClassType(IRType* type)
    {
        if (auto specialize = as<IRSpecialize>(type))
        {
            return findSpecializeReturnVal(specialize)->getOp() == kIROp_ClassType;
        }
        else if (auto genericInst = as<IRGeneric>(type))
        {
            return findGenericReturnVal(genericInst)->getOp() == kIROp_ClassType;
        }
        return type->getOp() == kIROp_ClassType;
    }

    LoweredValInfo lowerFuncDecl(FunctionDeclBase* decl)
    {
        // We are going to use a nested builder, because we will
        // change the parent node that things get nested into.
        //
        NestedContext nestedContextFunc(this);
        auto subBuilder = nestedContextFunc.getBuilder();
        auto subContext = nestedContextFunc.getContext();

        auto outerGeneric = emitOuterGenerics(subContext, decl, decl);

        // need to create an IR function here

        IRFunc* irFunc = subBuilder->createFunc();
        addNameHint(context, irFunc, decl);
        addLinkageDecoration(context, irFunc, decl);

        FuncDeclBaseTypeInfo info;
        _lowerFuncDeclBaseTypeInfo(
            subContext,
            createDefaultSpecializedDeclRef(context, decl),
            info);

        auto irFuncType = info.type;
        auto& irResultType = info.resultType;
        auto& parameterLists = info.parameterLists;
        auto& paramTypes = info.paramTypes;

        irFunc->setFullType(irFuncType);

        subBuilder->setInsertInto(irFunc);

        // If a function is imported from another module then
        // we usually don't want to emit it as a definition, and
        // will instead only emit a declaration for it with an
        // appropriate `[import(...)]` linkage decoration.
        //
        // However, if the function is marked with `[__unsafeForceInlineEarly]`
        // then we need to make sure the IR for its definition is available
        // to the mandatory optimization passes.
        //
        // TODO: The design here means that we will re-emit the inline
        // function from its AST in every module that uses it. We should
        // instead have logic to clone the target function in from the
        // pre-generated IR for the module that defines it (or do some kind
        // of minimal linking to bring in the inline functions).
        //
        if (isImportedDecl(decl) && !isForceInlineEarly(decl))
        {
            // Always emit imported declarations as declarations,
            // and not definitions.
        }
        else if (!decl->body)
        {
            // This is a function declaration without a body.
            // In Slang we currently try not to support forward declarations
            // (although we might have to give in eventually), so
            // this case should really only occur for builtin declarations.
        }
        else
        {
            // This is a function definition, so we need to actually
            // construct IR for the body...
            IRBlock* entryBlock = subBuilder->emitBlock();
            subBuilder->setInsertInto(entryBlock);

            UInt paramTypeIndex = 0;
            for( auto paramInfo : parameterLists.params )
            {
                auto irParamType = paramTypes[paramTypeIndex++];

                LoweredValInfo paramVal;

                IRParam* irParam = nullptr;

                switch( paramInfo.direction )
                {
                default:
                    {
                        // The parameter is being used for input/output purposes,
                        // so it will lower to an actual parameter with a pointer type.
                        //
                        // TODO: Is this the best representation we can use?

                        irParam = subBuilder->emitParam(irParamType);
                        if(auto paramDecl = paramInfo.decl)
                        {
                            addVarDecorations(context, irParam, paramDecl);
                            subBuilder->addHighLevelDeclDecoration(irParam, paramDecl);
                        }
                        addParamNameHint(irParam, paramInfo);

                        paramVal = LoweredValInfo::ptr(irParam);

                        // TODO: We might want to copy the pointed-to value into
                        // a temporary at the start of the function, and then copy
                        // back out at the end, so that we don't have to worry
                        // about things like aliasing in the function body.
                        //
                        // For now we will just use the storage that was passed
                        // in by the caller, knowing that our current lowering
                        // at call sites will guarantee a fresh/unique location.
                    }
                    break;

                case kParameterDirection_In:
                    {
                        // Simple case of a by-value input parameter.
                        //
                        // We start by declaring an IR parameter of the same type.
                        //
                        auto paramDecl = paramInfo.decl;
                        irParam = subBuilder->emitParam(irParamType);
                        if( paramDecl )
                        {
                            addVarDecorations(context, irParam, paramDecl);
                            subBuilder->addHighLevelDeclDecoration(irParam, paramDecl);
                        }
                        addParamNameHint(irParam, paramInfo);
                        paramVal = LoweredValInfo::simple(irParam);
                        //
                        // HLSL allows a function parameter to be used as a local
                        // variable in the function body (just like C/C++), so
                        // we need to support that case as well.
                        //
                        // However, if we notice that the parameter was marked
                        // `const`, then we can skip this step.
                        //
                        // TODO: we should consider having all parameter be implicitly
                        // immutable except in a specific "compatibility mode."
                        //
                        if(paramDecl && paramDecl->findModifier<ConstModifier>())
                        {
                            // This parameter was declared to be immutable,
                            // so there should be no assignment to it in the
                            // function body, and we don't need a temporary.
                        }
                        else
                        {
                            // The parameter migth get used as a temporary in
                            // the function body. We will allocate a mutable
                            // local variable for is value, and then assign
                            // from the parameter to the local at the start
                            // of the function.
                            //
                            auto irLocal = subBuilder->emitVar(irParamType);
                            auto localVal = LoweredValInfo::ptr(irLocal);
                            assign(subContext, localVal, paramVal);
                            //
                            // When code later in the body of the function refers
                            // to the parameter declaration, it will actually refer
                            // to the value stored in the local variable.
                            //
                            paramVal = localVal;
                        }
                    }
                    break;
                }

                if( auto paramDecl = paramInfo.decl )
                {
                    setValue(subContext, paramDecl, paramVal);
                }

                if (paramInfo.isThisParam)
                {
                    subContext->thisVal = paramVal;
                }
            }

            {

                auto attr = decl->findModifier<PatchConstantFuncAttribute>();

                // I needed to test for patchConstantFuncDecl here
                // because it is only set if validateEntryPoint is called with Hull as the required stage
                // If I just build domain shader, and then the attribute exists, but patchConstantFuncDecl is not set
                // and thus leads to a crash.
                if (attr && attr->patchConstantFuncDecl)
                {
                    // We need to lower the function
                    FuncDecl* patchConstantFunc = attr->patchConstantFuncDecl;
                    assert(patchConstantFunc);

                    // Convert the patch constant function into IRInst
                    IRInst* irPatchConstantFunc = getSimpleVal(context, ensureDecl(subContext, patchConstantFunc));

                    // Attach a decoration so that our IR function references
                    // the patch constant function.
                    //
                    subContext->irBuilder->addPatchConstantFuncDecoration(
                        irFunc,
                        irPatchConstantFunc);

                }
            }


            // We will now set about emitting the code for the body of
            // the function/callable.
            //
            // In the case of an initializer ("constructor") declaration,
            // the `this` value is not a parameter, but rather a placeholder
            // for the value that will be returned. We thus need to set up
            // a local variable to represent this value.
            //
            auto constructorDecl = as<ConstructorDecl>(decl);
            if(constructorDecl)
            {
                auto thisVar = subContext->irBuilder->emitVar(irResultType);
                subContext->thisVal = LoweredValInfo::ptr(thisVar);

                // For class-typed objects, we need to allocate it from heap.
                if (isClassType(irResultType))
                {
                    auto allocatedObj = subContext->irBuilder->emitAllocObj(irResultType);
                    subContext->irBuilder->emitStore(thisVar, allocatedObj);
                }
            }

            // We lower whatever statement was stored on the declaration
            // as the body of the new IR function.
            //
            lowerStmt(subContext, decl->body);

            // We need to carefully add a terminator instruction to the end
            // of the body, in case the user didn't do so.
            //
            if (!subContext->irBuilder->getBlock()->getTerminator())
            {
                if(constructorDecl)
                {
                    // A constructor declaration should return the
                    // value of the `this` variable that was set
                    // up at the start.
                    //
                    // TODO: This should also apply if any code
                    // path in an initializer/constructor attempts
                    // to do an early `return;`.
                    //
                    subContext->irBuilder->emitReturn(
                        getSimpleVal(subContext, subContext->thisVal));
                }
                else if(as<IRVoidType>(irResultType))
                {
                    // `void`-returning function can get an implicit
                    // return on exit of the body statement.
                    IRInst* returnInst = subContext->irBuilder->emitReturn();

                    if (BlockStmt* blockStmt = as<BlockStmt>(decl->body))
                    {
                        returnInst->sourceLoc = blockStmt->closingSourceLoc;
                    }
                    else
                    {
                        returnInst->sourceLoc = SourceLoc();
                    }
                }
                else
                {
                    // Value-returning function is expected to `return`
                    // on every control-flow path. We need to enforce
                    // this by putting an `unreachable` terminator here,
                    // and then emit a dataflow error if this block
                    // can't be eliminated.
                    subContext->irBuilder->emitMissingReturn();
                }
            }
        }

        getBuilder()->addHighLevelDeclDecoration(irFunc, decl);

        // If this declaration was marked as being an intrinsic for a particular
        // target, then we should reflect that here.
        for( auto targetMod : decl->getModifiersOfType<SpecializedForTargetModifier>() )
        {
            // `targetMod` indicates that this particular declaration represents
            // a specialized definition of the particular function for the given
            // target, and we need to reflect that at the IR level.

            auto targetName = targetMod->targetToken.getContent();
            auto targetCap = findCapabilityAtom(targetName);

            getBuilder()->addTargetDecoration(irFunc, CapabilitySet(targetCap));
        }

        // If this declaration was marked as having a target-specific lowering
        // for a particular target, then handle that here.
        addTargetIntrinsicDecorations(irFunc, decl);

        addCatchAllIntrinsicDecorationIfNeeded(irFunc, decl);

        // If this declaration requires certain GLSL extension (or a particular GLSL version)
        // for it to be usable, then declare that here.
        //
        // TODO: We should wrap this an `SpecializedForTargetModifier` together into a single
        // case for enumerating the "capabilities" that a declaration requires.
        //
        for(auto extensionMod : decl->getModifiersOfType<RequiredGLSLExtensionModifier>())
        {
            getBuilder()->addRequireGLSLExtensionDecoration(irFunc, extensionMod->extensionNameToken.getContent());
        }
        for(auto versionMod : decl->getModifiersOfType<RequiredGLSLVersionModifier>())
        {
            getBuilder()->addRequireGLSLVersionDecoration(irFunc, Int(getIntegerLiteralValue(versionMod->versionNumberToken)));
        }
        for (auto versionMod : decl->getModifiersOfType<RequiredSPIRVVersionModifier>())
        {
            getBuilder()->addRequireSPIRVVersionDecoration(irFunc, versionMod->version);
        }
        for (auto versionMod : decl->getModifiersOfType<RequiredCUDASMVersionModifier>())
        {
            getBuilder()->addRequireCUDASMVersionDecoration(irFunc, versionMod->version);
        }

        if(decl->findModifier<RequiresNVAPIAttribute>())
        {
            getBuilder()->addSimpleDecoration<IRRequiresNVAPIDecoration>(irFunc);
        }

        if(decl->findModifier<NoInlineAttribute>())
        {
            getBuilder()->addSimpleDecoration<IRNoInlineDecoration>(irFunc);
        }

        if (auto attr = decl->findModifier<InstanceAttribute>())
        {
            IRIntLit* intLit = _getIntLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_InstanceDecoration, intLit);
        }

        if(auto attr = decl->findModifier<MaxVertexCountAttribute>())
        {
            IRIntLit* intLit = _getIntLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_MaxVertexCountDecoration, intLit);
        }

        if(auto attr = decl->findModifier<NumThreadsAttribute>())
        {
            auto builder = getBuilder();
            IRType* intType = builder->getIntType();

            IRInst* operands[3] = {
                builder->getIntValue(intType, attr->x),
                builder->getIntValue(intType, attr->y),
                builder->getIntValue(intType, attr->z)
            };

           builder->addDecoration(irFunc, kIROp_NumThreadsDecoration, operands, 3);
        }

        if(decl->findModifier<ReadNoneAttribute>())
        {
            getBuilder()->addSimpleDecoration<IRReadNoneDecoration>(irFunc);
        }

        if (decl->findModifier<EarlyDepthStencilAttribute>())
        {
            getBuilder()->addSimpleDecoration<IREarlyDepthStencilDecoration>(irFunc);
        }

        if (auto attr = decl->findModifier<DomainAttribute>())
        {
            IRStringLit* stringLit = _getStringLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_DomainDecoration, stringLit);
        }

        if (auto attr = decl->findModifier<PartitioningAttribute>())
        {
            IRStringLit* stringLit = _getStringLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_PartitioningDecoration, stringLit);
        }

        if (auto attr = decl->findModifier<OutputTopologyAttribute>())
        {
            IRStringLit* stringLit = _getStringLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_OutputTopologyDecoration, stringLit);
        }

        if (auto attr = decl->findModifier<OutputControlPointsAttribute>())
        {
            IRIntLit* intLit = _getIntLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_OutputControlPointsDecoration, intLit);
        }

        if (auto attr = decl->findModifier<SPIRVInstructionOpAttribute>())
        {
            IRIntLit* intLit = _getIntLitFromAttribute(getBuilder(), attr);
            getBuilder()->addDecoration(irFunc, kIROp_SPIRVOpDecoration, intLit);
        }

        if(decl->findModifier<UnsafeForceInlineEarlyAttribute>())
        {
            getBuilder()->addDecoration(irFunc, kIROp_UnsafeForceInlineEarlyDecoration);
        }

        if (auto attr = decl->findModifier<CustomJVPAttribute>())
        {
            auto loweredVal = lowerLValueExpr(this->context, attr->funcDeclRef);
            SLANG_ASSERT(loweredVal.flavor == LoweredValInfo::Flavor::Simple);
            IRFunc* jvpFunc = as<IRFunc>(loweredVal.val);
            getBuilder()->addDecoration(irFunc, kIROp_JVPDerivativeReferenceDecoration, jvpFunc);
        }

        // For convenience, ensure that any additional global
        // values that were emitted while outputting the function
        // body appear before the function itself in the list
        // of global values.
        irFunc->moveToEnd();

        // If this function is defined inside an interface, add a reference to the IRFunc from
        // the interface's type definition.
        auto finalVal = finishOuterGenerics(subBuilder, irFunc, outerGeneric);

        return LoweredValInfo::simple(finalVal);
    }

    LoweredValInfo visitGenericDecl(GenericDecl * genDecl)
    {
        // TODO: Should this just always visit/lower the inner decl?

        if (auto innerFuncDecl = as<FunctionDeclBase>(genDecl->inner))
            return ensureDecl(context, innerFuncDecl);
        else if (auto innerStructDecl = as<StructDecl>(genDecl->inner))
        {
            ensureDecl(context, innerStructDecl);
            return LoweredValInfo();
        }
        else if( auto extensionDecl = as<ExtensionDecl>(genDecl->inner) )
        {
            return ensureDecl(context, extensionDecl);
        }
        else if (auto interfaceDecl = as<InterfaceDecl>(genDecl->inner))
        {
            return ensureDecl(context, interfaceDecl);
        }
        SLANG_RELEASE_ASSERT(false);
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitFunctionDeclBase(FunctionDeclBase* decl)
    {
        // A function declaration may have multiple, target-specific
        // overloads, and we need to emit an IR version of each of these.

        // The front end will form a linked list of declarations with
        // the same signature, whenever there is any kind of redeclaration.
        // We will look to see if that linked list has been formed.
        auto primaryDecl = decl->primaryDecl;

        if (!primaryDecl)
        {
            // If there is no linked list then we are in the ordinary
            // case with a single declaration, and no special handling
            // is needed.
            return lowerFuncDecl(decl);
        }

        // Otherwise, we need to walk the linked list of declarations
        // and make sure to emit IR code for any targets that need it.

        // TODO: Need to be careful about how this is approached,
        // to avoid emitting a bunch of extra definitions in the IR.

        auto primaryFuncDecl = as<FunctionDeclBase>(primaryDecl);
        SLANG_ASSERT(primaryFuncDecl);
        LoweredValInfo result = lowerFuncDecl(primaryFuncDecl);
        for (auto dd = primaryDecl->nextDecl; dd; dd = dd->nextDecl)
        {
            auto funcDecl = as<FunctionDeclBase>(dd);
            SLANG_ASSERT(funcDecl);
            lowerFuncDecl(funcDecl);

            // Note: Because we are iterating over multiple declarations,
            // but only one will be registered as the value for `decl`
            // in the global mapping by `ensureDecl()`, we have to take
            // responsibility here for registering a lowered value
            // for the remaining (non-primary) declarations.
            //
            // It doesn't really matter which one we register here, because
            // they will all have the same mangled name in the IR, but we
            // default to the `result` that is returned from this visitor,
            // so that all the declarations share the same IR representative.
            //
            setGlobalValue(context->shared, funcDecl, result);
        }
        return result;
    }
};

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    DeclBase*       decl)
{
    IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, decl->loc);

    DeclLoweringVisitor visitor;
    visitor.context = context;

    try
    {
        return visitor.dispatch(decl);
    }
    // Don't emit any context message for an explicit `AbortCompilationException`
    // because it should only happen when an error is already emitted.
    catch(const AbortCompilationException&) { throw; }
    catch(...)
    {
        context->getSink()->noteInternalErrorLoc(decl->loc);
        throw;
    }
}

// Ensure that a version of the given declaration has been emitted to the IR
LoweredValInfo ensureDecl(
    IRGenContext*   context,
    Decl*           decl)
{
    auto shared = context->shared;

    LoweredValInfo result;

    // Look for an existing value installed in this context
    auto env = context->env;
    while(env)
    {
        if(env->mapDeclToValue.TryGetValue(decl, result))
            return result;

        env = env->outer;
    }

    // If we have a decl that's a generic value/type decl then something has gone seriously
    // wrong
    if (as<GenericValueParamDecl>(decl) || as<GenericTypeParamDecl>(decl))
    {
        SLANG_UNEXPECTED("Generic type/value shouldn't be handled here!");
    }

    IRBuilder subIRBuilder(context->irBuilder->getSharedBuilder());
    subIRBuilder.setInsertInto(subIRBuilder.getModule());

    IRGenEnv subEnv;
    subEnv.outer = context->env;

    IRGenContext subContext = *context;
    subContext.irBuilder = &subIRBuilder;
    subContext.env = &subEnv;

    result = lowerDecl(&subContext, decl);

    // By default assume that any value we are lowering represents
    // something that should be installed globally.
    setGlobalValue(shared, decl, result);

    return result;
}

// Can the IR lowered version of this declaration ever be an `IRGeneric`?
bool canDeclLowerToAGeneric(Decl* decl)
{
    // A callable decl lowers to an `IRFunc`, and can be generic
    if(as<CallableDecl>(decl)) return true;

    // An aggregate type decl lowers to an `IRStruct`, and can be generic
    if(as<AggTypeDecl>(decl)) return true;

    // An inheritance decl lowers to an `IRWitnessTable`, and can be generic
    if(as<InheritanceDecl>(decl)) return true;

    // A `typedef` declaration nested under a generic will turn into
    // a generic that returns a type (a simple type-level function).
    if(as<TypeDefDecl>(decl)) return true;

    // If we have a variable declaration that is *static* and *const* we can lower to a generic
    if (auto varDecl = as<VarDecl>(decl))
    {
        if (varDecl->hasModifier<HLSLStaticModifier>() && varDecl->hasModifier<ConstModifier>())
        {
            return true;
        }
    }
    
    return false;
}

static bool isInterfaceRequirement(Decl* decl)
{
   auto ancestor = decl->parentDecl;
   for(; ancestor; ancestor = ancestor->parentDecl )
   {
       if(as<InterfaceDecl>(ancestor))
           return true;

       if(as<ExtensionDecl>(ancestor))
           return false;
   }
   return false;
}

    /// Add flattened "leaf" elements from `val` to the `ioArgs` list
static void _addFlattenedTupleArgs(
    List<IRInst*>&  ioArgs,
    IRInst*         val)
{
    if( auto tupleVal = as<IRMakeTuple>(val) )
    {
        // If the value is a tuple, we can add its element directly.
        auto elementCount = tupleVal->getOperandCount();
        for( UInt i = 0; i < elementCount; ++i )
        {
            _addFlattenedTupleArgs(ioArgs, tupleVal->getOperand(i));
        }
    }
    //
    // TODO: We should handle the case here where `val`
    // is not a `makeTuple` instruction, but still has
    // a tuple *type*. In that case we should apply `getTupleElement`
    // for each of its elements and then recurse on them.
    //
    else
    {
        ioArgs.add(val);
    }
}

LoweredValInfo emitDeclRef(
    IRGenContext*           context,
    Decl*            decl,
    Substitutions*   subst,
    IRType*                 type)
{
    const auto initialSubst = subst;
    SLANG_UNUSED(initialSubst);

    // We need to proceed by considering the specializations that
    // have been put in place.

    // If the declaration would not get wrapped in a `IRGeneric`,
    // even if it is nested inside of an AST `GenericDecl`, then
    // we should also ignore any generic substitutions.
    if(!canDeclLowerToAGeneric(decl))
    {
        while(auto genericSubst = as<GenericSubstitution>(subst))
            subst = genericSubst->outer;
    }

    // In the simplest case, there is no specialization going
    // on, and the decl-ref turns into a reference to the
    // lowered IR value for the declaration.
    if(!subst)
    {
        LoweredValInfo loweredDecl = ensureDecl(context, decl);
        return loweredDecl;
    }

    // Otherwise, we look at the kind of substitution, and let it guide us.
    if(auto genericSubst = as<GenericSubstitution>(subst))
    {
        // A generic substitution means we will need to output
        // a `specialize` instruction to specialize the generic.
        //
        // First we want to emit the value without generic specialization
        // applied, to get a correct value for it.
        //
        // Note: we only "unwrap" a single layer from the
        // substitutions here, because the underlying declaration
        // might be nested in multiple generics, or it might
        // come from an interface.
        //
        LoweredValInfo genericVal = emitDeclRef(
            context,
            decl,
            genericSubst->outer,
            context->irBuilder->getGenericKind());

        // There's no reason to specialize something that maps to a NULL pointer.
        if (genericVal.flavor == LoweredValInfo::Flavor::None)
            return LoweredValInfo();

        // We can only really specialize things that map to single values.
        // It would be an error if we got a non-`None` value that
        // wasn't somehow a single value.
        auto irGenericVal = getSimpleVal(context, genericVal);

        // We have the IR value for the generic we'd like to specialize,
        // and now we need to get the value for the arguments.
        List<IRInst*> irArgs;
        for (auto argVal : genericSubst->args)
        {
            auto irArgVal = lowerSimpleVal(context, argVal);
            SLANG_ASSERT(irArgVal);

            // It is possible that some of the arguments to the generic
            // represent conformances to conjunction types like `A & B`.
            // These conjunction conformances will appear as tuples in
            // the IR, and we want to "flatten" them here so that we
            // pass each "leaf" witness table as its own argument (to
            // match the way that generic parameters are being emitted
            // to the IR).
            //
            // TODO: This isn't a robust strategy if we ever have to deal
            // with tuples as ordinary values.
            //
            _addFlattenedTupleArgs(irArgs, irArgVal);
        }

        // Once we have both the generic and its arguments,
        // we can emit a `specialize` instruction and use
        // its value as the result.
        auto irSpecializedVal = context->irBuilder->emitSpecializeInst(
            type,
            irGenericVal,
            irArgs.getCount(),
            irArgs.getBuffer());

        return LoweredValInfo::simple(irSpecializedVal);
    }
    else if(auto thisTypeSubst = as<ThisTypeSubstitution>(subst))
    {
        if(decl == thisTypeSubst->interfaceDecl)
        {
            // This is a reference to the interface type itself,
            // through the this-type substitution, so it is really
            // a reference to the this-type.
            return lowerType(context, thisTypeSubst->witness->sub);
        }

        if(isInterfaceRequirement(decl))
        {
            // Somebody is trying to look up an interface requirement
            // "through" some concrete type. We need to lower this decl-ref
            // as a lookup of the corresponding member in a witness table.
            //
            // The witness table itself is referenced by the this-type
            // substitution, so we can just lower that.
            //
            // Note: unlike the case for generics above, in the interface-lookup
            // case, we don't end up caring about any further outer substitutions.
            // That is because even if we are naming `ISomething<Foo>.doIt()`,
            // a method inside a generic interface, we don't actually care
            // about the substitution of `Foo` for the parameter `T` of
            // `ISomething<T>`. That is because we really care about the
            // witness table for the concrete type that conforms to `ISomething<Foo>`.
            //
            auto irWitnessTable = lowerSimpleVal(context, thisTypeSubst->witness);
            //
            // The key to use for looking up the interface member is
            // derived from the declaration.
            //
            auto irRequirementKey = getInterfaceRequirementKey(context, decl);
            //
            // Those two pieces of information tell us what we need to
            // do in order to look up the value that satisfied the requirement.
            //
            auto irSatisfyingVal = context->irBuilder->emitLookupInterfaceMethodInst(
                type,
                irWitnessTable,
                irRequirementKey);
            return LoweredValInfo::simple(irSatisfyingVal);
        }
        else
        {
            // This case is a reference to a member declaration of the interface
            // (or added by an extension of the interface) that does *not*
            // represent a requirement of the interface.
            //
            // Our policy is that concrete methods/members on an interface type
            // are lowered as generics, where the generic parameter represents
            // the `ThisType`.
            //
            auto genericVal = emitDeclRef(context, decl, thisTypeSubst->outer, context->irBuilder->getGenericKind());
            auto irGenericVal = getSimpleVal(context, genericVal);

            // In order to reference the member for a particular type, we
            // specialize the generic for that type.
            //
            IRInst* irSubType = lowerType(context, thisTypeSubst->witness->sub);
            IRInst* irSubTypeWitness = lowerSimpleVal(context, thisTypeSubst->witness);

            IRInst* irSpecializeArgs[] = { irSubType, irSubTypeWitness };
            auto irSpecializedVal = context->irBuilder->emitSpecializeInst(
                type,
                irGenericVal,
                2,
                irSpecializeArgs);
            return LoweredValInfo::simple(irSpecializedVal);
        }
    }
    else
    {
        SLANG_UNEXPECTED("uhandled substitution type");
        UNREACHABLE_RETURN(LoweredValInfo());
    }
}

LoweredValInfo emitDeclRef(
    IRGenContext*   context,
    DeclRef<Decl>   declRef,
    IRType*         type)
{
    return emitDeclRef(
        context,
        declRef.decl,
        declRef.substitutions.substitutions,
        type);
}

static void lowerFrontEndEntryPointToIR(
    IRGenContext*   context,
    EntryPoint*     entryPoint,
    String moduleName)
{
    // TODO: We should emit an entry point as a dedicated IR function
    // (distinct from the IR function used if it were called normally),
    // with a mangled name based on the original function name plus
    // the stage for which it is being compiled as an entry point (so
    // that entry points for distinct stages always have distinct names).
    //
    // For now we just have an (implicit) constraint that a given
    // function should only be used as an entry point for one stage,
    // and any such function should *not* be used as an ordinary function.

    auto entryPointFuncDecl = entryPoint->getFuncDecl();

    auto builder = context->irBuilder;
    builder->setInsertInto(builder->getModule()->getModuleInst());

    auto loweredEntryPointFunc = getSimpleVal(context,
        ensureDecl(context, entryPointFuncDecl));

    // Attach a marker decoration so that we recognize
    // this as an entry point.
    //
    IRInst* instToDecorate = loweredEntryPointFunc;
    if(auto irGeneric = as<IRGeneric>(instToDecorate))
    {
        instToDecorate = findGenericReturnVal(irGeneric);
    }

    {
        Name* entryPointName = entryPoint->getFuncDecl()->getName();
        builder->addEntryPointDecoration(instToDecorate, entryPoint->getProfile(), entryPointName->text.getUnownedSlice(), moduleName.getUnownedSlice());
    }

    // Go through the entry point parameters creating decorations from layout as appropriate
    // But only if this is a definition not a declaration
    if (isDefinition(instToDecorate))
    {
        FilteredMemberList<ParamDecl> params = entryPointFuncDecl->getParameters();

        IRGlobalValueWithParams* valueWithParams = as<IRGlobalValueWithParams>(instToDecorate);
        if (valueWithParams)
        {
            IRParam* irParam = valueWithParams->getFirstParam();

            for (auto param : params)
            {
                if (auto modifier = param->findModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
                {
                    IROp op = kIROp_Invalid;

                    if (as<HLSLTriangleModifier>(modifier))
                        op = kIROp_TriangleInputPrimitiveTypeDecoration;
                    else if (as<HLSLPointModifier>(modifier))
                        op = kIROp_PointInputPrimitiveTypeDecoration;
                    else if (as<HLSLLineModifier>(modifier))
                        op = kIROp_LineInputPrimitiveTypeDecoration; 
                    else if (as<HLSLLineAdjModifier>(modifier))
                        op = kIROp_LineAdjInputPrimitiveTypeDecoration;
                    else if (as<HLSLTriangleAdjModifier>(modifier))
                        op = kIROp_TriangleAdjInputPrimitiveTypeDecoration;

                    if (op != kIROp_Invalid)
                    {
                        builder->addDecoration(irParam, op);
                    }
                    else
                    {
                        SLANG_UNEXPECTED("unhandled primitive type");
                    }
                }

                irParam = irParam->getNextParam();
            }
        }
    }
}

static void lowerProgramEntryPointToIR(
    IRGenContext*                               context,
    EntryPoint*                                 entryPoint,
    EntryPoint::EntryPointSpecializationInfo*   specializationInfo)
{
    auto entryPointFuncDeclRef = entryPoint->getFuncDeclRef();
    if(specializationInfo)
        entryPointFuncDeclRef = specializationInfo->specializedFuncDeclRef;

    // First, lower the entry point like an ordinary function

    auto entryPointFuncType = lowerType(context, getFuncType(context->astBuilder, entryPointFuncDeclRef));

    auto builder = context->irBuilder;
    builder->setInsertInto(builder->getModule()->getModuleInst());

    auto loweredEntryPointFunc = getSimpleVal(context,
        emitDeclRef(context, entryPointFuncDeclRef, entryPointFuncType));

    if(!loweredEntryPointFunc->findDecoration<IRLinkageDecoration>())
    {
        builder->addExportDecoration(loweredEntryPointFunc, getMangledName(context->astBuilder, entryPointFuncDeclRef).getUnownedSlice());
    }

    // We may have shader parameters of interface/existential type,
    // which need us to supply concrete type information for specialization.
    //
    if(specializationInfo && specializationInfo->existentialSpecializationArgs.getCount() != 0)
    {
        List<IRInst*> existentialSlotArgs;
        for(auto arg : specializationInfo->existentialSpecializationArgs )
        {

            auto irArgType = lowerSimpleVal(context, arg.val);
            existentialSlotArgs.add(irArgType);

            if(auto witness = arg.witness)
            {
                auto irWitnessTable = lowerSimpleVal(context, witness);
                existentialSlotArgs.add(irWitnessTable);
            }
        }

        builder->addBindExistentialSlotsDecoration(loweredEntryPointFunc, existentialSlotArgs.getCount(), existentialSlotArgs.getBuffer());
    }
}

    /// Ensure that `decl` and all relevant declarations under it get emitted.
static void ensureAllDeclsRec(
    IRGenContext*   context,
    Decl*           decl)
{
    ensureDecl(context, decl);

    // Note: We are checking here for aggregate type declarations, and
    // not for `ContainerDecl`s in general. This is because many kinds
    // of container declarations will already take responsibility for emitting
    // their children directly (e.g., a function declaration is responsible
    // for emitting its own parameters).
    //
    // Aggregate types are the main case where we can emit an outer declaration
    // and not the stuff nested inside of it.
    //
    if(auto containerDecl = as<AggTypeDeclBase>(decl))
    {
        for (auto memberDecl : containerDecl->members)
        {
            ensureAllDeclsRec(context, memberDecl);
        }
    }
    else if (auto genericDecl = as<GenericDecl>(decl))
    {
        ensureAllDeclsRec(context, genericDecl->inner);
    }
}

RefPtr<IRModule> generateIRForTranslationUnit(
    ASTBuilder* astBuilder,
    TranslationUnitRequest* translationUnit)
{
    auto session = translationUnit->getSession();
    auto compileRequest = translationUnit->compileRequest;

    SharedIRGenContext sharedContextStorage(
        session,
        translationUnit->compileRequest->getSink(),
        translationUnit->compileRequest->getLinkage()->m_obfuscateCode,
        translationUnit->getModuleDecl());
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    IRGenContext contextStorage(sharedContext, astBuilder);
    IRGenContext* context = &contextStorage;

    RefPtr<IRModule> module = IRModule::create(session);

    SharedIRBuilder sharedBuilderStorage(module);
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;

    IRBuilder builderStorage(sharedBuilder);
    IRBuilder* builder = &builderStorage;

    context->irBuilder = builder;

    // We need to emit IR for all public/exported symbols
    // in the translation unit.
    //
    // For now, we will assume that *all* global-scope declarations
    // represent public/exported symbols.

    // First, ensure that all entry points have been emitted,
    // in case they require special handling.
    for (auto entryPoint : translationUnit->getEntryPoints())
    {
        List<SourceFile*> sources = translationUnit->getSourceFiles();
        SourceFile* source = sources.getFirst();
        PathInfo pInfo = source->getPathInfo();
        String path = pInfo.getMostUniqueIdentity();
        lowerFrontEndEntryPointToIR(context, entryPoint, Path::getFileNameWithoutExt(path));
    }

    //
    // Next, ensure that all other global declarations have
    // been emitted.
    for (auto decl : translationUnit->getModuleDecl()->members)
    {
        ensureAllDeclsRec(context, decl);
    }

    // Build a global instruction to hold all the string
    // literals used in the module.
    {
        auto& stringLits = sharedContext->m_stringLiterals;
        auto stringLitCount = stringLits.getCount();
        if(stringLitCount != 0)
        {
            builder->setInsertInto(module->getModuleInst());
            builder->emitIntrinsicInst(builder->getVoidType(), kIROp_GlobalHashedStringLiterals, stringLitCount, stringLits.getBuffer());
        }
    }

    if(auto nvapiSlotModifier = translationUnit->getModuleDecl()->findModifier<NVAPISlotModifier>())
    {
        builder->addNVAPISlotDecoration(
            module->getModuleInst(),
            nvapiSlotModifier->registerName.getUnownedSlice(),
            nvapiSlotModifier->spaceName.getUnownedSlice());
    }

#if 0
    {
        DiagnosticSinkWriter writer(compileRequest->getSink());
        dumpIR(module, &writer, "GENERATED");
    }
#endif

    validateIRModuleIfEnabled(compileRequest, module);
    
    // Process higher-order-function calls before any optimization passes
    // to allow the optimizations to affect the generated funcitons.
    // 1. Process JVP derivative functions.
    processJVPDerivativeMarkers(module, compileRequest->getSink());
    // 2. Process VJP derivative functions.
    // processVJPDerivativeMarkers(module); // Disabled currently. No impl yet.
    // 3. Replace JVP & VJP calls.
    processDerivativeCalls(module);


    // We will perform certain "mandatory" optimization passes now.
    // These passes serve two purposes:
    //
    // 1. To simplify the code that we use in backend compilation,
    // or when serializing/deserializing modules, so that we can
    // amortize this effort when we compile multiple entry points
    // that use the same module(s).
    //
    // 2. To ensure certain semantic properties that can't be
    // validated without dataflow information. For example, we want
    // to detect when a variable might be used before it is initialized.

    // Note: if you need to debug the IR that is created before
    // any mandatory optimizations have been applied, then
    // uncomment this line while debugging.

    //      dumpIR(module);

    // First, lower error handling logic into normal control flow.
    // This includes lowering throwing functions into functions that
    // returns a `Result<T,E>` value, translating `tryCall` into
    // normal `call` + `ifElse`, etc.
    lowerErrorHandling(module, compileRequest->getSink());

    // Next, inline calls to any functions that have been
    // marked for mandatory "early" inlining.
    //
    performMandatoryEarlyInlining(module);

    // Next, attempt to promote local variables to SSA
    // temporaries whenever possible.
    constructSSA(module);

    // Do basic constant folding and dead code elimination
    // using Sparse Conditional Constant Propagation (SCCP)
    //
    applySparseConditionalConstantPropagation(module);

    // Propagate `constexpr`-ness through the dataflow graph (and the
    // call graph) based on constraints imposed by different instructions.
    propagateConstExpr(module, compileRequest->getSink());

    // TODO: give error messages if any `undefined` or
    // `unreachable` instructions remain.

    checkForMissingReturns(module, compileRequest->getSink());

    // The "mandatory" optimization passes may make use of the
    // `IRHighLevelDeclDecoration` type to relate IR instructions
    // back to AST-level code in order to improve the quality
    // of diagnostics that are emitted.
    //
    // While it is important for these passes to have access
    // to AST-level information, allowing that information to
    // flow into later steps (e.g., code generation) could lead
    // to unclean layering of the parts of the compiler.
    // In principle, back-end steps should not need to know where
    // IR code came from.
    //
    // In order to avoid problems, we run a pass here to strip
    // out any decorations that should not be relied upon by
    // later passes.
    //
    {
        // Because we are already stripping out undesired decorations,
        // this is also a convenient place to remove any `IRNameHintDecoration`s
        // in the case where we are obfuscating code. We handle this
        // by setting up the options for the stripping pass appropriately.
        //
        IRStripOptions stripOptions;

        Linkage* linkage = compileRequest->getLinkage();

        stripOptions.shouldStripNameHints = linkage->m_obfuscateCode;
        stripOptions.stripSourceLocs = linkage->m_obfuscateCode;

        stripFrontEndOnlyInstructions(module, stripOptions);

        // Stripping out decorations could leave some dead code behind
        // in the module, and in some cases that extra code is also
        // undesirable (e.g., the string literals referenced by name-hint
        // decorations are just as undesirable as the decorations themselves).
        // To clean up after ourselves we also run a dead-code elimination
        // pass here, but make sure to set our options so that we don't
        // eliminate anything that has been marked for export.
        //
        IRDeadCodeEliminationOptions options;
        options.keepExportsAlive = true;
        eliminateDeadCode(module, options);
    }

    // TODO: consider doing some more aggressive optimizations
    // (in particular specialization of generics) here, so
    // that we can avoid doing them downstream.
    //
    // Note: doing specialization or inlining involving code
    // from other modules potentially makes the IR we generate
    // "fragile" in that we'd now need to recompile when
    // a module we depend on changes.

    validateIRModuleIfEnabled(compileRequest, module);

    // If we are being asked to dump IR during compilation,
    // then we can dump the initial IR for the module here.
    if(compileRequest->shouldDumpIR)
    {
        DiagnosticSinkWriter writer(compileRequest->getSink());

        dumpIR(module, compileRequest->m_irDumpOptions, "LOWER-TO-IR", compileRequest->getSourceManager(), &writer);
    }

    return module;
}

    /// Context for generating IR code to represent a `SpecializedComponentType`
struct SpecializedComponentTypeIRGenContext : ComponentTypeVisitor
{
    DiagnosticSink* sink;
    Linkage* linkage;
    Session* session;
    IRGenContext* context;
    IRBuilder* builder;

    RefPtr<IRModule> process(
        SpecializedComponentType*   componentType,
        DiagnosticSink*             inSink)
    {
        sink = inSink;

        linkage = componentType->getLinkage();
        session = linkage->getSessionImpl();

        SharedIRGenContext sharedContextStorage(
            session,
            sink,
            linkage->m_obfuscateCode
        );
        SharedIRGenContext* sharedContext = &sharedContextStorage;

        IRGenContext contextStorage(sharedContext, linkage->getASTBuilder());
        context = &contextStorage;

        RefPtr<IRModule> module = IRModule::create(session);

        SharedIRBuilder sharedBuilderStorage(module);
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;

        IRBuilder builderStorage(sharedBuilder);
        builder = &builderStorage;

        builder->setInsertInto(module);

        context->irBuilder = builder;

        componentType->acceptVisitor(this, nullptr);

        return module;
    }

    void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // We need to emit symbols for all of the entry
        // points in the program; this is especially
        // important in the case where a generic entry
        // point is being specialized.
        //
        lowerProgramEntryPointToIR(context, entryPoint, specializationInfo);
    }

    void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        entryPoint->getBase()->acceptVisitor(this, specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // We've hit a leaf module, so we should be able to bind any global
        // generic type parameters here...
        //
        if( specializationInfo )
        {
            for( auto genericArgInfo : specializationInfo->genericArgs )
            {
                IRInst* irParam = getSimpleVal(context, ensureDecl(context, genericArgInfo.paramDecl));
                IRInst* irVal = lowerSimpleVal(context, genericArgInfo.argVal);

                // bind `irParam` to `irVal`
                builder->emitBindGlobalGenericParam(irParam, irVal);
            }

            auto shaderParamCount = module->getShaderParamCount();
            Index existentialArgOffset = 0;

            for( Index ii = 0; ii < shaderParamCount; ++ii )
            {
                auto shaderParam = module->getShaderParam(ii);
                auto specializationArgCount = shaderParam.specializationParamCount;

                IRInst* irParam = getSimpleVal(context, ensureDecl(context, shaderParam.paramDeclRef));
                List<IRInst*> irSlotArgs;
                // Tracks if there are any type args that is not an IRDynamicType.
                bool hasConcreteTypeArg = false;
                for( Index jj = 0; jj < specializationArgCount; ++jj )
                {
                    auto& specializationArg = specializationInfo->existentialArgs[existentialArgOffset++];

                    auto irType = lowerSimpleVal(context, specializationArg.val);
                    auto irWitness = lowerSimpleVal(context, specializationArg.witness);

                    if (irType->getOp() != kIROp_DynamicType)
                        hasConcreteTypeArg = true;

                    irSlotArgs.add(irType);
                    irSlotArgs.add(irWitness);
                }
                // Only insert a `BindExistentialSlots` decoration when there are at least
                // one non-dynamic type argument.
                if (hasConcreteTypeArg)
                {
                    builder->addBindExistentialSlotsDecoration(
                        irParam,
                        irSlotArgs.getCount(),
                        irSlotArgs.getBuffer());
                }
            }
        }
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);
    }

    void visitTypeConformance(TypeConformance* conformance) SLANG_OVERRIDE
    {
        SLANG_UNUSED(conformance);
    }
};

RefPtr<IRModule> generateIRForSpecializedComponentType(
    SpecializedComponentType*   componentType,
    DiagnosticSink*             sink)
{
    SpecializedComponentTypeIRGenContext context;
    return context.process(componentType, sink);
}

    /// Context for generating IR code to represent a `TypeConformance`
struct TypeConformanceIRGenContext
{
    DiagnosticSink* sink;
    Linkage* linkage;
    Session* session;
    IRGenContext* context;
    IRBuilder* builder;

    RefPtr<IRModule> process(
        TypeConformance* typeConformance,
        Int conformanceIdOverride,
        DiagnosticSink* inSink)
    {
        sink = inSink;

        linkage = typeConformance->getLinkage();
        session = linkage->getSessionImpl();

        SharedIRGenContext sharedContextStorage(session, sink, linkage->m_obfuscateCode);
        SharedIRGenContext* sharedContext = &sharedContextStorage;

        IRGenContext contextStorage(sharedContext, linkage->getASTBuilder());
        context = &contextStorage;

        RefPtr<IRModule> module = IRModule::create(session);

        SharedIRBuilder sharedBuilderStorage(module);
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;

        IRBuilder builderStorage(sharedBuilder);
        builder = &builderStorage;

        builder->setInsertInto(module);

        context->irBuilder = builder;

        auto witness = lowerSimpleVal(context, typeConformance->getSubtypeWitness());
        builder->addPublicDecoration(witness);
        if (conformanceIdOverride != -1)
        {
            builder->addSequentialIDDecoration(witness, conformanceIdOverride);
        }
        return module;
    }
};

RefPtr<IRModule> generateIRForTypeConformance(
    TypeConformance* typeConformance,
    Int conformanceIdOverride,
    DiagnosticSink* sink)
{
    TypeConformanceIRGenContext context;
    return context.process(typeConformance, conformanceIdOverride, sink);
}

RefPtr<IRModule> TargetProgram::getOrCreateIRModuleForLayout(DiagnosticSink* sink)
{
    getOrCreateLayout(sink);
    return m_irModuleForLayout;
}

    /// Specialized IR generation context for when generating IR for layouts.
struct IRLayoutGenContext : IRGenContext
{
    IRLayoutGenContext(SharedIRGenContext* shared, ASTBuilder* astBuilder)
        : IRGenContext(shared, astBuilder)
    {}

        /// Cache for custom key instructions used for entry-point parameter layout information.
    Dictionary<ParamDecl*, IRInst*> mapEntryPointParamToKey;
};

    /// Lower an AST-level type layout to an IR-level type layout.
IRTypeLayout* lowerTypeLayout(
    IRLayoutGenContext* context,
    TypeLayout*         typeLayout);

    /// Lower an AST-level variable layout to an IR-level variable layout.
IRVarLayout* lowerVarLayout(
    IRLayoutGenContext* context,
    VarLayout*          varLayout);

    /// Shared code for most `lowerTypeLayout` cases.
    ///
    /// Handles copying of resource usage and pending data type layout
    /// from the AST `typeLayout` to the specified `builder`.
    ///
static IRTypeLayout* _lowerTypeLayoutCommon(
    IRLayoutGenContext*     context,
    IRTypeLayout::Builder*  builder,
    TypeLayout*             typeLayout)
{
    for( auto resInfo : typeLayout->resourceInfos )
    {
        builder->addResourceUsage(resInfo.kind, resInfo.count);
    }

    if( auto pendingTypeLayout = typeLayout->pendingDataTypeLayout )
    {
        builder->setPendingTypeLayout(
            lowerTypeLayout(context, pendingTypeLayout));
    }

    return builder->build();
}

IRTypeLayout* lowerTypeLayout(
    IRLayoutGenContext* context,
    TypeLayout*         typeLayout)
{
    // TODO: We chould consider caching the layouts we create based on `typeLayout`
    // and re-using them. This isn't strictly necessary because we emit the
    // instructions as "hoistable" which should give us de-duplication, and it wouldn't
    // help much until/unless the AST level gets less wasteful about how it computes layout.

    // We will use casting to detect if `typeLayout` is
    // one of the cases that requires a dedicated sub-type
    // of IR type layout.
    //
    if( auto paramGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout) )
    {
        IRParameterGroupTypeLayout::Builder builder(context->irBuilder);

        builder.setContainerVarLayout(
            lowerVarLayout(context, paramGroupTypeLayout->containerVarLayout));
        builder.setElementVarLayout(
            lowerVarLayout(context, paramGroupTypeLayout->elementVarLayout));
        builder.setOffsetElementTypeLayout(
            lowerTypeLayout(context, paramGroupTypeLayout->offsetElementTypeLayout));

        return _lowerTypeLayoutCommon(context, &builder, paramGroupTypeLayout);
    }
    else if( auto structTypeLayout = as<StructTypeLayout>(typeLayout) )
    {
        IRStructTypeLayout::Builder builder(context->irBuilder);

        for( auto fieldLayout : structTypeLayout->fields )
        {
            auto fieldDecl = fieldLayout->varDecl;

            IRInst* irFieldKey = nullptr;
            if(auto paramDecl = as<ParamDecl>(fieldDecl) )
            {
                // There is a subtle special case here.
                //
                // A `StructTypeLayout` might be used to represent
                // the parameters of an entry point, and this is the
                // one and only case where the "fields" being used
                // might actually be `ParamDecl`s.
                //
                // The IR encoding of structure type layouts relies
                // on using field "key" instructions to identify
                // the fields, but these don't exist (by default)
                // for function parameters.
                //
                // To get around this problem we will create key
                // instructions to stand in for the entry-point parameters
                // as needed when generating layout.
                //
                // We need to cache the generated keys on the context,
                // so that if we run into another type layout for the
                // same entry point we will re-use the same keys.
                //
                if( !context->mapEntryPointParamToKey.TryGetValue(paramDecl, irFieldKey) )
                {
                    irFieldKey = context->irBuilder->createStructKey();

                    // TODO: It might eventually be a good idea to attach a mangled
                    // name to the key we just generated (derived from the entry point
                    // and parameter name), even though parameters don't usually have
                    // linkage.
                    //
                    // Doing so would ensure that if we ever combined partial layout
                    // information from different modules they would agree on the key
                    // to use for entry-point parameters.
                    //
                    // For now this is a non-issue because both the creation and use
                    // of these keys will be local to a single `IREntryPointLayout`,
                    // and we don't support combination at a finer granularity than that.

                    context->mapEntryPointParamToKey.Add(paramDecl, irFieldKey);
                }
            }
            else
            {
                irFieldKey = getSimpleVal(context,
                    ensureDecl(context, fieldDecl));
            }
            SLANG_ASSERT(irFieldKey);

            auto irFieldLayout = lowerVarLayout(context, fieldLayout);
            builder.addField(irFieldKey, irFieldLayout);
        }

        return _lowerTypeLayoutCommon(context, &builder, structTypeLayout);
    }
    else if( auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout) )
    {
        auto irElementTypeLayout = lowerTypeLayout(context, arrayTypeLayout->elementTypeLayout);
        IRArrayTypeLayout::Builder builder(context->irBuilder, irElementTypeLayout);
        return _lowerTypeLayoutCommon(context, &builder, arrayTypeLayout);
    }
    else if( auto taggedUnionTypeLayout = as<TaggedUnionTypeLayout>(typeLayout) )
    {
        IRTaggedUnionTypeLayout::Builder builder(context->irBuilder, taggedUnionTypeLayout->tagOffset);

        for( auto caseTypeLayout : taggedUnionTypeLayout->caseTypeLayouts )
        {
            builder.addCaseTypeLayout(
                lowerTypeLayout(
                    context,
                    caseTypeLayout));
        }

        return _lowerTypeLayoutCommon(context, &builder, taggedUnionTypeLayout);
    }
    else if( auto streamOutputTypeLayout = as<StreamOutputTypeLayout>(typeLayout) )
    {
        auto irElementTypeLayout = lowerTypeLayout(context, streamOutputTypeLayout->elementTypeLayout);

        IRStreamOutputTypeLayout::Builder builder(context->irBuilder, irElementTypeLayout);
        return _lowerTypeLayoutCommon(context, &builder, streamOutputTypeLayout);
    }
    else if( auto matrixTypeLayout = as<MatrixTypeLayout>(typeLayout) )
    {
        // TODO: Our support for explicit layouts on matrix types is minimal, so whether
        // or not we even include `IRMatrixTypeLayout` doesn't impact any behavior we
        // currently test.
        //
        // Our handling of matrix types and their layout needs a complete overhaul, but
        // that isn't something we can get to right away, so we'll just try to pass
        // along this data as best we can for now.

        IRMatrixTypeLayout::Builder builder(context->irBuilder, matrixTypeLayout->mode);
        return _lowerTypeLayoutCommon(context, &builder, matrixTypeLayout);
    }
    else if( auto existentialTypeLayout = as<ExistentialTypeLayout>(typeLayout) )
    {
        IRExistentialTypeLayout::Builder builder(context->irBuilder);
        return _lowerTypeLayoutCommon(context, &builder, existentialTypeLayout);
    }
    else
    {
        // If no special case applies we will build a generic `IRTypeLayout`.
        //
        IRTypeLayout::Builder builder(context->irBuilder);
        return _lowerTypeLayoutCommon(context, &builder, typeLayout);
    }
}

IRVarLayout* lowerVarLayout(
    IRLayoutGenContext* context,
    VarLayout*          varLayout,
    IRTypeLayout*       irTypeLayout)
{
    IRVarLayout::Builder irLayoutBuilder(context->irBuilder, irTypeLayout);

    for( auto resInfo : varLayout->resourceInfos )
    {
        auto irResInfo = irLayoutBuilder.findOrAddResourceInfo(resInfo.kind);
        irResInfo->offset = resInfo.index;
        irResInfo->space = resInfo.space;
    }

    if( auto pendingVarLayout = varLayout->pendingVarLayout )
    {
        irLayoutBuilder.setPendingVarLayout(
            lowerVarLayout(context, pendingVarLayout));
    }

    // We will only generate layout information with *either* a system-value
    // semantic or a user-defined semantic, and we will always check for
    // the system-value semantic first because the AST-level representation
    // seems to encode both when a system-value semantic is present.
    //
    if( varLayout->systemValueSemantic.getLength() )
    {
        irLayoutBuilder.setSystemValueSemantic(
            varLayout->systemValueSemantic,
            varLayout->systemValueSemanticIndex);
    }
    else if( varLayout->semanticName.getLength() )
    {
        irLayoutBuilder.setUserSemantic(
            varLayout->semanticName,
            varLayout->semanticIndex);
    }

    if( varLayout->stage != Stage::Unknown )
    {
        irLayoutBuilder.setStage(varLayout->stage);
    }

    return irLayoutBuilder.build();
}

IRVarLayout* lowerVarLayout(
    IRLayoutGenContext* context,
    VarLayout*          varLayout)
{
    auto irTypeLayout = lowerTypeLayout(context, varLayout->typeLayout);
    return lowerVarLayout(context, varLayout, irTypeLayout);
}

    /// Handle the lowering of an entry-point result layout to the IR
IRVarLayout* lowerEntryPointResultLayout(
    IRLayoutGenContext* context,
    VarLayout*          layout)
{
    // The easy case is when there is a non-null `layout`, because we
    // can handle it like any other var layout.
    //
    if(layout)
        return lowerVarLayout(context, layout);

    // Right now the AST-level layout logic will leave a null layout
    // for the result when an entry point has a `void` result type.
    //
    // TODO: We should fix this at the AST level instead of the IR,
    // but doing so would impact reflection, where clients could
    // be using a null check to test for a `void` result.
    //
    // As a workaround, we will create an empty type layout and
    // an empty var layout that represents it, consistent with the
    // way that a `void` value consumes no resources.
    //
    IRTypeLayout::Builder typeLayoutBuilder(context->irBuilder);
    auto irTypeLayout = typeLayoutBuilder.build();
    IRVarLayout::Builder varLayoutBuilder(context->irBuilder, irTypeLayout);
    return varLayoutBuilder.build();
}

    /// Lower AST-level layout information for an entry point to the IR
IREntryPointLayout* lowerEntryPointLayout(
    IRLayoutGenContext* context,
    EntryPointLayout*   entryPointLayout)
{
    auto irParamsLayout = lowerVarLayout(context, entryPointLayout->parametersLayout);
    auto irResultLayout = lowerEntryPointResultLayout(context, entryPointLayout->resultLayout);

    return context->irBuilder->getEntryPointLayout(
        irParamsLayout,
        irResultLayout);
}

RefPtr<IRModule> TargetProgram::createIRModuleForLayout(DiagnosticSink* sink)
{
    if(m_irModuleForLayout)
        return m_irModuleForLayout;


    // Okay, now we need to fill it in.

    auto programLayout = getOrCreateLayout(sink);
    if(!programLayout)
        return nullptr;

    auto program = getProgram();
    auto linkage = program->getLinkage();
    auto session = linkage->getSessionImpl();

    SharedIRGenContext sharedContextStorage(
        session,
        sink,
        linkage->m_obfuscateCode);
    auto sharedContext = &sharedContextStorage;

    ASTBuilder* astBuilder = linkage->getASTBuilder();

    IRLayoutGenContext contextStorage(sharedContext, astBuilder);
    auto context = &contextStorage;

    RefPtr<IRModule> irModule = IRModule::create(session);

    SharedIRBuilder sharedBuilderStorage(irModule);
    auto sharedBuilder = &sharedBuilderStorage;

    IRBuilder builderStorage(sharedBuilder);
    auto builder = &builderStorage;

    builder->setInsertInto(irModule);

    context->irBuilder = builder;


    // Okay, now we need to walk through and decorate everything.
    auto globalStructLayout = getScopeStructLayout(programLayout);

    IRStructTypeLayout::Builder globalStructTypeLayoutBuilder(builder);

    for(auto varLayout : globalStructLayout->fields)
    {
        auto varDecl = varLayout->varDecl;

        // Ensure that an `[import(...)]` declaration for the variable
        // has been emitted to this module, so that we will have something
        // to decorate.
        //
        auto irVar = getSimpleVal(context, ensureDecl(context, varDecl));

        auto irLayout = lowerVarLayout(context, varLayout);

        // Now attach the decoration to the variable.
        //
        builder->addLayoutDecoration(irVar, irLayout);

        // Also add this to our mapping for the global-scope structure type
        globalStructTypeLayoutBuilder.addField(irVar, irLayout);
    }
    auto irGlobalStructTypeLayout = _lowerTypeLayoutCommon(context, &globalStructTypeLayoutBuilder, globalStructLayout);

    auto globalScopeVarLayout = programLayout->parametersLayout;
    auto globalScopeTypeLayout = globalScopeVarLayout->typeLayout;
    IRTypeLayout* irGlobalScopeTypeLayout = irGlobalStructTypeLayout;
    if( auto paramGroupTypeLayout = as<ParameterGroupTypeLayout>(globalScopeTypeLayout) )
    {
        IRParameterGroupTypeLayout::Builder globalParameterGroupTypeLayoutBuilder(builder);

        auto irElementTypeLayout = irGlobalStructTypeLayout;
        auto irElementVarLayout = lowerVarLayout(context, paramGroupTypeLayout->elementVarLayout, irElementTypeLayout);

        globalParameterGroupTypeLayoutBuilder.setContainerVarLayout(
            lowerVarLayout(context, paramGroupTypeLayout->containerVarLayout));
        globalParameterGroupTypeLayoutBuilder.setElementVarLayout(irElementVarLayout);
        globalParameterGroupTypeLayoutBuilder.setOffsetElementTypeLayout(
            lowerTypeLayout(context, paramGroupTypeLayout->offsetElementTypeLayout));

        auto irParamGroupTypeLayout = _lowerTypeLayoutCommon(context, &globalParameterGroupTypeLayoutBuilder, paramGroupTypeLayout);

        irGlobalScopeTypeLayout = irParamGroupTypeLayout;
    }

    auto irGlobalScopeVarLayout = lowerVarLayout(context, globalScopeVarLayout, irGlobalScopeTypeLayout);

    builder->addLayoutDecoration(irModule->getModuleInst(), irGlobalScopeVarLayout);

    for( auto entryPointLayout : programLayout->entryPoints )
    {
        auto funcDeclRef = entryPointLayout->entryPoint;

        // HACK: skip over entry points that came from deserialization,
        // and thus don't have AST-level information for us to work with.
        //
        if(!funcDeclRef)
            continue;

        auto irFuncType = lowerType(context, getFuncType(astBuilder, funcDeclRef));
        auto irFunc = getSimpleVal(context, emitDeclRef(context, funcDeclRef, irFuncType));

        if( !irFunc->findDecoration<IRLinkageDecoration>() )
        {
            builder->addImportDecoration(irFunc, getMangledName(astBuilder, funcDeclRef).getUnownedSlice());
        }

        auto irEntryPointLayout = lowerEntryPointLayout(context, entryPointLayout);

        builder->addLayoutDecoration(irFunc, irEntryPointLayout);
    }

    for( auto taggedUnionTypeLayout : programLayout->taggedUnionTypeLayouts )
    {
        auto taggedUnionType = taggedUnionTypeLayout->getType();
        auto irType = lowerType(context, taggedUnionType);

        auto irTypeLayout = lowerTypeLayout(context, taggedUnionTypeLayout);

        builder->addLayoutDecoration(irType, irTypeLayout);
    }

    // Lets strip and run DCE here
    if (linkage->m_obfuscateCode)
    {
        IRStripOptions stripOptions;

        stripOptions.shouldStripNameHints = linkage->m_obfuscateCode;
        stripOptions.stripSourceLocs = linkage->m_obfuscateCode;

        stripFrontEndOnlyInstructions(irModule, stripOptions);

        IRDeadCodeEliminationOptions options;
        options.keepExportsAlive = true;
        options.keepLayoutsAlive = true;

        // Eliminate any dead code
        eliminateDeadCode(irModule, options);
    }

    m_irModuleForLayout = irModule;
    return irModule;
}



} // namespace Slang
