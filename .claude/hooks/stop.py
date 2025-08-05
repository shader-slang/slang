#!/usr/bin/env python3

import argparse
import json
import os
import sys


def parse_transcript_for_todos(transcript_path):
    """Parse transcript to find the last TodoWrite and check if all todos are completed."""
    if not os.path.exists(transcript_path):
        return True  # If no transcript, assume OK to proceed

    try:
        last_todo_write = None

        # Read .jsonl file and find the last TodoWrite
        with open(transcript_path, "r") as f:
            for line in f:
                line = line.strip()
                if line:
                    try:
                        entry = json.loads(line)
                        # Check if this is an assistant message with TodoWrite tool use
                        if (
                            entry.get("type") == "assistant"
                            and "message" in entry
                            and "content" in entry["message"]
                        ):
                            content = entry["message"]["content"]
                            if isinstance(content, list):
                                for item in content:
                                    if (
                                        isinstance(item, dict)
                                        and item.get("type") == "tool_use"
                                        and item.get("name") == "TodoWrite"
                                        and "input" in item
                                        and "todos" in item["input"]
                                    ):
                                        last_todo_write = item["input"]["todos"]
                    except json.JSONDecodeError:
                        continue  # Skip invalid lines

        # If no TodoWrite found, assume OK to proceed
        if not last_todo_write:
            return True

        # Check if all todos are completed
        incomplete_todos = []
        for todo in last_todo_write:
            if todo.get("status") != "completed":
                incomplete_todos.append(todo)

        return len(incomplete_todos) == 0, incomplete_todos

    except Exception:
        # If any error occurs during parsing, assume OK to proceed
        return True


def main():
    try:
        # Parse command line arguments
        parser = argparse.ArgumentParser()
        parser.add_argument(
            "--validate",
            action="store_true",
            help="Validate that all todos are completed before allowing stop",
        )
        args = parser.parse_args()

        # Read JSON input from stdin
        input_data = json.load(sys.stdin)

        # Extract required fields
        session_id = input_data.get("session_id", "")
        stop_hook_active = input_data.get("stop_hook_active", False)

        # Handle --validate switch
        if args.validate and "transcript_path" in input_data:
            transcript_path = input_data["transcript_path"]
            validation_result = parse_transcript_for_todos(transcript_path)

            # Check if validation returned a tuple (incomplete todos found)
            if isinstance(validation_result, tuple):
                all_complete, incomplete_todos = validation_result
                if not all_complete:
                    # Create a detailed message about incomplete todos
                    incomplete_items = []
                    for todo in incomplete_todos:
                        status = todo.get("status", "unknown")
                        content = todo.get("content", "unknown task")
                        incomplete_items.append(f"- {content} ({status})")

                    incomplete_list = "\n".join(incomplete_items)
                    reason = f"Tasks are not yet complete. Please finish the following todos:\n{incomplete_list}\n\nUse TodoWrite to mark tasks as completed when finished."

                    # Return JSON decision to block stopping
                    output = {"decision": "block", "reason": reason}
                    print(json.dumps(output))
                    sys.exit(0)
            elif not validation_result:
                # Single boolean returned as False
                reason = "Tasks are not yet complete. Please finish all todos before stopping. Use TodoWrite to mark tasks as completed when finished."
                output = {"decision": "block", "reason": reason}
                print(json.dumps(output))
                sys.exit(0)

        sys.exit(0)

    except json.JSONDecodeError:
        # Handle JSON decode errors gracefully
        sys.exit(0)
    except Exception:
        # Handle any other errors gracefully
        sys.exit(0)


if __name__ == "__main__":
    main()
