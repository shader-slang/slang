#pragma once

#include "ir-yaml-types.h"

#include <iostream>
#include <unordered_map>

// Map string flags to enum values
extern const std::unordered_map<std::string, IROpFlags> FLAG_MAP;

// Helper function to convert snake_case to PascalCase
std::string toPascalCase(const std::string& snake_case);

// Convert string flags to enum
IROpFlags parseFlags(const std::vector<std::string>& flag_strings);

// Parser class
class YAMLInstructionParser
{
public:
    InstructionSet parse(std::istream& input);
};
