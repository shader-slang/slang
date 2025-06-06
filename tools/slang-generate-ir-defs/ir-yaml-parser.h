#pragma once

#include "ir-yaml-types.h"

InstructionSet parseInstDefs(const Slang::String& filename, const Slang::String& contents);
