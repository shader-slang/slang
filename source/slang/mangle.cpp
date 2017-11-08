#include "mangle.h"

#include "name.h"
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

    void emitType(
        ManglingContext*    context,
        Type*               type)
    {
        // TODO: actually implement this bit...

        if( auto basicType = dynamic_cast<BasicExpressionType*>(type) )
        {
            switch( basicType->baseType )
            {
            case BaseType::Void:    emitRaw(context, "V");  break;
            case BaseType::Bool:    emitRaw(context, "b");  break;
            case BaseType::Int:     emitRaw(context, "i");  break;
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
        else if (auto proxyVal = dynamic_cast<IRProxyVal*>(val))
        {
            // This is a proxy standing in for some IR-level
            // value, so we certainly don't want to include
            // it in the mangling.
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
            emitName(context, genericParamIntVal->declRef.GetName());
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

        emitName(context, declRef.GetName());

        // Are we the "inner" declaration beneath a generic decl?
        if(parentGenericDeclRef && (parentGenericDeclRef.getDecl()->inner.Ptr() == declRef.getDecl()))
        {
            // There are two cases here: either we have specializations
            // in place for the parent generic declaration, or we don't.

            auto subst = declRef.substitutions.As<GenericSubstitution>();
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
            emitRaw(context, "p");

            auto parameters = GetParameters(callableDeclRef);
            UInt parameterCount = parameters.Count();
            emit(context, parameterCount);
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

    String mangleSpecializedFuncName(String baseName, RefPtr<Substitutions> subst)
    {
        ManglingContext context;
        emitRaw(&context, baseName.Buffer());
        emitRaw(&context, "_G");
        while (subst)
        {
            if (auto genSubst = subst.As<GenericSubstitution>())
            {
                for (auto a : genSubst->args)
                    emitVal(&context, a);
                break;
            }
            subst = subst->outer;
        }
        return context.sb.ProduceString();
    }

    String getMangledName(Decl* decl)
    {
        return getMangledName(makeDeclRef(decl));
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

}
