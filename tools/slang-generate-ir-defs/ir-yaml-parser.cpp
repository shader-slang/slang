#include "ir-yaml-parser.h"

#include "ir-yaml-types.h"

#include <cctype>
#include <fkYAML/node.hpp>

// Map string flags to enum values
const std::unordered_map<std::string, IROpFlags> FLAG_MAP = {
    {"Parent", IROpFlags::Parent},
    {"UseOther", IROpFlags::UseOther},
    {"Hoistable", IROpFlags::Hoistable},
    {"Global", IROpFlags::Global}};

// Helper function to convert snake_case to PascalCase
std::string toPascalCase(const std::string& snake_case)
{
    std::string result;
    bool capitalize_next = true;

    for (size_t i = 0; i < snake_case.length(); ++i)
    {
        if (snake_case[i] == '_')
        {
            // Handle _t suffix -> _type conversion
            if (i == snake_case.length() - 2 && snake_case[i + 1] == 't')
            {
                result += "Type";
                break;
            }
            capitalize_next = true;
        }
        else
        {
            if (capitalize_next)
            {
                result += std::toupper(snake_case[i]);
                capitalize_next = false;
            }
            else
            {
                result += snake_case[i];
            }
        }
    }

    return result;
}

// Convert string flags to enum
IROpFlags parseFlags(const std::vector<std::string>& flag_strings)
{
    IROpFlags result = IROpFlags::None;
    for (const auto& flag : flag_strings)
    {
        auto it = FLAG_MAP.find(flag);
        if (it != FLAG_MAP.end())
        {
            result = result | it->second;
        }
    }
    return result;
}

// Implementation of RangeEntry helper methods
const InstructionEntry* RangeEntry::findFirstInstruction() const
{
    for (const auto& entry : insts)
    {
        if (entry->getType() == Entry::INSTRUCTION)
        {
            return static_cast<const InstructionEntry*>(entry.get());
        }
        else
        {
            const auto* range = static_cast<const RangeEntry*>(entry.get());
            const auto* first = range->findFirstInstruction();
            if (first)
                return first;
        }
    }
    return nullptr;
}

const InstructionEntry* RangeEntry::findLastInstruction() const
{
    for (auto it = insts.rbegin(); it != insts.rend(); ++it)
    {
        if ((*it)->getType() == Entry::INSTRUCTION)
        {
            return static_cast<const InstructionEntry*>(it->get());
        }
        else
        {
            const auto* range = static_cast<const RangeEntry*>(it->get());
            const auto* last = range->findLastInstruction();
            if (last)
                return last;
        }
    }
    return nullptr;
}

// Parser implementation
class YAMLInstructionParserImpl
{
private:
    // Parse a single entry (instruction or range)
    std::unique_ptr<Entry> parseEntry(const fkyaml::node& entry_node)
    {
        if (!entry_node.is_mapping())
        {
            throw std::runtime_error("Invalid entry format - expected mapping");
        }

        if (entry_node.size() != 1)
        {
            throw std::runtime_error("Entry should have exactly one key");
        }

        auto it = entry_node.begin();
        std::string key = it.key().get_value<std::string>();
        const fkyaml::node& value = it.value();

        if (value.is_mapping() && value.contains("insts"))
        {
            return parseRange(key, value);
        }
        else
        {
            return parseInstruction(key, value);
        }
    }

    // Parse an instruction entry
    std::unique_ptr<InstructionEntry> parseInstruction(
        const std::string& mnemonic,
        const fkyaml::node& inst_node)
    {
        auto inst = std::make_unique<InstructionEntry>();
        inst->mnemonic = mnemonic;
        inst->type_name = toPascalCase(mnemonic);

        if (inst_node.is_mapping() && !inst_node.empty())
        {
            if (inst_node.contains("comment"))
            {
                inst->comment = inst_node["comment"].get_value<std::string>();
            }

            if (inst_node.contains("type_name"))
            {
                inst->type_name = inst_node["type_name"].get_value<std::string>();
            }

            if (inst_node.contains("operands"))
            {
                inst->operands = inst_node["operands"].get_value<int>();
            }

            if (inst_node.contains("flags"))
            {
                for (const auto& flag : inst_node["flags"])
                {
                    inst->flag_strings.push_back(flag.get_value<std::string>());
                }
                inst->flags = parseFlags(inst->flag_strings);
            }
        }

        return inst;
    }

    // Parse a range entry
    std::unique_ptr<RangeEntry> parseRange(const std::string& name, const fkyaml::node& range_node)
    {
        auto range = std::make_unique<RangeEntry>();
        range->name = name;

        if (range_node.contains("comment"))
        {
            range->comment = range_node["comment"].get_value<std::string>();
        }

        if (range_node.contains("flags"))
        {
            for (const auto& flag : range_node["flags"])
            {
                range->flag_strings.push_back(flag.get_value<std::string>());
            }
            range->flags = parseFlags(range->flag_strings);
        }

        if (range_node.contains("insts"))
        {
            const auto& insts_array = range_node["insts"];
            if (!insts_array.is_sequence())
            {
                throw std::runtime_error("'insts' should be a sequence");
            }

            for (const auto& entry : insts_array)
            {
                range->insts.push_back(parseEntry(entry));
            }
        }

        return range;
    }

public:
    InstructionSet parse(std::istream& input)
    {
        InstructionSet result;

        auto root = fkyaml::node::deserialize(input);

        if (!root.contains("insts"))
        {
            throw std::runtime_error("Missing 'insts' key in root");
        }

        const auto& insts_array = root["insts"];
        if (!insts_array.is_sequence())
        {
            throw std::runtime_error("'insts' should be a sequence");
        }

        for (size_t i = 0; i < insts_array.size(); ++i)
        {
            result.insts.push_back(parseEntry(insts_array[i]));
        }

        return result;
    }
};

InstructionSet YAMLInstructionParser::parse(std::istream& input)
{
    YAMLInstructionParserImpl impl;
    return impl.parse(input);
}
