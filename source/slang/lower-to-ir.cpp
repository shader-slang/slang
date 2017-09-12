// lower.cpp
#include "lower-to-ir.h"

#include "ir.h"
#include "ir-insts.h"
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
};

// Represents the result of a swizzle operation in
// an l-value context. A swizzle without duplicate
// elements is allowed as an l-value, even if the
// element are non-contiguous (`.xz`) or out of
// order (`.zxy`).
//
struct SwizzledLValueInfo : ExtendedValueInfo
{
    // IR-level The type of the expression.
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

struct SharedIRGenContext
{
    EntryPointRequest*  entryPoint;
    ProgramLayout*      programLayout;
    CodeGenTarget       target;

    Dictionary<DeclRef<Decl>, LoweredValInfo> declValues;

    // Arrays we keep around strictly for memory-management purposes:

    // Any extended values created during lowering need
    // to be cleaned up after the fact. We don't try
    // to reference-count these along the way because
    // they need to get stored into a `union` inside `LoweredValInfo`
    List<RefPtr<ExtendedValueInfo>> extValues;
};


struct IRGenContext
{
    SharedIRGenContext* shared;

    IRBuilder* irBuilder;
};

LoweredValInfo ensureDecl(
    IRGenContext*           context,
    DeclRef<Decl> const&    declRef);


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

    assert(argCount == 2);
    auto leftPtr = args[0];
    auto rightVal = args[1];

    auto leftVal = builder->emitLoad(leftPtr);

    IRInst* innerArgs[] = { leftVal, rightVal };
    auto innerOp = builder->emitIntrinsicInst(type, op, 2, innerArgs);

    builder->emitStore(leftPtr, innerOp);

    return LoweredValInfo::ptr(leftPtr);
}

// Given a `DeclRef` for something callable, along with a bunch of
// arguments, emit an appropriate call to it.
LoweredValInfo emitCallToDeclRef(
    IRGenContext*   context,
    IRType*         type,
    DeclRef<Decl>   funcDeclRef,
    UInt            argCount,
    IRValue* const* args)
{
    auto builder = context->irBuilder;


    if (auto subscriptDeclRef = funcDeclRef.As<SubscriptDecl>())
    {
        // A reference to a subscript declaration is potentially a
        // special case, if we have more than just a getter.

        DeclRef<GetterDecl> getterDeclRef;
        bool justAGetter = true;
        for (auto accessorDeclRef : getMembersOfType<AccessorDecl>(subscriptDeclRef))
        {
            if (auto foundGetterDeclRef = accessorDeclRef.As<GetterDecl>())
            {
                getterDeclRef = foundGetterDeclRef;
            }
            else
            {
                justAGetter = false;
                break;
            }
        }

        if (!justAGetter)
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
        if (getterDeclRef)
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
        return LoweredValInfo::simple(builder->emitConstructorInst(type, argCount, args));
    }

    // Fallback case is to emit an actual call.
    LoweredValInfo funcVal = ensureDecl(context, funcDeclRef);
    return emitCallToVal(context, type, funcVal, argCount, args);
}

LoweredValInfo emitCallToDeclRef(
    IRGenContext*           context,
    IRType*                 type,
    DeclRef<Decl>           funcDeclRef,
    List<IRValue*> const&   args)
{
    return emitCallToDeclRef(context, type, funcDeclRef, args.Count(), args.Buffer());
}

IRValue* getSimpleVal(IRGenContext* context, LoweredValInfo lowered)
{
    auto builder = context->irBuilder;

top:
    switch(lowered.flavor)
    {
    case LoweredValInfo::Flavor::None:
        return nullptr;

    case LoweredValInfo::Flavor::Simple:
        return lowered.val;

    case LoweredValInfo::Flavor::Ptr:
        return builder->emitLoad(lowered.val);

    case LoweredValInfo::Flavor::BoundSubscript:
        {
            auto boundSubscriptInfo = lowered.getBoundSubscriptInfo();

            for (auto getter : getMembersOfType<GetterDecl>(boundSubscriptInfo->declRef))
            {
                lowered = emitCallToDeclRef(
                    context,
                    boundSubscriptInfo->type,
                    getter,
                    boundSubscriptInfo->args);
                goto top;
            }

            SLANG_UNEXPECTED("subscript had no getter");
            return nullptr;
        }
        break;

    case LoweredValInfo::Flavor::SwizzledLValue:
        {
            auto swizzleInfo = lowered.getSwizzledLValueInfo();
            
            return builder->emitSwizzle(
                swizzleInfo->type,
                getSimpleVal(context, swizzleInfo->base),
                swizzleInfo->elementCount,
                swizzleInfo->elementIndices);
        }

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        return nullptr;
    }
}

struct LoweredTypeInfo
{
    enum class Flavor
    {
        None,
        Simple,
    };

    union
    {
        IRType* type;
    };
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

IRType* getSimpleType(LoweredTypeInfo lowered)
{
    switch(lowered.flavor)
    {
    case LoweredTypeInfo::Flavor::None:
        return nullptr;

    case LoweredTypeInfo::Flavor::Simple:
        return lowered.type;

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        return nullptr;
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
IRType* lowerSimpleType(
    IRGenContext*   context,
    Type*           type)
{
    auto lowered = lowerType(context, type);
    return getSimpleType(lowered);
}

IRType* lowerSimpleType(
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
    DeclBase*       decl,
    Layout*         layout);

//

struct ValLoweringVisitor : ValVisitor<ValLoweringVisitor, LoweredValInfo, LoweredTypeInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    LoweredValInfo visitVal(Val* val)
    {
        SLANG_UNIMPLEMENTED_X("value lowering");
    }

    LoweredValInfo visitConstantIntVal(ConstantIntVal* val)
    {
        // TODO: it is a bit messy here that the `ConstantIntVal` representation
        // has no notion of a *type* associated with the value...

        auto type = getBuilder()->getBaseType(BaseType::Int);
        return LoweredValInfo::simple(getBuilder()->getIntValue(type, val->value));
    }

    LoweredTypeInfo visitType(Type* type)
    {
        SLANG_UNIMPLEMENTED_X("type lowering");
    }

    LoweredTypeInfo visitFuncType(FuncType* type)
    {
        LoweredValInfo loweredFunc = ensureDecl(context, type->declRef);
        auto loweredFuncVal = getSimpleVal(context, loweredFunc);

        // HACK: deal with the case where the decl might not
        // lower to anything, and so we don't have a type to
        // work with.
        if (!loweredFuncVal)
            return LoweredTypeInfo();

        return loweredFuncVal->getType();
    }

    void addGenericArgs(List<IRValue*>* ioArgs, DeclRefBase declRef)
    {
        auto subs = declRef.substitutions;
        while(subs)
        {
            for(auto aa : subs->args)
            {
                (*ioArgs).Add(getSimpleVal(context, lowerVal(context, aa)));
            }
            subs = subs->outer;
        }
    }

    LoweredTypeInfo visitDeclRefType(DeclRefType* type)
    {
        // We need to detect builtin/intrinsic types here, since they should map to custom modifiers
        // We need to catch builtin/intrinsic types here
        if( auto intrinsicTypeMod = type->declRef.getDecl()->FindModifier<IntrinsicTypeModifier>() )
        {
            auto builder = getBuilder();
            auto intType = builder->getBaseType(BaseType::Int);
            //
            List<IRValue*> irArgs;
            for( auto val : intrinsicTypeMod->irOperands )
            {
                irArgs.Add(builder->getIntValue(intType, val));
            }

            addGenericArgs(&irArgs, type->declRef);

            auto irType = getBuilder()->getIntrinsicType(IROp(intrinsicTypeMod->irOp), irArgs.Count(), irArgs.Buffer());
            return LoweredTypeInfo(irType);
        }

        // Catch-all for user-defined type references
        LoweredValInfo loweredDeclRef = ensureDecl(context, type->declRef);

        // TODO: make sure that the value is actually a type...

        switch (loweredDeclRef.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
            return LoweredTypeInfo((IRType*)loweredDeclRef.val);

        default:
            SLANG_UNIMPLEMENTED_X("type lowering");
        }

    }

    LoweredTypeInfo visitBasicExpressionType(BasicExpressionType* type)
    {
        return getBuilder()->getBaseType(type->BaseType);
    }

    LoweredTypeInfo visitVectorExpressionType(VectorExpressionType* type)
    {
        auto irElementType = lowerSimpleType(context, type->elementType);
        auto irElementCount = lowerSimpleVal(context, type->elementCount);

        return getBuilder()->getVectorType(irElementType, irElementCount);
    }

    LoweredTypeInfo visitMatrixExpressionType(MatrixExpressionType* type)
    {
        auto irElementType = lowerSimpleType(context, type->getElementType());
        auto irRowCount = lowerSimpleVal(context, type->getRowCount());
        auto irColumnCount = lowerSimpleVal(context, type->getColumnCount());

        return getBuilder()->getMatrixType(irElementType, irRowCount, irColumnCount);
    }

    LoweredTypeInfo getArrayType(
        LoweredTypeInfo const&  loweredElementType,
        IRValue*                irElementCount)
    {
        switch (loweredElementType.flavor)
        {
        case LoweredTypeInfo::Flavor::Simple:
            return getBuilder()->getArrayType(
                loweredElementType.type,
                irElementCount);
            break;

        default:
            SLANG_UNEXPECTED("array element type");
            break;
        }
    }

    LoweredTypeInfo visitArrayExpressionType(ArrayExpressionType* type)
    {
        auto loweredElementType = lowerType(context, type->BaseType);
        if (auto elementCount = type->ArrayLength)
        {
            auto irElementCount = lowerSimpleVal(context, elementCount);
            return getArrayType(loweredElementType, irElementCount);
        }
        else
        {
            return getArrayType(loweredElementType, nullptr);
        }
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

#if 0
struct LoweringVisitor
    : ExprVisitor<LoweringVisitor, LoweredExpr>
    , StmtVisitor<LoweringVisitor, void>
    , DeclVisitor<LoweringVisitor, LoweredDecl>
    , ValVisitor<LoweringVisitor, RefPtr<Val>, RefPtr<Type>>
#endif

LoweredValInfo createVar(
    IRGenContext*   context,
    LoweredTypeInfo type,
    Decl*           decl = nullptr,
    Layout*         layout = nullptr,
    IRAddressSpace  addressSpace = kIRAddressSpace_Default)
{
    auto builder = context->irBuilder;
    switch( type.flavor )
    {
    case LoweredTypeInfo::Flavor::Simple:
        {
            auto irAlloc = builder->emitVar(getSimpleType(type), addressSpace);

            if (decl)
            {
                builder->addHighLevelDeclDecoration(irAlloc, decl);
            }

            if (layout)
            {
                builder->addLayoutDecoration(irAlloc, layout);
            }


            return LoweredValInfo::ptr(irAlloc);
        }
        break;

    default:
        SLANG_UNIMPLEMENTED_X("var type");
        return LoweredValInfo();
    }

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
        return dispatch(expr);
    }


    LoweredValInfo visitVarExpr(VarExpr* expr)
    {
        LoweredValInfo info = ensureDecl(context, expr->declRef);
        return info;
    }

    LoweredValInfo visitOverloadedExpr(OverloadedExpr* expr)
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

    LoweredValInfo visitMemberExpr(MemberExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);
        auto loweredBase = lowerRValueExpr(context, expr->BaseExpression);

        auto declRef = expr->declRef;
        if (auto fieldDeclRef = declRef.As<StructField>())
        {
            // Okay, easy enough: we have a reference to a field of a struct type...

            auto loweredField = ensureDecl(context, fieldDeclRef);
            return extractField(loweredType, loweredBase, loweredField);
        }
        else if (auto callableDeclRef = declRef.As<CallableDecl>())
        {
            RefPtr<BoundMemberInfo> boundMemberInfo = new BoundMemberInfo();
            boundMemberInfo->base = loweredBase;
            boundMemberInfo->declRef = callableDeclRef;
            return LoweredValInfo::boundMember(boundMemberInfo);
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
        auto loweredBaseVal = getSimpleVal(context, loweredBase);
        auto loweredBaseType = loweredBaseVal->getType();
        switch( loweredBaseType->op )
        {
        case kIROp_PtrType:
        // TODO: should we enumerate these explicitly?
        case kIROp_ConstantBufferType:
        case kIROp_TextureBufferType:
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

        default:
            SLANG_UNIMPLEMENTED_X("codegen for deref expression");
            return LoweredValInfo();
        }
    }

    LoweredValInfo visitParenExpr(ParenExpr* expr)
    {
        return lowerSubExpr(expr->base);
    }

    LoweredValInfo visitInitializerListExpr(InitializerListExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for initializer list expression");
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

    LoweredValInfo visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for aggregate type constructor expression");
    }

    // Add arguments that appeared directly in an argument list
    // to the list of argument values for a call.
    void addDirectCallArgs(
        InvokeExpr*     expr,
        List<IRValue*>* ioArgs)
    {
        auto& irArgs = *ioArgs;

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
        auto funcDecl = funcDeclRef.getDecl();
        auto& args = expr->Arguments;
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
            auto paramType = lowerType(context, GetType(paramDeclRef));
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
            addDirectCallArgs(expr, ioArgs);
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
        if (auto memberFuncExpr = funcExpr.As<MemberExpr>())
        {
            auto loweredBaseVal = lowerRValueExpr(context, memberFuncExpr->BaseExpression);
            addArgs(context, &irArgs, loweredBaseVal);

            auto funcDeclRef = memberFuncExpr->declRef;

            addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
            auto result = emitCallToDeclRef(context, type, funcDeclRef, irArgs);
            applyOutArgumentFixups(argFixups);
            return result;
        }
        else if (auto staticMemberFuncExpr = funcExpr.As<StaticMemberExpr>())
        {
            auto funcDeclRef = staticMemberFuncExpr->declRef;
            addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
            auto result = emitCallToDeclRef(context, type, funcDeclRef, irArgs);
            applyOutArgumentFixups(argFixups);
            return result;
        }
        else if (auto varExpr = funcExpr.As<VarExpr>())
        {
            auto funcDeclRef = varExpr->declRef;
            addDirectCallArgs(expr, funcDeclRef, &irArgs, &argFixups);
            auto result = emitCallToDeclRef(context, type, funcDeclRef, irArgs);
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
                    builder->getPtrType(getSimpleType(type)),
                    baseVal.val,
                    indexVal));

        default:
            SLANG_UNIMPLEMENTED_X("subscript expr");
            return LoweredValInfo();
        }

    }

    LoweredValInfo extractField(
        LoweredTypeInfo fieldType,
        LoweredValInfo  base,
        LoweredValInfo  field)
    {
        switch (base.flavor)
        {
        default:
            {
                IRValue* irBase = getSimpleVal(context, base);
                return LoweredValInfo::simple(
                    getBuilder()->emitFieldExtract(
                        getSimpleType(fieldType),
                        irBase,
                        (IRStructField*) getSimpleVal(context, field)));
            }
            break;

        case LoweredValInfo::Flavor::Ptr:
            {
                // We are "extracting" a field from an lvalue address,
                // which means we should just compute an lvalue
                // representing the field address.
                IRValue* irBasePtr = base.val;
                return LoweredValInfo::ptr(
                    getBuilder()->emitFieldAddress(
                        getBuilder()->getPtrType(getSimpleType(fieldType)),
                        irBasePtr,
                        (IRStructField*) getSimpleVal(context, field)));
            }
            break;
        }
    }

    LoweredValInfo visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        return ensureDecl(context, expr->declRef);
    }

    LoweredValInfo visitTypeCastExpr(TypeCastExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for type cast expression");
    }

    LoweredValInfo visitSelectExpr(SelectExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for select expression");
    }

    LoweredValInfo visitGenericAppExpr(GenericAppExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("generic application expression during code generation");
    }

    LoweredValInfo visitSharedTypeExpr(SharedTypeExpr* expr)
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

        auto irIntType = builder->getBaseType(BaseType::Int);

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
    LValueExprLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(expr);
}

LoweredValInfo lowerRValueExpr(
    IRGenContext*   context,
    Expr*           expr)
{
    RValueExprLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(expr);
}

struct StmtLoweringVisitor : StmtVisitor<StmtLoweringVisitor>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    void visitStmt(Stmt* stmt)
    {
        SLANG_UNIMPLEMENTED_X("stmt catch-all");
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
        auto parent = builder->parentInst;

        IRBlock* prevBlock = nullptr;
        IRFunc* parentFunc = nullptr;

        switch (parent->op)
        {
        case kIROp_Block:
            prevBlock = (IRBlock*)parent;
            parentFunc = prevBlock->getParent();
            break;

        default:
            SLANG_UNEXPECTED("bad parent kind for block");
            return;
        }

        // If the previous block doesn't already have
        // a terminator instruction, then be sure to
        // emit a branch to the new block.
        if (!isTerminatorInst(prevBlock->lastChild))
        {
            builder->emitBranch(block);
        }

        builder->parentInst = parentFunc;
        builder->addInst(block);
        builder->parentInst = block;
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

        // TODO: register `loopHead` as the target for a
        // `continue` statement.

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
        lowerDecl(context, stmt->decl, nullptr);
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
};

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt)
{
    StmtLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(stmt);
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
            IRInst* irLeftVal = getSimpleVal(context, loweredBase);
            auto irRightVal = getSimpleVal(context, right);

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
            for (auto setterDeclRef : getMembersOfType<SetterDecl>(subscriptInfo->declRef))
            {
                auto allArgs = subscriptInfo->args;
                
                addArgs(context, &allArgs, right);

                emitCallToDeclRef(
                    context,
                    builder->getVoidType(),
                    setterDeclRef,
                    allArgs);
                return;
            }

            // No setter found? Then we have an error!
            SLANG_UNEXPECTED("no setter found");
            break;
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
    Layout*         layout;

    IRBuilder* getBuilder()
    {
        return context->irBuilder;
    }

    Layout* getLayout()
    {
        return layout;
    }

    LoweredValInfo visitDeclBase(DeclBase* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }

    LoweredValInfo visitDecl(Decl* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
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

            ensureDecl(context, makeDeclRef(accessor.Ptr()));
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

    LoweredValInfo visitVarDeclBase(VarDeclBase* decl)
    {
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

        auto varType = lowerType(context, decl->getType());

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

        IRAddressSpace addressSpace = kIRAddressSpace_Default;
        if (decl->HasModifier<HLSLGroupSharedModifier>())
        {
            addressSpace = kIRAddressSpace_GroupShared;
        }

        LoweredValInfo varVal = createVar(context, varType, decl, getLayout(), addressSpace);

        if( auto initExpr = decl->initExpr )
        {
            auto initVal = lowerRValueExpr(context, initExpr);

            assign(context, varVal, initVal);
        }

        context->shared->declValues.Add(
            DeclRef<VarDeclBase>(decl, nullptr),
            varVal);

        return varVal;
    }

    LoweredValInfo visitAggTypeDecl(AggTypeDecl* decl)
    {
        // User-defined aggregate type: need to translate into
        // a corresponding IR aggregate type.

        auto builder = getBuilder();
        IRStructDecl* irStruct = builder->createStructType();

        for (auto fieldDecl : decl->GetFields())
        {
            // TODO: need to track relationship to original fields...

            // TODO: need to be prepared to deal with tuple-ness of fields here
            auto fieldType = lowerType(context, fieldDecl->getType());

            switch (fieldType.flavor)
            {
            case LoweredTypeInfo::Flavor::Simple:
                {
                    auto irField = builder->createStructField(getSimpleType(fieldType));
                    builder->addInst(irStruct, irField);

                    builder->addHighLevelDeclDecoration(irField, fieldDecl);

                    context->shared->declValues.Add(
                        DeclRef<StructField>(fieldDecl, nullptr),
                        LoweredValInfo::simple(irField));
                }
                break;

            default:
                SLANG_UNIMPLEMENTED_X("struct field type");
            }
        }

        builder->addHighLevelDeclDecoration(irStruct, decl);

        builder->addInst(irStruct);

        return LoweredValInfo::simple(irStruct);
    }

    LoweredValInfo visitFunctionDeclBase(FunctionDeclBase* decl)
    {
        IRBuilder subBuilderStorage = *getBuilder();
        IRBuilder* subBuilder = &subBuilderStorage;

        // need to create an IR function here

        IRFunc* irFunc = subBuilder->createFunc();
        subBuilder->parentInst = irFunc;

        IRBlock* entryBlock = subBuilder->emitBlock();
        subBuilder->parentInst = entryBlock;

        IRGenContext subContextStorage = *context;
        IRGenContext* subContext = &subContextStorage;
        subContext->irBuilder = subBuilder;

        // set up sub context for generating our new function

        CallableDecl* declForParameters = decl;
        CallableDecl* declForReturnType = decl;
        if (auto accessorDecl = dynamic_cast<AccessorDecl*>(decl))
        {
            auto parentDecl = accessorDecl->ParentDecl;
            if (auto subscriptDecl = dynamic_cast<SubscriptDecl*>(parentDecl))
            {
                declForParameters = subscriptDecl;
                declForReturnType = subscriptDecl;
            }
        }


        List<IRType*> paramTypes;

        for( auto paramDecl : declForParameters->GetParameters() )
        {
            IRType* irParamType = lowerSimpleType(context, paramDecl->getType());
            paramTypes.Add(irParamType);

            IRParam* irParam = subBuilder->emitParam(irParamType);

            subBuilder->addHighLevelDeclDecoration(irParam, paramDecl);

            DeclRef<ParamDecl> paramDeclRef = makeDeclRef(paramDecl.Ptr());

            LoweredValInfo irParamVal = LoweredValInfo::simple(irParam);

            subContext->shared->declValues.Add(paramDeclRef, irParamVal);
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
            IRParam* irParam = subBuilder->emitParam(irParamType);

            // TODO: we need some way to wire this up to the `newValue`
            // or whatever name we give for that parameter inside
            // the setter body.

            // Instead, a setter always returns `void`
            //
            irResultType = getBuilder()->getVoidType();
        }

        auto irFuncType = getBuilder()->getFuncType(
            paramTypes.Count(),
            paramTypes.Buffer(),
            irResultType);
        irFunc->type.init(irFunc, irFuncType);

        lowerStmt(subContext, decl->Body);

        // We need to carefully add a terminator instruction to the end
        // of the body, in case the user didn't do so.
        if (!isTerminatorInst(subContext->irBuilder->parentInst->lastChild))
        {
            if (irResultType->op == kIROp_VoidType)
            {
                subContext->irBuilder->emitReturn();
            }
            else
            {
                SLANG_UNEXPECTED("Needed a return here");
                subContext->irBuilder->emitReturn();
            }
        }

        getBuilder()->addHighLevelDeclDecoration(irFunc, decl);

        getBuilder()->addInst(irFunc);

        return LoweredValInfo::simple(irFunc);
    }
};

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    DeclBase*       decl,
    Layout*         layout)
{
    DeclLoweringVisitor visitor;
    visitor.layout = layout;
    visitor.context = context;
    return visitor.dispatch(decl);
}

LoweredValInfo ensureDecl(
    IRGenContext*           context,
    DeclRef<Decl> const&    declRef)
{
    auto shared = context->shared;

    LoweredValInfo result;
    if(shared->declValues.TryGetValue(declRef, result))
        return result;

    // TODO: this is where we need to apply any specializations
    // from the declaration reference, so that they can be
    // applied correctly to the declaration itself...

    IRBuilder subIRBuilder;
    subIRBuilder.shared = context->irBuilder->shared;
    subIRBuilder.parentInst = subIRBuilder.shared->module;

    IRGenContext subContext = *context;

    subContext.irBuilder = &subIRBuilder;

    RefPtr<VarLayout> layout;
    auto globalScopeLayout = shared->programLayout->globalScopeLayout;
    if (auto globalParameterBlockLayout = globalScopeLayout.As<ParameterBlockTypeLayout>())
    {
        globalScopeLayout = globalParameterBlockLayout->elementTypeLayout;
    }
    if (auto globalStructTypeLayout = globalScopeLayout.As<StructTypeLayout>())
    {
        globalStructTypeLayout->mapVarToLayout.TryGetValue(declRef.getDecl(), layout);
    }

    result = lowerDecl(&subContext, declRef.getDecl(), layout);

    shared->declValues[declRef] = result;

    return result;
}


EntryPointLayout* findEntryPointLayout(
    SharedIRGenContext* shared,
    EntryPointRequest*  entryPointRequest)
{
    for( auto entryPointLayout : shared->programLayout->entryPoints )
    {
        if(entryPointLayout->entryPoint->getName() != entryPointRequest->name)
            continue;

        if(entryPointLayout->profile != entryPointRequest->profile)
            continue;

        // TODO: can't easily filter on translation unit here...
        // Ideally the `EntryPointRequest` should get filled in with a pointer
        // the specific function declaration that represents the entry point.

        return entryPointLayout.Ptr();
    }

    return nullptr;
}

static void lowerEntryPointToIR(
    IRGenContext*       context,
    EntryPointRequest*  entryPointRequest,
    EntryPointLayout*   entryPointLayout)
{
    // First, lower the entry point like an ordinary function
    auto entryPointFuncDecl = entryPointLayout->entryPoint;
    auto loweredEntryPointFunc = lowerDecl(context, entryPointFuncDecl, entryPointLayout);
    auto irFunc = getSimpleVal(context, loweredEntryPointFunc);

    auto builder = context->irBuilder;

    // We are going to attach all the entry-point-specific information
    // to the declaration as meta-data decorations for now.
    //
    // I'm not convinced this is the right way to go, but it is
    // the easiest and most expedient thing.
    //
    auto profile = entryPointRequest->profile;
    auto stage = profile.GetStage();

    auto entryPointDecoration = builder->addDecoration<IREntryPointDecoration>(irFunc);
    entryPointDecoration->profile = profile;

    // Next, we need to start attaching the meta-data that is
    // required based on the particular stage we are targetting:
    switch (stage)
    {
    case Stage::Compute:
        {
            // We need to attach information about the thread group size here.
            auto threadGroupSizeDecoration = builder->addDecoration<IRComputeThreadGroupSizeDecoration>(irFunc);
            static const UInt kAxisCount = 3;

            // TODO: this is kind of gross because we are using a public
            // reflection API function, rather than some kind of internal
            // utility it forwards to...
            spReflectionEntryPoint_getComputeThreadGroupSize(
                (SlangReflectionEntryPoint*)entryPointLayout,
                kAxisCount,
                &threadGroupSizeDecoration->sizeAlongAxis[0]);
        }
        break;

    default:
        break;
    }
}

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

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->shared = sharedBuilder;
    builder->parentInst = nullptr;

    IRModule* module = builder->createModule();
    sharedBuilder->module = module;
    builder->parentInst = module;

    context->irBuilder = builder;

    auto entryPointLayout = findEntryPointLayout(sharedContext, entryPoint);

    lowerEntryPointToIR(context, entryPoint, entryPointLayout);

    return module;

}

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

}
