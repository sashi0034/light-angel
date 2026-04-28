"""Custom formatting fixups applied before clang-format.

Handles rules that clang-format cannot express.

# Rule: blank line after control statement block
Insert a blank line after if/else/while/for/do blocks (braced or braceless),
unless the next line is already blank or starts with 'else'.

  if (cond) return false;    ->  if (cond) return false;
  next_stmt();                   (blank)
                                 next_stmt();

  if (cond) { ... }          ->  if (cond) { ... }
  next_stmt();                   (blank)
                                 next_stmt();
"""

import os
import re
import subprocess

EXTENSIONS = {'.cpp', '.h', '.hpp', '.cc', '.cxx', '.hxx'}
EXCLUDE_DIRS = {'fmt', '.idea', 'build', '.git'}

CONTROL_RE = re.compile(r'^(\s*)(if|else\s+if|else|while|for|do)\b(.*)')
OPEN_BRACE_RE = re.compile(r'^\s*\{\s*$')
CLOSE_BRACE_RE = re.compile(r'^\s*\}\s*$')
ELSE_RE = re.compile(r'^\s*else\b')


def process_lines(lines: list[str]) -> tuple[list[str], bool]:
    result: list[str] = []
    changed = False
    # Stack of bools: True if the matching '{' was opened by a control statement
    brace_stack: list[bool] = []
    pending_control = False  # saw a control keyword, waiting for its '{'

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.rstrip('\n')

        if CONTROL_RE.match(stripped):
            rstripped = stripped.rstrip()
            if rstripped.endswith('{'):
                # K&R style: body starts on same line
                brace_stack.append(True)
                pending_control = False
            elif rstripped.endswith(';'):
                # Single-line form: if (cond) stmt; — insert blank after this line
                result.append(line)
                next_i = i + 1
                if next_i < len(lines):
                    next_line = lines[next_i]
                    if next_line.strip() != '' and not ELSE_RE.match(next_line):
                        result.append('\n')
                        changed = True
                i += 1
                continue
            else:
                # Body is on the next line
                pending_control = True
        elif OPEN_BRACE_RE.match(stripped):
            brace_stack.append(pending_control)
            pending_control = False
        elif CLOSE_BRACE_RE.match(stripped):
            is_control = brace_stack.pop() if brace_stack else False
            result.append(line)
            if is_control:
                next_i = i + 1
                if next_i < len(lines):
                    next_line = lines[next_i]
                    if next_line.strip() != '' and not ELSE_RE.match(next_line):
                        result.append('\n')
                        changed = True
            i += 1
            continue
        elif pending_control and stripped.strip() != '':
            # Braceless body: the first non-blank, non-brace line after a control keyword
            pending_control = False
            result.append(line)
            next_i = i + 1
            if next_i < len(lines):
                next_line = lines[next_i]
                if next_line.strip() != '' and not ELSE_RE.match(next_line):
                    result.append('\n')
                    changed = True
            i += 1
            continue

        result.append(line)
        i += 1

    return result, changed


def process_file(filepath: str) -> bool:
    with open(filepath, encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    result, changed = process_lines(lines)

    if changed:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(result)

    return changed


def find_cpp_files(root: str):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS and not d.startswith('.')]
        for name in filenames:
            if os.path.splitext(name)[1] in EXTENSIONS:
                yield os.path.join(dirpath, name)


if __name__ == '__main__':
    root = os.path.dirname(os.path.abspath(__file__))
    modified = [p for p in find_cpp_files(root) if process_file(p)]

    if modified:
        print(f"Modified {len(modified)} file(s):")
        for p in modified:
            print(f"  {p}")
        print("Running clang-format...")
        subprocess.run(["clang-format", "-i"] + modified, check=True)
        print("Done.")
    else:
        print("No files needed changes.")
