// slang-ir-version-check-main.cpp
//
// CI gate that enforces the IR module version-bump policy documented in
// docs/design/ir-instruction-definition.md. Given the base (pre-change) and new
// (post-change) versions of the IR instruction stable-names table and of
// slang-ir.h, it decides whether the change requires a version bump and whether
// that bump is present.
//
// The stable-names table (source/slang/slang-ir-insts-stable-names.lua) is the
// primary signal: it is machine-generated, append-only by ID, and kept in sync
// with slang-ir-insts.lua by the separate, required "Check Stable Names Table"
// CI job. So a key present in the new table but not the base one is a genuinely
// new (or renamed) IR instruction, and a key present in the base but not the new
// one is a removed (or renamed) instruction. A reorder or comment-only edit
// leaves the key set unchanged and is therefore never mistaken for either.
//
// Policy (docs/design/ir-instruction-definition.md):
//   - Additive (a new instruction) requires bumping k_maxSupportedModuleVersion.
//     This is mechanically unambiguous, so it is ENFORCED: a new key without a
//     k_maxSupportedModuleVersion increase fails the check (exit 1).
//   - Breaking (a removed or renamed instruction) is documented to bump both
//     k_minSupportedModuleVersion and k_maxSupportedModuleVersion. Whether that
//     is operationally required is an open design question (k_min is advisory
//     documentation today — nothing on the load path reads it), so this is only
//     DETECTED and RECOMMENDED, never enforced (exit 0).
//
// The existing advisory PR comment (posted by check-ir-version.yml) remains the
// fallback for changes this tool cannot decide from names alone (a pure semantic
// change with no name delta, some optional-operand cases, etc.).

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
        "Enforces the IR module version-bump policy: a new instruction (a new\n"
        "entry in the stable-names table) requires bumping\n"
        "k_maxSupportedModuleVersion in slang-ir.h.\n"
        "\n"
        "Known gap: this tool keys on the stable-names key set, so it does not\n"
        "detect an operand-count/type change to an EXISTING instruction (a\n"
        "breaking change with no key delta). That case is deferred to a\n"
        "follow-up; the existing advisory PR comment remains the fallback for it.\n");
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

    // A removed key means an instruction was removed or renamed (a rename shows
    // up as a removed key alongside an added one). Both are breaking changes,
    // which are detected and recommended but NOT enforced: whether they must
    // also bump k_min is an open design question. Because a rename also adds a
    // key, this case is handled first so it is never treated as a pure addition
    // by the enforced gate below.
    if (removedKeys.getCount() > 0)
    {
        StringBuilder message;
        message << "note: " << removedKeys.getCount() << " IR instruction(s) removed or renamed:\n";
        for (auto& k : removedKeys)
            message << "  - " << k << "\n";
        message << "Per docs/design/ir-instruction-definition.md this is a breaking change and "
                   "should bump both k_minSupportedModuleVersion and k_maxSupportedModuleVersion "
                   "in source/slang/slang-ir.h.\n";
        fprintf(stderr, "%s", message.getBuffer());
        return 0;
    }

    if (addedKeys.getCount() == 0)
        return 0;

    // A pure addition (new keys, none removed) requires a k_maxSupportedModuleVersion
    // bump; this is the mechanically unambiguous rule that is enforced.
    if (!maxBumped)
    {
        StringBuilder message;
        message << "::error::" << addedKeys.getCount()
                << " new IR instruction(s) were added to "
                   "source/slang/slang-ir-insts-stable-names.lua but "
                   "k_maxSupportedModuleVersion in source/slang/slang-ir.h was not bumped:\n";
        for (auto& k : addedKeys)
            message << "  - " << k << "\n";
        message << "Increment k_maxSupportedModuleVersion when adding an IR instruction (see "
                   "docs/design/ir-instruction-definition.md).\n";
        fprintf(stderr, "%s", message.getBuffer());
        return 1;
    }

    fprintf(
        stderr,
        "note: %d new IR instruction(s); k_maxSupportedModuleVersion bumped %d -> %d.\n",
        (int)addedKeys.getCount(),
        (int)baseMax,
        (int)newMax);
    return 0;
}
