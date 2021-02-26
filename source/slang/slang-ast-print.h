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

    struct Section
    {
        enum class Part
        {
            ParamType,          ///< The type associated with a name
            ParamName,          ///< The name associated with a parameter 
            ReturnType,         ///< The return type
            DeclPath,           ///< The declaration path (NOT including the actual decl name)
            GenericParamType,   ///< Generic parameter type
            GenericParamValue,  ///< Generic parameter value
            GenericValueType,   ///< The type requirement for a value type
        };

        static Section make(Part part, Index start, Index end) { return Section{part, start, end}; }

        Part part;
        Index start;
        Index end;
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
    ASTPrinter(ASTBuilder* astBuilder, OptionFlags optionFlags = 0, List<Section>* sections = nullptr):
        m_astBuilder(astBuilder),
        m_sections(sections),
        m_optionFlags(optionFlags)
    {
    }

    static String getDeclSignatureString(const LookupResultItem& item, ASTBuilder* astBuilder);
    static String getDeclSignatureString(DeclRef<Decl> declRef, ASTBuilder* astBuilder);

protected:

    void _addDeclPathRec(const DeclRef<Decl>& declRef);
    void _addDeclName(Decl* decl);
    void _addSection(Section::Part part, Index startIndex)
    {
        if (m_sections)
        {
            m_sections->add(Section{ part, startIndex, m_builder.getLength() }); 
        }
    }

    OptionFlags m_optionFlags;
    List<Section>* m_sections;
    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

} // namespace Slang

#endif // SLANG_AST_PRINT_H
