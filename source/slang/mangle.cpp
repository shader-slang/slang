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

    void emitType(
        ManglingContext*    context,
        Type*               type)
    {
        // TODO: actually implement this bit...
    }

    void emitQualifiedName(
        ManglingContext*    context,
        Decl*               decl)
    {
        auto parentDecl = decl->ParentDecl;
        if( parentDecl )
        {
            emitQualifiedName(context, parentDecl);
        }

        // A generic declaration is kind of a pseudo-declaration
        // as far as the user is concerned; so we don't want
        // to emit its name.
        if( auto genericDecl = dynamic_cast<GenericDecl*>(decl) )
        {
            return;
        }

        emitName(context, decl->nameAndLoc.name);

        if( auto parentGenericDecl = dynamic_cast<GenericDecl*>(parentDecl))
        {
            emitRaw(context, "g");
            UInt genericParameterCount = 0;
            for( auto mm : parentGenericDecl->Members )
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
            for( auto mm : parentGenericDecl->Members )
            {
                if(auto genericTypeParamDecl = mm.As<GenericTypeParamDecl>())
                {
                    emitRaw(context, "T");
                }
                else if(auto genericValueParamDecl = mm.As<GenericValueParamDecl>())
                {
                    emitRaw(context, "v");
                    emitType(context, genericValueParamDecl->getType());
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

        // If the declaration has parameters, then we need to emit
        // those parameters to distinguish it from other declarations
        // of the same name that might have different parameters.
        //
        // We'll also go ahead and emit the result type as well,
        // just for completeness.
        //
        if( auto callableDecl = dynamic_cast<CallableDecl*>(decl) )
        {
            emitRaw(context, "p");
            UInt parameterCount = callableDecl->GetParameters().Count();
            emit(context, parameterCount);
            for(auto pp : callableDecl->GetParameters())
            {
                emitType(context, pp->getType());
            }

            emitType(context, callableDecl->ReturnType);
        }
    }

    void mangleName(
        ManglingContext*    context,
        Decl*               decl)
    {
        // TODO: catch cases where the declaration should
        // forward to something else? E.g., what if we
        // are asked to mangle the name of a `typedef`?

        // We will start with a unique prefix to avoid
        // clashes with user-defined symbols:
        emitRaw(context, "_S");

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
        emitQualifiedName(context, decl);
    }



    String getMangledName(Decl* decl)
    {
        ManglingContext context;

        mangleName(&context, decl);

        return context.sb.ProduceString();
    }
}
