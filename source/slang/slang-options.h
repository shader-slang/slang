// slang-options.h
#ifndef SLANG_OPTIONS_H
#define SLANG_OPTIONS_H

#include "../core/slang-basic.h"

namespace Slang
{

struct CommandOptions;

UnownedStringSlice getCodeGenTargetName(SlangCompileTarget target);

SlangResult parseOptions(
    SlangCompileRequest*    compileRequestIn,
    int                     argc,
    char const* const*      argv);

// Initialize command options. Holds the details how parsing works. 
void initCommandOptions(CommandOptions& commandOptions);

enum class Stage : SlangUInt32;

SlangSourceLanguage findSourceLanguageFromPath(const String& path, Stage& outImpliedStage);

}
#endif
