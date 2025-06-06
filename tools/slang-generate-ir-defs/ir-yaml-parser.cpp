#include "ir-yaml-parser.h"

#include "core/slang-dictionary.h"
#include "core/slang-smart-pointer.h"
#include "ir-yaml-types.h"

#include <cctype>
#include <fkYAML/node.hpp>

using namespace Slang;

//
//
//

namespace Slang
{
// In the Slang namespace to ADL works
void from_node(const fkyaml::node& n, String& b)
{
    b = n.get_value<std::string>().c_str();
}
}; // namespace Slang

//
//
//

// Map string flags to enum values
const Dictionary<String, IROpFlags> s_flagMap = {
    {"Parent", IROpFlags::Parent},
    {"UseOther", IROpFlags::UseOther},
    {"Hoistable", IROpFlags::Hoistable},
    {"Global", IROpFlags::Global}};

static String toPascalCase(const String& snake_case)
{
    StringBuilder result;
    bool capitalize_next = true;

    for (size_t i = 0; i < snake_case.getLength(); ++i)
    {
        if (snake_case[i] == '_')
        {
            // Handle _t suffix -> _type conversion
            if (i == snake_case.getLength() - 2 && snake_case[i + 1] == 't')
            {
                result << "Type";
                break;
            }
            capitalize_next = true;
        }
        else
        {
            if (capitalize_next)
            {
                result.appendChar(std::toupper(snake_case[i]));
                capitalize_next = false;
            }
            else
            {
                result << snake_case[i];
            }
        }
    }

    return result;
}

// Convert string flags to enum
static IROpFlags parseFlags(const fkyaml::node& flags_node)
{
    IROpFlags result = IROpFlags::None;
    for (const auto& flag : flags_node)
    {
        String flag_str = flag.get_value<String>();
        const auto it = s_flagMap.tryGetValue(flag_str);
        if (it)
        {
            result = result | *it;
        }
    }
    return result;
}

const Entry* Entry::findFirstInstruction() const
{
    if (isInstruction())
        return this;

    for (const auto& child : children)
    {
        const auto* first = child.findFirstInstruction();
        if (first)
            return first;
    }
    return nullptr;
}

const Entry* Entry::findLastInstruction() const
{
    if (isInstruction())
        return this;

    for (Index i = children.getCount(); i > 0; --i)
    {
        const auto* last = children[i - 1].findLastInstruction();
        if (last)
            return last;
    }
    return nullptr;
}

static Entry parseEntry(const fkyaml::node& entry_node)
{
    if (!entry_node.is_mapping() || entry_node.size() != 1)
    {
        throw std::runtime_error("Invalid entry format");
    }

    auto it = entry_node.begin();
    String key = it.key().get_value<String>();
    const fkyaml::node& value = it.value();

    Entry entry;
    entry.name = key;

    if (value.is_mapping() && value.contains("insts"))
    {
        entry.flavor = Entry::Range;

        if (value.contains("comment"))
            entry.comment = value["comment"].get_value<String>();

        if (value.contains("flags"))
        {
            entry.flags = parseFlags(value["flags"]);
        }

        // Parse nested entries
        const auto& insts_array = value["insts"];
        for (const auto& child : insts_array)
        {
            entry.children.add(parseEntry(child));
        }
    }
    else
    {
        entry.flavor = Entry::Instruction;
        entry.type_name = toPascalCase(key);

        if (value.is_mapping() && !value.empty())
        {
            if (value.contains("comment"))
                entry.comment = value["comment"].get_value<String>();

            if (value.contains("type_name"))
                entry.type_name = value["type_name"].get_value<String>();

            if (value.contains("operands"))
                entry.operands = value["operands"].get_value<int>();

            if (value.contains("flags"))
            {
                entry.flags = parseFlags(value["flags"]);
            }
        }
    }

    return entry;
}

InstructionSet parseInstDefs(std::istream& input)
{
    InstructionSet result;
    auto root = fkyaml::node::deserialize(input);

    if (!root.contains("insts"))
        throw std::runtime_error("Missing 'insts' key in root");

    const auto& insts_array = root["insts"];
    for (size_t i = 0; i < insts_array.size(); ++i)
    {
        result.entries.add(parseEntry(insts_array[i]));
    }

    return result;
}
