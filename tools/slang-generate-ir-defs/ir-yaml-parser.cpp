#include "ir-yaml-parser.h"

#include "core/slang-dictionary.h"
#include "ir-yaml-types.h"

#include <ryml.hpp>

using namespace Slang;

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

// Helper to convert ryml substring to String
static String fromSubstr(ryml::csubstr s)
{
    return String(s.data(), s.data() + s.size());
}

// Helper to get source location info
static String getLocation(const ryml::Parser& parser, const ryml::ConstNodeRef& node)
{
    auto loc = parser.location(node);
    return fromSubstr(parser.filename()) + ":" + String(uint64_t{loc.line + 1}) + ":" +
           String(uint64_t{loc.col + 1});
}

// Convert string flags to enum
static IROpFlags parseFlags(const ryml::Parser& parser, const ryml::ConstNodeRef& flags_node)
{
    IROpFlags result = IROpFlags::None;

    if (!flags_node.is_seq())
    {
        String error = "Expected sequence for flags at " + getLocation(parser, flags_node);
        throw std::runtime_error(error.getBuffer());
    }

    for (const auto& flag : flags_node)
    {
        String flag_str = fromSubstr(flag.val());
        const auto it = s_flagMap.tryGetValue(flag_str);
        if (it)
        {
            result = result | *it;
        }
        else
        {
            String error = "Unknown flag '" + flag_str + "' at " + getLocation(parser, flag);
            throw std::runtime_error(error.getBuffer());
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

static Entry parseEntry(const ryml::Parser& parser, const ryml::ConstNodeRef& entry_node)
{
    if (!entry_node.is_map() || entry_node.num_children() != 1)
    {
        String error =
            "Invalid entry format: expected single-key map at " + getLocation(parser, entry_node);
        throw std::runtime_error(error.getBuffer());
    }

    auto child = entry_node.first_child();
    String key = fromSubstr(child.key());

    Entry entry;
    entry.name = key;

    if (child.is_map() && child.has_child("insts"))
    {
        entry.flavor = Entry::Range;

        if (child.has_child("comment"))
        {
            entry.comment = fromSubstr(child["comment"].val());
        }

        if (child.has_child("flags"))
        {
            entry.flags = parseFlags(parser, child["flags"]);
        }

        // Parse nested entries
        auto insts_node = child["insts"];
        if (!insts_node.is_seq())
        {
            String error =
                String("Expected sequence for 'insts' at ") + getLocation(parser, insts_node);
            throw std::runtime_error(error.getBuffer());
        }

        for (const auto& inst_child : insts_node)
        {
            entry.children.add(parseEntry(parser, inst_child));
        }
    }
    else
    {
        entry.flavor = Entry::Instruction;
        entry.type_name = toPascalCase(key);

        if (child.is_map() && child.num_children() > 0)
        {
            if (child.has_child("comment"))
            {
                entry.comment = fromSubstr(child["comment"].val());
            }

            if (child.has_child("type_name"))
            {
                entry.type_name = fromSubstr(child["type_name"].val());
            }

            if (child.has_child("operands"))
            {
                auto operands_node = child["operands"];
                if (!operands_node.is_keyval())
                {
                    String error = String("Expected scalar value for 'operands' at ") +
                                   getLocation(parser, operands_node);
                    throw std::runtime_error(error.getBuffer());
                }

                int operands;
                if (!ryml::atoi(operands_node.val(), &operands))
                {
                    String error = String("Invalid integer value for 'operands' at ") +
                                   getLocation(parser, operands_node);
                    throw std::runtime_error(error.getBuffer());
                }
                entry.operands = operands;
            }

            if (child.has_child("flags"))
            {
                entry.flags = parseFlags(parser, child["flags"]);
            }
        }
    }

    return entry;
}

InstructionSet parseInstDefs(const String& filename, const String& contents)
{
    InstructionSet result;

    // Parse YAML
    ryml::EventHandlerTree evt_handler = {};
    ryml::Parser parser(&evt_handler, ryml::ParserOptions().locations(true));
    ryml::Tree tree =
        ryml::parse_in_arena(&parser, filename.getBuffer(), ryml::csubstr(contents.getBuffer()));
    ryml::ConstNodeRef root = tree.rootref();

    if (!root.has_child("insts"))
    {
        throw std::runtime_error("Missing 'insts' key in root");
    }

    auto insts_node = root["insts"];
    if (!insts_node.is_seq())
    {
        String error =
            String("Expected sequence for 'insts' at ") + getLocation(parser, insts_node);
        throw std::runtime_error(error.getBuffer());
    }

    for (const auto& inst : insts_node)
    {
        result.entries.add(parseEntry(parser, inst));
    }

    return result;
}
