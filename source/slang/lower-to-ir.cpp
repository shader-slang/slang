// lower.cpp
#include "lower-to-ir.h"

#include "ir.h"
#include "type-layout.h"
#include "visitor.h"

namespace Slang
{

struct LoweredValInfo
{
    enum class Flavor
    {
        None,
        Simple,
        Ptr,
    };

    union
    {
        IRValue*    val;
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
    }};

struct SharedIRGenContext
{
    EntryPointRequest*  entryPoint;
    ProgramLayout*      programLayout;
    CodeGenTarget       target;

    Dictionary<DeclRef<Decl>, LoweredValInfo> declValues;
};


struct IRGenContext
{
    SharedIRGenContext* shared;

    IRBuilder* irBuilder;
};

IRValue* getSimpleVal(LoweredValInfo lowered)
{
    switch(lowered.flavor)
    {
    case LoweredValInfo::Flavor::None:
        return nullptr;

    case LoweredValInfo::Flavor::Simple:
        return lowered.val;

    default:
        SLANG_UNEXPECTED("unhandled value flavor");
        return nullptr;
    }
}

IRValue* getSimpleVal(IRGenContext* context, LoweredValInfo lowered)
{
    switch(lowered.flavor)
    {
    case LoweredValInfo::Flavor::None:
        return nullptr;

    case LoweredValInfo::Flavor::Simple:
        return lowered.val;

    case LoweredValInfo::Flavor::Ptr:
        return context->irBuilder->emitLoad(lowered.val);

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
    return getSimpleVal(lowered);
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


LoweredValInfo lowerExpr(
    IRGenContext*   context,
    Expr*           expr);

void lowerStmt(
    IRGenContext*   context,
    Stmt*           stmt);

LoweredValInfo ensureDecl(
    IRGenContext*           context,
    DeclRef<Decl> const&    declRef);

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
        return getSimpleVal(loweredFunc)->getType();
    }

    void addGenericArgs(List<IRValue*>* ioArgs, DeclRefBase declRef)
    {
        auto subs = declRef.substitutions;
        while(subs)
        {
            for(auto aa : subs->args)
            {
                (*ioArgs).Add(getSimpleVal(lowerVal(context, aa)));
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


//

struct ExprLoweringVisitor : ExprVisitor<ExprLoweringVisitor, LoweredValInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder() { return context->irBuilder; }

    LoweredValInfo visitVarExpr(VarExpr* expr)
    {
        LoweredValInfo info = ensureDecl(context, expr->declRef);
        return info;
    }

    LoweredValInfo visitOverloadedExpr(OverloadedExpr* expr)
    {
        SLANG_UNEXPECTED("overloaded expressions should not occur in checked AST");
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

    void addArgs(List<IRValue*>* ioArgs, LoweredValInfo argInfo)
    {
        auto& args = *ioArgs;
        switch( argInfo.flavor )
        {
        case LoweredValInfo::Flavor::Simple:
        case LoweredValInfo::Flavor::Ptr:
            args.Add(getSimpleVal(context, argInfo));
            break;

        default:
            SLANG_UNIMPLEMENTED_X("addArgs case");
            break;
        }
    }

    // Collect the lowered arguments for a call expression
    void addCallArgs(
        InvokeExpr*     expr,
        List<IRValue*>* ioArgs)
    {
        auto& irArgs = *ioArgs;

        // TODO: should unwrap any layers of identity expressions around this...
        if( auto baseMemberExpr = expr->FunctionExpr.As<MemberExpr>() )
        {
            // This call took the form of a member function call, so
            // we need to correctly add the `this` argument as
            // an explicit argument.
            //
            auto loweredBase = lowerExpr(context, baseMemberExpr->BaseExpression);
            addArgs(&irArgs, loweredBase);
        }

        for( auto arg : expr->Arguments )
        {
            auto loweredArg = lowerExpr(context, arg);
            addArgs(&irArgs, loweredArg);
        }
    }

    LoweredValInfo lowerIntrinsicCall(
        InvokeExpr* expr,
        IROp intrinsicOp)
    {
        auto type = lowerSimpleType(context, expr->type);

        List<IRValue*>  irArgs;
        addCallArgs(expr, &irArgs);
        UInt argCount = irArgs.Count();

        return LoweredValInfo::simple(getBuilder()->emitIntrinsicInst(type, intrinsicOp, argCount, &irArgs[0]));
    }

    LoweredValInfo lowerSimpleCall(InvokeExpr* expr)
    {
        auto type = lowerSimpleType(context, expr->type);

        auto loweredFunc = lowerExpr(context, expr->FunctionExpr);

        List<IRValue*> irArgs;
        addCallArgs(expr, &irArgs);
        UInt argCount = irArgs.Count();

        return LoweredValInfo::simple(getBuilder()->emitCallInst(type, getSimpleVal(loweredFunc), argCount, irArgs.Buffer()));
    }

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

    LoweredValInfo visitInvokeExpr(InvokeExpr* expr)
    {
        // TODO: need to detect calls to builtins here, so that we can expand
        // them as their own special opcodes...

        auto funcExpr = expr->FunctionExpr;
        if( auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>() )
        {
            auto funcDeclRef = funcDeclRefExpr->declRef;
            auto funcDecl = funcDeclRef.getDecl();
            if(auto intrinsicOpModifier = funcDecl->FindModifier<IntrinsicOpModifier>())
            {
                auto op = getIntrinsicOp(funcDecl, intrinsicOpModifier);
                return lowerIntrinsicCall(expr, op);
                // 
            }
            // TODO: handle target intrinsic modifier too...

            if( auto ctorDeclRef = funcDeclRef.As<ConstructorDecl>() )
            {
                // HACK: we know all constructors are builtins for now,
                // so we need to emit them as a call to the corresponding
                // builtin operation.

                auto type = lowerSimpleType(context, expr->type);

                List<IRValue*>  irArgs;
                for( auto arg : expr->Arguments )
                {
                    auto loweredArg = lowerExpr(context, arg);
                    addArgs(&irArgs, loweredArg);
                }

                UInt argCount = irArgs.Count();

                return LoweredValInfo::simple(getBuilder()->emitConstructorInst(type, argCount, &irArgs[0]));
            }
        }

        return lowerSimpleCall(expr);
    }

    LoweredValInfo visitIndexExpr(IndexExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
    }

    LoweredValInfo extractField(
        LoweredTypeInfo fieldType,
        LoweredValInfo  base,
        LoweredValInfo  field)
    {
        switch (base.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
            {
                IRValue* irBase = base.val;
                return LoweredValInfo::simple(
                    getBuilder()->emitFieldExtract(
                        getSimpleType(fieldType),
                        irBase,
                        (IRStructField*) getSimpleVal(field)));
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
                        (IRStructField*) getSimpleVal(field)));
            }
            break;
        default:
            SLANG_UNIMPLEMENTED_X("codegen for field extract");
        }
    }

    LoweredValInfo visitMemberExpr(MemberExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);
        auto loweredBase = lowerExpr(context, expr->BaseExpression);

        auto declRef = expr->declRef;
        if (auto fieldDeclRef = declRef.As<StructField>())
        {
            // Okay, easy enough: we have a reference to a field of a struct type...

            auto loweredField = ensureDecl(context, fieldDeclRef);
            return extractField(loweredType, loweredBase, loweredField);
        }

        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
    }

    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for swizzle expression");
    }

    LoweredValInfo visitDerefExpr(DerefExpr* expr)
    {
        auto loweredType = lowerType(context, expr->type);
        auto loweredBase = lowerExpr(context, expr->base);

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
        SLANG_UNIMPLEMENTED_X("shared type expression during code generation");
    }

    LoweredValInfo visitParenExpr(ParenExpr* expr)
    {
        return lowerExpr(context, expr->base);
    }
};

LoweredValInfo lowerExpr(
    IRGenContext*   context,
    Expr*           expr)
{
    ExprLoweringVisitor visitor;
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

    void visitBlockStmt(BlockStmt* stmt)
    {
        lowerStmt(context, stmt->body);
    }

    void visitReturnStmt(ReturnStmt* stmt)
    {
        if( auto expr = stmt->Expression )
        {
            auto loweredExpr = lowerExpr(context, expr);

            getBuilder()->emitReturn(getSimpleVal(loweredExpr));
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
    LoweredValInfo const&   left,
    LoweredValInfo const&   right)
{
    SLANG_UNIMPLEMENTED_X("assignment");
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

        LoweredValInfo varVal;

        switch( varType.flavor )
        {
        case LoweredTypeInfo::Flavor::Simple:
            {
                auto irAlloc = getBuilder()->emitVar(getSimpleType(varType));

                getBuilder()->addHighLevelDeclDecoration(irAlloc, decl);

                if (getLayout())
                {
                    getBuilder()->addLayoutDecoration(irAlloc, getLayout());
                }


                varVal = LoweredValInfo::ptr(irAlloc);
            }
            break;

        default:
            SLANG_UNIMPLEMENTED_X("struct field type");
        }

        if( auto initExpr = decl->initExpr )
        {
            auto initVal = lowerExpr(context, initExpr);

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

        List<IRType*> paramTypes;

        for( auto paramDecl : decl->GetParameters() )
        {
            IRType* irParamType = lowerSimpleType(context, paramDecl->getType());
            paramTypes.Add(irParamType);

            IRParam* irParam = subBuilder->emitParam(irParamType);

            DeclRef<ParamDecl> paramDeclRef = makeDeclRef(paramDecl.Ptr());

            LoweredValInfo irParamVal = LoweredValInfo::simple(irParam);

            subContext->shared->declValues.Add(paramDeclRef, irParamVal);
        }

        auto irResultType = lowerSimpleType(context, decl->ReturnType);

        auto irFuncType = getBuilder()->getFuncType(
            paramTypes.Count(),
            paramTypes.Buffer(),
            irResultType);
        irFunc->type.init(irFunc, irFuncType);

        lowerStmt(subContext, decl->Body);

        getBuilder()->addHighLevelDeclDecoration(irFunc, decl);

        getBuilder()->addInst(irFunc);

        return LoweredValInfo::simple(irFunc);
    }
};

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    Decl*           decl,
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
    auto entryPointFunc = entryPointLayout->entryPoint;

    // TODO: entry point lowering is probably *not* just like lowering a function...

    lowerDecl(context, entryPointFunc, entryPointLayout);
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

}
