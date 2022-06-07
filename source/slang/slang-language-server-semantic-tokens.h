#pragma once

#include "../../slang.h"
#include "../core/slang-basic.h"
#include "slang-ast-all.h"
#include "slang-syntax.h"
#include "slang-compiler.h"

namespace Slang
{
enum class SemanticTokenType
{
    Type, EnumMember, Variable, Parameter, Function, Property, Namespace, NormalText
};
extern const char* kSemanticTokenTypes[(int)SemanticTokenType::NormalText];

struct SemanticToken
{
    int line;
    int col;
    int length;
    SemanticTokenType type;
    bool operator<(const SemanticToken& other) const
    {
        if (line < other.line)
            return true;
        if (line == other.line)
            return col < other.col;
        return false;
    }
};
List<SemanticToken> getSemanticTokens(
    Linkage* linkage, Module* module, UnownedStringSlice fileName);
List<uint32_t> getEncodedTokens(List<SemanticToken>& tokens);

} // namespace Slang
