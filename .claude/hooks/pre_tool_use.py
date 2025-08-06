#!/usr/bin/env python3

import json
import subprocess
import sys
import os


def main():
    try:
        # Read JSON input from stdin
        input_data = json.load(sys.stdin)

        tool_name = input_data.get("tool_name", "")
        tool_input = input_data.get("tool_input", {})

        # Check if this is a Bash tool with git add/commit command
        if tool_name == "Bash":
            command = tool_input.get("command", "")

            # Check if command starts with git add or git commit
            if command.strip().startswith(("git add", "git commit")):
                print("Running formatter before git command...", file=sys.stderr)

                # Run the formatter
                try:
                    result = subprocess.run(
                        [
                            "./extras/formatting.sh",
                            "--no-version-check",
                            "--cpp",
                            "--since",
                            "master",
                        ],
                        capture_output=True,
                        text=True,
                    )

                    if result.returncode != 0:
                        print(f"Formatter warning: {result.stderr}", file=sys.stderr)
                    else:
                        print("Formatting completed successfully", file=sys.stderr)

                except Exception as e:
                    print(f"Formatter error: {e}", file=sys.stderr)

        sys.exit(0)

    except json.JSONDecodeError:
        # Handle JSON decode errors gracefully
        sys.exit(0)
    except Exception:
        # Handle any other errors gracefully
        sys.exit(0)


if __name__ == "__main__":
    main()
