// slang-ir-version-check-main.cpp
//
// Checker for the CI gate that enforces the IR module version-bump policy
// documented in docs/design/ir-instruction-definition.md. A workflow step runs it
// per pull request via extras/run-ir-version-check.sh.
//
// Given the base (pre-change) and new (post-change) versions of the IR
// instruction stable-names table and of slang-ir.h, it decides whether the change
// requires a version bump and whether that bump is present.
//
// The stable-names table (source/slang/slang-ir-insts-stable-names.lua) is the
// signal: it is machine-generated, and the separate required "Check Stable Names
// Table" CI job fails unless every instruction in slang-ir-insts.lua has an entry
// here. That invariant holds one way only -- a new instruction cannot avoid
// producing a new key, but an added key is not by itself proof of a new
// instruction. A reorder or comment-only edit leaves the key set unchanged and so
// is never mistaken for a change at all.
//
// Policy (docs/design/ir-instruction-definition.md): adding an instruction bumps
// k_maxSupportedModuleVersion. Since a removal is documented to bump k_max as
// well, any change to the key set -- including a rename, which drops one key and
// adds another -- is enforced against k_max; a changed key set with no k_max
// increase fails the check (exit 1). The additional k_min bump expected of a
// removal is only RECOMMENDED: k_min is advisory documentation today, as nothing
// on the module load path reads it.
//
// What this tool does NOT see, because the stable-names table is append-only by
// design (stable IDs are permanent, so a retired instruction keeps its entry as
// a tolerated "extra"): deleting an instruction from slang-ir-insts.lua produces
// no key-set delta, so a pure removal is invisible here. Neither is an
// operand-count/type change to an existing instruction. The advisory PR comment
// from check-ir-version.yml remains the fallback for both.

#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-string.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include <stdio.h>

using namespace Slang;

namespace
{

struct Options
{
    String baseStableNames;
    String newStableNames;
    String baseIrHeader;
    String newIrHeader;
};

void printUsage()
{
    fprintf(
        stderr,
        "Usage: slang-ir-version-check\n"
        "         --base-stable-names <file> --new-stable-names <file>\n"
        "         --base-ir-h <file> --new-ir-h <file>\n"
        "\n"
        "Enforces the IR module version-bump policy: any change to the\n"
        "stable-names instruction key set -- an addition, or a rename, which drops\n"
        "one key and adds another -- requires bumping k_maxSupportedModuleVersion\n"
        "in slang-ir.h. A dropped key is additionally advised to bump\n"
        "k_minSupportedModuleVersion, which is reported but not enforced.\n"
        "\n"
        "Known gaps, both deferred to the advisory PR comment: an\n"
        "operand-count/type change to an EXISTING instruction has no key delta,\n"
        "and neither does deleting an instruction, because the stable-names table\n"
        "is append-only -- a retired instruction keeps its entry so its permanent\n"
        "ID is never reused.\n");
}

// Parse the command line into Options, returning false (and printing usage) on
// any unrecognized or missing argument so a misinvocation fails loudly rather
// than silently checking nothing.
bool parseArgs(int argc, char const* const* argv, Options& outOptions)
{
    struct
    {
        char const* flag;
        String* dest;
    } const flags[] = {
        {"--base-stable-names", &outOptions.baseStableNames},
        {"--new-stable-names", &outOptions.newStableNames},
        {"--base-ir-h", &outOptions.baseIrHeader},
        {"--new-ir-h", &outOptions.newIrHeader},
    };

    for (int i = 1; i < argc; ++i)
    {
        bool matched = false;
        for (auto& f : flags)
        {
            if (strcmp(argv[i], f.flag) == 0)
            {
                if (i + 1 >= argc)
                {
                    fprintf(stderr, "error: %s requires an argument\n", f.flag);
                    return false;
                }
                *f.dest = argv[++i];
                matched = true;
                break;
            }
        }
        if (!matched)
        {
            fprintf(stderr, "error: unrecognized argument '%s'\n", argv[i]);
            return false;
        }
    }

    if (outOptions.baseStableNames.getLength() == 0 || outOptions.newStableNames.getLength() == 0 ||
        outOptions.baseIrHeader.getLength() == 0 || outOptions.newIrHeader.getLength() == 0)
    {
        fprintf(stderr, "error: all four file arguments are required\n");
        return false;
    }
    return true;
}

// Load the stable-names Lua table from a file into a set of instruction-name
// keys. The file is a Lua chunk returning `{ ["Name"] = id, ... }`; we execute
// it in an embedded interpreter and read back the keys. Returns false on any
// load/execution/shape error so the caller can fail closed rather than treat an
// unreadable table as "no instructions".
//
// Only the keys are read. The IDs are what the required "Check Stable Names
// Table" job validates (it rejects duplicates), and a version bump turns on
// which instructions exist, not on what they are numbered.
//
// A missing base file is a valid input (the table did not exist before this
// change), handled by the caller; this function is only called on files that
// are expected to parse.
bool loadStableNameKeys(String const& path, List<String>& outKeys)
{
    String contents;
    if (SLANG_FAILED(File::readAllText(path, contents)))
    {
        fprintf(stderr, "error: could not read stable-names file '%s'\n", path.getBuffer());
        return false;
    }

    lua_State* L = luaL_newstate();
    if (!L)
    {
        fprintf(stderr, "error: could not create Lua state\n");
        return false;
    }

    bool result = false;
    do
    {
        if (luaL_loadbuffer(L, contents.getBuffer(), contents.getLength(), path.getBuffer()) !=
            LUA_OK)
        {
            fprintf(
                stderr,
                "error: could not parse '%s': %s\n",
                path.getBuffer(),
                lua_tostring(L, -1));
            break;
        }

        if (lua_pcall(L, 0, 1, 0) != LUA_OK)
        {
            fprintf(
                stderr,
                "error: could not evaluate '%s': %s\n",
                path.getBuffer(),
                lua_tostring(L, -1));
            break;
        }

        if (!lua_istable(L, -1))
        {
            fprintf(stderr, "error: '%s' did not return a table\n", path.getBuffer());
            break;
        }

        // Iterate the returned table, collecting its string keys.
        bool ok = true;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            // Key is at -2, value at -1. Read the key without coercing it in
            // place (that would confuse lua_next), so require it to be a string.
            if (lua_type(L, -2) != LUA_TSTRING)
            {
                fprintf(stderr, "error: '%s' has a non-string key\n", path.getBuffer());
                ok = false;
                lua_pop(L, 2);
                break;
            }
            size_t len = 0;
            char const* key = lua_tolstring(L, -2, &len);
            outKeys.add(String(UnownedStringSlice(key, len)));
            lua_pop(L, 1);
        }
        result = ok;
    } while (false);

    lua_close(L);
    return result;
}

bool isIdentChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// Replace the contents of C/C++ comments with spaces, preserving length and line
// structure, so a later scan cannot match text inside a comment. String literals
// are not handled because slang-ir.h has no string literal containing the
// version constant; this is a targeted scanner, not a full C++ lexer.
String stripComments(String const& src)
{
    UnownedStringSlice s = src.getUnownedSlice();
    Index n = s.getLength();
    StringBuilder out;
    for (Index i = 0; i < n;)
    {
        if (i + 1 < n && s[i] == '/' && s[i + 1] == '/')
        {
            while (i < n && s[i] != '\n')
            {
                out.append(' ');
                ++i;
            }
        }
        else if (i + 1 < n && s[i] == '/' && s[i + 1] == '*')
        {
            out.append("  ");
            i += 2;
            while (i < n && !(i + 1 < n && s[i] == '*' && s[i + 1] == '/'))
            {
                out.append(s[i] == '\n' ? '\n' : ' ');
                ++i;
            }
            if (i + 1 < n)
            {
                out.append("  ");
                i += 2;
            }
        }
        else
        {
            out.append(s[i]);
            ++i;
        }
    }
    return out.produceString();
}

// Read the integer value of k_maxSupportedModuleVersion from a slang-ir.h file.
// The declaration reads `const static UInt k_maxSupportedModuleVersion = 26;`.
// The identifier also appears in comments, in the static_assert, and in a
// default-initializer (`m_version = k_maxSupportedModuleVersion`), so we strip
// comments and then match the ASSIGNMENT specifically: a whole-identifier
// occurrence (identifier boundary on the left) immediately followed — after only
// whitespace — by `=` and then a digit. Returns false if the file cannot be read
// or no such assignment is found, so a malformed header fails closed rather than
// comparing against a bogus value.
bool readMaxModuleVersion(String const& path, Int& outValue)
{
    String rawContents;
    if (SLANG_FAILED(File::readAllText(path, rawContents)))
    {
        fprintf(stderr, "error: could not read '%s'\n", path.getBuffer());
        return false;
    }

    String contents = stripComments(rawContents);
    char const* markerText = "k_maxSupportedModuleVersion";
    Index markerLen = (Index)strlen(markerText);
    UnownedStringSlice all = contents.getUnownedSlice();
    Index length = all.getLength();

    Index searchFrom = 0;
    for (;;)
    {
        Index at = contents.indexOf(markerText, searchFrom);
        if (at < 0)
            break;
        searchFrom = at + markerLen;

        // Require an identifier boundary on the left so we don't match a longer
        // identifier that ends in this name.
        if (at > 0 && isIdentChar(all[at - 1]))
            continue;
        // And on the right, so this is exactly the identifier, not a prefix.
        Index cursor = at + markerLen;
        if (cursor < length && isIdentChar(all[cursor]))
            continue;

        // The assignment has `=` (not `==`) next, after optional whitespace.
        while (cursor < length && (all[cursor] == ' ' || all[cursor] == '\t'))
            ++cursor;
        if (cursor >= length || all[cursor] != '=')
            continue; // e.g. the static_assert (`<=`) or the `;`-terminated use
        if (cursor + 1 < length && all[cursor + 1] == '=')
            continue; // an equality comparison, not an assignment
        ++cursor;     // skip '='
        while (cursor < length && (all[cursor] == ' ' || all[cursor] == '\t'))
            ++cursor;

        Index start = cursor;
        while (cursor < length && all[cursor] >= '0' && all[cursor] <= '9')
            ++cursor;
        if (cursor == start)
            continue; // `=` not followed by an integer literal

        outValue =
            stringToInt(String(UnownedStringSlice(all.begin() + start, all.begin() + cursor)));
        return true;
    }

    fprintf(
        stderr,
        "error: k_maxSupportedModuleVersion assignment not found in '%s'\n",
        path.getBuffer());
    return false;
}

// Return the keys present in `a` but not in `b`.
List<String> keysMissingFrom(List<String> const& a, List<String> const& bList)
{
    HashSet<String> b;
    for (auto& k : bList)
        b.add(k);

    List<String> result;
    for (auto& k : a)
    {
        if (!b.contains(k))
            result.add(k);
    }
    return result;
}

} // namespace

int main(int argc, char const* const* argv)
{
    Options options;
    if (!parseArgs(argc, argv, options))
    {
        printUsage();
        return 1;
    }

    // A stable-names file may not exist at the base revision (first
    // introduction); treat that as an empty key set so every current key counts
    // as new. Any other read/parse failure fails closed.
    List<String> baseKeys;
    if (File::exists(options.baseStableNames))
    {
        if (!loadStableNameKeys(options.baseStableNames, baseKeys))
            return 1;
    }

    List<String> newKeys;
    if (!loadStableNameKeys(options.newStableNames, newKeys))
        return 1;

    List<String> addedKeys = keysMissingFrom(newKeys, baseKeys);
    List<String> removedKeys = keysMissingFrom(baseKeys, newKeys);

    Int baseMax = 0;
    Int newMax = 0;
    if (!readMaxModuleVersion(options.baseIrHeader, baseMax) ||
        !readMaxModuleVersion(options.newIrHeader, newMax))
    {
        return 1;
    }

    bool maxBumped = newMax > baseMax;

    if (addedKeys.getCount() == 0 && removedKeys.getCount() == 0)
        return 0;

    // Adding an instruction bumps k_max; dropping a key bumps k_max and k_min. As
    // k_max moves either way, any key-set change is enforced against it. Exempting
    // dropped keys would also let a new instruction ride along unbumped beside an
    // unrelated one, which is how a rename could evade the gate entirely.
    if (!maxBumped)
    {
        StringBuilder message;
        message << "::error::the IR instruction set in "
                   "source/slang/slang-ir-insts-stable-names.lua changed but "
                   "k_maxSupportedModuleVersion in source/slang/slang-ir.h was not bumped:\n";
        for (auto& k : addedKeys)
            message << "  - added: " << k << "\n";
        for (auto& k : removedKeys)
            message << "  - removed: " << k << "\n";
        message << "Increment k_maxSupportedModuleVersion when the instruction set changes (see "
                   "docs/design/ir-instruction-definition.md).\n";
        fprintf(stderr, "%s", message.getBuffer());
        return 1;
    }

    // Whether a dropped key must ALSO bump k_minSupportedModuleVersion is left as
    // a recommendation rather than enforced: k_min is advisory documentation today,
    // since nothing on the module load path reads it.
    if (removedKeys.getCount() > 0)
    {
        StringBuilder message;
        message << "note: " << removedKeys.getCount()
                << " IR instruction stable name(s) dropped (a rename, or a retired "
                   "instruction whose entry was deleted):\n";
        for (auto& k : removedKeys)
            message << "  - " << k << "\n";
        message << "Per docs/design/ir-instruction-definition.md this is a breaking change and "
                   "should also bump k_minSupportedModuleVersion in source/slang/slang-ir.h.\n";
        fprintf(stderr, "%s", message.getBuffer());
    }

    fprintf(
        stderr,
        "note: instruction key set changed (%d added, %d dropped); "
        "k_maxSupportedModuleVersion bumped %d -> %d.\n",
        (int)addedKeys.getCount(),
        (int)removedKeys.getCount(),
        (int)baseMax,
        (int)newMax);
    return 0;
}
