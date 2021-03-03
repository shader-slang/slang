// slang-doc-markdown.h
#ifndef SLANG_DOC_MARK_DOWN_H
#define SLANG_DOC_MARK_DOWN_H

#include "slang-doc-extractor.h"
#include "slang-ast-print.h"

namespace Slang {

class ASTBuilder;

struct DocMarkDownWriter
{
    typedef ASTPrinter::Part Part;
    typedef ASTPrinter::PartPair PartPair;

    struct Signature
    {
        struct GenericParam
        {
            Part name;
            Part type;
        };

        Part returnType;
        List<PartPair> params;
        List<GenericParam> genericParams;
        Part name;
    };

        /// Write out all documentation to the output buffer
    void writeAll();

    void writeCallable(const DocMarkup::Entry& entry, CallableDecl* callable);
    void writeEnum(const DocMarkup::Entry& entry, EnumDecl* enumDecl);
    void writeAggType(const DocMarkup::Entry& entry, AggTypeDeclBase* aggTypeDecl);
    void writeDecl(const DocMarkup::Entry& entry, Decl* decl);
    void writeVar(const DocMarkup::Entry& entry, VarDecl* varDecl);

    void writePreamble(const DocMarkup::Entry& entry);
    void writeDescription(const DocMarkup::Entry& entry);
    
        /// Get the output string
    const StringBuilder& getOutput() const { return m_builder; }

        /// Ctor.
    DocMarkDownWriter(DocMarkup* markup, ASTBuilder* astBuilder) :
        m_markup(markup),
        m_astBuilder(astBuilder)
    {
    }

        /// Given a list of ASTPrinter::Parts, works out the different parts of the sig
    static void getSignature(const List<Part>& parts, Signature& outSig);

    template <typename T>
    void _appendAsBullets(FilteredMemberList<T>& in);
    void _appendAsBullets(const List<Decl*>& in);

        /// Appends prefix and the list of types derived from
    void _appendDerivedFrom(const UnownedStringSlice& prefix, AggTypeDeclBase* aggTypeDecl);

    DocMarkup* m_markup;
    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

} // namespace Slang

#endif
