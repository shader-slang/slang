// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-slice-pool.h"
#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-file-system.h"

#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/compiler-core/slang-diagnostic-sink.h"
#include "../../source/compiler-core/slang-name.h"
#include "../../source/compiler-core/slang-name-convention-util.h"

#include "node.h"
#include "diagnostics.h"
#include "options.h"
#include "parser.h"

/*
Some command lines:

-d source/slang slang-ast-support-types.h slang-ast-base.h slang-ast-decl.h slang-ast-expr.h slang-ast-modifier.h slang-ast-stmt.h slang-ast-type.h slang-ast-val.h -strip-prefix slang- -o slang-generated -output-fields -mark-suffix _CLASS
*/

namespace CppExtract
{

using namespace Slang;

static void _indent(Index indentCount, StringBuilder& out)
{
    for (Index i = 0; i < indentCount; ++i)
    {
        out << CPP_EXTRACT_INDENT_STRING;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractorApp !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class App
{
public:
    
    SlangResult readAllText(const Slang::String& fileName, String& outRead);
    SlangResult writeAllText(const Slang::String& fileName, const UnownedStringSlice& text);

    SlangResult execute(const Options& options);

        /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv);

        /// Write output
    SlangResult writeOutput(Parser& extractor);

        /// Write def files
    SlangResult writeDefs(Parser& extractor);

        /// Calculate the header 
    SlangResult calcTypeHeader(Parser& extractor, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcChildrenHeader(Parser& exctractor, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcOriginHeader(Parser& extractor, StringBuilder& out);

    SlangResult calcDef(Parser& extractor, SourceOrigin* origin, StringBuilder& out);

    const Options& getOptions() const { return m_options; }

    App(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager),
        m_slicePool(StringSlicePool::Style::Default)
    {
        m_namePool.setRootNamePool(rootNamePool);
    }

protected:

        /// Called to set up identifier lookup. Must be performed after options are initials
    static void _initIdentifierLookup(const Options& options, IdentifierLookup& outLookup);

    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;
    
    StringSlicePool m_slicePool;
};

SlangResult App::readAllText(const Slang::String& fileName, String& outRead)
{
    try
    {
        StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
        outRead = reader.ReadToEnd();
    }
    catch (const IOException&)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }
    catch (...)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult App::writeAllText(const Slang::String& fileName, const UnownedStringSlice& text)
{
    try
    {
        if (File::exists(fileName))
        {
            String existingText;
            if (readAllText(fileName, existingText) == SLANG_OK)
            {
                if (existingText == text)
                    return SLANG_OK;
            }
        }
        StreamWriter writer(new FileStream(fileName, FileMode::Create));
        writer.Write(text);
    }
    catch (const IOException&)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult App::calcDef(Parser& extractor, SourceOrigin* origin, StringBuilder& out)
{
    Node* currentScope = nullptr;

    for (Node* node : origin->m_nodes)
    {
        if (node->isReflected())
        {
            if (auto classLikeNode = as<ClassLikeNode>(node))
            {
                if (classLikeNode->m_marker.getContent().indexOf(UnownedStringSlice::fromLiteral("ABSTRACT")) >= 0)
                {
                    out << "ABSTRACT_";
                }

                out << "SYNTAX_CLASS(" << node->m_name.getContent() << ", " << classLikeNode->m_super.getContent() << ")\n";
                out << "END_SYNTAX_CLASS()\n\n";
            }
        }
    }
    return SLANG_OK;
}

SlangResult App::calcChildrenHeader(Parser& extractor, TypeSet* typeSet, StringBuilder& out)
{
    const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    List<ClassLikeNode*> classNodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        ClassLikeNode* baseType = baseTypes[i];        
        baseType->calcDerivedDepthFirst(classNodes);
    }

    //Node::filter(Node::isClassLike, nodes);

    List<ClassLikeNode*> derivedTypes;

    out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHILDREN !!!!!!!!!!!!!!!!!!!!!!!!!!!! */ \n\n";

    // Now the children 
    for (ClassLikeNode* classNode : classNodes)
    {
        classNode->getReflectedDerivedTypes(derivedTypes);

        // Define the derived types
        out << "#define " << m_options.m_markPrefix << "CHILDREN_" << reflectTypeName << "_"  << classNode->m_name.getContent() << "(x, param)";

        if (derivedTypes.getCount())
        {
            out << " \\\n";
            for (Index j = 0; j < derivedTypes.getCount(); ++j)
            {
                Node* derivedType = derivedTypes[j];
                _indent(1, out);
                out << m_options.m_markPrefix << "ALL_" << reflectTypeName << "_" << derivedType->m_name.getContent() << "(x, param)";
                if (j < derivedTypes.getCount() - 1)
                {
                    out << "\\\n";
                }
            }    
        }
        out << "\n\n";
    }

    out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ALL !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

    for (ClassLikeNode* classNode : classNodes)
    {
        // Define the derived types
        out << "#define " << m_options.m_markPrefix << "ALL_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(x, param) \\\n";
        _indent(1, out);
        out << m_options.m_markPrefix << reflectTypeName << "_"  << classNode->m_name.getContent() << "(x, param)";

        // If has derived types output them
        if (classNode->hasReflectedDerivedType())
        {
            out << " \\\n";
            _indent(1, out);
            out << m_options.m_markPrefix << "CHILDREN_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(x, param)";
        }
        out << "\n\n";
    }

    if (m_options.m_outputFields)
    {
        out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

        for (ClassLikeNode* classNode : classNodes)
        {
            // Define the derived types
            out << "#define " << m_options.m_markPrefix << "FIELDS_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(_x_, _param_)";

            // Find all of the fields
            List<FieldNode*> fields;
            for (Node* child : classNode->m_children)
            {
                if (auto field = as<FieldNode>(child))
                {
                    fields.add(field);
                }
            }

            if (fields.getCount() > 0)
            {
                out << "\\\n";

                const Index fieldsCount = fields.getCount();
                bool previousField = false;
                for (Index j = 0; j < fieldsCount; ++j) 
                {
                    const FieldNode* field = fields[j];
                        
                    if (field->isReflected())
                    {
                        if (previousField)
                        {
                            out << "\\\n";
                        }

                        _indent(1, out);

                        // NOTE! We put the type field in brackets, such that there is no issue with templates containing a comma.
                        // If stringified
                        out << "_x_(" << field->m_name.getContent() << ", (" << field->m_fieldType << "), _param_)";
                        previousField = true;
                    }
                }
            }
                
            out << "\n\n";
        }
    }

    return SLANG_OK;
}

SlangResult App::calcOriginHeader(Parser& extractor, StringBuilder& out)
{
    // Do macros by origin

    out << "// Origin macros\n\n";

    for (SourceOrigin* origin : extractor.getSourceOrigins())
    {   
        out << "#define " << m_options.m_markPrefix << "ORIGIN_" << origin->m_macroOrigin << "(x, param) \\\n";

        for (Node* node : origin->m_nodes)
        {
            if (!(node->isReflected() && node->isClassLike()))
            {
                continue;
            }

            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", param) \\\n";
        }
        out << "/* */\n\n";
    }

    return SLANG_OK;
}

SlangResult App::calcTypeHeader(Parser& extractor, TypeSet* typeSet, StringBuilder& out)
{
    const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    if (baseTypes.getCount() == 0)
    {
        return SLANG_OK;
    }

    // Set up the scope
    List<Node*> baseScopePath;
    baseTypes[0]->calcScopePath(baseScopePath);

    // Remove the global scope
    baseScopePath.removeAt(0);
    // Remove the type itself
    baseScopePath.removeLast();

    for (Node* scopeNode : baseScopePath)
    {
        SLANG_ASSERT(scopeNode->m_type == Node::Type::Namespace);
        out << "namespace " << scopeNode->m_name.getContent() << " {\n";
    }

    // Add all the base types, with in order traversals
    List<ClassLikeNode*> nodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        ClassLikeNode* baseType = baseTypes[i];
        baseType->calcDerivedDepthFirst(nodes);
    }

    Node::filter(Node::isClassLikeAndReflected, nodes);

    // Write out the types
    {
        out << "\n";
        out << "enum class " << reflectTypeName << "Type\n";
        out << "{\n";

        Index typeIndex = 0;
        for (ClassLikeNode* node : nodes)
        {
            // Okay first we are going to output the enum values
            const Index depth = node->calcDerivedDepth() - 1;
            _indent(depth, out);
            out << node->m_name.getContent() << " = " << typeIndex << ",\n";
            typeIndex++;
        }

        _indent(1, out);
        out << "CountOf\n";

        out << "};\n\n";
    }

    // TODO(JS):
    // Strictly speaking if we wanted the types to be in different scopes, we would have to
    // change the namespaces here

    // Predeclare the classes
    {
        out << "// Predeclare\n\n";
        for (ClassLikeNode* node : nodes)
        {
            // If it's not reflected we don't output, in the enum list
            if (node->isReflected())
            {
                const char* type = (node->m_type == Node::Type::ClassType) ? "class" : "struct";
                out << type << " " << node->m_name.getContent() << ";\n";
            }
        }
    }

    // Do the macros for each of the types

    {
        out << "// Type macros\n\n";

        out << "// Order is (NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \n";
        out << "// NAME - is the class name\n";
        out << "// SUPER - is the super class name (or NO_SUPER)\n";
        out << "// ORIGIN - where the definition was found\n";
        out << "// LAST - is the class name for the last in the range (or NO_LAST)\n";
        out << "// MARKER - is the text inbetween in the prefix/postix (like ABSTRACT). If no inbetween text is is 'NONE'\n";
        out << "// TYPE - Can be BASE, INNER or LEAF for the overall base class, an INNER class, or a LEAF class\n";
        out << "// param is a user defined parameter that can be parsed to the invoked x macro\n\n";

        // Output all of the definitions for each type
        for (ClassLikeNode* node : nodes)
        {
            out << "#define " << m_options.m_markPrefix <<  reflectTypeName << "_" << node->m_name.getContent() << "(x, param) ";

            // Output the X macro part
            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", ";

            if (node->m_superNode)
            {
                out << node->m_superNode->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_SUPER, ";
            }

            // Output the (file origin)
            out << node->m_origin->m_macroOrigin;
            out << ", ";

            // The last type
            Node* lastDerived = node->findLastDerived();
            if (lastDerived)
            {
                out << lastDerived->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_LAST, ";
            }

            // Output any specifics of the markup
            UnownedStringSlice marker = node->m_marker.getContent();
            // Need to extract the name
            if (marker.getLength() > m_options.m_markPrefix.getLength() + m_options.m_markSuffix.getLength())
            {
                marker = UnownedStringSlice(marker.begin() + m_options.m_markPrefix.getLength(), marker.end() - m_options.m_markSuffix.getLength());
            }
            else
            {
                marker = UnownedStringSlice::fromLiteral("NONE");
            }
            out << marker << ", ";

            if (node->m_superNode == nullptr)
            {
                out << "BASE, ";
            }
            else if (node->hasReflectedDerivedType())
            {
                out << "INNER, ";
            }
            else
            {
                out << "LEAF, ";
            }
            out << "param)\n";
        }
    }

    // Now pop the scope in revers
    for (Index j = baseScopePath.getCount() - 1; j >= 0; j--)
    {
        Node* scopeNode = baseScopePath[j];
        out << "} // namespace " << scopeNode->m_name.getContent() << "\n";
    }

    return SLANG_OK;
}

SlangResult App::writeDefs(Parser& extractor)
{
    const auto& origins = extractor.getSourceOrigins();

    for (SourceOrigin* origin : origins)
    {
        const String path = origin->m_sourceFile->getPathInfo().foundPath;

        // We need to work out the name of the def file

        String ext = Path::getPathExt(path);
        String pathWithoutExt = Path::getPathWithoutExt(path);

        // The output path

        StringBuilder outPath;
        outPath << pathWithoutExt << "-defs." << ext;

        StringBuilder content;
        SLANG_RETURN_ON_FAIL(calcDef(extractor, origin, content));

        // Write the defs file
        SLANG_RETURN_ON_FAIL(writeAllText(outPath, content.getUnownedSlice()));
    }

    return SLANG_OK;
}

SlangResult App::writeOutput(Parser& extractor)
{
    String path;
    if (m_options.m_inputDirectory.getLength())
    {
        path = Path::combine(m_options.m_inputDirectory, m_options.m_outputPath);
    }
    else
    {
        path = m_options.m_outputPath;
    }

    // Get the ext
    String ext = Path::getPathExt(path);
    if (ext.getLength() == 0)
    {
        // Default to .h if not specified
        ext = "h";
    }

    // Strip the extension if set
    path = Path::getPathWithoutExt(path);

    for (TypeSet* typeSet : extractor.getTypeSets())
    {
        {
            /// Calculate the header
            StringBuilder header;
            SLANG_RETURN_ON_FAIL(calcTypeHeader(extractor, typeSet, header));

            // Write it out

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "." << ext;
            SLANG_RETURN_ON_FAIL(writeAllText(headerPath, header.getUnownedSlice()));
        }

        {
            StringBuilder childrenHeader;
            SLANG_RETURN_ON_FAIL(calcChildrenHeader(extractor, typeSet, childrenHeader));

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "-macro." + ext;
            SLANG_RETURN_ON_FAIL(writeAllText(headerPath, childrenHeader.getUnownedSlice()));
        }
    }

    return SLANG_OK;
}

/* static */void App::_initIdentifierLookup(const Options& options, IdentifierLookup& outLookup)
{
    outLookup.reset();

    // Some keywords
    {
        const char* names[] = { "virtual", "typedef", "continue", "if", "case", "break", "catch", "default", "delete", "do", "else", "for", "new", "goto", "return", "switch", "throw", "using", "while", "operator" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Keyword);
    }

    // Type modifier keywords
    {
        const char* names[] = { "const", "volatile" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::TypeModifier);
    }

    // Special markers
    {
        const char* names[] = {"PRE_DECLARE", "TYPE_SET", "REFLECTED", "UNREFLECTED"};
        const IdentifierStyle styles[] = { IdentifierStyle::PreDeclare, IdentifierStyle::TypeSet, IdentifierStyle::Reflected, IdentifierStyle::Unreflected };
        SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(names) == SLANG_COUNT_OF(styles));

        StringBuilder buf;        
        for (Index i = 0; i < SLANG_COUNT_OF(names); ++i)
        {
            buf.Clear();
            buf << options.m_markPrefix << names[i];
            outLookup.set(buf.getUnownedSlice(), styles[i]);
        }
    }

    // Keywords which introduce types/scopes
    {
        outLookup.set("struct", IdentifierStyle::Struct);
        outLookup.set("class", IdentifierStyle::Class);
        outLookup.set("namespace", IdentifierStyle::Namespace);
    }

    // Keywords that control access
    {
        const char* names[] = { "private", "protected", "public" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Access);
    }
}

SlangResult App::execute(const Options& options)
{
    m_options = options;

    IdentifierLookup identifierLookup;
    _initIdentifierLookup(options, identifierLookup);

    Parser extractor(&m_slicePool, &m_namePool, m_sink, &identifierLookup);

    // Read in each of the input files
    for (Index i = 0; i < m_options.m_inputPaths.getCount(); ++i)
    {
        String inputPath;

        if (m_options.m_inputDirectory.getLength())
        {
            inputPath = Path::combine(m_options.m_inputDirectory, m_options.m_inputPaths[i]);
        }
        else
        {
            inputPath = m_options.m_inputPaths[i];
        }

        // Read the input file
        String contents;
        SLANG_RETURN_ON_FAIL(readAllText(inputPath, contents));

        PathInfo pathInfo = PathInfo::makeFromString(inputPath);

        SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(pathInfo, contents);

        SLANG_RETURN_ON_FAIL(extractor.parse(sourceFile, &m_options));
    }

    SLANG_RETURN_ON_FAIL(extractor.calcDerivedTypes());

    // Okay let's check out the typeSets
    {
        for (TypeSet* typeSet : extractor.getTypeSets())
        {
            // The macro name is in upper snake, so split it 
            List<UnownedStringSlice> slices;
            NameConventionUtil::split(typeSet->m_macroName, slices);

            if (typeSet->m_fileMark.getLength() == 0)
            {
                StringBuilder buf;
                // Let's guess a 'fileMark' (it becomes part of the filename) based on the macro name. Use lower kabab.
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Lower, NameConvention::Kabab, buf);
                typeSet->m_fileMark = buf.ProduceString();
            }

            if (typeSet->m_typeName.getLength() == 0)
            {
                // Let's guess a typename if not set -> go with upper camel
                StringBuilder buf;
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Upper, NameConvention::Camel, buf);
                typeSet->m_typeName = buf.ProduceString();
            }
        }
    }

    // Dump out the tree
    if (options.m_dump)
    {
        {
            StringBuilder buf;
            extractor.getRootNode()->dump(0, buf);
            m_sink->writer->write(buf.getBuffer(), buf.getLength());
        }

        for (TypeSet* typeSet : extractor.getTypeSets())
        {
            const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;

            for (ClassLikeNode* baseType : baseTypes)
            {
                StringBuilder buf;
                baseType->dumpDerived(0, buf);
                m_sink->writer->write(buf.getBuffer(), buf.getLength());
            }
        }
    }

    if (options.m_defs)
    {
        SLANG_RETURN_ON_FAIL(writeDefs(extractor));
    }

    if (options.m_outputPath.getLength())
    {
        SLANG_RETURN_ON_FAIL(writeOutput(extractor));
    }

    return SLANG_OK;
}

/// Execute
SlangResult App::executeWithArgs(int argc, const char*const* argv)
{
    Options options;
    OptionsParser optionsParser;
    SLANG_RETURN_ON_FAIL(optionsParser.parse(argc, argv, m_sink, options));
    SLANG_RETURN_ON_FAIL(execute(options));
    return SLANG_OK;
}

} // namespace CppExtract

int main(int argc, const char*const* argv)
{
    using namespace CppExtract;
    using namespace Slang;

    {
        ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

        RootNamePool rootNamePool;

        SourceManager sourceManager;
        sourceManager.initialize(nullptr, nullptr);

        DiagnosticSink sink(&sourceManager, Lexer::sourceLocationLexer);
        sink.writer = writer;

        // Set to true to see command line that initiated C++ extractor. Helpful when finding issues from solution building failing, and then so
        // being able to repeat the issue
        bool dumpCommandLine = false;

        if (dumpCommandLine)
        {
            StringBuilder builder;

            for (Index i = 1; i < argc; ++i)
            {
                builder << argv[i] << " ";
            }

            sink.diagnose(SourceLoc(), CPPDiagnostics::commandLine, builder);
        }

        App app(&sink, &sourceManager, &rootNamePool);

        try
        {
            if (SLANG_FAILED(app.executeWithArgs(argc - 1, argv + 1)))
            {
                sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
                return 1;
            }
            if (sink.getErrorCount())
            {
                sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
                return 1;
            }
        }
        catch (...)
        {
            sink.diagnose(SourceLoc(), CPPDiagnostics::internalError);
            return 1;
        }
    }
    return 0;
}

