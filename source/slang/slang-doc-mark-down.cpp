// slang-doc-mark-down.cpp
#include "slang-doc-mark-down.h"

#include "../core/slang-string-util.h"

#include "slang-ast-builder.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DocMarkDownWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

static void _appendAsSingleLine(const UnownedStringSlice& in, StringBuilder& out)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(in, lines);

    // Ideally we'd remove any extraneous whitespace, but for now just join
    StringUtil::join(lines.getBuffer(), lines.getCount(), ' ', out);
}

template <typename T>
void DocMarkDownWriter::_appendAsBullets(FilteredMemberList<T>& list)
{
    auto& out = m_builder;
    for (auto element : list)
    {
        DocMarkup::Entry* paramEntry = m_markup->getEntry(element);

        out << "* ";

        Name* name = element->getName();
        if (name)
        {
            out << toSlice("_") << name->text << toSlice("_ ");
        }

        if (paramEntry)
        {
            // Hmm, we'll want to make something multiline into a single line
            _appendAsSingleLine(paramEntry->m_markup.getUnownedSlice(), out);
        }

        out << "\n";
    }

    out << toSlice("\n");
}

/* static */void DocMarkDownWriter::getSignature(const List<Part>& parts, Signature& outSig)
{
    const Index count = parts.getCount();
    for (Index i = 0; i < count; ++i)
    {
        const auto& part = parts[i];
        switch (part.type)
        {
            case Part::Type::ParamType:
            {
                PartPair pair;
                pair.first = part;
                if (parts[i + 1].type == Part::Type::ParamName)
                {
                    pair.second = parts[i + 1];
                    i++;
                }
                outSig.params.add(pair);
                break;
            }
            case Part::Type::ReturnType:
            {
                outSig.returnType = part;
                break;
            }
            case Part::Type::DeclPath:
            {
                outSig.name = part;
                break;
            }
            default: break;
        }
    }
}

void DocMarkDownWriter::writeCallable(const DocMarkup::Entry& entry, CallableDecl* callableDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    StringBuilder sigBuffer;
    List<ASTPrinter::Part> parts;
    ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames, &parts);

    printer.addDeclSignature(DeclRef<Decl>(callableDecl, nullptr));

    Signature signature;
    getSignature(parts, signature);

    const Index paramCount = signature.params.getCount();

    // Output the signature
    {        
        // Extract the name
        out << toSlice("# ") << printer.getPartSlice(signature.name) << toSlice("\n\n");

        out << toSlice("## Signature \n");
        out << toSlice("```\n");
        out << printer.getPartSlice(signature.returnType) << toSlice(" ");

        out << printer.getPartSlice(signature.name);

        
        if (paramCount > 0)
        {
            out << toSlice("(\n");

            StringBuilder line;
            for (Index i = 0; i < paramCount; ++i)
            {
                const auto& param = signature.params[i];
                line.Clear();
                // If we want to tab these over... we'll need to know how must space I have
                line << "    " << printer.getPartSlice(param.first);

                Index indent = 25;
                if (line.getLength() < indent)
                {
                    line.appendRepeatedChar(' ', indent - line.getLength());
                }
                else
                {
                    line.appendChar(' ');
                }

                line << printer.getPartSlice(param.second);
                if (i < paramCount - 1)
                {
                    line << ",\n";
                }

                out << line;
            }

            out << ");\n";
        }
        else
        {
            out << toSlice("();\n");
        }

        out << "```\n\n";
    }

    // Only output params if there are any
    if (paramCount)
    {
        out << "## Parameters\n\n";

        auto params = callableDecl->getParameters();
        _appendAsBullets(params);
    }

    writeDescription(entry);
}

void DocMarkDownWriter::writeEnum(const DocMarkup::Entry& entry, EnumDecl* enumDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    out << toSlice("# enum ");
    Name* name = enumDecl->getName();
    if (name)
    {
        out << name->text;
    }
    out << toSlice("\n\n");

    out << toSlice("## Values \n\n");

    auto cases = enumDecl->getMembersOfType<EnumCaseDecl>();
    _appendAsBullets(cases);

    writeDescription(entry);
}

void DocMarkDownWriter::writeAggType(const DocMarkup::Entry& entry, AggTypeDecl* aggTypeDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    // This could be lots of different things - struct/class/extension/interface/..

    out << toSlice("# ");
    if (as<StructDecl>(aggTypeDecl))
    {
        out << toSlice("struct ");
    }
    else if (as<ClassDecl>(aggTypeDecl))
    {
        out << toSlice("class ");
    }
    else
    {
        out << toSlice("?");
    }

    Name* name = aggTypeDecl->getName();
    if (name)
    {
        out << name->text;
    }
    out << toSlice("\n\n");

    out << "## Fields\n\n";

    auto fields = aggTypeDecl->getMembersOfType<VarDecl>();
    _appendAsBullets(fields);

    writeDescription(entry);
}

void DocMarkDownWriter::writePreamble(const DocMarkup::Entry& entry)
{
    SLANG_UNUSED(entry);
    auto& out = m_builder;

    out << toSlice("\n");
    out.appendRepeatedChar('-', 80);
    out << toSlice("\n");
}


void DocMarkDownWriter::writeDescription(const DocMarkup::Entry& entry)
{
    auto& out = m_builder;

    out << toSlice("\n## Description\n\n");
    out << entry.m_markup;
}

void DocMarkDownWriter::writeAll()
{
    for (const auto& entry : m_markup->getEntries())
    {
        NodeBase* node = entry.m_node;
        Decl* decl = as<Decl>(node);
        if (!decl)
        {
            continue;
        }

        // Skip these they will be output as part of their respective 'containers'
        if (as<ParamDecl>(decl) || as<EnumCaseDecl>(decl))
        {
            continue;
        }

        if (CallableDecl* callableDecl = as<CallableDecl>(decl))
        {
            writeCallable(entry, callableDecl);
        }
        else if (EnumDecl* enumDecl = as<EnumDecl>(decl))
        {
            writeEnum(entry, enumDecl);
        }
        else if (AggTypeDecl* aggType = as<AggTypeDecl>(decl))
        {
            writeAggType(entry, aggType);
        }
    }
}

} // namespace Slang
