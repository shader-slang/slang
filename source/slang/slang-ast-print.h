// slang-ast-print.h

#ifndef SLANG_AST_PRINT_H
#define SLANG_AST_PRINT_H

#include "slang-ast-all.h"

namespace Slang {

class ASTPrinter
{
public:
    typedef uint32_t OptionFlags;
    struct OptionFlag
    {
        enum Enum : OptionFlags
        {
            ParamNames = 0x1,               ///< If set will output parameter names
        };
    };

    /// Defines part of the structure of the output printed.
    /// That we could have a hierarchy of Parts, but sorting out the relationship
    /// requires looking at how the spans overlap.
    /// For example we could have Param span, and then that could contain a Name and a Type
    struct Part
    {
        enum class Kind
        {
            None,
            Type,
            Value,
            Name,
        };

        enum class Type
        {
            ParamType,          ///< The type associated with a parameter
            ParamName,          ///< The name associated with a parameter 
            ReturnType,         ///< The return type
            DeclPath,           ///< The declaration path (NOT including the actual decl name)
            GenericParamType,   ///< Generic parameter type
            GenericParamValue,  ///< Generic parameter value
            GenericValueType,   ///< The type requirement for a value type
        };

        static Kind getKind(Type type);
        static Part make(Type type, Index start, Index end) { return Part{ type, start, end}; }

        Type type;
        Index start;
        Index end;
    };

    struct ScopePart
    {
        ScopePart(ASTPrinter* printer, Part::Type type):
            m_printer(printer),
            m_type(type),
            m_startIndex(printer->m_builder.getLength())
        {
        }
        ~ScopePart()
        {
            List<Part>* parts = m_printer->m_parts;
            if (parts)
            {
                parts->add(Part{m_type, m_startIndex, m_printer->m_builder.getLength()});
            }
        }

        Part::Type m_type;
        Index m_startIndex;
        ASTPrinter* m_printer;
    };

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

        /// Ctor
    ASTPrinter(ASTBuilder* astBuilder, OptionFlags optionFlags = 0, List<Part>* parts = nullptr):
        m_astBuilder(astBuilder),
        m_parts(parts),
        m_optionFlags(optionFlags)
    {
    }

    static String getDeclSignatureString(const LookupResultItem& item, ASTBuilder* astBuilder);
    static String getDeclSignatureString(DeclRef<Decl> declRef, ASTBuilder* astBuilder);

protected:

    void _addDeclPathRec(const DeclRef<Decl>& declRef);
    void _addDeclName(Decl* decl);
    
    OptionFlags m_optionFlags;
    List<Part>* m_parts;
    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

} // namespace Slang

#endif // SLANG_AST_PRINT_H
