// slang-ast-print.h

#ifndef SLANG_AST_PRINT_H
#define SLANG_AST_PRINT_H

#include "slang-ast-all.h"

namespace Slang {

class ASTPrinter
{
public:
        /// We might want options to change how things are output, for example we may want to output parameter names
        /// if there are any

        /// Get the currently built up string
    StringBuilder& getStringBuilder() { return m_builder; }
        /// Get the current offset, for the end of the string builder - useful for building up ranges
    Index getOffset() const { return m_builder.getLength(); }

        /// Reset the state
    void reset() { m_builder.Clear(); }

        /// Get the current string
    String getString() { return m_builder.ProduceString(); }

        /// Add a type
    void addType(Type* type);
        /// Add a value
    void addVal(Val* val);

        /// Add a declaration name 
    void addDeclName(Decl* decl);
        /// Add the path to the declaration including the declaration name
    void addDeclPath(const DeclRef<Decl>& declRef);

        /// Add just the parameters from a declaration.
        /// Will output the generic parameters (if it's a generic) in <> before the parameters ()
    void addDeclParams(const DeclRef<Decl>& declRef);

        /// Add a prefix that describes the kind of declaration
    void addDeclKindPrefix(Decl* decl);

        /// Add the result type
        /// Should be called after the decl params
    void addDeclResultType(DeclRef<Decl> const& inDeclRef);

        /// Add the signature for the decl
    void addDeclSignature(const DeclRef<Decl>& declRef);

    ASTPrinter(ASTBuilder* astBuilder):
        m_astBuilder(astBuilder)
    {
    }

    static String getDeclSignatureString(const LookupResultItem& item, ASTBuilder* astBuilder);
    static String getDeclSignatureString(DeclRef<Decl> declRef, ASTBuilder* astBuilder);

protected:

    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

struct ASTPrintUtil
{
    

    
};

} // namespace Slang

#endif // SLANG_AST_PRINT_H
