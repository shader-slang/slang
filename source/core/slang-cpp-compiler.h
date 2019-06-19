#ifndef SLANG_CPP_COMPILER_H
#define SLANG_CPP_COMPILER_H

#include "slang-common.h"
#include "slang-string.h"

#include "slang-process-util.h"

namespace Slang
{

class CPPCompiler: public RefObject
{
public:
    typedef RefObject Super;
    enum class Type
    {
        Unknown,
        VisualStudio,
        GCC,
        Clang,
        SNC,
        GHS,
        CountOf,
    };
    enum class SourceType
    {
        C,              ///< C source
        CPP,            ///< C++ source
    };

    struct Desc
    {
        typedef Desc ThisType;

        UInt GetHashCode() const { return combineHash(int(type), combineHash(int(majorVersion), int(minorVersion))); }
        bool operator==(const ThisType& rhs) const { return type == rhs.type && majorVersion == rhs.majorVersion && minorVersion == rhs.minorVersion;  }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Get the version as a value
        Int getVersionValue() const { return majorVersion * 100 + minorVersion;  }

            /// Ctor
        Desc(Type inType = Type::Unknown, Int inMajorVersion = 0, Int inMinorVersion = 0):type(inType), majorVersion(inMajorVersion), minorVersion(inMinorVersion) {}

        Type type;                      ///< The type of the compiler
        Int majorVersion;               ///< Major version (interpretation is type specific)
        Int minorVersion;               ///< Minor version
    };

    enum class OptimizationLevel
    {
        Normal,             ///< Normal optimization
        Debug,              ///< General has no optimizations
    };

    enum DebugInfoType
    {
        None,               ///< Binary has no debug information
        Maximum,            ///< Has maximum debug information
        Normal,             ///< Has normal debug information
    };
    enum TargetType
    {
        Executable,         ///< Produce an executable
        SharedLibrary,      ///< Produce a shared library object/dll 
        Object,             ///< Produce an object file
    };

    struct Define
    {
        String nameWithSig;             ///< If macro takes parameters include in brackets
        String value;
    };

    struct CompileOptions
    {
        typedef uint32_t Flags;
        struct Flag 
        {
            enum Enum : Flags
            {
                EnableExceptionHandling = 0x01,
            };
        };

        OptimizationLevel optimizationLevel = OptimizationLevel::Debug;
        DebugInfoType debugInfoType = DebugInfoType::Normal;
        TargetType targetType = TargetType::Executable;
        SourceType sourceType = SourceType::CPP;

        Flags flags = Flag::EnableExceptionHandling;

        String modulePath;      ///< The path/name of the output module. Should not have the extension, as that will be added for each of the target types

        List<Define> defines;

        List<String> sourceFiles;

        List<String> includePaths;
        List<String> libraryPaths;
    };

    struct OutputMessage
    {
        enum class Type
        {
            Unknown,
            Info,
            Warning,
            Error,
            CountOf,
        };
        enum class Stage
        {
            Compile,
            Link,
        };

        void reset()
        {
            type = Type::Unknown;
            stage = Stage::Compile;
            fileLine = 0;
        }

        Type type;                      ///< The type of error
        Stage stage;                    ///< The stage the error came from
        String text;                    ///< The text of the error
        String code;                    ///< The compiler specific error code
        String filePath;                ///< The path the error originated from
        Int fileLine;                   ///< The line number the error came from
    };

    struct Output
    {
        void reset() { messages.clear(); result = SLANG_OK; }
        
            /// Get the number of messages by type
        Index getCountByType(OutputMessage::Type type) const
        {
            Index count = 0;
            for (const auto& msg : messages)
            {
                count += Index(msg.type == type);
            }
            return count;
        }
            /// True if there are any messages of the type
        bool has(OutputMessage::Type type) const { return getCountByType(type) > 0; }

        Int countByStage(OutputMessage::Stage stage, Index counts[Int(OutputMessage::Type::CountOf)]) const
        {
            Int count = 0;
            ::memset(counts, 0, sizeof(Index) * Int(OutputMessage::Type::CountOf));
            for (const auto& msg : messages)
            {
                if (msg.stage == stage)
                {
                    count++;
                    counts[Index(msg.type)]++;
                }
            }
            return count++;
        }

        static UnownedStringSlice getTypeText(OutputMessage::Type type)
        {
            typedef OutputMessage::Type Type;
            switch (type)
            {
                default:            return UnownedStringSlice::fromLiteral("Unknown");
                case Type::Info:    return UnownedStringSlice::fromLiteral("Info");
                case Type::Warning: return UnownedStringSlice::fromLiteral("Warning");
                case Type::Error:   return UnownedStringSlice::fromLiteral("Error");
            }
        }

        static void appendCounts(const Index counts[Int(OutputMessage::Type::CountOf)], StringBuilder& out)
        {
            for (Index i = 0; i < Int(OutputMessage::Type::CountOf); i++)
            {
                if (counts[i] > 0)
                {
                    out << getTypeText(OutputMessage::Type(i)) << "(" << counts[i] << ") ";
                }
            }
        }
        static void appendHas(const Index counts[Int(OutputMessage::Type::CountOf)], StringBuilder& out)
        {
            for (Index i = 0; i < Int(OutputMessage::Type::CountOf); i++)
            {
                if (counts[i] > 0)
                {
                    out << getTypeText(OutputMessage::Type(i)) << " ";
                }
            }
        }
        void appendSummary(StringBuilder& out) const
        {
            Index counts[Int(OutputMessage::Type::CountOf)];
            if (countByStage(OutputMessage::Stage::Compile, counts) > 0)
            {
                out << "Compile: ";
                appendCounts(counts, out);
                out << "\n";
            }
            if (countByStage(OutputMessage::Stage::Link, counts) > 0)
            {
                out << "Link: ";
                appendCounts(counts, out);
                out << "\n";
            }
        }
            /// Appends a summary that just identifies if there is an error of a type (not a count)
        void appendHasSummary(StringBuilder& out) const
        {
            Index counts[Int(OutputMessage::Type::CountOf)];
            if (countByStage(OutputMessage::Stage::Compile, counts) > 0)
            {
                out << "Compile: ";
                appendHas(counts, out);
                out << "\n";
            }
            if (countByStage(OutputMessage::Stage::Link, counts) > 0)
            {
                out << "Link: ";
                appendHas(counts, out);
                out << "\n";
            }
        }

        void removeByType(OutputMessage::Type type)
        {
            Index count = messages.getCount();
            for (Index i = 0; i < count; ++i)
            {
                if (messages[i].type == type)
                {
                    messages.removeAt(i);
                    i--;
                    count--;
                }
            }
        }

        SlangResult result;
        List<OutputMessage> messages;
    };

        /// Get the desc of this compiler
    const Desc& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const CompileOptions& options, Output& outOutput) = 0;

protected:

    CPPCompiler(const Desc& desc) :
        m_desc(desc)
    {}

    Desc m_desc;
};

class GenericCPPCompiler : public CPPCompiler
{
public:
    typedef CPPCompiler Super;

    typedef void(*CalcArgsFunc)(const CPPCompiler::CompileOptions& options, CommandLine& cmdLine);
    typedef void(*ParseOutputFunc)(const ExecuteResult& exeResult, Output& output);

    virtual SlangResult compile(const CompileOptions& options, Output& outOutput) SLANG_OVERRIDE;

    GenericCPPCompiler(const Desc& desc, const String& exeName, CalcArgsFunc calcArgsFunc, ParseOutputFunc parseOutputFunc) :
        Super(desc),
        m_calcArgsFunc(calcArgsFunc),
        m_parseOutputFunc(parseOutputFunc)
    {
        m_cmdLine.setExecutableFilename(exeName);
    }

    GenericCPPCompiler(const Desc& desc, const CommandLine& cmdLine, CalcArgsFunc calcArgsFunc, ParseOutputFunc parseOutputFunc) :
        Super(desc),
        m_cmdLine(cmdLine),
        m_calcArgsFunc(calcArgsFunc),
        m_parseOutputFunc(parseOutputFunc)
    {}

    CalcArgsFunc m_calcArgsFunc;
    ParseOutputFunc m_parseOutputFunc;
    CommandLine m_cmdLine;
};

class CPPCompilerSet : public RefObject
{
public:
    typedef RefObject Super;

    
        /// Find all the available compilers
    void getCompilerDescs(List<CPPCompiler::Desc>& outCompilerDescs) const;
        /// Returns list of all compilers
    void getCompilers(List<CPPCompiler*>& outCompilers) const;

        /// Get a compiler
    CPPCompiler* getCompiler(const CPPCompiler::Desc& compilerDesc) const;
  
        /// Will replace if there is one with same desc
    void addCompiler(CPPCompiler* compiler);

        /// Get a default compiler
    CPPCompiler* getDefaultCompiler() const { return m_defaultCompiler;  }
        /// Set the default compiler
    void setDefaultCompiler(CPPCompiler* compiler) { m_defaultCompiler = compiler;  }

protected:

    Index _findIndex(const CPPCompiler::Desc& desc) const;

    RefPtr<CPPCompiler> m_defaultCompiler;
    // This could be a dictionary/map - but doing a linear search is going to be fine and it makes
    // somethings easier.
    List<RefPtr<CPPCompiler>> m_compilers;
};

struct CPPCompilerUtil
{
    typedef CPPCompiler::CompileOptions CompileOptions;
    typedef CPPCompiler::OptimizationLevel OptimizationLevel;
    typedef CPPCompiler::TargetType TargetType;
    typedef CPPCompiler::DebugInfoType DebugInfoType;
    typedef CPPCompiler::SourceType SourceType;

    enum class MatchType
    {
        MinGreaterEqual,
        MinAbsolute,
        Newest,
    };

        /// Find a compiler
    static CPPCompiler* findCompiler(const CPPCompilerSet* set, MatchType matchType, const CPPCompiler::Desc& desc);
    static CPPCompiler* findCompiler(const List<CPPCompiler*>& compilers, MatchType matchType, const CPPCompiler::Desc& desc);

        /// Find the compiler closest to the desc 
    static CPPCompiler* findClosestCompiler(const List<CPPCompiler*>& compilers, const CPPCompiler::Desc& desc);
    static CPPCompiler* findClosestCompiler(const CPPCompilerSet* set, const CPPCompiler::Desc& desc);

        /// Get the information on the compiler used to compile this source
    static const CPPCompiler::Desc& getCompiledWithDesc();

        /// Given a set, registers compilers found through standard means and determines a reasonable default compiler if possible
    static SlangResult initializeSet(CPPCompilerSet* set);
};


}

#endif
