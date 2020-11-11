#include "slang-mangle.h"

#include "slang-name.h"
#include "slang-syntax.h"

namespace Slang
{
    struct ManglingContext
    {
        ManglingContext(ASTBuilder* inAstBuilder):
            astBuilder(inAstBuilder)
        {
        }
        ASTBuilder* astBuilder;
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
        Index length = str.getLength();
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
        if( auto constVal = as<ConstantIntVal>(val) )
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

        if( auto basicType = dynamicCast<BasicExpressionType>(type) )
        {
            emitBaseType(context, basicType->baseType);
        }
        else if( auto vecType = dynamicCast<VectorExpressionType>(type) )
        {
            emitRaw(context, "v");
            emitSimpleIntVal(context, vecType->elementCount);
            emitType(context, vecType->elementType);
        }
        else if( auto matType = dynamicCast<MatrixExpressionType>(type) )
        {
            emitRaw(context, "m");
            emitSimpleIntVal(context, matType->getRowCount());
            emitRaw(context, "x");
            emitSimpleIntVal(context, matType->getColumnCount());
            emitType(context, matType->getElementType());
        }
        else if( auto namedType = dynamicCast<NamedExpressionType>(type) )
        {
            emitType(context, getType(context->astBuilder, namedType->declRef));
        }
        else if( auto declRefType = dynamicCast<DeclRefType>(type) )
        {
            emitQualifiedName(context, declRefType->declRef);
        }
        else if (auto arrType = dynamicCast<ArrayExpressionType>(type))
        {
            emitRaw(context, "a");
            emitSimpleIntVal(context, arrType->arrayLength);
            emitType(context, arrType->baseType);
        }
        else if( auto taggedUnionType = dynamicCast<TaggedUnionType>(type) )
        {
            emitRaw(context, "u");
            for( auto caseType : taggedUnionType->caseTypes )
            {
                emitType(context, caseType);
            }
            emitRaw(context, "U");
        }
        else if( auto thisType = dynamicCast<ThisType>(type) )
        {
            emitRaw(context, "t");
            emitQualifiedName(context, thisType->interfaceDeclRef);
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
        if( auto type = dynamicCast<Type>(val) )
        {
            emitType(context, type);
        }
        else if( auto witness = dynamicCast<Witness>(val) )
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
        else if( auto genericParamIntVal = dynamicCast<GenericParamIntVal>(val) )
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
            emitName(context, genericParamIntVal->declRef.getName());
        }
        else if( auto constantIntVal = dynamicCast<ConstantIntVal>(val) )
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

    void emitQualifiedName(
        ManglingContext*    context,
        DeclRef<Decl>       declRef)
    {
        auto parentDeclRef = declRef.getParent();
        auto parentGenericDeclRef = parentDeclRef.as<GenericDecl>();
        if( parentDeclRef )
        {
            // In certain cases we want to skip emitting the parent
            if(parentGenericDeclRef && (parentGenericDeclRef.getDecl()->inner != declRef.getDecl()))
            {
            }
            else if(parentDeclRef.as<FunctionDeclBase>())
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
        if(auto genericDeclRef = declRef.as<GenericDecl>())
        {
            return;
        }

        // Inheritance declarations don't have meaningful names,
        // and so we should emit them based on the type
        // that is doing the inheriting.
        if(auto inheritanceDeclRef = declRef.as<InheritanceDecl>())
        {
            emit(context, "I");
            emitType(context, getSup(context->astBuilder, inheritanceDeclRef));
            return;
        }

        // Similarly, an extension doesn't have a name worth
        // emitting, and we should base things on its target
        // type instead.
        if(auto extensionDeclRef = declRef.as<ExtensionDecl>())
        {
            // TODO: as a special case, an "unconditional" extension
            // that is in the same module as the type it extends should
            // be treated as equivalent to the type itself.
            emit(context, "X");
            emitType(context, getTargetType(context->astBuilder, extensionDeclRef));
            return;
        }

        emitName(context, declRef.getName());

        // Special case: accessors need some way to distinguish themselves
        // so that a getter/setter/ref-er don't all compile to the same name.
        {
            if (declRef.is<GetterDecl>())        emitRaw(context, "Ag");
            if (declRef.is<SetterDecl>())        emitRaw(context, "As");
            if (declRef.is<RefAccessorDecl>())   emitRaw(context, "Ar");
        }

        // Special case: need a way to tell prefix and postfix unary
        // operators apart.
        {
            if(declRef.getDecl()->hasModifier<PostfixModifier>()) emitRaw(context, "P");
            if(declRef.getDecl()->hasModifier<PrefixModifier>()) emitRaw(context, "p");
        }


        // Are we the "inner" declaration beneath a generic decl?
        if(parentGenericDeclRef && (parentGenericDeclRef.getDecl()->inner == declRef.getDecl()))
        {
            // There are two cases here: either we have specializations
            // in place for the parent generic declaration, or we don't.

            auto subst = findInnerMostGenericSubstitution(declRef.substitutions);
            if( subst && subst->genericDecl == parentGenericDeclRef.getDecl() )
            {
                // This is the case where we *do* have substitutions.
                emitRaw(context, "G");
                UInt genericArgCount = subst->args.getCount();
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
                    if(mm.is<GenericTypeParamDecl>())
                    {
                        genericParameterCount++;
                    }
                    else if(mm.is<GenericValueParamDecl>())
                    {
                        genericParameterCount++;
                    }
                    else if(mm.is<GenericTypeConstraintDecl>())
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
                    if(auto genericTypeParamDecl = mm.as<GenericTypeParamDecl>())
                    {
                        emitRaw(context, "T");
                    }
                    else if(auto genericValueParamDecl = mm.as<GenericValueParamDecl>())
                    {
                        emitRaw(context, "v");
                        emitType(context, getType(context->astBuilder, genericValueParamDecl));
                    }
                    else if(mm.as<GenericTypeConstraintDecl>())
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
        if( auto callableDeclRef = declRef.as<CallableDecl>())
        {
            auto parameters = getParameters(callableDeclRef);
            UInt parameterCount = parameters.getCount();

            emitRaw(context, "p");
            emit(context, parameterCount);
            emitRaw(context, "p");

            for(auto paramDeclRef : parameters)
            {
                emitType(context, getType(context->astBuilder, paramDeclRef));
            }

            // Don't print result type for an initializer/constructor,
            // since it is implicit in the qualified name.
            if (!callableDeclRef.is<ConstructorDecl>())
            {
                emitType(context, getResultType(context->astBuilder, callableDeclRef));
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
        if(as<FuncDecl>(decl))
        {}
        // Types will get a `T` prefix:
        else if(as<AggTypeDecl>(decl))
            emitRaw(context, "T");
        else if(as<TypeDefDecl>(decl))
            emitRaw(context, "T");
        // Variables will get a `V` prefix:
        //
        // TODO: probably need to pull constant-buffer
        // declarations out of this...
        else if(as<VarDeclBase>(decl))
            emitRaw(context, "V");
        else if(DeclRef<GenericDecl> genericDecl = declRef.as<GenericDecl>())
        {
            // Mark that this is a generic, so we can differentiate bewteen when
            // mangling the generic and the inner entity
            emitRaw(context, "G");

            SLANG_ASSERT(genericDecl.substitutions == nullptr);

            auto innerDecl = makeDeclRef(getInner(genericDecl));

            emitQualifiedName(context, innerDecl);
            return;
        }
        else
        {
            // TODO: handle other cases
        }

        // Now we encode the qualified name of the decl.
        emitQualifiedName(context, declRef);
    }

    String getMangledName(ASTBuilder* astBuilder, DeclRef<Decl> const& declRef)
    {
        ManglingContext context(astBuilder);
        mangleName(&context, declRef);
        return context.sb.ProduceString();
    }

    String getMangledName(ASTBuilder* astBuilder, DeclRefBase const & declRef)
    {
        return getMangledName(astBuilder,
            DeclRef<Decl>(declRef.decl, declRef.substitutions));
    }

    String getMangledName(ASTBuilder* astBuilder, Decl* decl)
    {
        return getMangledName(astBuilder, makeDeclRef(decl));
    }
    
    String getMangledNameForConformanceWitness(
        ASTBuilder* astBuilder,
        DeclRef<Decl> sub,
        DeclRef<Decl> sup)
    {
        ManglingContext context(astBuilder);
        emitRaw(&context, "_SW");
        emitQualifiedName(&context, sub);
        emitQualifiedName(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledNameForConformanceWitness(
        ASTBuilder* astBuilder,
        DeclRef<Decl> sub,
        Type* sup)
    {
        // The mangled form for a witness that `sub`
        // conforms to `sup` will be named:
        //
        //     {Conforms(sub,sup)} => _SW{sub}{sup}
        //
        ManglingContext context(astBuilder);
        emitRaw(&context, "_SW");
        emitQualifiedName(&context, sub);
        emitType(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledNameForConformanceWitness(
        ASTBuilder* astBuilder,
        Type* sub,
        Type* sup)
    {
        // The mangled form for a witness that `sub`
        // conforms to `sup` will be named:
        //
        //     {Conforms(sub,sup)} => _SW{sub}{sup}
        //
        ManglingContext context(astBuilder);
        emitRaw(&context, "_SW");
        emitType(&context, sub);
        emitType(&context, sup);
        return context.sb.ProduceString();
    }

    String getMangledTypeName(ASTBuilder* astBuilder, Type* type)
    {
        ManglingContext context(astBuilder);
        emitType(&context, type);
        return context.sb.ProduceString();
    }

    String getHashedName(const UnownedStringSlice& mangledName)
    {
        HashCode64 hash = getStableHashCode64(mangledName.begin(), mangledName.getLength());

        StringBuilder builder;
        builder << "_Sh";
        builder.append(uint64_t(hash), 16);

        return builder;
    }

}
