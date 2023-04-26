#ifndef SLANG_CORE_COMMAND_OPTIONS_H
#define SLANG_CORE_COMMAND_OPTIONS_H

#include "slang-basic.h"
#include "slang-string-slice-pool.h"

namespace Slang
{

struct CommandOptions
{
    typedef uint32_t Flags;

    enum class CategoryKind
    {
        Option,             ///< Command line option
        Value,              ///< One of a set of values
    };

    struct Category
    {
        CategoryKind kind;
        UnownedStringSlice name;
        UnownedStringSlice description;
    };

    struct Flag
    {
        enum Enum : Flags
        {
            CanPrefix = 0x1,                /// Allows -Dfsggf or -D fdsfsd
            IsPrefix = 0x2,                /// Is an option that can only be a prefix
        };
    };

    struct Option
    {
        UnownedStringSlice names;               ///< Comma delimited list of names, first name is the default 
        UnownedStringSlice usage;               ///< Describes usage, can be empty
        UnownedStringSlice description;         ///< A description of usage

        Index startNameIndex = -1;
        Index endNameIndex = -1;

        Index categoryIndex = -1;               ///< Category this option belongs to
        Flags flags = 0;                        ///< Flags about this option
    };

        /// Add a category
    Index addCategory(CategoryKind kind, const char* name, const char* description);
        /// Use an already known category. It's an error if the category isn't found
    void setCategory(const char* name);

    void add(const char* name, const char* usage, const char* description, Flags flags = 0);
    void add(const UnownedStringSlice* names, Count namesCount, const char* usage, const char* description, Flags flags = 0);

    void addValue(const UnownedStringSlice& name);
    void addValue(const UnownedStringSlice& name, const UnownedStringSlice& description); 
    void addValue(const char* name, const char* description);
    void addValue(const char* name);
    void addValue(const UnownedStringSlice* names, Count namesCount);

        /// Finds the category by name or -1 if not found
    Index findCategoryByName(const UnownedStringSlice& name) const;

        /// Get the categories
    const List<Category>& getCategories() const { return m_categories; }

        /// Get all the options
    const List<Option>& getOptions() const { return m_options; }

        /// Find all of the categories in the usage slice
    void findCategoryIndicesFromUsage(const UnownedStringSlice& usageSlice, List<Index>& outCategories) const;
        /// Get all the option names associated with a category index
    void getCategoryOptionNames(Index categoryIndex, List<UnownedStringSlice>& outNames) const;

    /// Ctor
    CommandOptions() :
        m_optionPool(StringSlicePool::Style::Default),
        m_arena(1024 * 2)
    {
    }

        /// Returns name in the m_optionPool or -1 on error
    Index _addOptionName(const UnownedStringSlice& name, Flags flags);
 
    Index _addOption(const UnownedStringSlice& name, const Option& inOption);
    Index _addOption(const UnownedStringSlice* names, Count namesCount, const Option& option);

    Index _addValue(const UnownedStringSlice& name, const Option& inOption);

    
    UnownedStringSlice _addString(const char* text);
    UnownedStringSlice _addString(const UnownedStringSlice& slice);

    Index m_currentCategoryIndex = -1;

    List<Category> m_categories;

    // We can have a map from pool entries to the actual options.
    List<Index> m_optionMap;

    // Holds a bit for all valid prefix sizes. Max prefix size is therefore 32 chars
    uint32_t m_prefixSizes = 0;

    List<Option> m_options;                  ///< All of the entries describing each of the options
    StringSlicePool m_optionPool;            ///< Only holds options, and handle therefore matches up to m_entries 

    MemoryArena m_arena;                        ///< For other misc storage
};

struct CommandOptionsWriter
{
    typedef CommandOptions::CategoryKind CategoryKind;
    
        /// Appends a description of all of the options
    void appendDescription(const CommandOptions& options);

        /// Get the builder that string is being written to
    StringBuilder& getBuilder() { return m_builder; }

        /// Ctor 
    CommandOptionsWriter():
        m_indentSlice(toSlice("  "))
    {
    }

    Count _getCurrentLineLength();

    void _appendWithWrap(Count indentCount, List<UnownedStringSlice>& slices, const UnownedStringSlice& delimit);
    void _appendWithWrap(Count indentCount, List<UnownedStringSlice>& lines);
    void _requireIndent(Count indentCount);

    UnownedStringSlice m_indentSlice;
    Count m_lineLength = 80;

    StringBuilder m_builder;
};

} // namespace Slang

#endif 
