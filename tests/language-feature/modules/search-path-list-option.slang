//TEST:SIMPLE: -search-path-list tests/language-feature/modules/search-path-list/search-paths.txt

import search_path_list_dependency;

int useSearchPathList()
{
    return searchPathListValue();
}
