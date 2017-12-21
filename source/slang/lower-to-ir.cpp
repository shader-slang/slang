// lower.cpp
#include "lower-to-ir.h"

#include "../../slang.h"

#include "ir.h"
#include "ir-insts.h"
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
    RefPtr<Type>            type;
    List<IRValue*>          args;
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

        // An l-value reprsented as an IR
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
        IRValue*            val;
        ExtendedValueInfo*  ext;
    };
    Flavor flavor;

    LoweredValInfo()
    {
        flavor = Flavor::None;
        val = nullptr;
    }

    static LoweredValInfo simple(IRValue* v)
    {
        LoweredValInfo info;
        info.flavor = Flavor::Simple;
        info.val = v;
        return info;
    }

    static LoweredValInfo ptr(IRValue* v)
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
        assert(flavor == Flavor::BoundMember);
        return (BoundMemberInfo*)ext;
    }

    static LoweredValInfo subscript(
        SubscriptInfo* subscriptInfo);

    SubscriptInfo* getSubscriptInfo()
    {
        assert(flavor == Flavor::Subscript);
        return (SubscriptInfo*)ext;
    }

    static LoweredValInfo boundSubscript(
        BoundSubscriptInfo* boundSubscriptInfo);

    BoundSubscriptInfo* getBoundSubscriptInfo()
    {
        assert(flavor == Flavor::BoundSubscript);
        return (BoundSubscriptInfo*)ext;
    }

    static LoweredValInfo swizzledLValue(
        SwizzledLValueInfo* extInfo);

    SwizzledLValueInfo* getSwizzledLValueInfo()
    {
        assert(flavor == Flavor::SwizzledLValue);
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
    RefPtr<Type> type;
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
    RefPtr<Type>    type;

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

struct SharedIRGenContext
{
    CompileRequest* compileRequest;
    ModuleDecl*     mainModuleDecl;

    Dictionary<Decl*, LoweredValInfo> declValues;

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
    SharedIRGenContext* shared;

    IRBuilder* irBuilder;

    // The value to use for any `this` expressions
    // that appear in the current context.
    //
    // TODO: If we ever allow nesting of (non-static)
    // types, then we may need to support references
    // to an "outer `this`", and this representation
    // might be insufficient.
    LoweredValInfo thisVal;

    Session* getSession()
    {
        return shared->compileRequest->mSession;
    }
};

// Ensure that a version of the given declaration has been emitted to the IR
LoweredValInfo ensureDecl(
    IRGenContext*   context,
    Decl*           decl);

// Emit code as needed to construct a reference to the given declaration with
// any needed specializations in place.
LoweredValInfo emitDeclRef(
    IRGenContext*   context,
    DeclRef<Decl>   declRef);

// Emit necessary `specialize` instruction needed by a declRef.
// This is currently used by emitDeclRef() and emitFuncRef()
LoweredValInfo maybeEmitSpecializeInst(IRGenContext*   context, 
    LoweredValInfo loweredDecl,  // the lowered value of the inner decl
    DeclRef<Decl>   declRef      // the full decl ref containing substitutions
);


IRValue* getSimpleVal(IRGenContext* context, LoweredValInfo lowered);

IROp getIntrinsicOp(
    Decl*                   decl,
    IntrinsicOpModifier*    intrinsicOpMod)
{
    if (int(intrinsicOpMod->op) != 0)
        return intrinsicOpMod->op;

    // No specified modifier? Then we need to look it up
    // based on the name of the declaration...

    auto name = decl->getName();
    auto nameText = getText(name);

    IROp op = findIROp(nameText.Buffer());
    assert(op != kIROp_Invalid);
    return op;
}

// Given a `LoweredValInfo` for something callable, along with a
// bunch of arguments, emit an appropriate call to it.
LoweredValInfo emitCallToVal(
    IRGenContext*   context,
    IRType*         type,
    LoweredValInfo  funcVal,
    UInt            argCount,
    IRValue* const* args)
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
    IRValue* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    assert(argCount == 2);
    auto leftPtr = args[0];
    auto rightVal = args[1];

    auto leftVal = builder->emitLoad(leftPtr);

    IRValue* innerArgs[] = { leftVal, rightVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(leftPtr, innerOp);

    return LoweredValInfo::ptr(leftPtr);
}

IRValue* getOneValOfType(
    IRGenContext*   context,
    IRType*         type)
{
    if (auto basicType = dynamic_cast<BasicExpressionType*>(type))
    {
        switch (basicType->baseType)
        {
        case BaseType::Int:
        case BaseType::UInt:
        case BaseType::UInt64:
            return context->irBuilder->getIntValue(type, 1);

        case BaseType::Float:
        case BaseType::Double:
            return context->irBuilder->getFloatValue(type, 1.0);

        default:
            break;
        }
    }
    // TODO: should make sure to handle vector and matrix types here

    SLANG_UNEXPECTED("inc/dec type");
    UNREACHABLE_RETURN(nullptr);
}

LoweredValInfo emitPreOp(
    IRGenContext*   context,
    IRType*         type,
    IROp            op,
    UInt            argCount,
    IRValue* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    assert(argCount == 1);
    auto argPtr = args[0];

    auto preVal = builder->emitLoad(argPtr);

    IRValue* oneVal = getOneValOfType(context, type);

    IRValue* innerArgs[] = { preVal, oneVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(argPtr, innerOp);

    return LoweredValInfo::simple(preVal);
}

LoweredValInfo emitPostOp(
    IRGenContext*   context,
    IRType*         type,
    IROp            op,
    UInt            argCount,
    IRValue* const* args)
{
    auto builder = context->irBuilder;
    SLANG_UNREFERENCED_PARAMETER(argCount);
    assert(argCount == 1);
    auto argPtr = args[0];

    auto preVal = builder->emitLoad(argPtr);

    IRValue* oneVal = getOneValOfType(context, type);

    IRValue* innerArgs[] = { preVal, oneVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(argPtr, innerOp);

    return LoweredValInfo::ptr(argPtr);
}

// Emit a reference to a function, where we have concluded
// that the original AST referenced `funcDeclRef`. The
// optional expression `funcExpr` can provide additional
// detail that might modify how we go about looking up
// the actual value to call.
LoweredValInfo emitFuncRef(
    IRGenContext*   context,
    DeclRef<Decl>   funcDeclRef,
    Expr*           funcExpr)
{
    if( !funcExpr )
    {
        return emitDeclRef(context, funcDeclRef);
    }

    // Let's look at the expression to see what additional
    // information it gives us.

    if(auto funcMemberExpr = dynamic_cast<MemberExpr*>(funcExpr))
    {
        auto baseExpr = funcMemberExpr->BaseExpression;
        if(auto baseMemberExpr = baseExpr.As<MemberExpr>())
        {
            auto baseMemberDeclRef = baseMemberExpr->declRef;
            if(auto baseConstraintDeclRef = baseMemberDeclRef.As<GenericTypeConstraintDecl>())
            {
                // We are calling a method "through" a generic type
                // parameter that was constrained to some type.
                // That means `funcDeclRef` is a reference to the method
                // on the `interface` type (which doesn't actually have
                // a body, so we don't want to emit or call it), and
                // we actually want to perform a lookup step to
                // find the corresponding member on our chosen type.

                RefPtr<Type> type = funcExpr->type;

                auto loweredVal = LoweredValInfo::simple(context->irBuilder->emitLookupInterfaceMethodInst(
                    type,
                    baseMemberDeclRef,
                    funcDeclRef));
                return maybeEmitSpecializeInst(context, loweredVal, funcDeclRef);
            }
        }
    }

    // We didn't trigger a special case, so just emit a reference
    // to the function itself.
    return emitDeclRef(context, funcDeclRef);
}

// Given a `DeclRef` for something callable, along with a bunch of
// arguments, emit an appropriate call to it.
LoweredValInfo emitCallToDeclRef(
    IRGenContext*   context,
    IRType*         type,
    DeclRef<Decl>   funcDeclRef,
    Expr*           funcExpr,
    UInt            argCount,
    IRValue* const* args)
{
    auto builder = context->irBuilder;


    if (auto subscriptDeclRef = funcDeclRef.As<SubscriptDecl>())
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
            // If the subscript declares a `ref` accessor, then we can just
            // invoke that directly to get an l-value we can use.
            if(auto refAccessorDeclRef = accessorDeclRef.As<RefAccessorDecl>())
            {
                // The `ref` accessor will return a pointer to the value, so
                // we need to reflect that in the type of our `call` instruction.
                RefPtr<Type> ptrType = context->getSession()->getPtrType(type);

                // Rather than call `emitCallToVal` here, we make a recursive call
                // to `emitCallToDeclRef` so that it can handle things like intrinsic-op
                // modifiers attached to the acecssor.
                LoweredValInfo callVal = emitCallToDeclRef(
                    context,
                    ptrType,
                    refAccessorDeclRef,
                    funcExpr,
                    argCount,
                    args);

                // The result from the call needs to be implicitly dereferenced,
                // so that it can work as an l-value of the desired result type.
                return LoweredValInfo::ptr(getSimpleVal(context, callVal));
            }

            // If we don't find a `ref` accessor, then we want to track whether
            // this subscript has any accessors other than `get` (assuming
            // that everything except `get` can be used for setting...).

            if (auto foundGetterDeclRef = accessorDeclRef.As<GetterDecl>())
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

        if (Int(op) < 0)
        {
            switch (op)
            {
            case kIRPseudoOp_Pos:
                return LoweredValInfo::simple(args[0]);

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
            case COMPOUND: return emitPreOp(context, type, OP, argCount, args)
            CASE(kIRPseudoOp_PreInc, kIROp_Add);
            CASE(kIRPseudoOp_PreDec, kIROp_Sub);
#undef CASE

#define CASE(COMPOUND, OP)  \
            case COMPOUND: return emitPostOp(context, type, OP, argCount, args)
            CASE(kIRPseudoOp_PostInc, kIROp_Add);
            CASE(kIRPseudoOp_PostDec, kIROp_Sub);
#undef CASE

            default:
                SLANG_UNIMPLEMENTED_X("IR pseudo-op");
                break;
            }
        }

        return LoweredValInfo::simple(builder->emitIntrinsicInst(
            type,
            op,
            argCount,
            args));
    }
    // TODO: handle target intrinsic modifier too...

    if( auto ctorDeclRef = funcDeclRef.As<ConstructorDecl>() )
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
    LoweredValInfo funcVal = emitFuncRef(context, funcDeclRef, funcExpr);
    return emitCallToVal(context, type, funcVal, argCount, args);
}

LoweredValInfo emitCallToDeclRef(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<Decl>           funcDeclRef,
    Expr*                   funcExpr,
    List<IRValue*> const&   args)
{
    return emitCallToDeclRef(context, type, funcDeclRef, funcExpr, args.Count(), args.Buffer());
}

LoweredValInfo extractField(
    IRGenContext*           context,
    Type*                   fieldType,
    LoweredValInfo          base,
    DeclRef<StructField>    field)
{
    IRBuilder* builder = context->irBuilder;

    switch (base.flavor)
    {
    default:
        {
            IRValue* irBase = getSimpleVal(context, base);
            return LoweredValInfo::simple(
                builder->emitFieldExtract(
                    fieldType,
                    irBase,
                    builder->getDeclRefVal(field)));
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
            IRValue* irBasePtr = base.val;
            return LoweredValInfo::ptr(
                builder->emitFieldAddress(
                    context->getSession()->getPtrType(fieldType),
                    irBasePtr,
                    builder->getDeclRefVal(field)));
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

            SLANG_UNEXPECTED("subscript had no getter");
            UNREACHABLE_RETURN(LoweredValInfo());
        }
        break;

    case LoweredValInfo::Flavor::BoundMember:
        {
            auto boundMemberInfo = lowered.getBoundMemberInfo();
            auto base = materialize(context, boundMemberInfo->base);

            auto declRef = boundMemberInfo->declRef;
            if( auto fieldDeclRef = declRef.As<StructField>() )
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

IRValue* getSimpleVal(IRGenContext* context, LoweredValInfo lowered)
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

struct LoweredTypeInfo
{
    enum class Flavor
    {
        None,
        Simple,
    };

    RefPtr<IRType> type;
    Flavor flavor;

    LoweredTypeInfo()
    {
        flavor = Flavor::None;
    }

    LoweredTypeInfo(IRType* t)
    {
        flavor = Flavor::Simple;
        type = t;
    }
};

RefPtr<Type> getSimpleType(LoweredTypeInfo lowered)
{
    switch(lowered.flavor)
    {
    case LoweredTypeInfo::Flavor::None:
        return nullptr;

    case LoweredTypeInfo::Flavor::Simple:
        return lowered.type;

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        UNREACHABLE_RETURN(nullptr);
    }
}

LoweredValInfo lowerVal(
    IRGenContext*   context,
    Val*            val);

IRValue* lowerSimpleVal(
    IRGenContext*   context,
    Val*            val)
{
    auto lowered = lowerVal(context, val);
    return getSimpleVal(context, lowered);
}

LoweredTypeInfo lowerType(
    IRGenContext*   context,
    Type*           type);

static LoweredTypeInfo lowerType(
    IRGenContext*   context,
    QualType const& type)
{
    return lowerType(context, type.type);
}

// Lower a type and expect the result to be simple
RefPtr<Type> lowerSimpleType(
    IRGenContext*   context,
    Type*           type)
{
    auto lowered = lowerType(context, type);
    return getSimpleType(lowered);
}

RefPtr<Type> lowerSimpleType(
    IRGenContext*   context,
    QualType const& type)
{
    auto lowered = lowerType(context, type);
    return getSimpleType(lowered);
}

LoweredValInfo lowerLValueExpr(
    IRGenContext*   context,
    Expr*           expr);

LoweredValInfo lowerRValueExpr(
    IRGenContext*   context,
    Expr*           expr);

void assign(
    IRGenContext*           context,
    LoweredValInfo const&   left,
    LoweredValInfo const&   right);

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt);

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    DeclBase*       decl);

IRType* getIntType(
    IRGenContext* context)
{
    return context->getSession()->getBuiltinType(BaseType::Int);
}

RefPtr<IRFuncType> getFuncType(
    IRGenContext*           context,
    UInt                    paramCount,
    RefPtr<IRType>  const*  paramTypes,
    IRType*                 resultType)
{
    RefPtr<FuncType> funcType = new FuncType();
    funcType->setSession(context->getSession());
    funcType->resultType = resultType;
    for (UInt pp = 0; pp < paramCount; ++pp)
    {
        funcType->paramTypes.Add(paramTypes[pp]);
    }
    return funcType;
}

//

struct ValLoweringVisitor : ValVisitor<ValLoweringVisitor, LoweredValInfo, LoweredTypeInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    LoweredValInfo visitVal(Val* /*val*/)
    {
        SLANG_UNIMPLEMENTED_X("value lowering");
    }

    LoweredValInfo visitConstantIntVal(ConstantIntVal* val)
    {
        // TODO: it is a bit messy here that the `ConstantIntVal` representation
        // has no notion of a *type* associated with the value...

        auto type = getIntType(context);
        return LoweredValInfo::simple(getBuilder()->getIntValue(type, val->value));
    }

    LoweredTypeInfo visitType(Type* type)
    {
        // TODO(tfoley): Now that we use the AST types directly in the IR, there
        // isn't much to do in the "lowering" step. Still, there might be cases
        // where certain kinds of legalization need to take place, so this
        // visitor setup might still be needed in the long run.
        return LoweredTypeInfo(type);
//        SLANG_UNIMPLEMENTED_X("type lowering");
    }

    LoweredTypeInfo visitFuncType(FuncType* type)
    {
        return LoweredTypeInfo(type);
    }

    void addGenericArgs(List<IRValue*>* ioArgs, DeclRefBase declRef)
    {
        auto subs = declRef.substitutions;
        while(subs)
        {
            if (auto genSubst = subs.As<GenericSubstitution>())
            {
                for (auto aa : genSubst->args)
                {
                    (*ioArgs).Add(getSimpleVal(context, lowerVal(context, aa)));
                }
            }
            subs = subs->outer;
        }
    }

    LoweredTypeInfo visitDeclRefType(DeclRefType* type)
    {
        // If the type in question comes from the module we are
        // trying to lower, then we need to make sure to
        // emit everything relevant to its declaration.

        // TODO: actually test what module the type is coming from.

        lowerDecl(context, type->declRef);


        return LoweredTypeInfo(type);
    }

    LoweredTypeInfo visitBasicExpressionType(BasicExpressionType* type)
    {
        return LoweredTypeInfo(type);
    }

    LoweredTypeInfo visitVectorExpressionType(VectorExpressionType* type)
    {
        return LoweredTypeInfo(type);
    }

    LoweredTypeInfo visitMatrixExpressionType(MatrixExpressionType* type)
    {
        return LoweredTypeInfo(type);
    }

    LoweredTypeInfo visitArrayExpressionType(ArrayExpressionType* type)
    {
        return LoweredTypeInfo(type);
    }

    LoweredTypeInfo visitIRBasicBlockType(IRBasicBlockType* type)
    {
        return LoweredTypeInfo(type);
    }
};

LoweredValInfo lowerVal(
    IRGenContext*   context,
    Val*            val)
{
    ValLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(val);
}

LoweredTypeInfo lowerType(
    IRGenContext*   context,
    Type*           type)
{
    ValLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatchType(type);
}

LoweredValInfo createVar(
    IRGenContext*   context,
    RefPtr<Type>    type,
    Decl*           decl = nullptr)
{
    auto builder = context->irBuilder;
    auto irAlloc = builder->emitVar(type);

    if (decl)
    {
        builder->addHighLevelDeclDecoration(irAlloc, decl);
    }

    return LoweredValInfo::ptr(irAlloc);
}

void addArgs(
    IRGenContext*   context,
    List<IRValue*>* ioArgs,
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
        LoweredValInfo info = emitDeclRef(context, expr->declRef);
        return info;
    }

    LoweredValInfo visitOverloadedExpr(OverloadedExpr* /*expr*/)
    {
        SLANG_UNEXPECTED("overloaded expressions should not occur in checked AST");
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
        if (auto fieldDeclRef = declRef.As<StructField>())
        {
            // Okay, easy enough: we have a reference to a field of a struct type...
            return extractField(loweredType, loweredBase, fieldDeclRef);
        }
        else if (auto callableDeclRef = declRef.As<CallableDecl>())
        {
            RefPtr<BoundMemberInfo> boundMemberInfo = new BoundMemberInfo();
            boundMemberInfo->type = nullptr;
            boundMemberInfo->base = loweredBase;
            boundMemberInfo->declRef = callableDeclRef;
            return LoweredValInfo::boundMember(boundMemberInfo);
        }
        else if(auto constraintDeclRef = declRef.As<GenericTypeConstraintDecl>())
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
    }

    // We will always lower a dereference expression (`*ptr`)
    // as an l-value, since that is the easiest way to handle it.
    LoweredValInfo visitDerefExpr(DerefExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);
        auto loweredBase = lowerRValueExpr(context, expr->base);

        // TODO: handle tupel-type for `base`

        // The type of the lowered base must by some kind of pointer,
        // in order for a dereference to make senese, so we just
        // need to extract the value type from that pointer here.
        //
        IRValue* loweredBaseVal = getSimpleVal(context, loweredBase);
        RefPtr<Type> loweredBaseType = loweredBaseVal->getType();

        if (loweredBaseType->As<PointerLikeType>()
            || loweredBaseType->As<PtrTypeBase>())
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

    LoweredValInfo visitInitializerListExpr(InitializerListExpr* expr)
    {
        // Allocate a temporary of the given type
        RefPtr<Type> type = lowerSimpleType(context, expr->type);
        LoweredValInfo val = createVar(context, type);

        UInt argCount = expr->args.Count();

        // Now for each argument in the initializer list,
        // fill in the appropriate field of the result
        if (auto arrayType = type->As<ArrayExpressionType>())
        {
            UInt elementCount = (UInt) GetIntVal(arrayType->ArrayLength);
            auto elementType = lowerType(context, arrayType->baseType);
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                IRValue* indexVal = context->irBuilder->getIntValue(
                        getIntType(context),
                        ee);
                LoweredValInfo elemVal = subscriptValue(
                    elementType,
                    val,
                    indexVal);

                if (ee < argCount)
                {
                    auto argExpr = expr->args[ee];
                    LoweredValInfo argVal = lowerRValueExpr(context, argExpr);

                    assign(context, elemVal, argVal);
                }
                else
                {
                    SLANG_UNEXPECTED("need to default-initialize array elements");
                }
            }
        }
        else if (auto declRefType = type->As<DeclRefType>())
        {
            DeclRef<Decl> declRef = declRefType->declRef;
            if (auto aggTypeDeclRef = declRef.As<AggTypeDecl>())
            {
                UInt argCounter = 0;
                for (auto ff : getMembersOfType<StructField>(aggTypeDeclRef))
                {
                    if (ff.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    auto loweredFieldType = lowerType(
                        context,
                        GetType(ff));
                    LoweredValInfo fieldVal = extractField(
                        loweredFieldType,
                        val,
                        ff);

                    UInt argIndex = argCounter++;
                    if (argIndex < argCount)
                    {
                        auto argExpr = expr->args[argIndex];
                        LoweredValInfo argVal = lowerRValueExpr(context, argExpr);
                        assign(context, fieldVal, argVal);
                    }
                    else
                    {
                        SLANG_UNEXPECTED("need to default-initialize struct fields");
                    }
                }
            }
            else
            {
                SLANG_UNEXPECTED("not sure how to initialize this type");
            }
        }
        else
        {
            SLANG_UNEXPECTED("not sure how to initialize this type");
        }


        return val;
    }

    LoweredValInfo visitConstantExpr(ConstantExpr* expr)
    {
        auto type = lowerSimpleType(context, expr->type);

        switch( expr->ConstType )
        {
        case ConstantExpr::ConstantType::Bool:
            return LoweredValInfo::simple(context->irBuilder->getBoolValue(expr->integerValue != 0));
        case ConstantExpr::ConstantType::Int:
            return LoweredValInfo::simple(context->irBuilder->getIntValue(type, expr->integerValue));
        case ConstantExpr::ConstantType::Float:
            return LoweredValInfo::simple(context->irBuilder->getFloatValue(type, expr->floatingPointValue));
        case ConstantExpr::ConstantType::String:
            break;
        }

        SLANG_UNEXPECTED("unexpected constant type");
    }

    LoweredValInfo visitAggTypeCtorExpr(AggTypeCtorExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("codegen for aggregate type constructor expression");
    }

    // Add arguments that appeared directly in an argument list
    // to the list of argument values for a call.
    void addDirectCallArgs(
        InvokeExpr*     expr,
        List<IRValue*>* ioArgs)
    {
        for( auto arg : expr->Arguments )
        {
            // TODO: Need to handle case of l-value arguments,
            // when they are matched to `out` or `in out` parameters.
            auto loweredArg = lowerRValueExpr(context, arg);
            addArgs(context, ioArgs, loweredArg);
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

    void addDirectCallArgs(
        InvokeExpr*             expr,
        DeclRef<CallableDecl>   funcDeclRef,
        List<IRValue*>*         ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        UInt argCount = expr->Arguments.Count();
        UInt argIndex = 0;
        for (auto paramDeclRef : getMembersOfType<ParamDecl>(funcDeclRef))
        {
            if (argIndex >= argCount)
            {
                // The remaining parameters must be defaulted...
                break;
            }

            auto paramDecl = paramDeclRef.getDecl();
            RefPtr<Type> paramType = lowerSimpleType(context, GetType(paramDeclRef));
            auto argExpr = expr->Arguments[argIndex++];

            if (paramDecl->HasModifier<OutModifier>()
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
                assert(tempVar.flavor == LoweredValInfo::Flavor::Ptr);
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
        List<IRValue*>*         ioArgs,
        List<OutArgumentFixup>* ioFixups)
    {
        if (auto callableDeclRef = funcDeclRef.As<CallableDecl>())
        {
            addDirectCallArgs(expr, callableDeclRef, ioArgs, ioFixups);
        }
        else
        {
            SLANG_UNEXPECTED("shouldn't relaly happen");
            UNREACHABLE(addDirectCallArgs(expr, ioArgs));
        }
    }

    void addFuncBaseArgs(
        LoweredValInfo funcVal,
        List<IRValue*>* ioArgs)
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
        auto declRefExpr = funcExpr.As<DeclRefExpr>();
        if(!declRefExpr)
            return false;

        // A little bit of future proofing here: if we ever
        // allow higher-order functions, then we might be
        // calling through a variable/field that has a function
        // type, but is not itself a function.
        // In such a case we should be careful to not statically
        // resolve things.
        //
        if(auto callableDecl = dynamic_cast<CallableDecl*>(declRefExpr->declRef.getDecl()))
        {
            // Okay, the declaration is directly callable, so we can continue.
        }
        else
        {
            // The callee declaration isn't itself a callable (it must have
            // a funciton type, though).
            return false;
        }

        // Now we can look at the specific kinds of declaration references,
        // and try to tease them apart.
        if (auto memberFuncExpr = funcExpr.As<MemberExpr>())
        {
            outInfo->funcDeclRef = memberFuncExpr->declRef;
            outInfo->baseExpr = memberFuncExpr->BaseExpression;
            return true;
        }
        else if (auto staticMemberFuncExpr = funcExpr.As<StaticMemberExpr>())
        {
            outInfo->funcDeclRef = staticMemberFuncExpr->declRef;
            return true;
        }
        else if (auto varExpr = funcExpr.As<VarExpr>())
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
        auto type = lowerSimpleType(context, expr->type);

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
        List<IRValue*> irArgs;

        // We will also collect "fixup" actions that need
        // to be performed after teh call, in order to
        // copy the final values for `out` parameters
        // back to their arguments.
        List<OutArgumentFixup> argFixups;

        auto funcExpr = expr->FunctionExpr;
        ResolvedCallInfo resolvedInfo;
        if( tryResolveDeclRefForCall(funcExpr, &resolvedInfo) )
        {
            // In this case we know exaclty what declaration we
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
            addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
            auto result = emitCallToDeclRef(
                context,
                type,
                funcDeclRef,
                funcExpr,
                irArgs);
            applyOutArgumentFixups(argFixups);
            return result;
        }

        // The default case is to assume that we just have
        // an ordinary expression, and can lower it as such.
        LoweredValInfo funcVal = lowerRValueExpr(context, expr->FunctionExpr);

        // Now we add any direct arguments from the call expression itself.
        addDirectCallArgs(expr, &irArgs);

        // Delegate to the logic for invoking a value.
        auto result = emitCallToVal(context, type, funcVal, irArgs.Count(), irArgs.Buffer());

        // TODO: because of the nature of how the `emitCallToVal` case works
        // right now, we don't have information on in/out parameters, and
        // so we can't collect info to apply fixups.
        //
        // Once we have a better representation for function types, though,
        // this should be fixable.

        return result;
    }

    LoweredValInfo subscriptValue(
        LoweredTypeInfo type,
        LoweredValInfo  baseVal,
        IRValue*        indexVal)
    {
        auto builder = getBuilder();
        switch (baseVal.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
            return LoweredValInfo::simple(
                builder->emitElementExtract(
                    getSimpleType(type),
                    getSimpleVal(context, baseVal),
                    indexVal));

        case LoweredValInfo::Flavor::Ptr:
            return LoweredValInfo::ptr(
                builder->emitElementAddress(
                    context->getSession()->getPtrType(getSimpleType(type)),
                    baseVal.val,
                    indexVal));

        default:
            SLANG_UNIMPLEMENTED_X("subscript expr");
            UNREACHABLE_RETURN(LoweredValInfo());
        }

    }

    LoweredValInfo extractField(
        LoweredTypeInfo         fieldType,
        LoweredValInfo          base,
        DeclRef<StructField>    field)
    {
        return Slang::extractField(context, getSimpleType(fieldType), base, field);
    }

    LoweredValInfo visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        return emitDeclRef(context, expr->declRef);
    }

    LoweredValInfo visitGenericAppExpr(GenericAppExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("generic application expression during code generation");
    }

    LoweredValInfo visitSharedTypeExpr(SharedTypeExpr* /*expr*/)
    {
        SLANG_UNIMPLEMENTED_X("shared type expression during code generation");
    }

    LoweredValInfo visitAssignExpr(AssignExpr* expr)
    {
        // Because our representation of lowered "values"
        // can encompass l-values explicitly, we can
        // lower assignment easily. We just lower the left-
        // and right-hand sides, and then peform an assignment
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
};

struct LValueExprLoweringVisitor : ExprLoweringVisitorBase<LValueExprLoweringVisitor>
{
    // When visiting a swizzle expression in an l-value context,
    // we need to construct a "sizzled l-value."
    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        auto irType = lowerSimpleType(context, expr->type);
        auto loweredBase = lowerRValueExpr(context, expr->base);

        RefPtr<SwizzledLValueInfo> swizzledLValue = new SwizzledLValueInfo();
        swizzledLValue->type = irType;
        swizzledLValue->base = loweredBase;

        UInt elementCount = (UInt)expr->elementCount;
        swizzledLValue->elementCount = elementCount;
        for (UInt ii = 0; ii < elementCount; ++ii)
        {
            swizzledLValue->elementIndices[ii] = (UInt) expr->elementIndices[ii];
        }

        context->shared->extValues.Add(swizzledLValue);
        return LoweredValInfo::swizzledLValue(swizzledLValue);
    }

};

struct RValueExprLoweringVisitor : ExprLoweringVisitorBase<RValueExprLoweringVisitor>
{
    // A swizzle in an r-value context can save time by just
    // emitting the swizzle instuctions directly.
    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        auto irType = lowerSimpleType(context, expr->type);
        auto irBase = getSimpleVal(context, lowerRValueExpr(context, expr->base));

        auto builder = getBuilder();

        auto irIntType = getIntType(context);

        UInt elementCount = (UInt)expr->elementCount;
        IRValue* irElementIndices[4];
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
        auto varType = varDecl->type;

        for (IntegerLiteralValue ii = rangeBeginVal; ii < rangeEndVal; ++ii)
        {
            auto constVal = getBuilder()->getIntValue(
                varType,
                ii);

            context->shared->declValues[varDecl] = LoweredValInfo::simple(constVal);

            lowerStmt(context, stmt->body);
        }
    }

    // Create a basic block in the current function,
    // so that it can be used for a label.
    IRBlock* createBlock()
    {
        return getBuilder()->createBlock();
    }

    // Insert a block at the current location (ending
    // the previous block with an unconditional jump
    // if needed).
    void insertBlock(IRBlock* block)
    {
        auto builder = getBuilder();

        auto prevBlock = builder->curBlock;
        auto parentFunc = prevBlock ? prevBlock->parentFunc : builder->curFunc;

        // If the previous block doesn't already have
        // a terminator instruction, then be sure to
        // emit a branch to the new block.
        if (prevBlock && !isTerminatorInst(prevBlock->lastInst))
        {
            builder->emitBranch(block);
        }

        parentFunc->addBlock(block);

        builder->curFunc = parentFunc;
        builder->curBlock = block;
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

    void visitIfStmt(IfStmt* stmt)
    {
        auto builder = getBuilder();

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
            builder->emitBranch(afterBlock);

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
        for(auto attr : stmt->GetModifiersOfType<HLSLUncheckedAttribute>())
        {
            // TODO: We should actually catch these attributes during
            // semantic checking, so that they have a strongly-typed
            // representation in the AST.
            if(getText(attr->getName()) == "unroll")
            {
                auto decoration = getBuilder()->addDecoration<IRLoopControlDecoration>(inst);
                decoration->mode = kIRLoopControl_Unroll;
            }
        }
    }

    void visitForStmt(ForStmt* stmt)
    {
        auto builder = getBuilder();

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
        builder->emitBranch(loopHead);

        // Finally we insert the label that a `break` will jump to
        insertBlock(breakLabel);
    }

    void visitWhileStmt(WhileStmt* stmt)
    {
        // Generating IR for `while` statement is similar to a
        // `for` statement, but without a lot of the complications.

        auto builder = getBuilder();

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
        builder->emitBranch(loopHead);

        // Finally we insert the label that a `break` will jump to
        insertBlock(breakLabel);
    }

    void visitDoWhileStmt(DoWhileStmt* stmt)
    {
        // Generating IR for `do {...} while` statement is similar to a
        // `while` statement, just with the test in a different place

        auto builder = getBuilder();

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

    void visitDiscardStmt(DiscardStmt* /*stmt*/)
    {
        getBuilder()->emitDiscard();
    }

    void visitBreakStmt(BreakStmt* stmt)
    {
        // Semantic checking is responsible for finding
        // the statement taht this `break` breaks out of
        auto parentStmt = stmt->parentStmt;
        SLANG_ASSERT(parentStmt);

        // We just need to look up the basic block that
        // corresponds to the break label for that statement,
        // and then emit an instruction to jump to it.
        IRBlock* targetBlock;
        context->shared->breakLabels.TryGetValue(parentStmt, targetBlock);
        SLANG_ASSERT(targetBlock);
        getBuilder()->emitBreak(targetBlock);
    }

    void visitContinueStmt(ContinueStmt* stmt)
    {
        // Semantic checking is responsible for finding
        // the loop that this `continue` statement continues
        auto parentStmt = stmt->parentStmt;
        SLANG_ASSERT(parentStmt);


        // We just need to look up the basic block that
        // corresponds to the continue label for that statement,
        // and then emit an instruction to jump to it.
        IRBlock* targetBlock;
        context->shared->continueLabels.TryGetValue(parentStmt, targetBlock);
        SLANG_ASSERT(targetBlock);
        getBuilder()->emitContinue(targetBlock);
    }

    // Lowering a `switch` statement can get pretty involved,
    // so we need to track a bit of extra data:
    struct SwitchStmtInfo
    {
        // The label for the `default` case, if any.
        IRBlock*    defaultLabel = nullptr;

        // The label of the current "active" case block.
        IRBlock*    currentCaseLabel = nullptr;

        // Has anything been emitted to the current "active" case block?
        bool anythingEmittedToCurrentCaseBlock = false;

        // The collected (value, label) pairs for
        // all the `case` statements.
        List<IRValue*>  cases;
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
        // statements need to be direclty nested under the `switch`,
        // and so we can find them with a simpler walk.

        Stmt* stmt = inStmt;

        // Unwrap any surrounding `{ ... }` so we can look
        // at the statement inside.
        while(auto blockStmt = dynamic_cast<BlockStmt*>(stmt))
        {
            stmt = blockStmt->body;
            continue;
        }

        if(auto seqStmt = dynamic_cast<SeqStmt*>(stmt))
        {
            // Walk through teh children and process each.
            for(auto childStmt : seqStmt->stmts)
            {
                lowerSwitchCases(childStmt, info);
            }
        }
        else if(auto caseStmt = dynamic_cast<CaseStmt*>(stmt))
        {
            // A full `case` statement has a value we need
            // to test against. It is expected to be a
            // compile-time constant, so we will emit
            // it like an expression here, and then hope
            // for the best.
            //
            // TODO: figure out something cleaner.
            auto caseVal = getSimpleVal(context, lowerRValueExpr(context, caseStmt->expr));

            // Figure out where we are branching to.
            auto label = getLabelForCase(info);


            // Add this `case` to the list for the enclosing `switch`.
            info->cases.Add(caseVal);
            info->cases.Add(label);
        }
        else if(auto defaultStmt = dynamic_cast<DefaultStmt*>(stmt))
        {
            auto label = getLabelForCase(info);

            // We expect to only find a single `default` stmt.
            SLANG_ASSERT(!info->defaultLabel);

            info->defaultLabel = label;
        }
        else if(auto emptyStmt = dynamic_cast<EmptyStmt*>(stmt))
        {
            // Special-case empty statements so they don't
            // mess up our "trivial fall-through" optimization.
        }
        else
        {
            // We have an ordinary statement, that needs to get
            // emitted to the currrent case block.
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
        auto initialBlock = builder->curBlock;

        // Next, create a block to use as the target for any `break` statements
        auto breakLabel = createBlock();

        // Register the `break` label so
        // that we can find it for nested statements.
        context->shared->breakLabels.Add(stmt, breakLabel);

        builder->curFunc = initialBlock->parentFunc;
        builder->curBlock = nullptr;

        // Iterate over the body of the statement, looking
        // for `case` or `default` statements:
        SwitchStmtInfo info;
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
        if(builder->curBlock != initialBlock)
        {
            // Is the block already terminated?
            auto lastInst = builder->curBlock->lastInst;
            if(!lastInst || !isTerminatorInst(lastInst))
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
        builder->curBlock = initialBlock;
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
    }
};

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt)
{
    IRBuilderSourceLocRAII sourceLocInfo(context->irBuilder, stmt->loc);

    StmtLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(stmt);
}

static LoweredValInfo maybeMoveMutableTemp(
    IRGenContext*           context,
    LoweredValInfo const&   val)
{
    switch(val.flavor)
    {
    case LoweredValInfo::Flavor::Ptr:
        return val;

    default:
        {
            IRValue* irVal = getSimpleVal(context, val);
            auto type = irVal->getType();
            auto var = createVar(context, type);

            assign(context, var, LoweredValInfo::simple(irVal));
            return var;
        }
        break;
    }
}

void assign(
    IRGenContext*           context,
    LoweredValInfo const&   inLeft,
    LoweredValInfo const&   inRight)
{
    LoweredValInfo left = inLeft;
    LoweredValInfo right = inRight;

    auto builder = context->irBuilder;

top:
    switch (left.flavor)
    {
    case LoweredValInfo::Flavor::Ptr:
        switch (right.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
        case LoweredValInfo::Flavor::Ptr:
        case LoweredValInfo::Flavor::SwizzledLValue:
        case LoweredValInfo::Flavor::BoundSubscript:
        case LoweredValInfo::Flavor::BoundMember:
            {
                builder->emitStore(
                    left.val,
                    getSimpleVal(context, right));
            }
            break;

        default:
            SLANG_UNIMPLEMENTED_X("assignment");
            break;
        }
        break;

    case LoweredValInfo::Flavor::SwizzledLValue:
        {
            // The `left` value is of the form `<someLValue>.<swizzleElements>`.
            //
            // We could conceivably define a custom "swizzled store" instruction
            // that would handle the common case where the base l-value is
            // a simple lvalue (`LowerdValInfo::Flavor::Ptr`):
            //
            //     float4 foo;
            //     foo.zxy = float3(...);
            //
            // However, this doesn't handle complex cases like the following:
            //
            //     RWStructureBuffer<float4> foo;
            //     ...
            //     foo[index].xzy = float3(...);
            //
            // In a case like that, we really need to lower through a temp:
            //
            //     float4 tmp = foo[index];
            //     tmp.xzy = float3(...);
            //     foo[index] = tmp;
            //
            // We want to handle the general case, we we might as well
            // try to handle everything uniformly.
            //
            auto swizzleInfo = left.getSwizzledLValueInfo();
            auto type = swizzleInfo->type;
            auto loweredBase = swizzleInfo->base;

            // Load from the base value:
            IRValue* irLeftVal = getSimpleVal(context, loweredBase);
            IRValue* irRightVal = getSimpleVal(context, right);

            // Now apply the swizzle
            IRInst* irSwizzled = builder->emitSwizzleSet(
                irLeftVal->getType(),
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

    case LoweredValInfo::Flavor::BoundSubscript:
        {
            // The `left` value refers to a subscript operation on
            // a resource type, bound to particular arguments, e.g.:
            // `someStructuredBuffer[index]`.
            //
            // When storing to such a value, we need to emit a call
            // to the appropriate builtin "setter" accessor.
            auto subscriptInfo = left.getBoundSubscriptInfo();
            auto type = subscriptInfo->type;

            // Search for an appropriate "setter" declaration
            auto setters = getMembersOfType<SetterDecl>(subscriptInfo->declRef);
            if (setters.Count())
            {
                auto allArgs = subscriptInfo->args;
                
                addArgs(context, &allArgs, right);

                emitCallToDeclRef(
                    context,
                    context->getSession()->getVoidType(),
                    *setters.begin(),
                    nullptr,
                    allArgs);
                return;
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
            if( auto fieldDeclRef = declRef.As<StructField>() )
            {
                // materialize the base value and move it into
                // a mutable temporary if needed
                auto baseVal = boundMemberInfo->base;
                auto tempVal = maybeMoveMutableTemp(context, materialize(context, baseVal));

                // extract the field l-value out of the temporary
                auto tempFieldVal = extractField(context, boundMemberInfo->type, tempVal, fieldDeclRef);

                // assign to the field of the temporary l-value
                assign(context, tempFieldVal, right);

                // write back the modified temporary to the base l-value
                assign(context, baseVal, tempVal);
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
    }

    LoweredValInfo visitDecl(Decl* /*decl*/)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }

    LoweredValInfo visitImportDecl(ImportDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitEmptyDecl(EmptyDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitTypeDefDecl(TypeDefDecl * decl)
    {
        return LoweredValInfo::simple(context->irBuilder->getTypeVal(decl->type.type));
    }

    LoweredValInfo visitGenericTypeParamDecl(GenericTypeParamDecl* /*decl*/)
    {
        return LoweredValInfo();
    }

    LoweredValInfo visitInheritanceDecl(InheritanceDecl* inheritanceDecl)
    {
        // Construct a type for the parent declaration.
        //
        // TODO: if this inheritance declaration is under an extension,
        // then we should construct the type that is being extended,
        // and not a reference to the extension itself.
        auto parentDecl = inheritanceDecl->ParentDecl;
        RefPtr<Type> type = DeclRefType::Create(
            context->getSession(),
            makeDeclRef(parentDecl));


        // TODO: if the parent type is generic, then I suppose these
        // need to be *generic* witness tables?

        // What is the super-type that we have declared we inherit from?
        RefPtr<Type> superType = inheritanceDecl->base.type;

        // Construct the mangled name for the witness table, which depends
        // on the type that is conforming, and the type that it conforms to.
        String mangledName = getMangledNameForConformanceWitness(
            type,
            superType);

        // Build an IR level witness table, which will represent the
        // conformance of the type to its super-type.
        auto witnessTable = context->irBuilder->createWitnessTable();
        witnessTable->mangledName = mangledName;

        // Register the value now, rather than later, to avoid
        // infinite recursion.
        context->shared->declValues[inheritanceDecl] = LoweredValInfo::simple(witnessTable);


        // Semantic checking will have filled in a dictionary of
        // witnesses for requirements in the interface, and we
        // will now navigate that dictionary to fill in the witness table.
        for (auto entry : inheritanceDecl->requirementWitnesses)
        {
            auto requiredMemberDeclRef = entry.Key;
            auto satisfyingMemberDecl = entry.Value;

            auto irRequirement = context->irBuilder->getDeclRefVal(requiredMemberDeclRef);
            auto irSatisfyingVal = getSimpleVal(context, ensureDecl(context, satisfyingMemberDecl));

            context->irBuilder->createWitnessTableEntry(
                witnessTable,
                irRequirement,
                irSatisfyingVal);
        }

        witnessTable->moveToEnd();

        // A direct reference to this inheritance relationship (e.g.,
        // as a subtype witness) will take the form of a reference to
        // the witness table in the IR.
        return LoweredValInfo::simple(witnessTable);
    }


    LoweredValInfo visitDeclGroup(DeclGroup* declGroup)
    {
        // To lowere a group of declarations, we just
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

    bool isGlobalVarDecl(VarDeclBase* decl)
    {
        auto parent = decl->ParentDecl;
        if (dynamic_cast<ModuleDecl*>(parent))
        {
            // Variable declared at global scope? -> Global.
            return true;
        }

        return false;
    }

    LoweredValInfo lowerGlobalVarDecl(VarDeclBase* decl)
    {
        RefPtr<Type> varType = lowerSimpleType(context, decl->getType());

        if (decl->HasModifier<HLSLGroupSharedModifier>())
        {
            varType = context->getSession()->getGroupSharedType(varType);
        }
        // TODO: There might be other cases of storage qualifiers
        // that should translate into "rate-qualified" types
        // for the variable's storage.
        //
        // TODO: Also worth asking whether we should have semantic
        // checking be responsible for applying qualifiers applied
        // to a variable over to its type, when it makes sense.

        auto builder = getBuilder();
        auto irGlobal = builder->createGlobalVar(varType);
        irGlobal->mangledName = getMangledName(decl);

        if (decl)
        {
            builder->addHighLevelDeclDecoration(irGlobal, decl);
        }

        // A global variable's SSA value is a *pointer* to
        // the underlying storage.
        auto globalVal = LoweredValInfo::ptr(irGlobal);
        context->shared->declValues[
            DeclRef<VarDeclBase>(decl, nullptr)] = globalVal;

        if (isImportedDecl(decl))
        {
            // Always emit imported declarations as declarations,
            // and not definitions.
        }
        else if( auto initExpr = decl->initExpr )
        {
            IRBuilder subBuilderStorage = *getBuilder();
            IRBuilder* subBuilder = &subBuilderStorage;

            subBuilder->curFunc = irGlobal;

            IRGenContext subContextStorage = *context;
            IRGenContext* subContext = &subContextStorage;

            subContext->irBuilder = subBuilder;

            // TODO: set up a parent IR decl to put the instructions into

            IRBlock* entryBlock = subBuilder->emitBlock();
            subBuilder->curBlock = entryBlock;

            LoweredValInfo initVal = lowerLValueExpr(subContext, initExpr);
            subContext->irBuilder->emitReturn(getSimpleVal(subContext, initVal));
        }

        return globalVal;
    }

    LoweredValInfo visitVarDeclBase(VarDeclBase* decl)
    {
        // Detect global (or effectively global) variables
        // and handle them differently.
        if (isGlobalVarDecl(decl))
        {
            return lowerGlobalVarDecl(decl);
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

        RefPtr<Type> varType = lowerSimpleType(context, decl->getType());

        // TODO: If the variable is marked `static` then we need to
        // deal with it specially: we should move its allocation out
        // to the global scope, and then we have to deal with its
        // initializer expression a bit carefully (it should only
        // be initialized on-demand at its first use).

        // Some qualifiers on a variable will change how we allocate it,
        // so we need to reflect that somehow. The first example
        // we run into is the `groupshared` qualifier, which marks
        // a variable in a compute shader as having per-group allocation
        // rather than the traditional per-thread (or rather per-thread
        // per-activation-record) allocation.
        //
        // Options include:
        //
        //  - Use a distinct allocation opration, so that the type
        //    of the variable address/value is unchanged.
        //
        //  - Add a notion of an "address space" to pointer types,
        //    so that we can allocate things in distinct spaces.
        //
        //  - Add a notion of a "rate" so that we can declare a
        //    variable with a distinct rate.
        //
        // For now we might do the expedient thing and handle this
        // via a notion of an "address space."

        if (decl->HasModifier<HLSLGroupSharedModifier>())
        {
            // TODO: This logic is duplicated with the global-variable
            // case. We should seek to share it.
            varType = context->getSession()->getGroupSharedType(varType);
        }

        LoweredValInfo varVal = createVar(context, varType, decl);

        if( auto initExpr = decl->initExpr )
        {
            auto initVal = lowerRValueExpr(context, initExpr);

            assign(context, varVal, initVal);
        }

        context->shared->declValues[
            DeclRef<VarDeclBase>(decl, nullptr)] = varVal;

        return varVal;
    }

    LoweredValInfo visitAggTypeDecl(AggTypeDecl* decl)
    {
        // Given a declaration of a type, we need to make sure
        // to output "witness tables" for any interfaces this
        // type has declared conformance to.
        for( auto inheritanceDecl : decl->getMembersOfType<InheritanceDecl>() )
        {
            ensureDecl(context, inheritanceDecl);
        }

        // TODO: we currently store a Decl* in the witness table, which causes this function
        // being invoked to translate the witness table entry into an IRValue.
        // We should really allow a witness table entry to represent a type and not having to
        // construct the type here. The current implementation will not work when the struct type
        // is defined in a generic parent (we lose the environmental substitutions).
        return LoweredValInfo::simple(context->irBuilder->getTypeVal(DeclRefType::Create(context->getSession(), 
            DeclRef<Decl>(decl, nullptr))));
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
        return declRef.As<D>();
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
        kParameterDirection_In,
        kParameterDirection_Out,
        kParameterDirection_InOut,
    };
    struct ParameterInfo
    {
        // This AST-level type of the parameter
        Type*               type;

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
        List<Decl*>         genericParams;
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
    bool isMemberDeclarationEffectivelyStatic(
        Decl*           decl,
        ContainerDecl*  parentDecl)
    {
        // Anything explicitly marked `static` counts.
        //
        // There is a subtle detail here with a global-scope `static`
        // variable not really meaning `static` in the same way, but
        // it doesn't matter because the module shouldn't introduce
        // any parameters we care about.
        if(decl->HasModifier<HLSLStaticModifier>())
            return true;

        // Next we need to deal with cases where a declaration is
        // effectively `static` even if the language doesn't make
        // the user say so. Most languages make the default assumption
        // that nested types are `static` even if they don't say
        // so (Java is an exception here, perhaps due to some
        // includence from the Scandanavian OOP tradition).
        if(dynamic_cast<AggTypeDecl*>(decl))
            return true;

        // Things nested inside functions may have dependencies
        // on values from the enclosing scope, but this needs to
        // be dealt with via "capture" so they are also effectively
        // `static`
        if(dynamic_cast<FunctionDeclBase*>(parentDecl))
            return true;

        return false;
    }
    // We also need to be able to detect whether a declaration is
    // either explicitly or implicitly treated as `static`:
    ParameterListCollectMode getModeForCollectingParentParameters(
        Decl*           decl,
        ContainerDecl*  parentDecl)
    {
        // If we have a `static` parameter, then it is obvious
        // that we should use the `static` mode
        if(isMemberDeclarationEffectivelyStatic(decl, parentDecl))
            return kParameterListCollectMode_Static;

        // Otherwise, let's default to collecting everything
        return kParameterListCollectMode_Default;
    }
    //
    // When dealing with a member function, we need to be able to add the `this`
    // parameter for the enclosing type:
    //
    void addThisParameter(
        Type*               type,
        ParameterLists*     ioParameterLists)
    {
        // For now we make any `this` parameter default to `in`. Eventually
        // we should add a way for the user to opt in to mutability of `this`
        // in cases where they really want it.
        //
        // Note: an alternative here might be to have the built-in types like
        // `Texture2D` actually use `class` declarations and say that the
        // `this` parameter for a class type is always `in`, while `struct`
        // types can default to `in out`.
        ParameterDirection direction = kParameterDirection_In;

        ParameterInfo info;
        info.type = type;
        info.decl = nullptr;
        info.direction = direction;
        info.isThisParam = true;

        ioParameterLists->params.Add(info);
    }
    void addThisParameter(
        AggTypeDecl*        typeDecl,
        ParameterLists*     ioParameterLists)
    {
        // We need to construct an appopriate declaration-reference
        // for the type declaration we were given. In particular,
        // we need to specialize it for any generic parameters
        // that are in scope here.
        auto declRef = createDefaultSpecializedDeclRef(typeDecl);
        auto type = DeclRefType::Create(context->getSession(), declRef);
        addThisParameter(
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
                if( auto aggTypeDecl = dynamic_cast<AggTypeDecl*>(parentDecl) )
                {
                    addThisParameter(aggTypeDecl, ioParameterLists);
                }
                else if( auto extensionDecl = dynamic_cast<ExtensionDecl*>(parentDecl) )
                {
                    addThisParameter(extensionDecl->targetType, ioParameterLists);
                }
            }
        }

        // Once we've added any parameters based on parent declarations,
        // we can see if this declaration itself introduces parameters.
        //
        if( auto callableDecl = dynamic_cast<CallableDecl*>(decl) )
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
        else if( auto genericDecl = dynamic_cast<GenericDecl*>(decl) )
        {
            for( auto memberDecl : genericDecl->Members )
            {
                if( auto genericTypeParamDecl = memberDecl.As<GenericTypeParamDecl>() )
                {
                    ioParameterLists->genericParams.Add(genericTypeParamDecl);
                }
                else if( auto genericValueParamDecl = memberDecl.As<GenericValueParamDecl>() )
                {
                    ioParameterLists->genericParams.Add(genericValueParamDecl);
                }
                else if( auto genericConstraintDel = memberDecl.As<GenericTypeConstraintDecl>() )
                {
                    // When lowering to the IR we need to reify the constraints on
                    // a generic parameter as concrete parameters of their own.
                    // These parameter will usually be satisfied by passing a "witness"
                    // as the argument to correspond to the parameter.
                    //
                    // TODO: it is possible that all witness parameters should come
                    // after the other generic parameters, and thus should be collected
                    // in a third list.
                    //
                    ioParameterLists->genericParams.Add(genericConstraintDel);
                }
            }
        }

    }

    void trySetMangledName(
        IRFunc* irFunc,
        Decl*   decl)
    {
        // We want to generate a mangled name for the given declaration and attach
        // it to the instruction.
        //
        // TODO: we probably want to start be doing an early-exit in cases
        // where it doesn't make sense to attach a mangled name (e.g., because
        // the declaration in question shouldn't have linkage).
        //

        String mangledName = getMangledName(decl);

        irFunc->mangledName = mangledName;
    }

    ModuleDecl* findModuleDecl(Decl* decl)
    {
        for (auto dd = decl; dd; dd = dd->ParentDecl)
        {
            if (auto moduleDecl = dynamic_cast<ModuleDecl*>(dd))
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

    bool isImportedDecl(Decl* decl)
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

        if (moduleDecl != this->context->shared->mainModuleDecl)
            return true;

        return false;
    }

    LoweredValInfo lowerFuncDecl(FunctionDeclBase* decl)
    {
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
        if (auto accessorDecl = dynamic_cast<AccessorDecl*>(decl))
        {
            // We are some kind of accessor, so the parent declaration should
            // know the correct return type to expose.
            //
            auto parentDecl = accessorDecl->ParentDecl;
            if (auto subscriptDecl = dynamic_cast<SubscriptDecl*>(parentDecl))
            {
                declForReturnType = subscriptDecl;
            }
        }


        IRBuilder subBuilderStorage = *getBuilder();
        IRBuilder* subBuilder = &subBuilderStorage;

        IRGenContext subContextStorage = *context;
        IRGenContext* subContext = &subContextStorage;
        subContext->irBuilder = subBuilder;

        // need to create an IR function here

        IRFunc* irFunc = subBuilder->createFunc();
        subBuilder->curFunc = irFunc;

        trySetMangledName(irFunc, decl);

        List<RefPtr<Type>> paramTypes;

        // We first need to walk the generic parameters (if any)
        // because these will influence the declared type of
        // the function.

        for(auto pp = decl->ParentDecl; pp; pp = pp->ParentDecl)
        {
            if(auto genericAncestor = dynamic_cast<GenericDecl*>(pp))
            {
                irFunc->genericDecl = genericAncestor;
                break;
            }
        }

        for( auto paramInfo : parameterLists.params )
        {
            RefPtr<Type> irParamType = lowerSimpleType(context, paramInfo.type);

            switch( paramInfo.direction )
            {
            case kParameterDirection_In:
                // Simple case of a by-value input parameter.
                paramTypes.Add(irParamType);
                break;

            // If the parameter is declared `out` or `inout`,
            // then we will represent it with a pointer type in
            // the IR, but we will use a specialized pointer
            // type that encodes the parameter direction information.
            case kParameterDirection_Out:
                paramTypes.Add(
                    context->getSession()->getOutType(irParamType));
                break;
            case kParameterDirection_InOut:
                paramTypes.Add(
                    context->getSession()->getInOutType(irParamType));
                break;

            default:
                SLANG_UNEXPECTED("unknown parameter direction");
                break;
            }
        }

        auto irResultType = lowerSimpleType(context, declForReturnType->ReturnType);

        if (auto setterDecl = dynamic_cast<SetterDecl*>(decl))
        {
            // We are lowering a "setter" accessor inside a subscript
            // declaration, which means we don't want to *return* the
            // stated return type of the subscript, but instead take
            // it as a parameter.
            //
            IRType* irParamType = irResultType;
            paramTypes.Add(irParamType);
            subBuilder->emitParam(irParamType);

            // TODO: we need some way to wire this up to the `newValue`
            // or whatever name we give for that parameter inside
            // the setter body.

            // Instead, a setter always returns `void`
            //
            irResultType = context->getSession()->getVoidType();
        }

        if( auto refAccessorDecl = dynamic_cast<RefAccessorDecl*>(decl) )
        {
            // A `ref` accessor needs to return a *pointer* to the value
            // being accessed, rather than a simple value.
            irResultType = context->getSession()->getPtrType(irResultType);
        }

        auto irFuncType = getFuncType(
            context,
            paramTypes.Count(),
            paramTypes.Buffer(),
            irResultType);
        irFunc->type = irFuncType;

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
            subBuilder->curBlock = entryBlock;

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

                        auto irPtrType = irParamType.As<PtrTypeBase>();

                        IRParam* irParamPtr = subBuilder->emitParam(irPtrType);
                        if(auto paramDecl = paramInfo.decl)
                            subBuilder->addHighLevelDeclDecoration(irParamPtr, paramDecl);

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
                        // But note that HLSL allows an input parameter
                        // to be used as a local variable inside of a
                        // function body, so we need to introduce a temporary
                        // and then copy over to it...
                        //
                        // TODO: we could skip this step if we knew
                        // the parameter was marked `const` or similar.

                        paramTypes.Add(irParamType);

                        IRParam* irParam = subBuilder->emitParam(irParamType);
                        if(auto paramDecl = paramInfo.decl)
                            subBuilder->addHighLevelDeclDecoration(irParam, paramDecl);
                        paramVal = LoweredValInfo::simple(irParam);

                        auto irLocal = subBuilder->emitVar(irParamType);
                        auto localVal = LoweredValInfo::ptr(irLocal);

                        assign(subContext, localVal, paramVal);

                        paramVal = localVal;
                    }
                    break;
                }

                if( auto paramDecl = paramInfo.decl )
                {
                    DeclRef<VarDeclBase> paramDeclRef = makeDeclRef(paramDecl);
                    subContext->shared->declValues[paramDeclRef] = paramVal;
                }

                if (paramInfo.isThisParam)
                {
                    subContext->thisVal = paramVal;
                }
            }

            lowerStmt(subContext, decl->Body);

            // We need to carefully add a terminator instruction to the end
            // of the body, in case the user didn't do so.
            if (!isTerminatorInst(subContext->irBuilder->curBlock->lastInst))
            {
                if (irResultType->Equals(context->getSession()->getVoidType()))
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
                    subContext->irBuilder->emitUnreachable();
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

            auto decoration = getBuilder()->addDecoration<IRTargetDecoration>(irFunc);
            decoration->targetName = targetMod->targetToken.Content;
        }

        // For convenience, ensure that any additional global
        // values that were emitted while outputting the function
        // body appear before the function itself in the list
        // of global values.
        irFunc->moveToEnd();

        return LoweredValInfo::simple(irFunc);
    }

    LoweredValInfo visitGenericDecl(GenericDecl * genDecl)
    {
        if (auto innerFuncDecl = genDecl->inner->As<FuncDecl>())
            return lowerFuncDecl(innerFuncDecl);
        else if (auto innerStructDecl = genDecl->inner->As<StructDecl>())
            return LoweredValInfo();
        SLANG_RELEASE_ASSERT(false);
        UNREACHABLE_RETURN(LoweredValInfo());
    }

    LoweredValInfo visitFunctionDeclBase(FunctionDeclBase* decl)
    {
        // A function declaration may have multiple, target-specific
        // overloads, and we need to emit an IR version of each of these.

        // The front end will form a linked list of declaratiosn with
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

        auto primaryFuncDecl = dynamic_cast<FunctionDeclBase*>(primaryDecl);
        assert(primaryFuncDecl);
        LoweredValInfo result = lowerFuncDecl(primaryFuncDecl);
        for (auto dd = primaryDecl->nextDecl; dd; dd = dd->nextDecl)
        {
            auto funcDecl = dynamic_cast<FunctionDeclBase*>(dd);
            assert(funcDecl);
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
    return visitor.dispatch(decl);
}

// Ensure that a version of the given declaration has been emitted to the IR
LoweredValInfo ensureDecl(
    IRGenContext*   context,
    Decl*           decl)
{
    auto shared = context->shared;

    LoweredValInfo result;
    if(shared->declValues.TryGetValue(decl, result))
        return result;

    IRBuilder subIRBuilder;
    subIRBuilder.sharedBuilder = context->irBuilder->sharedBuilder;

    IRGenContext subContext = *context;

    subContext.irBuilder = &subIRBuilder;

    result = lowerDecl(&subContext, decl);

    shared->declValues[decl] = result;

    return result;
}

IRWitnessTable* findWitnessTable(
    IRGenContext*   context,
    DeclRef<Decl>   declRef)
{
    IRValue* irVal = getSimpleVal(context, emitDeclRef(context, declRef));
    if (!irVal)
    {
        SLANG_UNEXPECTED("expected a witness table");
        return nullptr;
    }

    if (irVal->op != kIROp_witness_table)
    {
        // TODO: We might eventually have cases of `specialize` called
        // on a witness table...
        SLANG_UNEXPECTED("expected a witness table");
        return nullptr;
    }

    return (IRWitnessTable*)irVal;
}

RefPtr<Val> lowerSubstitutionArg(
    IRGenContext*   context,
    Val*            val)
{
    if (auto type = dynamic_cast<Type*>(val))
    {
        return lowerSimpleType(context, type);
    }
    else if (auto declaredSubtypeWitness = dynamic_cast<DeclaredSubtypeWitness*>(val))
    {
        // We do not have a concrete witness table yet for a GenericTypeConstraintDecl witness

        if (declaredSubtypeWitness->declRef.As<GenericTypeConstraintDecl>())
            return val;

        // We need to look up the IR-level representation of the witness
        // (which is a witness table).

        auto irWitnessTable = findWitnessTable(context, declaredSubtypeWitness->declRef);

        // We have an IR-level value, but we need to embed it into an AST-level
        // type, so we will use a proxy `Val` that wraps up an `IRValue` as
        // an AST-level value.
        //
        // TODO: This proxy value currently doesn't enter into use-def chaining,
        // and so Bad Things could happen quite easily. We need to fix that
        // up in a reasonably clean fashion.
        //
        RefPtr<IRProxyVal> proxyVal = new IRProxyVal();
        proxyVal->inst = irWitnessTable;
        return proxyVal;
    }
    else
    {
        // For now, jsut assume that all other values
        // lower to themselves.
        //
        // TODO: we should probably handle the case of
        // a `Val` that references an AST-level `constexpr`
        // variable, since that would need to be lowered
        // to a `Val` that references the IR equivalent.
        return val;
    }
}

// Given a set of substitutions, make sure that we have
// lowered the arguments being used into a form that
// is suitable for use in the IR.
RefPtr<Substitutions> lowerSubstitutions(
    IRGenContext*   context,
    Substitutions*  subst)
{
    if(!subst)
        return nullptr;
    RefPtr<Substitutions> result;
    if (auto genSubst = dynamic_cast<GenericSubstitution*>(subst))
    {
        RefPtr<GenericSubstitution> newSubst = new GenericSubstitution();
        newSubst->genericDecl = genSubst->genericDecl;

        for (auto arg : genSubst->args)
        {
            auto newArg = lowerSubstitutionArg(context, arg);
            newSubst->args.Add(newArg);
        }

        result = newSubst;
    }
    else if (auto thisSubst = dynamic_cast<ThisTypeSubstitution*>(subst))
    {
        RefPtr<ThisTypeSubstitution> newSubst = new ThisTypeSubstitution();
        newSubst->sourceType = lowerSubstitutionArg(context, thisSubst->sourceType);
        result = newSubst;
    }
    if (subst->outer)
    {
        result->outer = lowerSubstitutions(
            context,
            subst->outer);
    }
    return result;
}

LoweredValInfo emitDeclRef(
    IRGenContext*   context,
    DeclRef<Decl>   declRef)
{
    // First we need to construct an IR value representing the
    // unspecialized declaration.
    LoweredValInfo loweredDecl = ensureDecl(context, declRef.getDecl());

    return maybeEmitSpecializeInst(context, loweredDecl, declRef);
}

LoweredValInfo maybeEmitSpecializeInst(IRGenContext*   context,
    LoweredValInfo loweredDecl,
    DeclRef<Decl>   declRef)
{
    // If this declaration reference doesn't involve any specializations,
    // then we are done at this point.
    if (!hasGenericSubstitutions(declRef.substitutions))
        return loweredDecl;

    // There's no reason to specialize something that maps to a NULL pointer.
    if (loweredDecl.flavor == LoweredValInfo::Flavor::None)
        return loweredDecl;

    auto val = getSimpleVal(context, loweredDecl);

    // We have the "raw" substitutions from the AST, but we may
    // need to walk through those and replace things in
    // cases where the `Val`s used for substitution should
    // lower to something other than their original form.
    RefPtr<Substitutions> newSubst = lowerSubstitutions(context, declRef.substitutions);
    declRef.substitutions = newSubst;


    RefPtr<Type> type;
    if (auto declType = val->getType())
    {
        type = declType->Substitute(declRef.substitutions).As<Type>();
    }

    // Otherwise, we need to construct a specialization of the
    // given declaration.
    return LoweredValInfo::simple(context->irBuilder->emitSpecializeInst(
        type,
        val,
        declRef));
}


static void lowerEntryPointToIR(
    IRGenContext*       context,
    EntryPointRequest*  entryPointRequest)
{
    // First, lower the entry point like an ordinary function
    auto entryPointFuncDecl = entryPointRequest->decl;
    if (!entryPointFuncDecl)
    {
        // Something must have gone wrong earlier, if we
        // weren't able to associate a declaration with
        // the entry point request.
        return;
    }
    // we need to lower all global type arguments as well
    for (auto arg : entryPointRequest->genericParameterTypes)
        lowerType(context, arg);
    auto loweredEntryPointFunc = ensureDecl(context, entryPointFuncDecl);
}

#if 0
IRModule* lowerEntryPointToIR(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    SharedIRGenContext sharedContextStorage;
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    sharedContext->entryPoint       = entryPoint;
    sharedContext->programLayout    = programLayout;
    sharedContext->target           = target;

    IRGenContext contextStorage;
    IRGenContext* context = &contextStorage;

    context->shared = sharedContext;

    SharedIRBuilder sharedBuilderStorage;
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = entryPoint->compileRequest->mSession;

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->shared = sharedBuilder;

    IRModule* module = builder->createModule();
    sharedBuilder->module = module;

    context->irBuilder = builder;

    auto entryPointLayout = findEntryPointLayout(sharedContext, entryPoint);

    lowerEntryPointToIR(context, entryPoint, entryPointLayout);

    return module;

}
#endif

IRModule* generateIRForTranslationUnit(
    TranslationUnitRequest* translationUnit)
{
    // If the user did not opt into IR usage, then don't compile IR
    // for the translation unit.
    if (!(translationUnit->compileFlags & SLANG_COMPILE_FLAG_USE_IR))
        return nullptr;

    auto compileRequest = translationUnit->compileRequest;

    SharedIRGenContext sharedContextStorage;
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    sharedContext->compileRequest = compileRequest;
    sharedContext->mainModuleDecl = translationUnit->SyntaxNode;

    IRGenContext contextStorage;
    IRGenContext* context = &contextStorage;

    context->shared = sharedContext;

    SharedIRBuilder sharedBuilderStorage;
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = compileRequest->mSession;

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
        lowerEntryPointToIR(context, entryPoint);
    }
    //
    // Next, ensure that all other global declarations have
    // been emitted.
    for (auto decl : translationUnit->SyntaxNode->Members)
    {
        ensureDecl(context, decl);
    }

    // If we are being sked to dump IR during compilation,
    // then we can dump the initial IR for the module here.
    if(compileRequest->shouldDumpIR)
    {
        dumpIR(module);
    }

    return module;
}

#if 0
String emitSlangIRAssemblyForEntryPoint(
    EntryPointRequest*  entryPoint)
{
    auto compileRequest = entryPoint->compileRequest;
    auto irModule = lowerEntryPointToIR(
        entryPoint,
        compileRequest->layout.Ptr(),
        // TODO: we need to pick the target more carefully here
        CodeGenTarget::HLSL);

    return getSlangIRAssembly(irModule);
}
#endif


} // namespace Slang
