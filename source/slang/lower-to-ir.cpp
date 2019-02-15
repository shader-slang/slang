// lower.cpp
#include "lower-to-ir.h"

#include "../../slang.h"

#include "check.h"
#include "ir.h"
#include "ir-constexpr.h"
#include "ir-insts.h"
#include "ir-missing-return.h"
#include "ir-sccp.h"
#include "ir-ssa.h"
#include "ir-validate.h"
#include "mangle.h"
#include "type-layout.h"
#include "visitor.h"

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

// This case is used to indicate a reference to an AST-level
// subscript operation bound to particular arguments.
//
// For example in a case like this:
//
//     RWStructuredBuffer<Foo> gBuffer;
//     ... gBuffer[someIndex] ...
//
// the expression `gBuffer[someIndex]` will be lowered to
// a value that references `RWStructureBuffer<Foo>::operator[]`
// with arguments `(gBuffer, someIndex)`.
//
// Such a value can be an l-value, and depending on the context
// where it is used, can lower into a call to either the getter
// or setter operations of the subscript.
//
struct BoundSubscriptInfo : ExtendedValueInfo
{
    DeclRef<SubscriptDecl>  declRef;
    IRType*                 type;
    List<IRInst*>           args;
};

// Some cases of `ExtendedValueInfo` need to
// recursively contain `LoweredValInfo`s, and
// so we forward declare them here and fill
// them in later.
//
struct BoundMemberInfo;
struct SwizzledLValueInfo;


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
        BoundSubscript,

        // The result of applying swizzling to an l-value
        SwizzledLValue,
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

    static LoweredValInfo boundSubscript(
        BoundSubscriptInfo* boundSubscriptInfo);

    BoundSubscriptInfo* getBoundSubscriptInfo()
    {
        SLANG_ASSERT(flavor == Flavor::BoundSubscript);
        return (BoundSubscriptInfo*)ext;
    }

    static LoweredValInfo swizzledLValue(
        SwizzledLValueInfo* extInfo);

    SwizzledLValueInfo* getSwizzledLValueInfo()
    {
        SLANG_ASSERT(flavor == Flavor::SwizzledLValue);
        return (SwizzledLValueInfo*)ext;
    }
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

LoweredValInfo LoweredValInfo::boundSubscript(
    BoundSubscriptInfo* boundSubscriptInfo)
{
    LoweredValInfo info;
    info.flavor = Flavor::BoundSubscript;
    info.ext = boundSubscriptInfo;
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
        ModuleDecl*     mainModuleDecl = nullptr)
        : m_session(session)
        , m_sink(sink)
        , m_mainModuleDecl(mainModuleDecl)
    {}

    Session*        m_session = nullptr;
    DiagnosticSink* m_sink = nullptr;
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
};


struct IRGenContext
{
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

    explicit IRGenContext(SharedIRGenContext* inShared)
        : shared(inShared)
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
    for (auto dd = decl; dd; dd = dd->ParentDecl)
    {
        if (auto moduleDecl = as<ModuleDecl>(dd))
            return moduleDecl;
    }
    return nullptr;
}

bool isFromStdLib(Decl* decl)
{
    for (auto dd = decl; dd; dd = dd->ParentDecl)
    {
        if (dd->HasModifier<FromStdLibModifier>())
            return true;
    }
    return false;
}

bool isImportedDecl(IRGenContext* context, Decl* decl)
{
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

    /// Should the given `decl` nested in `parentDecl` be treated as a static rather than instance declaration?
bool isEffectivelyStatic(
    Decl*           decl,
    ContainerDecl*  parentDecl);

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

IROp getIntrinsicOp(
    Decl*                   decl,
    IntrinsicOpModifier*    intrinsicOpMod)
{
    if (int(intrinsicOpMod->op) != 0)
        return intrinsicOpMod->op;

    // No specified modifier? Then we need to look it up
    // based on the name of the declaration...

    auto name = decl->getName();
    auto nameText = getUnownedStringSliceText(name);

    IROp op = findIROp(nameText);
    SLANG_ASSERT(op != kIROp_Invalid);
    return op;
}

// Given a `LoweredValInfo` for something callable, along with a
// bunch of arguments, emit an appropriate call to it.
LoweredValInfo emitCallToVal(
    IRGenContext*   context,
    IRType*         type,
    LoweredValInfo  funcVal,
    UInt            argCount,
    IRInst* const* args)
{
    auto builder = context->irBuilder;
    switch (funcVal.flavor)
    {
    case LoweredValInfo::Flavor::None:
        SLANG_UNEXPECTED("null function");
    default:
        return LoweredValInfo::simple(
            builder->emitCallInst(type, getSimpleVal(context, funcVal), argCount, args));
    }
}

LoweredValInfo emitCompoundAssignOp(
    IRGenContext*   context,
    IRType*         type,
    IROp            op,
    UInt            argCount,
    IRInst* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    SLANG_ASSERT(argCount == 2);
    auto leftPtr = args[0];
    auto rightVal = args[1];

    auto leftVal = builder->emitLoad(leftPtr);

    IRInst* innerArgs[] = { leftVal, rightVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(leftPtr, innerOp);

    return LoweredValInfo::ptr(leftPtr);
}

IRInst* getOneValOfType(
    IRGenContext*   context,
    IRType*         type)
{
    switch(type->op)
    {
    case kIROp_IntType:
    case kIROp_UIntType:
    case kIROp_UInt64Type:
        return context->irBuilder->getIntValue(type, 1);

    case kIROp_HalfType:
    case kIROp_FloatType:
    case kIROp_DoubleType:
        return context->irBuilder->getFloatValue(type, 1.0);

    default:
        break;
    }

    // TODO: should make sure to handle vector and matrix types here

    SLANG_UNEXPECTED("inc/dec type");
    UNREACHABLE_RETURN(nullptr);
}

LoweredValInfo emitPrefixIncDecOp(
    IRGenContext*   context,
    IRType*         type,
    IROp            op,
    UInt            argCount,
    IRInst* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    SLANG_ASSERT(argCount == 1);
    auto argPtr = args[0];

    auto preVal = builder->emitLoad(argPtr);

    IRInst* oneVal = getOneValOfType(context, type);

    IRInst* innerArgs[] = { preVal, oneVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(argPtr, innerOp);

    // For a prefix operator like `++i` we return
    // the value after the increment/decrement has
    // been applied. In casual terms we "increment
    // the varaible, then return its value."
    //
    return LoweredValInfo::simple(innerOp);
}

LoweredValInfo emitPostfixIncDecOp(
    IRGenContext*   context,
    IRType*         type,
    IROp            op,
    UInt            argCount,
    IRInst* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    SLANG_ASSERT(argCount == 1);
    auto argPtr = args[0];

    auto preVal = builder->emitLoad(argPtr);

    IRInst* oneVal = getOneValOfType(context, type);

    IRInst* innerArgs[] = { preVal, oneVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(argPtr, innerOp);

    // For a postfix operator like `i++` we return
    // the value that we read before the increment/decrement
    // gets applied. In casual terms we "read
    // the variable, then increment it."
    //
    return LoweredValInfo::simple(preVal);
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
    IRInst* const* args)
{
    auto builder = context->irBuilder;


    if (auto subscriptDeclRef = funcDeclRef.as<SubscriptDecl>())
    {
        // A reference to a subscript declaration is a special case,
        // because it is not possible to call a subscript directly;
        // we must call one of its accessors.
        //
        // TODO: everything here will also apply to propery declarations
        // once we have them, so some of this code might be shared
        // some day.

        DeclRef<GetterDecl> getterDeclRef;
        bool justAGetter = true;
        for (auto accessorDeclRef : getMembersOfType<AccessorDecl>(subscriptDeclRef))
        {
            // We want to track whether this subscript has any accessors other than
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
            // represent the latent subscript operation (abstractly
            // this is a reference to a storage location).

            // The abstract storage location will need to include
            // all the arguments being passed to the subscript operation.

            RefPtr<BoundSubscriptInfo> boundSubscript = new BoundSubscriptInfo();
            boundSubscript->declRef = subscriptDeclRef;
            boundSubscript->type = type;
            boundSubscript->args.AddRange(args, argCount);

            context->shared->extValues.Add(boundSubscript);

            return LoweredValInfo::boundSubscript(boundSubscript);
        }

        // Otherwise we are just call the getter, and so that
        // is what we need to be emitting a call to...
        funcDeclRef = getterDeclRef;
    }

    auto funcDecl = funcDeclRef.getDecl();
    if(auto intrinsicOpModifier = funcDecl->FindModifier<IntrinsicOpModifier>())
    {
        auto op = getIntrinsicOp(funcDecl, intrinsicOpModifier);

        if (isPseudoOp(op))
        {
            switch (op)
            {
            case kIRPseudoOp_Pos:
                return LoweredValInfo::simple(args[0]);

            case kIRPseudoOp_Sequence:
                // The main effect of "operator comma" is to enforce
                // sequencing of its operands, but Slang already
                // implements a strictly left-to-right evaluation
                // order for function arguments, so in practice we
                // just need to compile `a, b` to the value of `b`
                // (because argument evaluation already happened).
                return LoweredValInfo::simple(args[1]);

#define CASE(COMPOUND, OP)  \
            case COMPOUND: return emitCompoundAssignOp(context, type, OP, argCount, args)

            CASE(kIRPseudoOp_AddAssign, kIROp_Add);
            CASE(kIRPseudoOp_SubAssign, kIROp_Sub);
            CASE(kIRPseudoOp_MulAssign, kIROp_Mul);
            CASE(kIRPseudoOp_DivAssign, kIROp_Div);
            CASE(kIRPseudoOp_ModAssign, kIROp_Mod);
            CASE(kIRPseudoOp_AndAssign, kIROp_BitAnd);
            CASE(kIRPseudoOp_OrAssign, kIROp_BitOr);
            CASE(kIRPseudoOp_XorAssign, kIROp_BitXor);
            CASE(kIRPseudoOp_LshAssign, kIROp_Lsh);
            CASE(kIRPseudoOp_RshAssign, kIROp_Rsh);

#undef CASE

#define CASE(COMPOUND, OP)  \
            case COMPOUND: return emitPrefixIncDecOp(context, type, OP, argCount, args)
            CASE(kIRPseudoOp_PreInc, kIROp_Add);
            CASE(kIRPseudoOp_PreDec, kIROp_Sub);
#undef CASE

#define CASE(COMPOUND, OP)  \
            case COMPOUND: return emitPostfixIncDecOp(context, type, OP, argCount, args)
            CASE(kIRPseudoOp_PostInc, kIROp_Add);
            CASE(kIRPseudoOp_PostDec, kIROp_Sub);
#undef CASE
            default:
                SLANG_UNIMPLEMENTED_X("IR pseudo-op");
                UNREACHABLE_RETURN(LoweredValInfo());
            }
        }

        return LoweredValInfo::simple(builder->emitIntrinsicInst(
            type,
            op,
            argCount,
            args));
    }
    // TODO: handle target intrinsic modifier too...

    if( auto ctorDeclRef = funcDeclRef.as<ConstructorDecl>() )
    {
        // HACK: we know all constructors are builtins for now,
        // so we need to emit them as a call to the corresponding
        // builtin operation.
        //
        // TODO: these should all either be intrinsic operations,
        // or calls to library functions.

        return LoweredValInfo::simple(builder->emitConstructorInst(type, argCount, args));
    }

    // Fallback case is to emit an actual call.
    if(!funcType)
    {
        List<IRType*> argTypes;
        for(UInt ii = 0; ii < argCount; ++ii)
        {
            argTypes.Add(args[ii]->getDataType());
        }
        funcType = builder->getFuncType(argCount, argTypes.Buffer(), type);
    }
    LoweredValInfo funcVal = emitDeclRef(context, funcDeclRef, funcType);
    return emitCallToVal(context, type, funcVal, argCount, args);
}

LoweredValInfo emitCallToDeclRef(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<Decl>           funcDeclRef,
    IRType*                 funcType,
    List<IRInst*> const&    args)
{
    return emitCallToDeclRef(context, type, funcDeclRef, funcType, args.Count(), args.Buffer());
}

IRInst* getFieldKey(
    IRGenContext*       context,
    DeclRef<VarDecl>    field)
{
    return getSimpleVal(context, emitDeclRef(context, field, context->irBuilder->getKeyType()));
}

LoweredValInfo extractField(
    IRGenContext*       context,
    IRType*             fieldType,
    LoweredValInfo      base,
    DeclRef<VarDecl>    field)
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
    case LoweredValInfo::Flavor::BoundSubscript:
        {
            // The base value is one that is trying to defer a get-vs-set
            // decision, so we will need to do the same.

            RefPtr<BoundMemberInfo> boundMemberInfo = new BoundMemberInfo();
            boundMemberInfo->type = fieldType;
            boundMemberInfo->base = base;
            boundMemberInfo->declRef = field;

            context->shared->extValues.Add(boundMemberInfo);
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

    case LoweredValInfo::Flavor::BoundSubscript:
        {
            auto boundSubscriptInfo = lowered.getBoundSubscriptInfo();

            // We are being asked to extract a value from a subscript call
            // (e.g., `base[index]`). We will first check if the subscript
            // declared a getter and use that if possible, and then fall
            // back to a `ref` accessor if one is defined.
            //
            // (Picking the `get` over the `ref` accessor simplifies things
            // in case the `get` operation has a natural translation for
            // a target, while the general `ref` case does not...)

            auto getters = getMembersOfType<GetterDecl>(boundSubscriptInfo->declRef);
            if (getters.Count())
            {
                lowered = emitCallToDeclRef(
                    context,
                    boundSubscriptInfo->type,
                    *getters.begin(),
                    nullptr,
                    boundSubscriptInfo->args);
                goto top;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(boundSubscriptInfo->declRef);
            if(refAccessors.Count())
            {
                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(boundSubscriptInfo->type);

                LoweredValInfo refVal = emitCallToDeclRef(
                    context,
                    ptrType,
                    *refAccessors.begin(),
                    nullptr,
                    boundSubscriptInfo->args);

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

    if(isImportedDecl(context, decl))
    {
        builder->addImportDecoration(inst, mangledName);
    }
    else
    {
        builder->addExportDecoration(inst, mangledName);
    }
}

static void addLinkageDecoration(
    IRGenContext*               context,
    IRInst*                     inst,
    Decl*                       decl)
{
    addLinkageDecoration(context, inst, decl, getMangledName(decl).getUnownedSlice());
}

IRStructKey* getInterfaceRequirementKey(
    IRGenContext*   context,
    Decl*           requirementDecl)
{
    IRStructKey* requirementKey = nullptr;
    if(context->shared->interfaceRequirementKeys.TryGetValue(requirementDecl, requirementKey))
    {
        return requirementKey;
    }

    IRBuilder builderStorage = *context->irBuilder;
    auto builder = &builderStorage;

    builder->setInsertInto(builder->sharedBuilder->module->getModuleInst());

    // Construct a key to serve as the representation of
    // this requirement in the IR, and to allow lookup
    // into the declaration.
    requirementKey = builder->createStructKey();

    addLinkageDecoration(context, requirementKey, requirementDecl);

    context->shared->interfaceRequirementKeys.Add(requirementDecl, requirementKey);

    return requirementKey;
}


SubstitutionSet lowerSubstitutions(IRGenContext* context, SubstitutionSet subst);
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
            lowerType(context, GetType(val->declRef)));
    }

    LoweredValInfo visitDeclaredSubtypeWitness(DeclaredSubtypeWitness* val)
    {
        return emitDeclRef(context, val->declRef,
            context->irBuilder->getWitnessTableType());
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
        IRInst* requirementKey = getInterfaceRequirementKey(context, val->midToSup.getDecl());

        // TODO: There are some ugly cases here if `midToSup` is allowed
        // to be an arbitrary witness, rather than just a declared one,
        // and we should probably change the front-end representation
        // to reflect the right constraints.

        return LoweredValInfo::simple(getBuilder()->emitLookupInterfaceMethodInst(
            nullptr,
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
        auto caseCount = val->caseWitnesses.Count();
        List<IRInst*> caseWitnessTables;
        for( auto caseWitness : val->caseWitnesses )
        {
            auto caseWitnessTable = lowerSimpleVal(context, caseWitness);
            caseWitnessTables.Add(caseWitnessTable);
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

        auto irWitnessTable = getBuilder()->createWitnessTable();

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

                IRBuilder subBuilderStorage;
                auto subBuilder = &subBuilderStorage;
                subBuilder->sharedBuilder = getBuilder()->sharedBuilder;
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
                irParamTypes.Add(irThisType);

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
                    auto irParamType = lowerType(context, GetType(paramDeclRef));
                    auto irParam = subBuilder->emitParam(irParamType);

                    irParams.Add(irParam);
                    irParamTypes.Add(irParamType);
                }

                auto irResultType = lowerType(context, GetResultType(callableDeclRef));

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

                for( UInt ii = 0; ii < caseCount; ++ii )
                {
                    auto caseTag = subBuilder->getIntValue(irTagVal->getDataType(), ii);

                    subBuilder->setInsertInto(irFunc);
                    auto caseLabel = subBuilder->emitBlock();

                    if(!defaultLabel)
                        defaultLabel = caseLabel;

                    switchCaseOperands.Add(caseTag);
                    switchCaseOperands.Add(caseLabel);

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
                    caseArgs.Add(caseThisArg);

                    // The remaining arguments to the call will just be forwarded from
                    // the parameters of the wrapper function.
                    //
                    // TODO: This would need to change if/when we started allowing `This` type
                    // or associated-type parameters to be used at call sites where a tagged
                    // union is used.
                    //
                    for( auto param : irParams )
                    {
                        caseArgs.Add(param);
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
                    switchCaseOperands.Count(),
                    switchCaseOperands.Buffer());
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
            paramTypes.Add(lowerType(context, type->getParamType(pp)));
        }
        return getBuilder()->getFuncType(
            paramCount,
            paramTypes.Buffer(),
            resultType);
    }

    IRType* visitDeclRefType(DeclRefType* type)
    {
        auto declRef = type->declRef;
        auto decl = declRef.getDecl();

        // Check for types with teh `__intrinsic_type` modifier.
        if(decl->FindModifier<IntrinsicTypeModifier>())
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
        return (IRType*)getSimpleVal(context, dispatchType(type->GetCanonicalType()));
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
        if (type->ArrayLength)
        {
            auto elementCount = lowerSimpleVal(context, type->ArrayLength);
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

    // Lower a type where the type declaration being referenced is assumed
    // to be an intrinsic type, which can thus be lowered to a simple IR
    // type with the appropriate opcode.
    IRType* lowerSimpleIntrinsicType(DeclRefType* type)
    {
        auto intrinsicTypeModifier = type->declRef.getDecl()->FindModifier<IntrinsicTypeModifier>();
        SLANG_ASSERT(intrinsicTypeModifier);
        IROp op = IROp(intrinsicTypeModifier->irOp);
        return getBuilder()->getType(op);
    }

    // Lower a type where the type declaration being referenced is assumed
    // to be an intrinsic type with a single generic type parameter, and
    // which can thus be lowered to a simple IR type with the appropriate opcode.
    IRType* lowerGenericIntrinsicType(DeclRefType* type, Type* elementType)
    {
        auto intrinsicTypeModifier = type->declRef.getDecl()->FindModifier<IntrinsicTypeModifier>();
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
        auto intrinsicTypeModifier = type->declRef.getDecl()->FindModifier<IntrinsicTypeModifier>();
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
        auto existentialType = lowerType(context, GetType(declRef));
        IRInst* existentialVal = getSimpleVal(context, emitDeclRef(context, declRef, existentialType));
        return getBuilder()->emitExtractExistentialType(existentialVal);
    }

    LoweredValInfo visitExtractExistentialSubtypeWitness(ExtractExistentialSubtypeWitness* witness)
    {
        auto declRef = witness->declRef;
        auto existentialType = lowerType(context, GetType(declRef));
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
            irCaseTypes.Add(irCaseType);
        }

        auto irType = getBuilder()->getTaggedUnionType(irCaseTypes);
        if(!irType->findDecoration<IRLinkageDecoration>())
        {
            // We need a way for later passes to attach layout information
            // to this type, so we will give it a mangled name here.
            //
            getBuilder()->addExportDecoration(
                irType,
                getMangledTypeName(type).getUnownedSlice());
        }
        return LoweredValInfo::simple(irType);
    }

    // We do not expect to encounter the following types in ASTs that have
    // passed front-end semantic checking.
#define UNEXPECTED_CASE(NAME) IRType* visit##NAME(NAME*) { SLANG_UNEXPECTED(#NAME); UNREACHABLE_RETURN(nullptr); }
    UNEXPECTED_CASE(GenericDeclRefType)
    UNEXPECTED_CASE(TypeType)
    UNEXPECTED_CASE(ErrorType)
    UNEXPECTED_CASE(InitializerListType)
    UNEXPECTED_CASE(OverloadGroupType)
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
    for(RefPtr<Modifier> mod : decl->modifiers)
    {
        if(as<HLSLNoInterpolationModifier>(mod))
        {
            builder->addInterpolationModeDecoration(inst, IRInterpolationMode::NoInterpolation);
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
        else if(as<VulkanRayPayloadAttribute>(mod))
        {
            builder->addSimpleDecoration<IRVulkanRayPayloadDecoration>(inst);
        }
        else if(as<VulkanCallablePayloadAttribute>(mod))
        {
            builder->addSimpleDecoration<IRVulkanCallablePayloadDecoration>(inst);
        }
        else if(as<VulkanHitAttributesAttribute>(mod))
        {
            builder->addSimpleDecoration<IRVulkanHitAttributesDecoration>(inst);
        }
        else if(as<GloballyCoherentModifier>(mod))
        {
            builder->addSimpleDecoration<IRGloballyCoherentDecoration>(inst);
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

    if (decl->HasModifier<HLSLGroupSharedModifier>())
    {
        inst->setFullType(builder->getRateQualifiedType(
            builder->getGroupSharedRate(),
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
    if(auto reflectionNameModifier = decl->FindModifier<ParameterGroupReflectionName>())
    {
        leafName = reflectionNameModifier->nameAndLoc.name;
    }

    // There is no point in trying to provide a name hint for something with no name,
    // or with an empty name
    if(!leafName)
        return String();
    if(leafName->text.Length() == 0)
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
    auto parentDecl = decl->ParentDecl;

    // Skip past a generic parent, if we are a declaration nested in a generic.
    if(auto genericParentDecl = as<GenericDecl>(parentDecl))
        parentDecl = genericParentDecl->ParentDecl;

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
        parentDecl = moduleParentDecl->ParentDecl;

    if(!parentDecl)
    {
        return leafName->text;
    }

    auto parentName = getNameForNameHint(context, parentDecl);
    if(parentName.Length() == 0)
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
    if(name.Length() == 0)
        return;
    context->irBuilder->addNameHintDecoration(inst, name.getUnownedSlice());
}

/// Add a name hint based on a fixed string.
static void addNameHint(
    IRGenContext*   context,
    IRInst*         inst,
    char const*     text)
{
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

void addArgs(
    IRGenContext*   context,
    List<IRInst*>* ioArgs,
    LoweredValInfo  argInfo)
{
    auto& args = *ioArgs;
    switch( argInfo.flavor )
    {
    case LoweredValInfo::Flavor::Simple:
    case LoweredValInfo::Flavor::Ptr:
    case LoweredValInfo::Flavor::SwizzledLValue:
    case LoweredValInfo::Flavor::BoundSubscript:
    case LoweredValInfo::Flavor::BoundMember:
        args.Add(getSimpleVal(context, argInfo));
        break;

    default:
        SLANG_UNIMPLEMENTED_X("addArgs case");
        break;
    }
}

//

template<typename Derived>
struct ExprLoweringVisitorBase : ExprVisitor<Derived, LoweredValInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    // Lower an expression that should have the same l-value-ness
    // as the visitor itself.
    LoweredValInfo lowerSubExpr(Expr* expr)
    {
        IRBuilderSourceLocRAII sourceLocInfo(getBuilder(), expr->loc);
        return this->dispatch(expr);
    }


    LoweredValInfo visitVarExpr(VarExpr* expr)
    {
        LoweredValInfo info = emitDeclRef(
            context,
            expr->declRef,
            lowerType(context, expr->type));
        return info;
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
        auto baseVal = lowerSubExpr(expr->BaseExpression);
        auto indexVal = getSimpleVal(context, lowerRValueExpr(context, expr->IndexExpression));

        return subscriptValue(type, baseVal, indexVal);
    }

    LoweredValInfo visitThisExpr(ThisExpr* /*expr*/)
    {
        return context->thisVal;
    }

    LoweredValInfo visitMemberExpr(MemberExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);
        auto loweredBase = lowerRValueExpr(context, expr->BaseExpression);

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
        else if(auto constraintDeclRef = declRef.as<TypeConstraintDecl>())
        {
            // The code is making use of a "witness" that a value of
            // some generic type conforms to an interface.
            //
            // For now we will just emit the base expression as-is.
            // TODO: we may need to insert an explicit instruction
            // for a cast here (that could become a no-op later).
            return loweredBase;
        }

        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
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
            UInt elementCount = (UInt) GetIntVal(vectorType->elementCount);

            auto irDefaultValue = getSimpleVal(context, getDefaultVal(vectorType->elementType));

            List<IRInst*> args;
            for(UInt ee = 0; ee < elementCount; ++ee)
            {
                args.Add(irDefaultValue);
            }
            return LoweredValInfo::simple(
                getBuilder()->emitMakeVector(irType, args.Count(), args.Buffer()));
        }
        else if (auto matrixType = as<MatrixExpressionType>(type))
        {
            UInt rowCount = (UInt) GetIntVal(matrixType->getRowCount());

            auto rowType = matrixType->getRowType();

            auto irDefaultValue = getSimpleVal(context, getDefaultVal(rowType));

            List<IRInst*> args;
            for(UInt rr = 0; rr < rowCount; ++rr)
            {
                args.Add(irDefaultValue);
            }
            return LoweredValInfo::simple(
                getBuilder()->emitMakeMatrix(irType, args.Count(), args.Buffer()));
        }
        else if (auto arrayType = as<ArrayExpressionType>(type))
        {
            UInt elementCount = (UInt) GetIntVal(arrayType->ArrayLength);

            auto irDefaultElement = getSimpleVal(context, getDefaultVal(arrayType->baseType));

            List<IRInst*> args;
            for(UInt ee = 0; ee < elementCount; ++ee)
            {
                args.Add(irDefaultElement);
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeArray(irType, args.Count(), args.Buffer()));
        }
        else if (auto declRefType = as<DeclRefType>(type))
        {
            DeclRef<Decl> declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
            {
                List<IRInst*> args;
                for (auto ff : getMembersOfType<VarDecl>(aggTypeDeclRef))
                {
                    if (ff.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    auto irFieldVal = getSimpleVal(context, getDefaultVal(ff));
                    args.Add(irFieldVal);
                }

                return LoweredValInfo::simple(
                    getBuilder()->emitMakeStruct(irType, args.Count(), args.Buffer()));
            }
        }

        SLANG_UNEXPECTED("unexpected type when creating default value");
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo getDefaultVal(VarDeclBase* decl)
    {
        if(auto initExpr = decl->initExpr)
        {
            return lowerRValueExpr(context, initExpr);
        }
        else
        {
            return getDefaultVal(decl->type);
        }
    }

    LoweredValInfo visitInitializerListExpr(InitializerListExpr* expr)
    {
        // Allocate a temporary of the given type
        auto type = expr->type;
        IRType* irType = lowerType(context, type);
        List<IRInst*> args;

        UInt argCount = expr->args.Count();

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
            UInt elementCount = (UInt) GetIntVal(arrayType->ArrayLength);

            for (UInt ee = 0; ee < argCount; ++ee)
            {
                auto argExpr = expr->args[ee];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.Add(getSimpleVal(context, argVal));
            }
            if(elementCount > argCount)
            {
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(arrayType->baseType));
                for(UInt ee = argCount; ee < elementCount; ++ee)
                {
                    args.Add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeArray(irType, args.Count(), args.Buffer()));
        }
        else if (auto vectorType = as<VectorExpressionType>(type))
        {
            UInt elementCount = (UInt) GetIntVal(vectorType->elementCount);

            for (UInt ee = 0; ee < argCount; ++ee)
            {
                auto argExpr = expr->args[ee];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.Add(getSimpleVal(context, argVal));
            }
            if(elementCount > argCount)
            {
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(vectorType->elementType));
                for(UInt ee = argCount; ee < elementCount; ++ee)
                {
                    args.Add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeVector(irType, args.Count(), args.Buffer()));
        }
        else if (auto matrixType = as<MatrixExpressionType>(type))
        {
            UInt rowCount = (UInt) GetIntVal(matrixType->getRowCount());

            for (UInt rr = 0; rr < argCount; ++rr)
            {
                auto argExpr = expr->args[rr];
                LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                args.Add(getSimpleVal(context, argVal));
            }
            if(rowCount > argCount)
            {
                auto rowType = matrixType->getRowType();
                auto irDefaultValue = getSimpleVal(context, getDefaultVal(rowType));

                for(UInt rr = argCount; rr < rowCount; ++rr)
                {
                    args.Add(irDefaultValue);
                }
            }

            return LoweredValInfo::simple(
                getBuilder()->emitMakeMatrix(irType, args.Count(), args.Buffer()));
        }
        else if (auto declRefType = as<DeclRefType>(type))
        {
            DeclRef<Decl> declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.as<AggTypeDecl>())
            {
                UInt argCounter = 0;
                for (auto ff : getMembersOfType<VarDecl>(aggTypeDeclRef))
                {
                    if (ff.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    UInt argIndex = argCounter++;
                    if (argIndex < argCount)
                    {
                        auto argExpr = expr->args[argIndex];
                        LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                        args.Add(getSimpleVal(context, argVal));
                    }
                    else
                    {
                        auto irDefaultValue = getSimpleVal(context, getDefaultVal(ff));
                        args.Add(irDefaultValue);
                    }
                }

                return LoweredValInfo::simple(
                    getBuilder()->emitMakeStruct(irType, args.Count(), args.Buffer()));
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
        return LoweredValInfo::simple(context->irBuilder->getStringValue(expr->value.getUnownedSlice()));
    }

    LoweredValInfo visitAggTypeCtorExpr(AggTypeCtorExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("codegen for aggregate type constructor expression");
        UNREACHABLE_RETURN(LoweredValInfo());
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

    void addDirectCallArgs(
        InvokeExpr*             expr,
        DeclRef<CallableDecl>   funcDeclRef,
        List<IRInst*>*         ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        UInt argCount = expr->Arguments.Count();
        UInt argCounter = 0;
        for (auto paramDeclRef : getMembersOfType<ParamDecl>(funcDeclRef))
        {
            auto paramDecl = paramDeclRef.getDecl();
            IRType* paramType = lowerType(context, GetType(paramDeclRef));

            UInt argIndex = argCounter++;
            RefPtr<Expr> argExpr;
            if(argIndex < argCount)
            {
                argExpr = expr->Arguments[argIndex];
            }
            else
            {
                // We have run out of arguments supplied at the call site,
                // but there are still parameters remaining. This must mean
                // that these parameters have default argument expressions
                // associated with them.
                argExpr = getInitExpr(paramDeclRef);

                // Assert that such an expression must have been present.
                SLANG_ASSERT(argExpr);

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
            }

            if(paramDecl->HasModifier<RefModifier>())
            {
                // A `ref` qualified parameter must be implemented with by-reference
                // parameter passing, so the argument value should be lowered as
                // an l-value.
                //
                LoweredValInfo loweredArg = lowerLValueExpr(context, argExpr);

                // According to our "calling convention" we need to
                // pass a pointer into the callee. Unlike the case for
                // `out` and `inout` below, it is never valid to do
                // copy-in/copy-out for a `ref` parameter, so we just
                // pass in the actual pointer.
                //
                IRInst* argPtr = getAddress(context, loweredArg, argExpr->loc);
                (*ioArgs).Add(argPtr);
            }
            else if (paramDecl->HasModifier<OutModifier>()
                || paramDecl->HasModifier<InOutModifier>())
            {
                // This is a `out` or `inout` parameter, and so
                // the argument must be lowered as an l-value.

                LoweredValInfo loweredArg = lowerLValueExpr(context, argExpr);

                // According to our "calling convention" we need to
                // pass a pointer into the callee.
                //
                // A naive approach would be to just take the address
                // of `loweredArg` above and pass it in, but that
                // has two issues:
                //
                // 1. The l-value might not be something that has a single
                //    well-defined "address" (e.g., `foo.xzy`).
                //
                // 2. The l-value argument might actually alias some other
                //    storage that the callee will access (e.g., we are
                //    passing in a global variable, or two `out` parameters
                //    are being passed the same location in an array).
                //
                // In each of these cases, the safe option is to create
                // a temporary variable to use for argument-passing,
                // and then do copy-in/copy-out around the call.

                LoweredValInfo tempVar = createVar(context, paramType);

                // If the parameter is `in out` or `inout`, then we need
                // to ensure that we pass in the original value stored
                // in the argument, which we accomplish by assigning
                // from the l-value to our temp.
                if (paramDecl->HasModifier<InModifier>()
                    || paramDecl->HasModifier<InOutModifier>())
                {
                    assign(context, tempVar, loweredArg);
                }

                // Now we can pass the address of the temporary variable
                // to the callee as the actual argument for the `in out`
                SLANG_ASSERT(tempVar.flavor == LoweredValInfo::Flavor::Ptr);
                (*ioArgs).Add(tempVar.val);

                // Finally, after the call we will need
                // to copy in the other direction: from our
                // temp back to the original l-value.
                OutArgumentFixup fixup;
                fixup.src = tempVar;
                fixup.dst = loweredArg;

                (*ioFixups).Add(fixup);

            }
            else
            {
                // This is a pure input parameter, and so we will
                // pass it as an r-value.
                LoweredValInfo loweredArg = lowerRValueExpr(context, argExpr);
                addArgs(context, ioArgs, loweredArg);
            }
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

    void applyOutArgumentFixups(List<OutArgumentFixup> const& fixups)
    {
        for (auto fixup : fixups)
        {
            assign(context, fixup.dst, fixup.src);
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
        RefPtr<Expr>        funcExpr,
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
            outInfo->baseExpr = memberFuncExpr->BaseExpression;
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
            return false;
        }
    }


    LoweredValInfo visitInvokeExpr(InvokeExpr* expr)
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

        auto funcExpr = expr->FunctionExpr;
        ResolvedCallInfo resolvedInfo;
        if( tryResolveDeclRefForCall(funcExpr, &resolvedInfo) )
        {
            // In this case we know exactly what declaration we
            // are going to call, and so we can resolve things
            // appropriately.
            auto funcDeclRef = resolvedInfo.funcDeclRef;
            auto baseExpr = resolvedInfo.baseExpr;

            // First comes the `this` argument if we are calling
            // a member function:
            if( baseExpr )
            {
                auto loweredBaseVal = lowerRValueExpr(context, baseExpr);
                addArgs(context, &irArgs, loweredBaseVal);
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
                irArgs);
            applyOutArgumentFixups(argFixups);
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

    LoweredValInfo visitCastToInterfaceExpr(
        CastToInterfaceExpr* expr)
    {
        // We have an expression that is "up-casting" some concrete value
        // to an existential type (aka interface type), using a subtype witness
        // (which will lower as a witness table) to show that the conversion
        // is valid.
        //
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
        auto existentialType = lowerType(context, expr->type);
        auto concreteValue = getSimpleVal(context, lowerRValueExpr(context, expr->valueArg));
        auto witnessTable = lowerSimpleVal(context, expr->witnessArg);
        auto existentialValue = getBuilder()->emitMakeExistential(existentialType, concreteValue, witnessTable);
        return LoweredValInfo::simple(existentialValue);
    }

    LoweredValInfo subscriptValue(
        IRType*         type,
        LoweredValInfo  baseVal,
        IRInst*         indexVal)
    {
        auto builder = getBuilder();
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
        IRType*             fieldType,
        LoweredValInfo      base,
        DeclRef<VarDecl>    field)
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
        // TODO: deal with the case where we might want to capture
        // a reference to the bound value...

        auto initVal = lowerLValueExpr(context, expr->decl->initExpr);
        setGlobalValue(context, expr->decl, initVal);
        auto bodyVal = lowerSubExpr(expr->body);
        return bodyVal;
    }

    LoweredValInfo visitExtractExistentialValueExpr(ExtractExistentialValueExpr* expr)
    {
        auto existentialType = lowerType(context, GetType(expr->declRef));
        auto existentialVal = getSimpleVal(context, emitDeclRef(context, expr->declRef, existentialType));

        auto openedType = lowerType(context, expr->type);

        return LoweredValInfo::simple(getBuilder()->emitExtractExistentialValue(openedType, existentialVal));
    }
};

struct LValueExprLoweringVisitor : ExprLoweringVisitorBase<LValueExprLoweringVisitor>
{
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

        context->shared->extValues.Add(swizzledLValue);
        return LoweredValInfo::swizzledLValue(swizzledLValue);
    }
};

struct RValueExprLoweringVisitor : ExprLoweringVisitorBase<RValueExprLoweringVisitor>
{
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

        auto rangeBeginVal = GetIntVal(stmt->rangeBeginVal);
        auto rangeEndVal = GetIntVal(stmt->rangeEndVal);

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

        auto condExpr = stmt->Predicate;
        auto thenStmt = stmt->PositiveStatement;
        auto elseStmt = stmt->NegativeStatement;

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
        if( stmt->FindModifier<UnrollAttribute>() )
        {
            getBuilder()->addLoopControlDecoration(inst, kIRLoopControl_Unroll);
        }
        // TODO: handle other cases here
    }

    void visitForStmt(ForStmt* stmt)
    {
        auto builder = getBuilder();
        startBlockIfNeeded(stmt);

        // The initializer clause for the statement
        // can always safetly be emitted to the current block.
        if (auto initStmt = stmt->InitialStatement)
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
        if (auto condExpr = stmt->PredicateExpression)
        {
            auto irCondition = getSimpleVal(context,
                lowerRValueExpr(context, stmt->PredicateExpression));

            // Now we want to `break` if the loop condition is false.
            builder->emitLoopTest(
                irCondition,
                bodyLabel,
                breakLabel);
        }

        // Emit the body of the loop
        insertBlock(bodyLabel);
        lowerStmt(context, stmt->Statement);

        // Insert the `continue` block
        insertBlock(continueLabel);
        if (auto incrExpr = stmt->SideEffectExpression)
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
        if (auto condExpr = stmt->Predicate)
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
        lowerStmt(context, stmt->Statement);

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
        lowerStmt(context, stmt->Statement);

        insertBlock(testLabel);

        // Now that we are within the header block, we
        // want to emit the expression for the loop condition:
        if (auto condExpr = stmt->Predicate)
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
        lowerLValueExpr(context, stmt->Expression);
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

        // A `return` statement turns into a return
        // instruction. If the statement had an argument
        // expression, then we need to lower that to
        // a value first, and then emit the resulting value.
        if( auto expr = stmt->Expression )
        {
            auto loweredExpr = lowerRValueExpr(context, expr);

            getBuilder()->emitReturn(getSimpleVal(context, loweredExpr));
        }
        else
        {
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
            info->cases.Add(caseVal);
            info->cases.Add(label);
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
            info.cases.Count(),
            info.cases.Buffer());

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
    catch(AbortCompilationException&) { throw; }
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

    case LoweredValInfo::Flavor::BoundSubscript:
        {
            // If we are are trying to turn a subscript operation like `buffer[index]`
            // into a pointer, then we need to find a `ref` accessor declared
            // as part of the subscript operation being referenced.
            //
            auto subscriptInfo = val.getBoundSubscriptInfo();

            // We don't want to immediately bind to a `ref` accessor if there is
            // a `set` accessor available, unless we are in an "aggressive" mode
            // where we really want/need a pointer to be able to make progress.
            //
            if(mode != TryGetAddressMode::Aggressive
                && getMembersOfType<SetterDecl>(subscriptInfo->declRef).Count())
            {
                // There is a setter that we should consider using,
                // so don't go and aggressively collapse things just yet.
                return val;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(subscriptInfo->declRef);
            if(refAccessors.Count())
            {
                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(subscriptInfo->type);

                LoweredValInfo refVal = emitCallToDeclRef(
                    context,
                    ptrType,
                    *refAccessors.begin(),
                    nullptr,
                    subscriptInfo->args);

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
            // a `BoundSubscript`, etc.).
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
            context->shared->extValues.Add(newSwizzleInfo);

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

    case LoweredValInfo::Flavor::BoundSubscript:
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
            auto subscriptInfo = left.getBoundSubscriptInfo();

            // Search for an appropriate "setter" declaration
            auto setters = getMembersOfType<SetterDecl>(subscriptInfo->declRef);
            if (setters.Count())
            {
                auto allArgs = subscriptInfo->args;
                addArgs(context, &allArgs, right);

                emitCallToDeclRef(
                    context,
                    builder->getVoidType(),
                    *setters.begin(),
                    nullptr,
                    allArgs);
                return;
            }

            auto refAccessors = getMembersOfType<RefAccessorDecl>(subscriptInfo->declRef);
            if(refAccessors.Count())
            {
                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                IRType* ptrType = context->irBuilder->getPtrType(subscriptInfo->type);

                LoweredValInfo refVal = emitCallToDeclRef(
                    context,
                    ptrType,
                    *refAccessors.begin(),
                    nullptr,
                    subscriptInfo->args);

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

    default:
        SLANG_UNIMPLEMENTED_X("assignment");
        break;
    }
}

struct DeclLoweringVisitor : DeclVisitor<DeclLoweringVisitor, LoweredValInfo>
{
    IRGenContext*   context;

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
        for (auto & member : decl->Members)
            ensureDecl(context, member);
        return LoweredValInfo();
    }

    LoweredValInfo visitImportDecl(ImportDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitEmptyDecl(EmptyDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitSyntaxDecl(SyntaxDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitAttributeDecl(AttributeDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitTypeDefDecl(TypeDefDecl* decl)
    {
        // A type alias declaration may be generic, if it is
        // nested under a generic type/function/etc.
        //
        NestedContext nested(this);
        auto subBuilder = nested.getBuilder();
        auto subContext = nested.getContet();
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

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, type));
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
        if(auto assocTypeDecl = as<AssocTypeDecl>(decl->ParentDecl))
        {
            // TODO: might need extra steps if we ever allow
            // generic associated types.


            if(auto interfaceDecl = as<InterfaceDecl>(assocTypeDecl->ParentDecl))
            {
                // Okay, this seems to be an interface rquirement, and
                // we should lower it as such.
                return LoweredValInfo::simple(getInterfaceRequirementKey(decl));
            }
        }

        if(auto globalGenericParamDecl = as<GlobalGenericParamDecl>(decl->ParentDecl))
        {
            // This is a constraint on a global generic type parameters,
            // and so it should lower as a parameter of its own.

            auto inst = getBuilder()->emitGlobalGenericParam();
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
        auto inst = getBuilder()->emitGlobalGenericParam();
        addLinkageDecoration(context, inst, decl);
        return LoweredValInfo::simple(inst);
    }

    void lowerWitnessTable(
        IRGenContext*                               subContext,
        WitnessTable*                               astWitnessTable,
        IRWitnessTable*                             irWitnessTable,
        Dictionary<WitnessTable*, IRWitnessTable*>  mapASTToIRWitnessTable)
    {
        auto subBuilder = subContext->irBuilder;

        for(auto entry : astWitnessTable->requirementDictionary)
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
                        irSatisfyingWitnessTable = subBuilder->createWitnessTable();

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
        auto parentDecl = inheritanceDecl->ParentDecl;
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
        RefPtr<Type> subType;
        if (auto extParentDecl = as<ExtensionDecl>(parentDecl))
        {
            subType = extParentDecl->targetType.type;
        }
        else
        {
            subType = DeclRefType::Create(
                context->getSession(),
                makeDeclRef(parentDecl));
        }

        // What is the super-type that we have declared we inherit from?
        RefPtr<Type> superType = inheritanceDecl->base.type;

        // Construct the mangled name for the witness table, which depends
        // on the type that is conforming, and the type that it conforms to.
        //
        // TODO: This approach doesn't really make sense for generic `extension` conformances.
        auto mangledName = getMangledNameForConformanceWitness(subType, superType);

        // A witness table may need to be generic, if the outer
        // declaration (either a type declaration or an `extension`)
        // is generic.
        //
        NestedContext nested(this);
        auto subBuilder = nested.getBuilder();
        auto subContext = nested.getContet();
        emitOuterGenerics(subContext, inheritanceDecl, inheritanceDecl);

        // Lower the super-type to force its declaration to be lowered.
        //
        // Note: we are using the "sub-context" here because the
        // type being inherited from could reference generic parameters,
        // and we need those parameters to lower as references to
        // the parameters of our IR-level generic.
        //
        lowerType(subContext, superType);

        // Create the IR-level witness table
        auto irWitnessTable = subBuilder->createWitnessTable();
        addLinkageDecoration(context, irWitnessTable, inheritanceDecl, mangledName.getUnownedSlice());

        // Register the value now, rather than later, to avoid any possible infinite recursion.
        setGlobalValue(context, inheritanceDecl, LoweredValInfo::simple(irWitnessTable));

        // Make sure that all the entries in the witness table have been filled in,
        // including any cases where there are sub-witness-tables for conformances
        Dictionary<WitnessTable*, IRWitnessTable*> mapASTToIRWitnessTable;
        lowerWitnessTable(
            subContext,
            inheritanceDecl->witnessTable,
            irWitnessTable,
            mapASTToIRWitnessTable);

        irWitnessTable->moveToEnd();

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irWitnessTable));
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

    LoweredValInfo visitSubscriptDecl(SubscriptDecl* decl)
    {
        // A subscript operation may encompass one or more
        // accessors, and these are what should actually
        // get lowered (they are effectively functions).

        for (auto accessor : decl->getMembersOfType<AccessorDecl>())
        {
            if (accessor->HasModifier<IntrinsicOpModifier>())
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

    bool isGlobalVarDecl(VarDecl* decl)
    {
        auto parent = decl->ParentDecl;
        if (as<ModuleDecl>(parent))
        {
            // Variable declared at global scope? -> Global.
            return true;
        }
        else if(as<AggTypeDeclBase>(parent))
        {
            if(decl->HasModifier<HLSLStaticModifier>())
            {
                // A `static` member variable is effectively global.
                return true;
            }
        }

        return false;
    }

    bool isMemberVarDecl(VarDecl* decl)
    {
        auto parent = decl->ParentDecl;
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

        // A global variable's SSA value is a *pointer* to
        // the underlying storage.
        setGlobalValue(context, decl, paramVal);

        irParam->moveToEnd();

        return paramVal;
    }

    LoweredValInfo lowerGlobalVarDecl(VarDecl* decl)
    {
        if(isGlobalShaderParameter(decl))
        {
            return lowerGlobalShaderParam(decl);
        }

        IRType* varType = lowerType(context, decl->getType());

        auto builder = getBuilder();

        IRGlobalValueWithCode* irGlobal = nullptr;
        LoweredValInfo globalVal;

        // a `static const` global is actually a compile-time constant
        if (decl->HasModifier<HLSLStaticModifier>() && decl->HasModifier<ConstModifier>())
        {
            irGlobal = builder->createGlobalConstant(varType);
            globalVal = LoweredValInfo::simple(irGlobal);
        }
        else
        {
            irGlobal = builder->createGlobalVar(varType);
            globalVal = LoweredValInfo::ptr(irGlobal);
        }
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
        if(!decl->FindModifier<HLSLStaticModifier>())
            return false;

        // The immediate parent of a function-scope variable
        // declaration will be a `ScopeDecl`.
        //
        // TODO: right now the parent links for scopes are *not*
        // set correctly, so we can't just scan up and look
        // for a function in the parent chain...
        auto parent = decl->ParentDecl;
        if( as<ScopeDecl>(parent) )
        {
            return true;
        }

        return false;
    }

    IRInst* defaultSpecializeOuterGeneric(
        IRInst*         outerVal,
        IRType*         type,
        GenericDecl*    genericDecl)
    {
        auto builder = getBuilder();

        // We need to specialize any generics that are further out...
        auto specialiedOuterVal = defaultSpecializeOuterGenerics(
            outerVal,
            builder->getGenericKind(),
            genericDecl);

        List<IRInst*> genericArgs;

        // Walk the parameters of the generic, and emit an argument for each,
        // which will be a reference to binding for that parameter in the
        // current scope.
        //
        // First we start with type and value parameters,
        // in the order they were declared.
        for (auto member : genericDecl->Members)
        {
            if (auto typeParamDecl = as<GenericTypeParamDecl>(member))
            {
                genericArgs.Add(getSimpleVal(context, ensureDecl(context, typeParamDecl)));
            }
            else if (auto valDecl = as<GenericValueParamDecl>(member))
            {
                genericArgs.Add(getSimpleVal(context, ensureDecl(context, valDecl)));
            }
        }
        // Then we emit constraint parameters, again in
        // declaration order.
        for (auto member : genericDecl->Members)
        {
            if (auto constraintDecl = as<GenericTypeConstraintDecl>(member))
            {
                genericArgs.Add(getSimpleVal(context, ensureDecl(context, constraintDecl)));
            }
        }

        return builder->emitSpecializeInst(type, specialiedOuterVal, genericArgs.Count(), genericArgs.Buffer());
    }

    IRInst* defaultSpecializeOuterGenerics(
        IRInst* val,
        IRType* type,
        Decl*   decl)
    {
        if(!val) return nullptr;

        auto parentVal = val->getParent();
        while(parentVal)
        {
            if(as<IRGeneric>(parentVal))
                break;
            parentVal = parentVal->getParent();
        }
        if(!parentVal)
            return val;

        for(auto pp = decl->ParentDecl; pp; pp = pp->ParentDecl)
        {
            if(auto genericAncestor = as<GenericDecl>(pp))
            {
                return defaultSpecializeOuterGeneric(parentVal, type, genericAncestor);
            }
        }

        return val;
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
        }

        IRBuilder* getBuilder() { return &subBuilderStorage; }
        IRGenContext* getContet() { return &subContextStorage; }
    };

    LoweredValInfo lowerFunctionStaticConstVarDecl(
        VarDeclBase* decl)
    {
        // We need to insert the constant at a level above
        // the function being emitted. This will usually
        // be the global scope, but it might be an outer
        // generic if we are lowering a generic function.
        //
        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContet();

        subBuilder->setInsertInto(subBuilder->getFunc()->getParent());

        IRType* subVarType = lowerType(subContext, decl->getType());

        IRGlobalConstant* irConstant = subBuilder->createGlobalConstant(subVarType);
        addVarDecorations(subContext, irConstant, decl);
        addNameHint(context, irConstant, decl);
        maybeSetRate(context, irConstant, decl);
        subBuilder->addHighLevelDeclDecoration(irConstant, decl);

        LoweredValInfo constantVal = LoweredValInfo::ptr(irConstant);
        setValue(context, decl, constantVal);

        if( auto initExpr = decl->initExpr )
        {
            NestedContext nestedInitContext(this);
            auto initBuilder = nestedInitContext.getBuilder();
            auto initContext = nestedInitContext.getContet();

            initBuilder->setInsertInto(irConstant);

            IRBlock* entryBlock = initBuilder->emitBlock();
            initBuilder->setInsertInto(entryBlock);

            LoweredValInfo initVal = lowerRValueExpr(initContext, initExpr);
            initBuilder->emitReturn(getSimpleVal(initContext, initVal));
        }

        return constantVal;
    }

    LoweredValInfo lowerFunctionStaticVarDecl(
        VarDeclBase*    decl)
    {
        // We know the variable is `static`, but it might also be `const.
        if(decl->HasModifier<ConstModifier>())
            return lowerFunctionStaticConstVarDecl(decl);

        // A global variable may need to be generic, if one
        // of the outer declarations is generic.
        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContet();
        subBuilder->setInsertInto(subBuilder->getModule()->getModuleInst());
        emitOuterGenerics(subContext, decl, decl);

        IRType* subVarType = lowerType(subContext, decl->getType());

        IRGlobalValueWithCode* irGlobal = subBuilder->createGlobalVar(subVarType);
        addVarDecorations(subContext, irGlobal, decl);

        addNameHint(context, irGlobal, decl);
        maybeSetRate(context, irGlobal, decl);

        subBuilder->addHighLevelDeclDecoration(irGlobal, decl);

        // We are inside of a function, and that function might be generic,
        // in which case the `static` variable will be lowered to another
        // generic. Let's start with a terrible example:
        //
        //      interface IHasCount { int getCount(); }
        //      int incrementCounter<T : IHasCount >(T val) {
        //          static int counter = 0;
        //          counter += val.getCount();
        //          return counter;
        //      }
        //
        // In this case, `incrementCounter` will lower to a function
        // nested in a generic, while `counter` will be lowered to
        // a global variable nested in a *different* generic.
        // The net result is something like this:
        //
        //      int counter<T:IHasCount> = 0;
        //
        //      int incrementCounter<T:IHasCount>(T val) {
        //          counter<T> += val.getCount();
        //          return counter<T>;
        //
        // The references to `counter` inside of `incrementCounter`
        // become references to `counter<T>`.
        //
        // At the IR level, this means that the value we install
        // for `decl` needs to be a specialized reference to `irGlobal`,
        // for any outer generics.
        //
        IRType* varType = lowerType(context, decl->getType());
        IRType* varPtrType = getBuilder()->getPtrType(varType);
        auto irSpecializedGlobal = defaultSpecializeOuterGenerics(irGlobal, varPtrType, decl);
        LoweredValInfo globalVal = LoweredValInfo::ptr(irSpecializedGlobal);
        setValue(context, decl, globalVal);

        // A `static` variable with an initializer needs special handling,
        // at least if the initializer isn't a compile-time constant.
        if( auto initExpr = decl->initExpr )
        {
            // We must create an ordinary global `bool isInitialized = false`
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

            // Of course, if we are under a generic, then the Boolean
            // variable need to be generic as well!
            NestedContext nestedBoolContext(this);
            auto boolBuilder = nestedBoolContext.getBuilder();
            auto boolContext = nestedBoolContext.getContet();
            boolBuilder->setInsertInto(boolBuilder->getModule()->getModuleInst());
            emitOuterGenerics(boolContext, decl, decl);

            auto irBoolType = boolBuilder->getBoolType();
            auto irBool = boolBuilder->createGlobalVar(irBoolType);
            boolBuilder->setInsertInto(irBool);
            boolBuilder->setInsertInto(boolBuilder->createBlock());
            boolBuilder->emitReturn(boolBuilder->getBoolValue(false));

            auto boolVal = LoweredValInfo::ptr(defaultSpecializeOuterGenerics(irBool, irBoolType, decl));


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

        irGlobal->moveToEnd();
        finishOuterGenerics(subBuilder, irGlobal);
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
        //
        // TODO: we may want to special-case things when the variable's
        // type, qualifiers, or context mark it as something that can't
        // be mutable (or even do some limited dataflow pass to check
        // which variables ever get assigned) so that we can directly
        // emit an SSA value in this common case.
        //

        IRType* varType = lowerType(context, decl->getType());

        // TODO: If the variable is marked `static` then we need to
        // deal with it specially: we should move its allocation out
        // to the global scope, and then we have to deal with its
        // initializer expression a bit carefully (it should only
        // be initialized on-demand at its first use).

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
        for (auto requirementDecl : decl->Members)
        {
            getInterfaceRequirementKey(requirementDecl);

            // As a special case, any type constraints placed
            // on an associated type will *also* need to be turned
            // into requirement keys for this interface.
            if (auto associatedTypeDecl = as<AssocTypeDecl>(requirementDecl))
            {
                for (auto constraintDecl : associatedTypeDecl->getMembersOfType<TypeConstraintDecl>())
                {
                    getInterfaceRequirementKey(constraintDecl);
                }
            }
        }


        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContet();

        // Emit any generics that should wrap the actual type.
        emitOuterGenerics(subContext, decl, decl);

        IRInterfaceType* irInterface = subBuilder->createInterfaceType();
        addNameHint(context, irInterface, decl);
        addLinkageDecoration(context, irInterface, decl);
        subBuilder->setInsertInto(irInterface);

        // TODO: are there any interface members that should be
        // nested inside the interface type itself?

        irInterface->moveToEnd();

        addTargetIntrinsicDecorations(irInterface, decl);


        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irInterface));
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
        auto subContext = nestedContext.getContet();

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
        auto subContext = nestedContext.getContet();
        emitOuterGenerics(subContext, decl, decl);

        // An `enum` declaration will currently lower directly to its "tag"
        // type, so that any references to the `enum` become referenes to
        // the tag type instead.
        //
        // TODO: if we ever support `enum` types with payloads, we would
        // need to make the `enum` lower to some kind of custom "tagged union"
        // type.

        IRType* loweredTagType = lowerType(subContext, decl->tagType);

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, loweredTagType));
    }

    LoweredValInfo visitAggTypeDecl(AggTypeDecl* decl)
    {
        // Don't generate an IR `struct` for intrinsic types
        if(decl->FindModifier<IntrinsicTypeModifier>() || decl->FindModifier<BuiltinTypeModifier>())
        {
            return LoweredValInfo();
        }

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
        auto subContext = nestedContext.getContet();

        // Emit any generics that should wrap the actual type.
        emitOuterGenerics(subContext, decl, decl);

        IRStructType* irStruct = subBuilder->createStructType();
        addNameHint(context, irStruct, decl);
        addLinkageDecoration(context, irStruct, decl);

        subBuilder->setInsertInto(irStruct);

        for (auto fieldDecl : decl->getMembersOfType<VarDeclBase>())
        {
            if (fieldDecl->HasModifier<HLSLStaticModifier>())
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
                irStruct,
                fieldKey,
                fieldType);
        }

        // There may be members not handled by the above logic (e.g.,
        // member functions), but we will not immediately force them
        // to be emitted here, so as not to risk a circular dependency.
        //
        // Instead we will force emission of all children of aggregate
        // type declarations later, from the top-level emit logic.

        irStruct->moveToEnd();
        addTargetIntrinsicDecorations(irStruct, decl);

        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irStruct));
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

        if (auto semanticModifier = fieldDecl->FindModifier<HLSLSimpleSemantic>())
        {
            builder->addSemanticDecoration(irFieldKey, semanticModifier->name.getName()->text.getUnownedSlice());
        }

        // We allow a field to be marked as a target intrinsic,
        // so that we can override its mangled name in the
        // output for the chosen target.
        addTargetIntrinsicDecorations(irFieldKey, fieldDecl);


        return LoweredValInfo::simple(irFieldKey);
    }


    DeclRef<Decl> createDefaultSpecializedDeclRefImpl(Decl* decl)
    {
        DeclRef<Decl> declRef;
        declRef.decl = decl;
        declRef.substitutions = createDefaultSubstitutions(context->getSession(), decl);
        return declRef;
    }
    //
    // The client should actually call the templated wrapper, to preserve type information.
    template<typename D>
    DeclRef<D> createDefaultSpecializedDeclRef(D* decl)
    {
        DeclRef<Decl> declRef = createDefaultSpecializedDeclRefImpl(decl);
        return declRef.as<D>();
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
    enum ParameterDirection
    {
        kParameterDirection_In,     ///< Copy in
        kParameterDirection_Out,    ///< Copy out
        kParameterDirection_InOut,  ///< Copy in, copy out
        kParameterDirection_Ref,    ///< By-reference
    };
    struct ParameterInfo
    {
        // This AST-level type of the parameter
        RefPtr<Type>        type;

        // The direction (`in` vs `out` vs `in out`)
        ParameterDirection  direction;

        // The variable/parameter declaration for
        // this parameter (if any)
        VarDeclBase*        decl;

        // Is this the representation of a `this` parameter?
        bool                isThisParam = false;
    };
    //
    // We need a way to compute the appropriate `ParameterDirection` for a
    // declared parameter:
    //
    ParameterDirection getParameterDirection(VarDeclBase* paramDecl)
    {
        if( paramDecl->HasModifier<RefModifier>() )
        {
            // The AST specified `ref`:
            return kParameterDirection_Ref;
        }
        if( paramDecl->HasModifier<InOutModifier>() )
        {
            // The AST specified `inout`:
            return kParameterDirection_InOut;
        }
        if (paramDecl->HasModifier<OutModifier>())
        {
            // We saw an `out` modifier, so now we need
            // to check if there was a paired `in`.
            if(paramDecl->HasModifier<InModifier>())
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
    // We need a way to be able to create a `ParameterInfo` given the declaration
    // of a parameter:
    //
    ParameterInfo getParameterInfo(VarDeclBase* paramDecl)
    {
        ParameterInfo info;
        info.type = paramDecl->getType();
        info.decl = paramDecl;
        info.direction = getParameterDirection(paramDecl);
        info.isThisParam = false;
        return info;
    }
    //

    // Here's the declaration for the type to hold the lists:
    struct ParameterLists
    {
        List<ParameterInfo> params;
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
        ParameterInfo info;
        info.type = type;
        info.decl = nullptr;
        info.direction = direction;
        info.isThisParam = true;

        ioParameterLists->params.Add(info);
    }
    void addThisParameter(
        ParameterDirection  direction,
        AggTypeDecl*        typeDecl,
        ParameterLists*     ioParameterLists)
    {
        // We need to construct an appopriate declaration-reference
        // for the type declaration we were given. In particular,
        // we need to specialize it for any generic parameters
        // that are in scope here.
        auto declRef = createDefaultSpecializedDeclRef(typeDecl);
        RefPtr<Type> type = DeclRefType::Create(context->getSession(), declRef);
        addThisParameter(
            direction,
            type,
            ioParameterLists);
    }
    //
    // And here is our function that will do the recursive walk:
    void collectParameterLists(
        Decl*                       decl,
        ParameterLists*             ioParameterLists,
        ParameterListCollectMode    mode)
    {
        // The parameters introduced by any "parent" declarations
        // will need to come first, so we'll deal with that
        // logic here.
        if( auto parentDecl = decl->ParentDecl )
        {
            // Compute the mode to use when collecting parameters from
            // the outer declaration. The most important question here
            // is whether parameters of the outer declaration should
            // also count as parameters of the inner declaration.
            ParameterListCollectMode innerMode = getModeForCollectingParentParameters(decl, parentDecl);

            // Don't down-grade our `static`-ness along the chain.
            if(innerMode < mode)
                innerMode = mode;

            // Now collect any parameters from the parent declaration itself
            collectParameterLists(parentDecl, ioParameterLists, innerMode);

            // We also need to consider whether the inner declaration needs to have a `this`
            // parameter corresponding to the outer declaration.
            if( innerMode != kParameterListCollectMode_Static )
            {
                // For now we make any `this` parameter default to `in`.
                //
                ParameterDirection direction = kParameterDirection_In;
                //
                // Applications can opt in to a mutable `this` parameter,
                // by applying the `[mutating]` attribute to their
                // declaration.
                //
                if( decl->HasModifier<MutatingAttribute>() )
                {
                    direction = kParameterDirection_InOut;
                }

                if( auto aggTypeDecl = as<AggTypeDecl>(parentDecl) )
                {
                    addThisParameter(direction, aggTypeDecl, ioParameterLists);
                }
                else if( auto extensionDecl = as<ExtensionDecl>(parentDecl) )
                {
                    addThisParameter(direction, extensionDecl->targetType, ioParameterLists);
                }
            }
        }

        // Once we've added any parameters based on parent declarations,
        // we can see if this declaration itself introduces parameters.
        //
        if( auto callableDecl = as<CallableDecl>(decl) )
        {
            // Don't collect parameters from the outer scope if
            // we are in a `static` context.
            if( mode == kParameterListCollectMode_Default )
            {
                for( auto paramDecl : callableDecl->GetParameters() )
                {
                    ioParameterLists->params.Add(getParameterInfo(paramDecl));
                }
            }
        }
    }

    bool isImportedDecl(Decl* decl)
    {
        return Slang::isImportedDecl(context, decl);
    }

    bool isConstExprVar(Decl* decl)
    {
        if( decl->HasModifier<ConstExprModifier>() )
        {
            return true;
        }
        else if(decl->HasModifier<HLSLStaticModifier>() && decl->HasModifier<ConstModifier>())
        {
            return true;
        }

        return false;
    }

    IRType* maybeGetConstExprType(IRType* type, Decl* decl)
    {
        if(isConstExprVar(decl))
        {
            return getBuilder()->getRateQualifiedType(
                getBuilder()->getConstExprRate(),
                type);
        }

        return type;
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
        for (auto member : genericDecl->Members)
        {
            if (auto typeParamDecl = as<GenericTypeParamDecl>(member))
            {
                // TODO: use a `TypeKind` to represent the
                // classifier of the parameter.
                auto param = subBuilder->emitParam(nullptr);
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
        for (auto member : genericDecl->Members)
        {
            if (auto constraintDecl = as<GenericTypeConstraintDecl>(member))
            {
                // TODO: use a `WitnessTableKind` to represent the
                // classifier of the parameter.
                auto param = subBuilder->emitParam(nullptr);
                addNameHint(context, param, constraintDecl);
                setValue(subContext, constraintDecl, LoweredValInfo::simple(param));
            }
        }

        return irGeneric;
    }

    // If the given `decl` is enclosed in any generic declarations, then
    // emit IR-level generics to represent them.
    // The `leafDecl` represents the inner-most declaration we are actually
    // trying to emit, which is the one that should receive the mangled name.
    //
    IRGeneric* emitOuterGenerics(IRGenContext* subContext, Decl* decl, Decl* leafDecl)
    {
        for(auto pp = decl->ParentDecl; pp; pp = pp->ParentDecl)
        {
            if(auto genericAncestor = as<GenericDecl>(pp))
            {
                return emitOuterGeneric(subContext, genericAncestor, leafDecl);
            }
        }

        return nullptr;
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
        IRInst*     val)
    {
        IRInst* v = val;
        for(;;)
        {
            auto parentBlock = as<IRBlock>(v->getParent());
            if (!parentBlock) break;

            auto parentGeneric = as<IRGeneric>(parentBlock->getParent());
            if (!parentGeneric) break;

            subBuilder->setInsertInto(parentBlock);
            subBuilder->emitReturn(v);
            parentGeneric->moveToEnd();

            // There might be more outer generics,
            // so we need to loop until we run out.
            v = parentGeneric;
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

        for (auto targetMod : decl->GetModifiersOfType<TargetIntrinsicModifier>())
        {
            String definition;
            auto definitionToken = targetMod->definitionToken;
            if (definitionToken.type == TokenType::StringLiteral)
            {
                definition = getStringLiteralTokenValue(definitionToken);
            }
            else
            {
                definition = definitionToken.Content;
            }

            builder->addTargetIntrinsicDecoration(irInst, targetMod->targetToken.Content, definition.getUnownedSlice());
        }
    }

    void addParamNameHint(IRInst* inst, ParameterInfo info)
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

    LoweredValInfo lowerFuncDecl(FunctionDeclBase* decl)
    {
        // We are going to use a nested builder, because we will
        // change the parent node that things get nested into.
        //
        NestedContext nestedContext(this);
        auto subBuilder = nestedContext.getBuilder();
        auto subContext = nestedContext.getContet();

        // The actual `IRFunction` that we emit needs to be nested
        // inside of one `IRGeneric` for every outer `GenericDecl`
        // in the declaration hierarchy.

        emitOuterGenerics(subContext, decl, decl);

        // Collect the parameter lists we will use for our new function.
        ParameterLists parameterLists;
        collectParameterLists(decl, &parameterLists, kParameterListCollectMode_Default);

        // TODO: if there are any generic parameters in the collected list, then
        // we need to output an IR function with generic parameters (or a generic
        // with a nested function... the exact representation is still TBD).

        // In most cases the return type for a declaration can be read off the declaration
        // itself, but things get a bit more complicated when we have to deal with
        // accessors for subscript declarations (and eventually for properties).
        //
        // We compute a declaration to use for looking up the return type here:
        CallableDecl* declForReturnType = decl;
        if (auto accessorDecl = as<AccessorDecl>(decl))
        {
            // We are some kind of accessor, so the parent declaration should
            // know the correct return type to expose.
            //
            auto parentDecl = accessorDecl->ParentDecl;
            if (auto subscriptDecl = as<SubscriptDecl>(parentDecl))
            {
                declForReturnType = subscriptDecl;
            }
        }

        // need to create an IR function here

        IRFunc* irFunc = subBuilder->createFunc();
        addNameHint(context, irFunc, decl);
        addLinkageDecoration(context, irFunc, decl);

        List<IRType*> paramTypes;

        for( auto paramInfo : parameterLists.params )
        {
            IRType* irParamType = lowerType(subContext, paramInfo.type);

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
                irParamType = subBuilder->getOutType(irParamType);
                break;
            case kParameterDirection_InOut:
                irParamType = subBuilder->getInOutType(irParamType);
                break;
            case kParameterDirection_Ref:
                irParamType = subBuilder->getRefType(irParamType);
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
                irParamType = maybeGetConstExprType(irParamType, paramInfo.decl);
            }

            paramTypes.Add(irParamType);
        }

        auto irResultType = lowerType(subContext, declForReturnType->ReturnType);

        if (auto setterDecl = as<SetterDecl>(decl))
        {
            // We are lowering a "setter" accessor inside a subscript
            // declaration, which means we don't want to *return* the
            // stated return type of the subscript, but instead take
            // it as a parameter.
            //
            IRType* irParamType = irResultType;
            paramTypes.Add(irParamType);

            // Instead, a setter always returns `void`
            //
            irResultType = subBuilder->getVoidType();
        }

        if( auto refAccessorDecl = as<RefAccessorDecl>(decl) )
        {
            // A `ref` accessor needs to return a *pointer* to the value
            // being accessed, rather than a simple value.
            irResultType = subBuilder->getPtrType(irResultType);
        }

        auto irFuncType = subBuilder->getFuncType(
            paramTypes.Count(),
            paramTypes.Buffer(),
            irResultType);
        irFunc->setFullType(irFuncType);

        subBuilder->setInsertInto(irFunc);

        if (isImportedDecl(decl))
        {
            // Always emit imported declarations as declarations,
            // and not definitions.
        }
        else if (!decl->Body)
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

                switch( paramInfo.direction )
                {
                default:
                    {
                        // The parameter is being used for input/output purposes,
                        // so it will lower to an actual parameter with a pointer type.
                        //
                        // TODO: Is this the best representation we can use?

                        IRParam* irParamPtr = subBuilder->emitParam(irParamType);
                        if(auto paramDecl = paramInfo.decl)
                        {
                            subBuilder->addHighLevelDeclDecoration(irParamPtr, paramDecl);
                        }
                        addParamNameHint(irParamPtr, paramInfo);

                        paramVal = LoweredValInfo::ptr(irParamPtr);

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
                        IRParam* irParam = subBuilder->emitParam(irParamType);
                        if( paramDecl )
                        {
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
                        if(paramDecl && paramDecl->FindModifier<ConstModifier>())
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

            if (auto setterDecl = as<SetterDecl>(decl))
            {
                // Add the IR parameter for the new value
                IRType* irParamType = irResultType;
                auto irParam = subBuilder->emitParam(irParamType);
                addNameHint(context, irParam, "newValue");

                // TODO: we need some way to wire this up to the `newValue`
                // or whatever name we give for that parameter inside
                // the setter body.
            }

            {

                auto attr = decl->FindModifier<PatchConstantFuncAttribute>();

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

            // Lower body

            lowerStmt(subContext, decl->Body);

            // We need to carefully add a terminator instruction to the end
            // of the body, in case the user didn't do so.
            if (!subContext->irBuilder->getBlock()->getTerminator())
            {
                if(as<IRVoidType>(irResultType))
                {
                    // `void`-returning function can get an implicit
                    // return on exit of the body statement.
                    subContext->irBuilder->emitReturn();
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
        for( auto targetMod : decl->GetModifiersOfType<SpecializedForTargetModifier>() )
        {
            // `targetMod` indicates that this particular declaration represents
            // a specialized definition of the particular function for the given
            // target, and we need to reflect that at the IR level.

            getBuilder()->addTargetDecoration(irFunc, targetMod->targetToken.Content);
        }

        // If this declaration was marked as having a target-specific lowering
        // for a particular target, then handle that here.
        addTargetIntrinsicDecorations(irFunc, decl);

        // If this declaration requires certain GLSL extension (or a particular GLSL version)
        // for it to be usable, then declare that here.
        //
        // TODO: We should wrap this an `SpecializedForTargetModifier` together into a single
        // case for enumerating the "capabilities" that a declaration requires.
        //
        for(auto extensionMod : decl->GetModifiersOfType<RequiredGLSLExtensionModifier>())
        {
            getBuilder()->addRequireGLSLExtensionDecoration(irFunc, extensionMod->extensionNameToken.Content);
        }
        for(auto versionMod : decl->GetModifiersOfType<RequiredGLSLVersionModifier>())
        {
            getBuilder()->addRequireGLSLVersionDecoration(irFunc, Int(getIntegerLiteralValue(versionMod->versionNumberToken)));
        }

        if(decl->FindModifier<ReadNoneAttribute>())
        {
            getBuilder()->addSimpleDecoration<IRReadNoneDecoration>(irFunc);
        }

        if (decl->FindModifier<EarlyDepthStencilAttribute>())
        {
            getBuilder()->addSimpleDecoration<IREarlyDepthStencilDecoration>(irFunc);
        }

        // For convenience, ensure that any additional global
        // values that were emitted while outputting the function
        // body appear before the function itself in the list
        // of global values.
        irFunc->moveToEnd();
        return LoweredValInfo::simple(finishOuterGenerics(subBuilder, irFunc));
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
    catch(AbortCompilationException&) { throw; }
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

    IRBuilder subIRBuilder;
    subIRBuilder.sharedBuilder = context->irBuilder->sharedBuilder;
    subIRBuilder.setInsertInto(subIRBuilder.sharedBuilder->module->getModuleInst());

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

IRInst* lowerSubstitutionArg(
    IRGenContext*   context,
    Val*            val)
{
    if (auto type = dynamicCast<Type>(val))
    {
        return lowerType(context, type);
    }
    else if (auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(val))
    {
        // We need to look up the IR-level representation of the witness (which will be a witness table).
        auto irWitnessTable = getSimpleVal(
            context,
            emitDeclRef(
                context,
                declaredSubtypeWitness->declRef,
                context->irBuilder->getWitnessTableType()));
        return irWitnessTable;
    }
    else
    {
        SLANG_UNIMPLEMENTED_X("value cases");
        UNREACHABLE_RETURN(nullptr);
    }
}

// Can the IR lowered version of this declaration ever be an `IRGeneric`?
bool canDeclLowerToAGeneric(RefPtr<Decl> decl)
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

    return false;
}

LoweredValInfo emitDeclRef(
    IRGenContext*           context,
    RefPtr<Decl>            decl,
    RefPtr<Substitutions>   subst,
    IRType*                 type)
{
    // We need to proceed by considering the specializations that
    // have been put in place.

    // Ignore any global generic type substitutions during lowering.
    // Really, we don't even expect these to appear.
    while(auto globalGenericSubst = as<GlobalGenericParamSubstitution>(subst))
        subst = globalGenericSubst->outer;

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
    if(auto genericSubst = subst.as<GenericSubstitution>())
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
            irArgs.Add(irArgVal);
        }

        // Once we have both the generic and its arguments,
        // we can emit a `specialize` instruction and use
        // its value as the result.
        auto irSpecializedVal = context->irBuilder->emitSpecializeInst(
            type,
            irGenericVal,
            irArgs.Count(),
            irArgs.Buffer());

        return LoweredValInfo::simple(irSpecializedVal);
    }
    else if(auto thisTypeSubst = subst.as<ThisTypeSubstitution>())
    {
        if(decl.Ptr() == thisTypeSubst->interfaceDecl)
        {
            // This is a reference to the interface type itself,
            // through the this-type substitution, so it is really
            // a reference to the this-type.
            return lowerType(context, thisTypeSubst->witness->sub);
        }

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
    EntryPoint*     entryPoint)
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
    builder->addEntryPointDecoration(instToDecorate);
}

static void lowerProgramEntryPointToIR(
    IRGenContext*   context,
    EntryPoint*     entryPoint)
{
    // First, lower the entry point like an ordinary function

    auto session = context->getSession();
    auto entryPointFuncDeclRef = entryPoint->getFuncDeclRef();
    auto entryPointFuncType = lowerType(context, getFuncType(session, entryPointFuncDeclRef));

    auto builder = context->irBuilder;
    builder->setInsertInto(builder->getModule()->getModuleInst());

    auto loweredEntryPointFunc = getSimpleVal(context,
        emitDeclRef(context, entryPointFuncDeclRef, entryPointFuncType));

    //
    if(!loweredEntryPointFunc->findDecoration<IRLinkageDecoration>())
    {
        builder->addExportDecoration(loweredEntryPointFunc, getMangledName(entryPointFuncDeclRef).getUnownedSlice());
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
    if(auto containerDecl = as<AggTypeDecl>(decl))
    {
        for (auto memberDecl : containerDecl->Members)
        {
            ensureAllDeclsRec(context, memberDecl);
        }
    }
}

IRModule* generateIRForTranslationUnit(
    TranslationUnitRequest* translationUnit)
{
    auto compileRequest = translationUnit->compileRequest;

    SharedIRGenContext sharedContextStorage(
        translationUnit->getSession(),
        translationUnit->compileRequest->getSink(),
        translationUnit->getModuleDecl());
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    IRGenContext contextStorage(sharedContext);
    IRGenContext* context = &contextStorage;

    SharedIRBuilder sharedBuilderStorage;
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = compileRequest->getSession();

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->sharedBuilder = sharedBuilder;

    IRModule* module = builder->createModule();
    sharedBuilder->module = module;

    context->irBuilder = builder;

    // We need to emit IR for all public/exported symbols
    // in the translation unit.
    //
    // For now, we will assume that *all* global-scope declarations
    // represent public/exported symbols.

    // First, ensure that all entry points have been emitted,
    // in case they require special handling.
    for (auto entryPoint : translationUnit->entryPoints)
    {
        lowerFrontEndEntryPointToIR(context, entryPoint);
    }

    //
    // Next, ensure that all other global declarations have
    // been emitted.
    for (auto decl : translationUnit->getModuleDecl()->Members)
    {
        ensureAllDeclsRec(context, decl);
    }

#if 0
    fprintf(stderr, "### GENERATED\n");
    dumpIR(module);
    fprintf(stderr, "###\n");
#endif

    validateIRModuleIfEnabled(compileRequest, module);

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

    // First, attempt to promote local variables to SSA
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

    // TODO: consider doing some more aggressive optimizations
    // (in particular specialization of generics) here, so
    // that we can avoid doing them downstream.
    //
    // Note: doing specialization or inlining involving code
    // from other modules potentially makes the IR we generate
    // "fragile" in that we'd now need to recompile when
    // a module we depend on changes.

    validateIRModuleIfEnabled(compileRequest, module);

    // If we are being sked to dump IR during compilation,
    // then we can dump the initial IR for the module here.
    if(compileRequest->shouldDumpIR)
    {
        DiagnosticSinkWriter writer(compileRequest->getSink());
        dumpIR(module, &writer);
    }

    return module;
}

RefPtr<IRModule> generateIRForProgram(
    Session*        session,
    Program*        program,
    DiagnosticSink* sink)
{
//    auto compileRequest = translationUnit->compileRequest;

    SharedIRGenContext sharedContextStorage(
        session,
        sink);
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    IRGenContext contextStorage(sharedContext);
    IRGenContext* context = &contextStorage;

    SharedIRBuilder sharedBuilderStorage;
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = session;

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->sharedBuilder = sharedBuilder;

    RefPtr<IRModule> module = builder->createModule();
    sharedBuilder->module = module;

    context->irBuilder = builder;

    // We need to emit symbols for all of the entry
    // points in the program; this is especially
    // important in the case where a generic entry
    // point is being specialized.
    //
    for(auto entryPoint : program->getEntryPoints())
    {
        lowerProgramEntryPointToIR(context, entryPoint);
    }


    // Now lower all the arguments supplied for global generic
    // type parameters.
    //
    for (RefPtr<Substitutions> subst = program->getGlobalGenericSubstitution(); subst; subst = subst->outer)
    {
        auto gSubst = subst.as<GlobalGenericParamSubstitution>();
        if(!gSubst)
            continue;

        IRInst* typeParam = getSimpleVal(context, ensureDecl(context, gSubst->paramDecl));
        IRType* typeVal = lowerType(context, gSubst->actualType);

        // bind `typeParam` to `typeVal`
        builder->emitBindGlobalGenericParam(typeParam, typeVal);

        for (auto& constraintArg : gSubst->constraintArgs)
        {
            IRInst* constraintParam = getSimpleVal(context, ensureDecl(context, constraintArg.decl));
            IRInst* constraintVal = lowerSimpleVal(context, constraintArg.val);

            // bind `constraintParam` to `constraintVal`
            builder->emitBindGlobalGenericParam(constraintParam, constraintVal);
        }
    }

    // TODO: Should we apply any of the validation or
    // mandatory optimization passes here?

    return module;
}

} // namespace Slang
