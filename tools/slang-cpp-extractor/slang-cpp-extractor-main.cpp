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

#include "../../source/slang/slang-source-loc.h"
#include "../../source/slang/slang-lexer.h"
#include "../../source/slang/slang-diagnostics.h"
#include "../../source/slang/slang-file-system.h"
#include "../../source/slang/slang-name.h"

namespace SlangExperimental
{

using namespace Slang;

struct TypeInfo
{
    enum class Type
    {
        Invalid,
        Struct,
        Class,
    };
    Type m_type = Type::Invalid;

    // These point to data in the input file
    UnownedStringSlice m_marker;
    UnownedStringSlice m_name;
    UnownedStringSlice m_super;

    StringSlicePool::Handle m_markerHandle = StringSlicePool::kEmptyHandle;     ///< From string pool
    StringSlicePool::Handle m_nameHandle = StringSlicePool::kEmptyHandle;       ///< The name of this class
    StringSlicePool::Handle m_superHandle = StringSlicePool::kEmptyHandle;      ///< Can be empty
    Index m_order = 0;
    Index m_sourceFileIndex = 0;
};


class TypeInfos
{
public:

    SlangResult add(const TypeInfo& inInfo, DiagnosticSink* sink)
    {
        TypeInfo info(inInfo);

        if (m_typePool.findOrAdd(info.m_name, info.m_nameHandle))
        {
            StringBuilder msg;
            msg << "Type with name  '" << info.m_name << "' already exists";
            sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
            return SLANG_FAIL;
        }

        if (info.m_super.getLength() > 0)
        {
            info.m_superHandle = m_typePool.add(info.m_super);
        }

        info.m_order = m_infos.getCount();
        info.m_sourceFileIndex = m_sourceFiles.getCount() - 1;
        m_infos.add(info);
        return SLANG_OK;
    }
    TypeInfos():
        m_typePool(StringSlicePool::Style::Default),
        m_stringPool(StringSlicePool::Style::Empty)
    {
        m_stringPool.add("SLANG_ABSTRACT_CLASS");
        m_stringPool.add("SLANG_CLASS");
    }

    List<SourceFile*> m_sourceFiles;
    List<TypeInfo> m_infos;                     ///< The infos found

    StringSlicePool m_typePool;                 ///< Only holds types
    StringSlicePool m_stringPool;               ///< Holds the markers
};

class CPPExtractor
{
public:

    static bool _isVisibilityKeyword(const UnownedStringSlice& slice)
    {
        return slice == UnownedStringSlice::fromLiteral("public") || slice == UnownedStringSlice::fromLiteral("protected") || slice == UnownedStringSlice::fromLiteral("private");
    }

    SlangResult expect(TokenType type, Token* outToken = nullptr)
    {
        if (m_reader.peekTokenType() != type)
        {
            StringBuilder buf;

            buf << "Expecting " << TokenTypeToString(type) << " found '" << TokenTypeToString(m_reader.peekTokenType());
            m_sink->diagnoseRaw(Severity::Error, buf.getUnownedSlice());

            return SLANG_FAIL;
        }

        if (outToken)
        {
            *outToken = m_reader.advanceToken();
        }
        else
        {
            m_reader.advanceToken();
        }
        return SLANG_OK;
    }

    bool advanceIfToken(TokenType type, Token* outToken = nullptr)
    {
        if (m_reader.peekTokenType() == type)
        {
            Token token = m_reader.advanceToken();
            if (outToken)
            {
                *outToken = token;
            }
            return true;
        }
        return false;
    }

    SlangResult _maybeParseTypeInfo(TypeInfo::Type type)
    {
        // We are looking for
        // struct/class identifier [: [public|private|protected] Identifier ] { [public|private|proctected:]* marker ( identifier );

        TypeInfo info;
        info.m_type = type;

        // consume class | struct
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier));

        // Next is the class name
        Token tok;
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &tok));
        info.m_name = tok.Content;

        if (m_reader.peekTokenType() == TokenType::Semicolon)
        {
            // pre declaration;
            return SLANG_OK;
        }

        if (advanceIfToken(TokenType::Colon))
        {
            // Could have public
            Token accessToken = m_reader.peekToken();
            if (accessToken.type == TokenType::Identifier && _isVisibilityKeyword(accessToken.Content))
            {
                m_reader.advanceToken();
            }

            Token superToken;
            if (!advanceIfToken(TokenType::Identifier, &superToken))
            {
                return SLANG_OK;
            }
            info.m_super = superToken.Content;
        }
            
        if (!advanceIfToken(TokenType::LBracket))
        {
            return SLANG_OK;
        }

        while (true)
        {
            // Okay now we are looking for the markers, or visibility qualifiers
            if (m_reader.peekTokenType() == TokenType::Identifier && _isVisibilityKeyword(m_reader.peekToken().Content))
            {
                m_reader.advanceToken();
                // Consume it and a colon
                if (!expect(TokenType::Colon))
                {
                    return SLANG_OK;
                }
                continue;
            }

            if (m_reader.peekTokenType() != TokenType::Identifier)
            {
                return SLANG_OK;
            }

            // If it's one of the markers, then we add it
            UnownedStringSlice lexeme = m_reader.peekToken().Content;
            Index markerIndex = m_infos->m_stringPool.findIndex(lexeme);
            if ( markerIndex >= 0)
            {
                info.m_marker = lexeme;
                info.m_markerHandle = StringSlicePool::Handle(markerIndex);
                m_reader.advanceToken();
                break;
            }

            // Didn't find the marker
            return SLANG_OK;
        }

        // Okay now looking for ( identifier)
        SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

        Token typeNameToken;
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeNameToken));
        
        if (typeNameToken.Content != info.m_name)
        {
            StringBuilder msg;
            msg << "Class name '" << info.m_name << "' doesn't match '" << typeNameToken.Content << "'";
            m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
            return SLANG_FAIL;
        }

        SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

        return m_infos->add(info, m_sink);
    }

    TypeInfo::Type _textToTypeInfo(const UnownedStringSlice& in)
    {
        if (in == UnownedStringSlice::fromLiteral("struct"))
        {
            return TypeInfo::Type::Struct;
        }
        else if (in == UnownedStringSlice::fromLiteral("class"))
        {
            return TypeInfo::Type::Class;
        }
        return TypeInfo::Type::Invalid;
    }

    SlangResult parse(SourceFile* sourceFile, NamePool* namePool, DiagnosticSink* sink)
    {
        SourceManager* manager = sourceFile->getSourceManager();

        SourceView* sourceView = manager->createSourceView(sourceFile, nullptr);

        Lexer lexer;

        lexer.initialize(sourceView, m_sink, namePool, manager->getMemoryArena());
        m_tokenList = lexer.lexAllTokens();
        // See if there were any errors
        if (m_sink->errorCount)
        {
            return SLANG_FAIL;
        }

        m_reader = TokenReader(m_tokenList);

        while (true)
        {
            switch (m_reader.peekTokenType())
            {
                case TokenType::Identifier:
                {
                    TypeInfo::Type type = _textToTypeInfo(m_reader.peekToken().Content);
                    if (type != TypeInfo::Type::Invalid)
                    {
                        SLANG_RETURN_ON_FAIL(_maybeParseTypeInfo(type));
                    }
                    break;
                }
                case TokenType::EndOfFile:
                {
                    return SLANG_OK;
                }
                default: break;
            }

            // Skip it then
            m_reader.advanceToken();
        }

    }

    CPPExtractor(TypeInfos* infos):
        m_infos(infos)
    {
    }

    TypeInfos* m_infos;
    
    TokenList m_tokenList;
    TokenReader m_reader;

    DiagnosticSink* m_sink;
};

class CPPExtractorApp
{
public:
    struct Options
    {
        void reset()
        {
            m_inputPaths.clear();
            m_outputPath = String();
        }

        List<String> m_inputPaths;
        String m_outputPath;
    };

    SlangResult readAllText(const Slang::String& fileName, String& outRead)
    {
        try
        {
            StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
            outRead = reader.ReadToEnd();

        }
        catch (const IOException& except)
        {
            m_sink->diagnoseRaw(Severity::Error, except.Message.getUnownedSlice());
            return SLANG_FAIL;
        }
        catch (...)
        {
            StringBuilder msg;
            msg << "Unable to read '" << fileName << "'";
            m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
            return SLANG_FAIL;
        }

        return SLANG_OK;
    }

    SlangResult parseContents(SourceFile* sourceFile, NamePool* namePool, TypeInfos* ioInfos)
    {
        ioInfos->m_sourceFiles.add(sourceFile);
     
        CPPExtractor parser(ioInfos);
        SLANG_RETURN_ON_FAIL(parser.parse(sourceFile, namePool, m_sink));
        return SLANG_OK;
    }

    SlangResult execute(const Options& options)
    {
        m_options = options;

        TypeInfos infos;

        // Read in each of the input files
        for (Index i = 0; i < m_options.m_inputPaths.getCount(); ++i)
        {
            const String& inputPath = m_options.m_inputPaths[i];

            // Read the input file
            String contents;
            SLANG_RETURN_ON_FAIL(readAllText(inputPath, contents));

            PathInfo pathInfo = PathInfo::makeFromString(inputPath);
            
            SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(pathInfo, contents);
            
            // Okay we now need to parse contents. We know this is 0 terminated (as all strings are).
            SLANG_RETURN_ON_FAIL(parseContents(sourceFile, &m_namePool, &infos));
        }
        return SLANG_OK;
    }

    /// Parse the parameters. NOTE! Must have the program path removed
    SlangResult parseArgs(int argc, const char*const* argv, Options& outOptions)
    {
        outOptions.reset();

        Index i = 0;
        while (i < argc)
        {
            const UnownedStringSlice arg = UnownedStringSlice(argv[i]);

            if (arg.getLength() > 0 && arg[0] == '-')
            {
                if (arg == "-o")
                {
                    if (i + 1 < argc)
                    {
                        // Next parameter is the output path, there can only be one

                        if (outOptions.m_outputPath.getLength())
                        {
                            // There already is output
                            StringBuilder msg;
                            msg << "Output has already been defined '" << outOptions.m_outputPath << "'";
                            m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                            return SLANG_FAIL;
                        }
                    }
                    else
                    {
                        StringBuilder msg;
                        msg << "Require a value after -o option";
                        m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                        return SLANG_FAIL;
                    }

                    outOptions.m_outputPath = argv[i + 1];
                    i += 2;
                    continue;
                }


                {
                    StringBuilder msg;
                    msg << "Unknown option '" << arg << "'";
                    m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                    return SLANG_FAIL;
                }
            }

            // If it starts with - then it an unknown option
            outOptions.m_inputPaths.add(arg);
            i++;
        }

        if (outOptions.m_inputPaths.getCount() < 0)
        {
            m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("No input paths specified"));
            return SLANG_FAIL;
        }
        if (outOptions.m_outputPath.getLength() == 0)
        {
            m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("No -o output path specified"));
            return SLANG_FAIL;
        }

        return SLANG_OK;
    }

    /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv)
    {
        Options options;
        SLANG_RETURN_ON_FAIL(parseArgs(argc, argv, options));
        SLANG_RETURN_ON_FAIL(execute(options));
        return SLANG_OK;
    }

    CPPExtractorApp(DiagnosticSink* sink, SourceManager* sourceManager):
        m_sink(sink),
        m_sourceManager(sourceManager)
    {}

    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;
};



} // namespace SlangExperimental

int main(int argc, const char*const* argv)
{
    using namespace SlangExperimental;
    using namespace Slang;

    {
        SourceManager sourceManager;
        sourceManager.initialize(nullptr, nullptr);

        ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

        DiagnosticSink sink(&sourceManager);
        sink.writer = writer;

        CPPExtractorApp app(&sink, &sourceManager);
        if (SLANG_FAILED(app.executeWithArgs(argc - 1, argv + 1)))
        {
            return 1;
        }
        if (sink.errorCount)
        {
            return 1;
        }

    }
    return 0;
}

