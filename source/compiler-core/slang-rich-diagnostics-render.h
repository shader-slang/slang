#pragma once

#include "core/slang-string.h"

namespace Slang
{

struct GenericDiagnostic;
struct SourceManager;

String renderDiagnostic(const GenericDiagnostic& diagnostic, SourceManager& sourceManager);

} // namespace Slang
