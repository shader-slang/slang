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
};

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

    LoweredTypeInfo visitDeclRefType(DeclRefType* type)
    {
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
            args.Add(getSimpleVal(argInfo));
            break;

        default:
            SLANG_UNIMPLEMENTED_X("addArgs case");
            break;
        }
    }

    LoweredValInfo lowerIntrinsicCall(
        InvokeExpr* expr,
        IntrinsicOp intrinsicOp)
    {
        auto type = lowerSimpleType(context, expr->type);

        List<IRValue*>  irArgs;
        for( auto arg : expr->Arguments )
        {
            auto loweredArg = lowerExpr(context, arg);
            addArgs(&irArgs, loweredArg);
        }

        UInt argCount = irArgs.Count();

        return LoweredValInfo::simple(getBuilder()->emitIntrinsicInst(type, intrinsicOp, argCount, &irArgs[0]));
    }

    LoweredValInfo lowerSimpleCall(InvokeExpr* expr)
    {
        auto loweredFunc = lowerExpr(context, expr->FunctionExpr);

        for( auto arg : expr->Arguments )
        {
            auto loweredArg = lowerExpr(context, arg);
        }

        SLANG_UNIMPLEMENTED_X("codegen for invoke expression");
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
                return lowerIntrinsicCall(expr, intrinsicOpModifier->op);
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
        UInt            fieldIndex)
    {
        switch (base.flavor)
        {
        case LoweredValInfo::Flavor::Simple:
            {
                IRValue* irBase = base.val;
                return LoweredValInfo::simple(
                    getBuilder()->createFieldExtract(
                        getSimpleType(fieldType),
                        irBase,
                        fieldIndex));
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

            // HACK: for now just scan the decl to find the right index.
            // TODO: we need to deal with the fact that the struct might get
            // tuple-ified.
            //
            UInt index = 0;
            for (auto fieldDecl : getMembersOfType<StructField>(fieldDeclRef.GetParent().As<AggTypeDecl>()))
            {
                if (fieldDecl == fieldDeclRef.getDecl())
                {
                    break;
                }

                index++;
            }

            return extractField(loweredType, loweredBase, index);
        }

        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
    }

    LoweredValInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for swizzle expression");
    }

    LoweredValInfo visitDerefExpr(DerefExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for deref expression");
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

            getBuilder()->createReturn(getSimpleVal(loweredExpr));
        }
        else
        {
            getBuilder()->createReturn();
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

struct DeclLoweringVisitor : DeclVisitor<DeclLoweringVisitor, LoweredValInfo>
{
    IRGenContext* context;

    IRBuilder* getBuilder()
    {
        return context->irBuilder;
    }

    LoweredValInfo visitDeclBase(DeclBase* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }

    LoweredValInfo visitDecl(Decl* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }

    LoweredValInfo visitAggTypeDecl(AggTypeDecl* decl)
    {
        // User-defined aggregate type: need to translate into
        // a corresponding IR aggregate type.

        List<LoweredTypeInfo>   fieldTypes;
        List<IRType*>           irFieldTypes;

        for (auto fieldDecl : decl->GetFields())
        {
            // TODO: need to be prepared to deal with tuple-ness of fields here
            auto fieldType = lowerType(context, fieldDecl->getType());

            fieldTypes.Add(fieldType);

            switch (fieldType.flavor)
            {
            case LoweredTypeInfo::Flavor::Simple:
                irFieldTypes.Add(fieldType.type);
                break;

            default:
                SLANG_UNIMPLEMENTED_X("struct field type");
            }
        }

        // TODO: need to track relationship to original fields...

        IRType* irStructType = getBuilder()->getStructType(
            irFieldTypes.Count(),
            &irFieldTypes[0]);

        return LoweredValInfo::simple(irStructType);
    }

    LoweredValInfo visitFunctionDeclBase(FunctionDeclBase* decl)
    {
        IRBuilder subBuilderStorage = *getBuilder();
        IRBuilder* subBuilder = &subBuilderStorage;

        // need to create an IR function here

        IRFunc* irFunc = subBuilder->createFunc();
        subBuilder->parentInst = irFunc;

        IRBlock* entryBlock = subBuilder->createBlock();
        subBuilder->parentInst = entryBlock;

        IRGenContext subContextStorage = *context;
        IRGenContext* subContext = &subContextStorage;
        subContext->irBuilder = subBuilder;

        // set up sub context for generating our new function

        for( auto paramDecl : decl->GetParameters() )
        {
            IRType* irParamType = lowerSimpleType(context, paramDecl->getType());
            IRParam* irParam = subBuilder->createParam(irParamType);

            DeclRef<ParamDecl> paramDeclRef = makeDeclRef(paramDecl.Ptr());

            LoweredValInfo irParamVal = LoweredValInfo::simple(irParam);

            subContext->shared->declValues.Add(paramDeclRef, irParamVal);
        }

        auto irResultType = lowerType(context, decl->ReturnType);


        lowerStmt(subContext, decl->Body);

        return LoweredValInfo::simple(irFunc);
    }
};

LoweredValInfo lowerDecl(
    IRGenContext*   context,
    Decl*           decl)
{
    DeclLoweringVisitor visitor;
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

    IRGenContext subContext = *context;

    result = lowerDecl(context, declRef.getDecl());

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

    lowerDecl(context, entryPointFunc);
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

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->module = nullptr;
    builder->parentInst = nullptr;

    IRModule* module = builder->createModule();
    builder->module = module;
    builder->parentInst = module;

    context->irBuilder = builder;

    auto entryPointLayout = findEntryPointLayout(sharedContext, entryPoint);

    lowerEntryPointToIR(context, entryPoint, entryPointLayout);

    return module;

}

}
