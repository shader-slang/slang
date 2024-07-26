#include <filesystem>

#include "slang.h"
#include "json-consumer.h"
#include "../util/record-utility.h"
#include "../util/emum-to-string.h"
#include "../../core/slang-string-util.h"

namespace SlangRecord
{
#define SANITY_CHECK() if (!m_isFileValid) return

#define WRITE_INDENT(builder, indent)            \
    do {                                        \
        for (int i = 0; i < (indent); i++) {    \
            (builder) << "  ";                  \
        }                                       \
    } while (0)

#define WRITE_STRING(builder, indent, str)      \
    do {                                        \
        WRITE_INDENT((builder), (indent));      \
        (builder) << (str);                     \
    } while (0)

#define WRITE_PAIR(builder, indent, name, value)            \
    do                                                      \
    {                                                       \
        WRITE_INDENT((builder), (indent));                  \
        (builder) << (name) << ": " << (value) << ",\n";    \
    } while (0)

#define WRITE_PAIR_NO_COMMA(builder, indent, name, value)   \
    do                                                      \
    {                                                       \
        WRITE_INDENT((builder), (indent));                  \
        (builder) << (name) << ": " << (value) << "\n";     \
    } while (0)

    class ScopeWritterForKey
    {
    public:
        ScopeWritterForKey(Slang::StringBuilder* pBuilder, int* pIndent, Slang::String const& keyName, bool isOutterScope = true)
            : m_pBuilder(pBuilder)
            , m_pIndent(pIndent)
            , m_isOutterScope(isOutterScope)
        {
            WRITE_STRING((*m_pBuilder), (*m_pIndent), keyName + ": {\n");
            (*m_pIndent)++;
        }

        ScopeWritterForKey(Slang::StringBuilder* pBuilder, int* pIndent, Slang::String const& keyName, Slang::String const& decoration)
            : m_pBuilder(pBuilder)
            , m_pIndent(pIndent)
        {
            WRITE_STRING((*m_pBuilder), (*m_pIndent), keyName + ":" + decoration + " {\n");
            (*m_pIndent)++;
        }

        ~ScopeWritterForKey()
        {
            (*m_pIndent)--;

            if (m_isOutterScope)
                WRITE_STRING((*m_pBuilder), (*m_pIndent), "}\n");
            else
                WRITE_STRING((*m_pBuilder), (*m_pIndent), "},\n");

        }
    private:
        Slang::StringBuilder* m_pBuilder;
        int* m_pIndent;
        bool m_isOutterScope;
    };

    void CommonInterfaceWriter::getSession(ObjectID objectId, ObjectID outSessionId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getSession";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retSession", Slang::StringUtil::makeStringWithFormat("0x%X", outSessionId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getLayout";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retProgramLayout", Slang::StringUtil::makeStringWithFormat("0x%X", retProgramLayoutId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getEntryPointCode";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "entryPointIndex", entryPointIndex);
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR(builder, indent, "outCode", Slang::StringUtil::makeStringWithFormat("0x%X", outCodeId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getTargetCode";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR(builder, indent, "outCode", Slang::StringUtil::makeStringWithFormat("0x%X", outCodeId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystemId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getResultAsFileSystem";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "entryPointIndex", entryPointIndex);
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR_NO_COMMA(builder, indent, "outFileSystem", Slang::StringUtil::makeStringWithFormat("0x%X", outFileSystemId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getEntryPointHash";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "entryPointIndex", entryPointIndex);
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR_NO_COMMA(builder, indent, "outHash", Slang::StringUtil::makeStringWithFormat("0x%X", outHashId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
            SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::specialize";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                if (specializationArgCount)
                {
                    ScopeWritterForKey scopeWritterForArgs(&builder, &indent, "specializationArgs", false);
                    for(int i = 0; i < specializationArgCount; i++)
                    {
                        bool isLastField = (i == specializationArgCount - 1);
                        ScopeWritterForKey scopeWritterForArg(&builder, &indent, Slang::StringUtil::makeStringWithFormat("[%d]\n", i), isLastField);
                        {
                            WRITE_PAIR(builder, indent, "kind", SpecializationArgKindToString(specializationArgs[i].kind));
                            WRITE_PAIR_NO_COMMA(builder, indent, "type", Slang::StringUtil::makeStringWithFormat("0x%X", specializationArgs[i].type));
                        }
                    }
                }
                else
                {
                    WRITE_PAIR(builder, indent, "specializationArgs", "nullptr");
                }
                WRITE_PAIR(builder, indent, "specializationArgCount", specializationArgCount);
                WRITE_PAIR(builder, indent, "outSpecializedComponentType", Slang::StringUtil::makeStringWithFormat("0x%X", outSpecializedComponentTypeId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outSpecializedComponentType", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::link";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "outLinkedComponentType", Slang::StringUtil::makeStringWithFormat("0x%X", outLinkedComponentTypeId));
                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibraryId,
            ObjectID outDiagnosticsId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::getEntryPointHostCallable";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "entryPointIndex", entryPointIndex);
                WRITE_PAIR(builder, indent, "targetIndex", targetIndex);
                WRITE_PAIR(builder, indent, "outSharedLibrary", Slang::StringUtil::makeStringWithFormat("0x%X", outSharedLibraryId));
                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::renameEntryPoint";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "newName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            newName != nullptr ? newName : "nullptr"));
                WRITE_PAIR(builder, indent, "outEntryPoint", Slang::StringUtil::makeStringWithFormat("0x%X", outEntryPointId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }

    void CommonInterfaceWriter::linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
            uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {

        Slang::StringBuilder builder;
        int indent = 0;

        Slang::String functionName = m_className;
        functionName = functionName + "::linkWithOptions";

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, functionName);
            {

                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "compilerOptionEntryCount", compilerOptionEntryCount);

                JsonConsumer::_writeCompilerOptionEntryHelper(builder, indent, compilerOptionEntries, compilerOptionEntryCount);

                WRITE_PAIR(builder, indent, "outLinkedComponentType", Slang::StringUtil::makeStringWithFormat("0x%X", outLinkedComponentTypeId));
                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            }
        }

        m_fileStream.write(builder.begin(), builder.getLength());
        m_fileStream.flush();
    }


    JsonConsumer::JsonConsumer(const std::string& filePath)
    {
        std::filesystem::path jsonFileDir(filePath);
        jsonFileDir = std::filesystem::absolute(jsonFileDir);

        if (!std::filesystem::exists(jsonFileDir.parent_path()))
        {
            slangRecordLog(LogLevel::Error, "Directory for json file does not exist: %s\n", filePath.c_str());
        }

        Slang::String path(filePath.c_str());
        Slang::FileMode fileMode = Slang::FileMode::Create;
        Slang::FileAccess fileAccess = Slang::FileAccess::Write;
        Slang::FileShare fileShare = Slang::FileShare::None;

        SlangResult res = m_fileStream.init(path, fileMode, fileAccess, fileShare);

        if (res != SLANG_OK)
        {
            slangRecordLog(LogLevel::Error, "Failed to open file %s\n", filePath.c_str());
        }

        m_isFileValid = true;
    }

    void JsonConsumer::_writeCompilerOptionEntryHelper(Slang::StringBuilder& builder, int indent, slang::CompilerOptionEntry* compilerOptionEntries, uint32_t compilerOptionEntryCount, bool isLastField)
    {
        if (compilerOptionEntryCount)
        {
            ScopeWritterForKey scopeWritterForCompilerOptionEntries(&builder, &indent, "compilerOptionEntries", isLastField);

            for (uint32_t j = 0; j < compilerOptionEntryCount; j++)
            {
                ScopeWritterForKey scopeWritterForCompileOptionElement(&builder, &indent, Slang::StringUtil::makeStringWithFormat("[%d]\n", j));
                {
                    WRITE_PAIR(builder, indent, "name", CompilerOptionNameToString(compilerOptionEntries[j].name));

                    bool isLastEntry = (j == compilerOptionEntryCount - 1);
                    ScopeWritterForKey scopeWritterValue(&builder, &indent, "value", isLastEntry);
                    {
                        WRITE_PAIR(builder, indent, "kind", CompilerOptionValueKindToString(compilerOptionEntries[j].value.kind));
                        WRITE_PAIR(builder, indent, "intValue0", compilerOptionEntries[j].value.intValue0);
                        WRITE_PAIR(builder, indent, "intValue1", compilerOptionEntries[j].value.intValue1);
                        WRITE_PAIR(builder, indent, "stringValue0", compilerOptionEntries[j].value.stringValue0);
                        WRITE_PAIR_NO_COMMA(builder, indent, "stringValue1", compilerOptionEntries[j].value.stringValue1);
                    }
                }
            }
        }
        else
        {
            WRITE_PAIR_NO_COMMA(builder, indent, "compilerOptionEntries", "nullptr");
        }
    }

    void JsonConsumer::_writeSessionDescHelper(Slang::StringBuilder& builder, int indent, slang::SessionDesc const& desc, Slang::String keyName, bool isLastField)
    {
        ScopeWritterForKey scopeWritterForSessionDesc(&builder, &indent, keyName);
        {
            WRITE_PAIR(builder, indent, "structureSize", desc.structureSize);

            if (desc.targetCount)
            {
                ScopeWritterForKey scopeWritterForTarget(&builder, &indent, Slang::StringUtil::makeStringWithFormat("targets (0x%X)", desc.targets), isLastField);
                {
                    for (int i = 0; i < desc.targetCount; i++)
                    {
                        bool isLastEntry = (i == desc.targetCount - 1);
                        ScopeWritterForKey scopeWritterForTargetElement(&builder, &indent, Slang::StringUtil::makeStringWithFormat("[%d]", i), isLastEntry);
                        {
                            WRITE_PAIR(builder, indent, "structureSize", desc.targets[i].structureSize);
                            WRITE_PAIR(builder, indent, "format", SlangCompileTargetToString(desc.targets[i].format));
                            WRITE_PAIR(builder, indent, "profile", SlangProfileIDToString(desc.targets[i].profile));
                            WRITE_PAIR(builder, indent, "flags", SlangTargetFlagsToString(desc.targets[i].flags));
                            WRITE_PAIR(builder, indent, "floatingPointMode", SlangFloatingPointModeToString(desc.targets[i].floatingPointMode));
                            WRITE_PAIR(builder, indent, "lineDirectiveMode", SlangLineDirectiveModeToString(desc.targets[i].lineDirectiveMode));
                            WRITE_PAIR(builder, indent, "forceGLSLScalarBufferLayout", (desc.targets[i].floatingPointMode ? "true" : "false"));

                            _writeCompilerOptionEntryHelper(builder, indent, desc.targets[i].compilerOptionEntries, desc.targets[i].compilerOptionEntryCount);
                        }
                    }
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "targets", "nullptr");
            }

            WRITE_PAIR(builder, indent, "targetCount", desc.targetCount);
            WRITE_PAIR(builder, indent, "flags", SessionFlagsToString(desc.flags));
            WRITE_PAIR(builder, indent, "defaultMatrixLayoutMode", SlangMatrixLayoutModeToString(desc.defaultMatrixLayoutMode));

            if (desc.searchPathCount)
            {
                ScopeWritterForKey scopeWritterForSearchPath(&builder, &indent, "searchPaths", false);
                for (int i = 0; i < desc.searchPathCount; i++)
                {
                    Slang::String searchPath(desc.searchPaths[i]);
                    searchPath = searchPath + ",\n";
                    WRITE_STRING(builder, indent, searchPath);
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "searchPaths", "nullptr");
            }
            WRITE_PAIR(builder, indent, "searchPathCount", desc.searchPathCount);

            if (desc.preprocessorMacroCount)
            {
                ScopeWritterForKey scopeWritterForMacro(&builder, &indent, "preprocessorMacros", false);
                for (int i = 0; i < desc.preprocessorMacroCount; i++)
                {
                    bool isLastField = (i == desc.preprocessorMacroCount - 1);
                    ScopeWritterForKey scopeWritterForMacroElement(&builder, &indent, Slang::StringUtil::makeStringWithFormat("[%d]", i), isLastField);

                    WRITE_PAIR(builder, indent, "name", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                                desc.preprocessorMacros[i].name != nullptr ? desc.preprocessorMacros[i].name : "nullptr"));

                    WRITE_PAIR(builder, indent, "value", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                                desc.preprocessorMacros[i].value != nullptr ? desc.preprocessorMacros[i].value : "nullptr"));
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "preprocessorMacros", "nullptr");
            }
            WRITE_PAIR(builder, indent, "preprocessorMacroCount", desc.preprocessorMacroCount);

            AddressFormat address = reinterpret_cast<AddressFormat>(desc.fileSystem);
            WRITE_PAIR(builder, indent, "fileSystem", Slang::StringUtil::makeStringWithFormat("0x%X", address));
            WRITE_PAIR(builder, indent, "enableEffectAnnotations", (desc.enableEffectAnnotations ? "true" : "false"));
            WRITE_PAIR(builder, indent, "allowGLSLSyntax", (desc.allowGLSLSyntax ? "true" : "false"));
            WRITE_PAIR(builder, indent, "compilerOptionEntryCount", desc.compilerOptionEntryCount);
            _writeCompilerOptionEntryHelper(builder, indent, desc.compilerOptionEntries, desc.compilerOptionEntryCount);
        }

    }

    void JsonConsumer::CreateGlobalSession(ObjectID outGlobalSessionId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "GlobalFunction::createGlobalSession: {\n");
        {
            indent++;
            WRITE_PAIR_NO_COMMA(builder, indent, "outGlobalSession", Slang::StringUtil::makeStringWithFormat("0x%X", outGlobalSessionId));
            indent--;
        }
        WRITE_STRING(builder, indent, "}\n");

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::IGlobalSession_createSession(ObjectID objectId, slang::SessionDesc const&  desc, ObjectID outSessionId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "IGlobalSession::createSession: {\n");

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::createSession");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));

                _writeSessionDescHelper(builder, indent, desc, "inDesc");

                WRITE_PAIR_NO_COMMA(builder, indent, "outSession", Slang::StringUtil::makeStringWithFormat("0x%X", outSessionId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::IGlobalSession_findProfile(ObjectID objectId, char const* name)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::findProfile");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "name", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            name != nullptr ? name : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerPath(ObjectID objectId, SlangPassThrough passThrough, char const* path)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setDownstreamCompilerPath");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "passThrough", SlangPassThroughToString(passThrough));
                WRITE_PAIR_NO_COMMA(builder, indent, "path", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            path != nullptr ? path : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, char const* prelude)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setDownstreamCompilerPrelude");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "passThrough", SlangPassThroughToString(inPassThrough));
                WRITE_PAIR_NO_COMMA(builder, indent, "preludeText", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            prelude != nullptr ? prelude : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, ObjectID outPreludeId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getDownstreamCompilerPrelude");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "passThrough", SlangPassThroughToString(inPassThrough));
                WRITE_PAIR_NO_COMMA(builder, indent, "outPrelude", Slang::StringUtil::makeStringWithFormat("0x%X", outPreludeId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setDefaultDownstreamCompiler");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "sourceLanguage", SlangSourceLanguageToString(sourceLanguage));
                WRITE_PAIR_NO_COMMA(builder, indent, "defaultCompiler", SlangPassThroughToString(defaultCompiler));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getDefaultDownstreamCompiler");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "sourceLanguage", SlangSourceLanguageToString(sourceLanguage));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, char const* prelude)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "IGlobalSession::setLanguagePrelude: {\n");

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setLanguagePrelude");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "sourceLanguage", SlangSourceLanguageToString(inSourceLanguage));
                WRITE_PAIR_NO_COMMA(builder, indent, "preludeText", (prelude != nullptr ? prelude : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, ObjectID outPreludeId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getLanguagePrelude");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "sourceLanguage", SlangSourceLanguageToString(inSourceLanguage));
                WRITE_PAIR_NO_COMMA(builder, indent, "outPrelude", Slang::StringUtil::makeStringWithFormat("0x%X", outPreludeId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequest)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "IGlobalSession::createCompileRequest: {\n");

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::createCompileRequest");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outCompileRequest", Slang::StringUtil::makeStringWithFormat("0x%X", outCompileRequest));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_addBuiltins(ObjectID objectId, char const* sourcePath, char const* sourceString)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "IGlobalSession::addBuiltins: {\n");

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::addBuiltins");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "sourcePath", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            sourcePath != nullptr ? sourcePath : "nullptr"));
                WRITE_PAIR_NO_COMMA(builder, indent, "sourceString", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            sourceString != nullptr ? sourceString : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setSharedLibraryLoader(ObjectID objectId, ObjectID loaderId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setSharedLibraryLoader");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "loader", Slang::StringUtil::makeStringWithFormat("0x%X", loaderId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getSharedLibraryLoader(ObjectID objectId, ObjectID outLoaderId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getSharedLibraryLoader");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retLoader", Slang::StringUtil::makeStringWithFormat("0x%X", outLoaderId));
            }
        }

        WRITE_STRING(builder, indent, "}\n");

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_checkCompileTargetSupport(ObjectID objectId, SlangCompileTarget target)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::checkCompileTargetSupport");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "target", SlangCompileTargetToString(target));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_checkPassThroughSupport(ObjectID objectId, SlangPassThrough passThrough)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::checkPassThroughSupport");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "passThrough", SlangPassThroughToString(passThrough));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_compileStdLib(ObjectID objectId, slang::CompileStdLibFlags flags)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::compileStdLib");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "flags", CompileStdLibFlagsToString(flags));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_loadStdLib(ObjectID objectId, const void* stdLib, size_t stdLibSizeInBytes)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;
        WRITE_STRING(builder, indent, "IGlobalSession::loadStdLib: {\n");

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::loadStdLib");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "stdLib-Ignore-Data", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "stdLibSizeInBytes", stdLibSizeInBytes);
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::IGlobalSession_saveStdLib(ObjectID objectId, SlangArchiveType archiveType, ObjectID outBlobId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::saveStdLib");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "archiveType", SlangArchiveTypeToString(archiveType));
                WRITE_PAIR_NO_COMMA(builder, indent, "outBlobId", Slang::StringUtil::makeStringWithFormat("0x%X", outBlobId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_findCapability(ObjectID objectId, char const* name)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::findCapability");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "name", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            name != nullptr ? name : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setDownstreamCompilerForTransition");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "source", SlangCompileTargetToString(source));
                WRITE_PAIR(builder, indent, "target", SlangCompileTargetToString(target));
                WRITE_PAIR_NO_COMMA(builder, indent, "compiler", SlangPassThroughToString(compiler));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getDownstreamCompilerForTransition");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "source", SlangCompileTargetToString(source));
                WRITE_PAIR_NO_COMMA(builder, indent, "target", SlangCompileTargetToString(target));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_setSPIRVCoreGrammar(ObjectID objectId, char const* jsonPath)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::setSPIRVCoreGrammar");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "jsonPath", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            jsonPath != nullptr ? jsonPath : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_parseCommandLineArguments(ObjectID objectId, int argc, const char* const* argv, ObjectID outSessionDescId, ObjectID outAllocationId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::parseCommandLineArguments");
            WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
            WRITE_PAIR(builder, indent, "argc", argc);

            if (argv)
            {
                WRITE_STRING(builder, indent, "argv: {\n");
                ScopeWritterForKey scopeWritteriForArgv(&builder, &indent, "argv");
                for (int i = 0; i < argc; i++)
                {
                    if (i == (argc -1))
                        WRITE_PAIR_NO_COMMA(builder, indent, Slang::StringUtil::makeStringWithFormat("[%d]", i), argv[i]);
                    else
                        WRITE_PAIR(builder, indent, Slang::StringUtil::makeStringWithFormat("[%d]", i), argv[i]);
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "argv", "nullptr");
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IGlobalSession_getSessionDescDigest(ObjectID objectId, slang::SessionDesc* sessionDesc, ObjectID outBlobId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IGlobalSession::getSessionDescDigest");
            WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));

            if (sessionDesc)
            {
                _writeSessionDescHelper(builder, indent, *sessionDesc, Slang::StringUtil::makeStringWithFormat("sessionDesc (0x%X)\n", sessionDesc));
            }
            else
            {
                WRITE_PAIR(builder, indent, "sessionDesc", "nullptr");
            }

            WRITE_PAIR(builder, indent, "outBlob", Slang::StringUtil::makeStringWithFormat("0x%X", outBlobId));
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    // ISession
    void JsonConsumer::ISession_getGlobalSession(ObjectID objectId, ObjectID outGlobalSessionId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::getGlobalSession");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retGlobalSession", Slang::StringUtil::makeStringWithFormat("0x%X", outGlobalSessionId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_loadModule(ObjectID objectId, const char* moduleName, ObjectID outDiagnostics, ObjectID outModuleId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::loadModule");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "moduleName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            moduleName != nullptr ? moduleName : "nullptr"));
                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retIModule", Slang::StringUtil::makeStringWithFormat("0x%X", outModuleId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::ISession_loadModuleFromIRBlob(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::loadModuleFromIRBlob");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "moduleName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            moduleName != nullptr ? moduleName : "nullptr"));
                WRITE_PAIR(builder, indent, "path", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            path != nullptr ? path : "nullptr"));
                if (source)
                {
                    void const* bufPtr = source->getBufferPointer();
                    size_t bufSize = source->getBufferSize();

                    ScopeWritterForKey scopeWritterForSource(&builder, &indent, Slang::StringUtil::makeStringWithFormat("source (0x%X): {\n", source), false);
                    {
                        WRITE_PAIR(builder, indent, "bufferPointer", Slang::StringUtil::makeStringWithFormat("0x%X", bufPtr));
                        WRITE_PAIR_NO_COMMA(builder, indent, "bufferSize", bufSize);
                    }
                }
                else
                {
                    WRITE_PAIR(builder, indent, "source", "nullptr");
                }

                WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
                WRITE_PAIR_NO_COMMA(builder, indent, "retIModule", Slang::StringUtil::makeStringWithFormat("0x%X", outModuleId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_loadModuleFromSource(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::loadModuleFromSource");
            WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
            WRITE_PAIR(builder, indent, "moduleName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            moduleName != nullptr ? moduleName : "nullptr"));
            WRITE_PAIR(builder, indent, "path", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            path != nullptr ? path : "nullptr"));
            if (source)
            {
                void const* bufPtr = source->getBufferPointer();
                size_t bufSize = source->getBufferSize();
                ScopeWritterForKey scopeWritterForSource(&builder, &indent, Slang::StringUtil::makeStringWithFormat("source (0x%X): {\n", source), false);
                {
                    WRITE_PAIR(builder, indent, "bufferPointer", Slang::StringUtil::makeStringWithFormat("0x%X", bufPtr));
                    WRITE_PAIR_NO_COMMA(builder, indent, "bufferSize", bufSize);
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "source", "nullptr");
            }

            WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            WRITE_PAIR_NO_COMMA(builder, indent, "retIModule", Slang::StringUtil::makeStringWithFormat("0x%X", outModuleId));
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_loadModuleFromSourceString(ObjectID objectId, const char* moduleName,
            const char* path, const char* string, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::loadModuleFromSourceString");
            WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
            WRITE_PAIR(builder, indent, "moduleName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            moduleName != nullptr ? moduleName : "nullptr"));

            WRITE_PAIR(builder, indent, "path", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            path != nullptr ? path : "nullptr"));

            WRITE_PAIR(builder, indent, "string", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            string != nullptr ? string : "nullptr"));

            WRITE_PAIR(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
            WRITE_PAIR_NO_COMMA(builder, indent, "retIModule", Slang::StringUtil::makeStringWithFormat("0x%X", outModuleId));
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_createCompositeComponentType(ObjectID objectId, ObjectID* componentTypeIds,
        SlangInt componentTypeCount, ObjectID outCompositeComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::createCompositeComponentType");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                if (componentTypeCount)
                {
                    ScopeWritterForKey scopeWritterForComponentTypes(&builder, &indent, "componentTypes", false);
                    for (int i = 0; i < componentTypeCount; i++)
                    {
                        if (i != componentTypeCount - 1)
                        {
                            WRITE_STRING(builder, indent, Slang::StringUtil::makeStringWithFormat("[%d]: 0x%X,\n", i, componentTypeIds[i]));
                        }
                        else
                        {
                            WRITE_STRING(builder, indent, Slang::StringUtil::makeStringWithFormat("[%d]: 0x%X\n", i, componentTypeIds[i]));
                        }
                    }
                }
                else
                {
                    WRITE_PAIR(builder, indent, "componentTypes", "nullptr");
                }
            }
            WRITE_PAIR(builder, indent, "componentTypeCount", componentTypeCount);
            WRITE_PAIR(builder, indent, "outCompositeComponentType", Slang::StringUtil::makeStringWithFormat("0x%X", outCompositeComponentTypeId));
            WRITE_PAIR_NO_COMMA(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnosticsId));
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_specializeType(ObjectID objectId, ObjectID typeId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::specializeType");
            WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
            WRITE_PAIR(builder, indent, "type", Slang::StringUtil::makeStringWithFormat("0x%X", typeId));

            if (specializationArgCount)
            {
                ScopeWritterForKey scopeWritterForArgs(&builder, &indent, "specializationArgs", false);
                for(int i = 0; i < specializationArgCount; i++)
                {
                    ScopeWritterForKey scopeWritterForArg(&builder, &indent, Slang::StringUtil::makeStringWithFormat("[%d]\n", i), false);
                    {
                        WRITE_PAIR(builder, indent, "kind", SpecializationArgKindToString(specializationArgs[i].kind));
                        WRITE_PAIR_NO_COMMA(builder, indent, "type", Slang::StringUtil::makeStringWithFormat("0x%X", specializationArgs[i].type));
                    }
                }
            }
            else
            {
                WRITE_PAIR(builder, indent, "specializationArgs", "nullptr");
            }

            WRITE_PAIR(builder, indent, "specializationArgCount", specializationArgCount);
            WRITE_PAIR(builder, indent, "outDiagnostics", outDiagnosticsId);
            WRITE_PAIR_NO_COMMA(builder, indent, "retTypeReflectionId", outTypeReflectionId);
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::ISession_getTypeLayout(ObjectID objectId, ObjectID typeId, SlangInt targetIndex,
            slang::LayoutRules rules,  ObjectID outDiagnosticsId, ObjectID outTypeLayoutReflectionId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::getTypeLayout");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "type", Slang::StringUtil::makeStringWithFormat("0x%X", typeId));
                WRITE_PAIR(builder, indent, "rules", LayoutRulesToString(rules));
                WRITE_PAIR(builder, indent, "outDiagnostics", outDiagnosticsId);
                WRITE_PAIR(builder, indent, "retTypeReflectionId", outTypeLayoutReflectionId);
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::ISession_getContainerType(ObjectID objectId, ObjectID elementType,
            slang::ContainerType containerType, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
    }

    void JsonConsumer::ISession_getDynamicType(ObjectID objectId, ObjectID outTypeReflectionId)
    {
    }

    void JsonConsumer::ISession_getTypeRTTIMangledName(ObjectID objectId, ObjectID typeId, ObjectID outNameBlobId)
    {
    }

    void JsonConsumer::ISession_getTypeConformanceWitnessMangledName(ObjectID objectId, ObjectID typeId,
			ObjectID interfaceTypeId, ObjectID outNameBlobId)
    {
    }


    void JsonConsumer::ISession_getTypeConformanceWitnessSequentialID(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, uint32_t outId)
    {
    }

    void JsonConsumer::ISession_createTypeConformanceComponentType(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, ObjectID outConformanceId,
            SlangInt conformanceIdOverride, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ISession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequestId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::createCompileRequest");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "outCompileRequest", outCompileRequestId);
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::ISession_getLoadedModule(ObjectID objectId, SlangInt index, ObjectID outModuleId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "ISession::getLoadedModule");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "index", index);
                WRITE_PAIR_NO_COMMA(builder, indent, "retModule", Slang::StringUtil::makeStringWithFormat("0x%X", outModuleId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    // IModule
    void JsonConsumer::IModule_findEntryPointByName(ObjectID objectId, char const* name, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IModule::findEntryPointByName");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "name", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            name != nullptr ? name : "nullptr"));

                WRITE_PAIR_NO_COMMA(builder, indent, "outEntryPoint", Slang::StringUtil::makeStringWithFormat("0x%X", outEntryPointId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IModule_getDefinedEntryPoint(ObjectID objectId, SlangInt32 index, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IModule::getDefinedEntryPoint");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "index", index);
                WRITE_PAIR_NO_COMMA(builder, indent, "outEntryPoint", Slang::StringUtil::makeStringWithFormat("0x%X", outEntryPointId));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IModule_serialize(ObjectID objectId, ObjectID outSerializedBlobId)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IModule::serialize");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outSerializedBlob", outSerializedBlobId);
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IModule_writeToFile(ObjectID objectId, char const* fileName)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IModule::writeToFile");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR_NO_COMMA(builder, indent, "fileName", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            fileName != nullptr ? fileName : "nullptr"));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }


    void JsonConsumer::IModule_findAndCheckEntryPoint(ObjectID objectId, char const* name, SlangStage stage, ObjectID outEntryPointId, ObjectID outDiagnostics)
    {
        SANITY_CHECK();
        Slang::StringBuilder builder;
        int indent = 0;

        {
            ScopeWritterForKey scopeWritter(&builder, &indent, "IModule::findAndCheckEntryPoint");
            {
                WRITE_PAIR(builder, indent, "this", Slang::StringUtil::makeStringWithFormat("0x%X", objectId));
                WRITE_PAIR(builder, indent, "name", Slang::StringUtil::makeStringWithFormat("\"%s\"",
                            name != nullptr ? name : "nullptr"));
                WRITE_PAIR(builder, indent, "stage", SlangStageToString(stage));
                WRITE_PAIR(builder, indent, "outEntryPoint", Slang::StringUtil::makeStringWithFormat("0x%X", outEntryPointId));
                WRITE_PAIR_NO_COMMA(builder, indent, "outDiagnostics", Slang::StringUtil::makeStringWithFormat("0x%X", outDiagnostics));
            }
        }

        m_fileStream.write(builder.produceString().begin(), builder.produceString().getLength());
        m_fileStream.flush();
    }

    void JsonConsumer::IModule_getSession(ObjectID objectId, ObjectID outSessionId)
    {
        SANITY_CHECK();
        m_moduleHelper.getSession(objectId, outSessionId);
    }

    void JsonConsumer::IModule_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
        SANITY_CHECK();
        m_moduleHelper.getLayout(objectId, targetIndex, outDiagnosticsId, retProgramLayoutId);
    }


    void JsonConsumer::IModule_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.getEntryPointCode(objectId, entryPointIndex, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::IModule_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.getTargetCode(objectId, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::IModule_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystemId)
    {
        SANITY_CHECK();
        m_moduleHelper.getResultAsFileSystem(objectId, entryPointIndex, targetIndex, outFileSystemId);
    }


    void JsonConsumer::IModule_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
        SANITY_CHECK();
        m_moduleHelper.getEntryPointHash(objectId, entryPointIndex, targetIndex, outHashId);
    }


    void JsonConsumer::IModule_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.specialize(objectId, specializationArgs, specializationArgCount, outSpecializedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::IModule_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.link(objectId, outLinkedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::IModule_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibraryId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.getEntryPointHostCallable(objectId, entryPointIndex, targetIndex, outSharedLibraryId, outDiagnosticsId);
    }


    void JsonConsumer::IModule_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        m_moduleHelper.renameEntryPoint(objectId, newName, outEntryPointId);
    }


    void JsonConsumer::IModule_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_moduleHelper.linkWithOptions(objectId, outLinkedComponentTypeId, compilerOptionEntryCount, compilerOptionEntries, outDiagnosticsId);
    }

    // IEntryPoint
    void JsonConsumer::IEntryPoint_getSession(ObjectID objectId, ObjectID outSessionId)
    {
        SANITY_CHECK();
        m_entryPointHelper.getSession(objectId, outSessionId);
    }


    void JsonConsumer::IEntryPoint_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
        SANITY_CHECK();
        m_entryPointHelper.getLayout(objectId, targetIndex, outDiagnosticsId, retProgramLayoutId);
    }


    void JsonConsumer::IEntryPoint_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_entryPointHelper.getEntryPointCode(objectId, entryPointIndex, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::IEntryPoint_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_entryPointHelper.getTargetCode(objectId, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::IEntryPoint_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
        SANITY_CHECK();
        m_entryPointHelper.getResultAsFileSystem(objectId, entryPointIndex, targetIndex, outFileSystem);
    }


    void JsonConsumer::IEntryPoint_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
        SANITY_CHECK();
        m_entryPointHelper.getEntryPointHash(objectId, entryPointIndex, targetIndex, outHashId);
    }


    void JsonConsumer::IEntryPoint_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_entryPointHelper.specialize(objectId, specializationArgs, specializationArgCount, outSpecializedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::IEntryPoint_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_entryPointHelper.link(objectId, outLinkedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::IEntryPoint_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
        SANITY_CHECK();
        m_entryPointHelper.getEntryPointHostCallable(objectId, entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
    }


    void JsonConsumer::IEntryPoint_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        m_entryPointHelper.renameEntryPoint(objectId, newName, outEntryPointId);
    }


    void JsonConsumer::IEntryPoint_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_entryPointHelper.linkWithOptions(objectId, outLinkedComponentTypeId, compilerOptionEntryCount, compilerOptionEntries, outDiagnosticsId);
    }



    // ICompositeComponentType
    void JsonConsumer::ICompositeComponentType_getSession(ObjectID objectId, ObjectID outSessionId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getSession(objectId, outSessionId);
    }


    void JsonConsumer::ICompositeComponentType_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getLayout(objectId, targetIndex, outDiagnosticsId, retProgramLayoutId);
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getEntryPointCode(objectId, entryPointIndex, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::ICompositeComponentType_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getTargetCode(objectId, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::ICompositeComponentType_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getResultAsFileSystem(objectId, entryPointIndex, targetIndex, outFileSystem);
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getEntryPointHash(objectId, entryPointIndex, targetIndex, outHashId);
    }


    void JsonConsumer::ICompositeComponentType_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.specialize(objectId, specializationArgs, specializationArgCount, outSpecializedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::ICompositeComponentType_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.link(objectId, outLinkedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.getEntryPointHostCallable(objectId, entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
    }


    void JsonConsumer::ICompositeComponentType_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.renameEntryPoint(objectId, newName, outEntryPointId);
    }


    void JsonConsumer::ICompositeComponentType_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_compositeComponentTypeHelper.linkWithOptions(objectId, outLinkedComponentTypeId, compilerOptionEntryCount, compilerOptionEntries, outDiagnosticsId);
    }



    // ITypeConformance
    void JsonConsumer::ITypeConformance_getSession(ObjectID objectId, ObjectID outSessionId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getSession(objectId, outSessionId);
    }


    void JsonConsumer::ITypeConformance_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getLayout(objectId, targetIndex, outDiagnosticsId, retProgramLayoutId);
    }


    void JsonConsumer::ITypeConformance_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getEntryPointCode(objectId, entryPointIndex, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::ITypeConformance_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCodeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getTargetCode(objectId, targetIndex, outCodeId, outDiagnosticsId);
    }


    void JsonConsumer::ITypeConformance_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getResultAsFileSystem(objectId, entryPointIndex, targetIndex, outFileSystem);
    }


    void JsonConsumer::ITypeConformance_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getEntryPointHash(objectId, entryPointIndex, targetIndex, outHashId);
    }


    void JsonConsumer::ITypeConformance_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.specialize(objectId, specializationArgs, specializationArgCount, outSpecializedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::ITypeConformance_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.link(objectId, outLinkedComponentTypeId, outDiagnosticsId);
    }


    void JsonConsumer::ITypeConformance_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.getEntryPointHostCallable(objectId, entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
    }


    void JsonConsumer::ITypeConformance_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.renameEntryPoint(objectId, newName, outEntryPointId);
    }


    void JsonConsumer::ITypeConformance_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
        SANITY_CHECK();
        m_typeConformanceHelper.linkWithOptions(objectId, outLinkedComponentTypeId, compilerOptionEntryCount, compilerOptionEntries, outDiagnosticsId);
    }
}; // namespace SlangRecord
