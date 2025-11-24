#!/usr/bin/env python3
"""
Common utilities for GitHub issue analysis scripts.
"""

import json
import re
from pathlib import Path
from typing import List, Dict, Any, Optional

# Data directory path
DATA_DIR = Path(__file__).parent / "data"


def get_file_loc(filepath: str) -> Optional[int]:
    """Get lines of code for a file.

    Args:
        filepath: Path to the file relative to repository root

    Returns:
        Number of lines in the file, or None if file doesn't exist
    """
    try:
        full_path = Path(__file__).parent.parent.parent / filepath
        if full_path.exists() and full_path.is_file():
            with open(full_path, 'r', encoding='utf-8', errors='ignore') as f:
                return len(f.readlines())
    except:
        pass
    return None


def get_component_from_file(filename: str) -> str:
    """Extract component from filename.

    Uses a systematic categorization scheme:
    - Special categories (test, build-system, docs, etc.) are handled first
    - Source files under source/ are categorized as: <dirname>-<first 2 parts of filename>

    Args:
        filename: Path to the file

    Returns:
        Component name (e.g., "slang-slang-ir", "test", "build-system")
    """
    # Check for tests FIRST (before other patterns)
    if "test" in filename.lower() or filename.startswith("tests/"):
        return "test"

    # Build system and CI
    if any(pattern in filename for pattern in ["CMakeLists.txt", "premake", ".github/workflows",
                                                 "build/visual-studio", ".vcxproj", "cmake/",
                                                 ".gitignore", "slang.sln", "CMakePresets.json",
                                                 "_build.sh", ".sh"]):
        return "build-system"

    # Documentation
    elif filename.startswith("docs/") or filename.endswith(".md"):
        return "docs"

    # Examples
    elif filename.startswith("examples/"):
        return "examples"

    # External dependencies
    elif filename.startswith("external/"):
        return "external"

    # Prelude/runtime
    elif filename.startswith("prelude/") or "prelude.h" in filename:
        return "prelude"

    # Graphics/RHI layer
    elif "tools/gfx/" in filename or "slang-gfx" in filename or "slang-rhi" in filename:
        return "gfx-rhi"

    # Tools
    elif "source/slangc/" in filename:
        return "slangc-tool"
    elif "tools/slang-generate/" in filename:
        return "code-generation-tool"
    elif "tools/platform/" in filename:
        return "platform-tools"

    # Source files: extract as <dirname>-<first 2 parts of filename>
    elif filename.startswith("source/"):
        parts = filename.split('/')
        if len(parts) >= 2:
            dirname = parts[1]  # e.g., "slang", "core", "compiler-core"
            basename = parts[-1]  # e.g., "slang-emit-spirv.cpp"
            # Remove extension
            basename = basename.rsplit('.', 1)[0]
            # Split by dash or underscore and take first 2 parts
            name_parts = re.split(r'[-_]', basename)[:2]
            component_suffix = '-'.join(name_parts)
            return f"{dirname}-{component_suffix}"
        else:
            return "source-other"

    else:
        return "other"


def load_issues() -> List[Dict[str, Any]]:
    """Load issues from JSON file.

    Returns:
        List of issue dictionaries

    Raises:
        SystemExit: If issues.json doesn't exist
    """
    issues_file = DATA_DIR / "issues.json"
    if not issues_file.exists():
        print(f"Error: {issues_file} not found. Run fetch_github_issues.py first.")
        import sys
        sys.exit(1)

    with open(issues_file) as f:
        return json.load(f)


def load_prs() -> List[Dict[str, Any]]:
    """Load pull requests from JSON file.

    Returns:
        List of PR dictionaries, or empty list if file doesn't exist
    """
    prs_file = DATA_DIR / "pull_requests.json"
    if not prs_file.exists():
        print(f"Warning: {prs_file} not found. PR analysis will be skipped.")
        return []

    with open(prs_file) as f:
        return json.load(f)


def load_all_data() -> tuple[List[Dict[str, Any]], List[Dict[str, Any]]]:
    """Load both issues and PRs.

    Returns:
        Tuple of (issues, prs)
    """
    return load_issues(), load_prs()

