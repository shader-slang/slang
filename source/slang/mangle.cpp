#include "mangle.h"

#include "name.h"
#include "ir-insts.h"
#include "syntax.h"

namespace Slang
{
    struct ManglingContext
    {
        StringBuilder sb;
    };

    void emitRaw(
        ManglingContext*    context,
        char const*         text)
    {
        context->sb.append(text);
    }

    void emit(
        ManglingContext*    context,
        UInt                value)
    {
        context->sb.append(value);
    }

    void emit(
        ManglingContext*    context,
        String const&       value)
    {
        context->sb.append(value);
    }

    void emitName(
        ManglingContext*    context,
        Name*               name)
    {
        String str = getText(name);

        // If the name consists of only traditional "identifer characters"
        // (`[a-zA-Z_]`), then we wnat to emit it more or less directly.
        //
        // If it contains code points outside that range, we'll need to
        // do something to encode them. I don't want to deal with
        // that right now, so I'm going to ignore it.

        // We prefix the string with its byte length, so that
        // decoding doesn't have to worry about finding a terminator.
        UInt length = str.Length();
        emit(context, length);
        context->sb.append(str);
    }

    void emitVal(
        ManglingContext*    context,
        Val*                val);

    void emitQualifiedName(
        ManglingContext*    context,
        DeclRef<Decl>       declRef);

    void emitSimpleIntVal(
        ManglingContext*    context,
        Val*                val)
    {
        if( auto constVal = dynamic_cast<ConstantIntVal*>(val) )
        {
            auto cVal = constVal->value;
            if(cVal >= 0 && cVal <= 9 )
            {
                emit(context, (UInt)cVal);
                return;
            }
        }

        // Fallback:
        emitVal(context, val);
    }

    void emitBaseType(
        ManglingContext*    context,
        BaseType            baseType)
    {
        switch( baseType )
        {
        case BaseType::Void:    emitRaw(context, "V");  break;
        case BaseType::Bool:    emitRaw(context, "b");  break;
        case BaseType::Int8:    emitRaw(context, "c");  break;
        case BaseType::Int16:   emitRaw(context, "s");  break;
        case BaseType::Int:     emitRaw(context, "i");  break;
        case BaseType::Int64:   emitRaw(context, "I");  break;
        case BaseType::UInt8:   emitRaw(context, "C");  break;
        case BaseType::UInt16:  emitRaw(context, "S");  break;
        case BaseType::UInt:    emitRaw(context, "u");  break;
        case BaseType::UInt64:  emitRaw(context, "U");  break;
        case BaseType::Half:    emitRaw(context, "h");  break;
        case BaseType::Float:   emitRaw(context, "f");  break;
        case BaseType::Double:  emitRaw(context, "d");  break;
            break;

        default:
            SLANG_UNEXPECTED("unimplemented case in mangling");
            break;
        }
    }

    void emitType(
        ManglingContext*    context,
        Type*               type)
    {
        // TODO: actually implement this bit...

        if( auto basicType = dynamic_cast<BasicExpressionType*>(type) )
        {
            emitBaseType(context, basicType->baseType);
        }
        else if( auto vecType = dynamic_cast<VectorExpressionType*>(type) )
        {
            emitRaw(context, "v");
            emitSimpleIntVal(context, vecType->elementCount);
            emitType(context, vecType->elementType);
        }
        else if( auto matType = dynamic_cast<MatrixExpressionType*>(type) )
        {
            emitRaw(context, "m");
            emitSimpleIntVal(context, matType->getRowCount());
            emitRaw(context, "x");
            emitSimpleIntVal(context, matType->getColumnCount());
            emitType(context, matType->getElementType());
        }
        else if( auto namedType = dynamic_cast<NamedExpressionType*>(type) )
        {
            emitType(context, GetType(namedType->declRef));
        }
        else if( auto declRefType = dynamic_cast<DeclRefType*>(type) )
        {
            emitQualifiedName(context, declRefType->declRef);
        }
        else if (auto arrType = dynamic_cast<ArrayExpressionType*>(type))
        {
            emitRaw(context, "a");
            emitSimpleIntVal(context, arrType->ArrayLength);
            emitType(context, arrType->baseType);
        }
        else
        {
            SLANG_UNEXPECTED("unimplemented case in mangling");
        }
    }

    void emitVal(
        ManglingContext*    context,
        Val*                val)
    {
        if( auto type = dynamic_cast<Type*>(val) )
        {
            emitType(context, type);
        }
        else if( auto witness = dynamic_cast<Witness*>(val) )
        {
            // We don't emit witnesses as part of a mangled
            // name, because the way that the front-end
            // arrived at the witness is not important;
            // what matters is that the type constraint
            // was satisfied.
            //
            // TODO: make sure we can't get name collisions
            // between specializations of declarations
            // with the same numbers of generic parameters,
            // but different constraints. We might have
            // to mangle in the constraints even when
            // the whole thing is specialized...
        }
        else if( auto genericParamIntVal = dynamic_cast<GenericParamIntVal*>(val) )
        {
            // TODO: we shouldn't be including the names of generic parameters
            // anywhere in mangled names, since changing parameter names
            // shouldn't break binary compatibility.
            //
            // The right solution in the long term is for generic parameters
            // (both types and values) to be mangled in terms of their
            // "depth" (how many outer generics) and "index" (which
            // parameter are they at the specified depth).
            emitRaw(context, "K");
            emitName(context, genericParamIntVal->declRef.GetName());
        }
        else if( auto constantIntVal = dynamic_cast<ConstantIntVal*>(val) )
        {
            // TODO: need to figure out what prefix/suffix is needed
            // to allow demangling later.
            emitRaw(context, "k");
            emit(context, (UInt) constantIntVal->value);
        }
        else
        {
            SLANG_UNEXPECTED("unimplemented case in mangling");
        }
    }

    void emitIRVal(
        ManglingContext*    context,
        IRInst*             inst);

    void emitIRSimpleIntVal(
        ManglingContext*    context,
        IRInst*             inst)
    {
        if (auto intLit = as<IRIntLit>(inst))
        {
            auto cVal = intLit->getValue();
            if(cVal >= 0 && cVal <= 9 )
            {
                emit(context, (UInt)cVal);
                return;
            }
        }

        // Fallback:
        emitIRVal(context, inst);
    }

    void emitIRVal(
        ManglingContext*    context,
        IRInst*             inst)
    {
        if(auto basicType = as<IRBasicType>(inst) )
        {
            emitBaseType(context, basicType->getBaseType());
            return;
        }

        if (auto globalVal = as<IRGlobalValue>(inst))
        {
            // If it is a global value, it has its own mangled name.
            emit(context, getText(globalVal->mangledName));
        }
        // TODO: need to handle various type cases here
        else if (auto intLit = as<IRIntLit>(inst))
        {
            // TODO: need to figure out what prefix/suffix is needed
            // to allow demangling later.
            emitRaw(context, "k");
            emit(context, (UInt) intLit->getValue());
        }
        // Note: the cases here handling types really should match
        // the cases above that handle AST-level `Type`s. This
        // seems to be a weakness in the way we mangle names, because
        // we may mangle in both IR-level and AST-level types.
        else if (auto vecType = as<IRVectorType>(inst))
        {
            emitRaw(context, "v");
            emitIRSimpleIntVal(context, vecType->getElementCount());
            emitIRVal(context, vecType->getElementType());

        }
        else if( auto matType = as<IRMatrixType>(inst) )
        {
            emitRaw(context, "m");
            emitIRSimpleIntVal(context, matType->getRowCount());
            emitRaw(context, "x");
            emitIRSimpleIntVal(context, matType->getColumnCount());
            emitIRVal(context, matType->getElementType());
        }
        else if (auto arrType = as<IRArrayType>(inst))
        {
            emitRaw(context, "a");
            emitIRSimpleIntVal(context, arrType->getElementCount());
            emitIRVal(context, arrType->getElementCount());
        }
        else
        {
            SLANG_UNEXPECTED("unimplemented case in mangling");
        }
    }

    void emitQualifiedName(
        ManglingContext*    context,
        DeclRef<Decl>       declRef)
    {
        auto parentDeclRef = declRef.GetParent();
        auto parentGenericDeclRef = parentDeclRef.As<GenericDecl>();
        if( parentDeclRef )
        {
            // In certain cases we want to skip emitting the parent
            if(parentGenericDeclRef && (parentGenericDeclRef.getDecl()->inner.Ptr() != declRef.getDecl()))
            {
            }
            else if(parentDeclRef.As<FunctionDeclBase>())
            {
            }
            else
            {
                emitQualifiedName(context, parentDeclRef);
            }
        }

        // A generic declaration is kind of a pseudo-declaration
        // as far as the user is concerned; so we don't want
        // to emit its name.
        if(auto genericDeclRef = declRef.As<GenericDecl>())
        {
            return;
        }

        // Inheritance declarations don't have meaningful names,
        // and so we should emit them based on the type
        // that is doing the inheriting.
        if(auto inheritanceDeclRef = declRef.As<InheritanceDecl>())
        {
            emit(context, "I");
            emitType(context, GetSup(inheritanceDeclRef));
            return;
        }

        // Similarly, an extension doesn't have a name worth
        // emitting, and we should base things on its target
        // type instead.
        if(auto extensionDeclRef = declRef.As<ExtensionDecl>())
        {
            // TODO: as a special case, an "unconditional" extension
            // that is in the same module as the type it extends should
            // be treated as equivalent to the type itself.
            emit(context, "X");
            emitType(context, GetTargetType(extensionDeclRef));
            return;
        }

        emitName(context, declRef.GetName());

        // Special case: accessors need some way to distinguish themselves
        // so that a getter/setter/ref-er don't all compile to the same name.
        if(declRef.As<GetterDecl>())        emitRaw(context, "Ag");
        if(declRef.As<SetterDecl>())        emitRaw(context, "As");
        if(declRef.As<RefAccessorDecl>())   emitRaw(context, "Ar");

        // Are we the "inner" declaration beneath a generic decl?
        if(parentGenericDeclRef && (parentGenericDeclRef.getDecl()->inner.Ptr() == declRef.getDecl()))
        {
            // There are two cases here: either we have specializations
            // in place for the parent generic declaration, or we don't.

            auto subst = findInnerMostGenericSubstitution(declRef.substitutions);
            if( subst && subst->genericDecl == parentGenericDeclRef.getDecl() )
            {
                // This is the case where we *do* have substitutions.
                emitRaw(context, "G");
                UInt genericArgCount = subst->args.Count();
                emit(context, genericArgCount);
                for( auto aa : subst->args )
                {
                    emitVal(context, aa);
                }
            }
            else
            {
                // We don't have substitutions, so we will emit
                // information about the parameters of the generic here.
                emitRaw(context, "g");
                UInt genericParameterCount = 0;
                for( auto mm : getMembers(parentGenericDeclRef) )
                {
                    if(mm.As<GenericTypeParamDecl>())
                    {
                        genericParameterCount++;
                    }
                    else if(mm.As<GenericValueParamDecl>())
                    {
                        genericParameterCount++;
                    }
                    else if(mm.As<GenericTypeConstraintDecl>())
                    {
                        genericParameterCount++;
                    }
                    else
                    {
                    }
                }

                emit(context, genericParameterCount);
                for( auto mm : getMembers(parentGenericDeclRef) )
                {
                    if(auto genericTypeParamDecl = mm.As<GenericTypeParamDecl>())
                    {
                        emitRaw(context, "T");
                    }
                    else if(auto genericValueParamDecl = mm.As<GenericValueParamDecl>())
                    {
                        emitRaw(context, "v");
                        emitType(context, GetType(genericValueParamDecl));
                    }
                    else if(mm.As<GenericTypeConstraintDecl>())
                    {
                        emitRaw(context, "C");
                        // TODO: actually emit info about the constraint
                    }
                    else
                    {
                    }
                }
            }
        }

        // If the declaration has parameters, then we need to emit
        // those parameters to distinguish it from other declarations
        // of the same name that might have different parameters.
        //
        // We'll also go ahead and emit the result type as well,
        // just for completeness.
        //
        if( auto callableDeclRef = declRef.As<CallableDecl>())
        {
            auto parameters = GetParameters(callableDeclRef);
            UInt parameterCount = parameters.Count();

            emitRaw(context, "p");
            emit(context, parameterCount);
            emitRaw(context, "p");

            for(auto paramDeclRef : parameters)
            {
                emitType(context, GetType(paramDeclRef));
            }

            // Don't print result type for an initializer/constructor,
            // since it is implicit in the qualified name.
            if (!callableDeclRef.As<ConstructorDecl>())
            {
                emitType(context, GetResultType(callableDeclRef));
            }
        }
    }

    void mangleName(
        ManglingContext*    context,
        DeclRef<Decl>       declRef)
    {
        // TODO: catch cases where the declaration should
        // forward to something else? E.g., what if we
        // are asked to mangle the name of a `typedef`?

        // We will start with a unique prefix to avoid
        // clashes with user-defined symbols:
        emitRaw(context, "_S");

        auto decl = declRef.getDecl();

        // Next we will add a bit of info to register
        // the *kind* of declaration we are dealing with.
        //
        // Functions will get no prefix, since we assume
        // they are a common case:
        if(dynamic_cast<FuncDecl*>(decl))
        {}
        // Types will get a `T` prefix:
        else if(dynamic_cast<AggTypeDecl*>(decl))
            emitRaw(context, "T");
        else if(dynamic_cast<TypeDefDecl*>(decl))
            emitRaw(context, "T");
        // Variables will get a `V` prefix:
        //
        // TODO: probably need to pull constant-buffer
        // declarations out of this...
        else if(dynamic_cast<VarDeclBase*>(decl))
            emitRaw(context, "V");
        else
        {
            // TODO: handle other cases
        }

        // Now we encode the qualified name of the decl.
        emitQualifiedName(context, declRef);
    }

    String getMangledName(DeclRef<Decl> const& declRef)
    {
        ManglingContext context;
        mangleName(&context, declRef);
        return context.sb.ProduceString();
    }

    String getMangledName(DeclRefBase const & declRef)
    {
        return getMangledName(
            DeclRef<Decl>(declRef.decl, declRef.substitutions));
    }

    String mangleSpecializedFuncName(String baseName, IRSpecialize* specializeInst)
    {
        ManglingContext context;
        emitRaw(&context, baseName.Buffer());
        emitRaw(&context, "_G");

        UInt argCount = specializeInst->getArgCount();
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            emitIRVal(&context, specializeInst->getArg(aa));
        }

        return context.sb.ProduceString();
    }

    String getMangledName(Decl* decl)
    {
        return getMangledName(makeDeclRef(decl));
    }
    
    String getMangledNameForConformanceWitness(
        DeclRef<Decl> sub,
        DeclRef<Decl> sup)
    {
        ManglingContext context;
        emitRaw(&context, "_SW");
        emitQualifiedName(&context, sub);
        emitQualifiedName(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledNameForConformanceWitness(
        DeclRef<Decl> sub,
        Type* sup)
    {
        // The mangled form for a witness that `sub`
        // conforms to `sup` will be named:
        //
        //     {Conforms(sub,sup)} => _SW{sub}{sup}
        //
        ManglingContext context;
        emitRaw(&context, "_SW");
        emitQualifiedName(&context, sub);
        emitType(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledNameForConformanceWitness(
        Type* sub,
        Type* sup)
    {
        // The mangled form for a witness that `sub`
        // conforms to `sup` will be named:
        //
        //     {Conforms(sub,sup)} => _SW{sub}{sup}
        //
        ManglingContext context;
        emitRaw(&context, "_SW");
        emitType(&context, sub);
        emitType(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledTypeName(Type* type)
    {
        ManglingContext context;
        emitType(&context, type);
        return context.sb.ProduceString();
    }


}
