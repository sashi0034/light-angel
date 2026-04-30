"""Custom formatting fixups applied after clang-format.

Handles rules that clang-format cannot express.

# Rule: blank line after control statement block
Insert a blank line after if/else/while/for/do/switch blocks (braced or braceless),
unless the next line is already blank or starts with a continuation keyword.

  if (cond) return false;    ->  if (cond) return false;
  next_stmt();                   (blank)
                                 next_stmt();

  if (cond) { ... }          ->  if (cond) { ... }
  next_stmt();                   (blank)
                                 next_stmt();
"""

import argparse
import os
import re
import subprocess

EXTENSIONS = {'.cpp', '.h', '.hpp', '.cc', '.cxx', '.hxx'}
EXCLUDE_DIRS = {'fmt', '.idea', 'build', '.git'}

CONTROL_RE = re.compile(r'^(\s*)(if|else\s+if|else|while|for|do|switch)\b(.*)')
OPEN_BRACE_RE = re.compile(r'^\s*\{\s*$')
CLOSE_BRACE_RE = re.compile(r'^\s*\}\s*$')
ELSE_RE = re.compile(r'^\s*else\b')
WHILE_RE = re.compile(r'^\s*while\b')


def should_insert_blank_after_control(next_line: str, control_kind: str) -> bool:
    if next_line.strip() == '' or ELSE_RE.match(next_line):
        return False
    if control_kind == 'do' and WHILE_RE.match(next_line):
        return False
    return True


def process_lines(lines: list[str]) -> tuple[list[str], bool]:
    result: list[str] = []
    changed = False
    # Stack item is the control keyword that opened the matching '{', if any.
    brace_stack: list[str | None] = []
    pending_control: str | None = None  # saw a control keyword, waiting for its body

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.rstrip('\n')

        match = CONTROL_RE.match(stripped)
        if match:
            control_kind = match.group(2).replace(' ', '')
            rstripped = stripped.rstrip()
            if rstripped.endswith('{'):
                # K&R style: body starts on same line
                brace_stack.append(control_kind)
                pending_control = None
            elif rstripped.endswith(';'):
                # Single-line form: if (cond) stmt; — insert blank after this line
                result.append(line)
                next_i = i + 1
                if next_i < len(lines):
                    next_line = lines[next_i]
                    if should_insert_blank_after_control(next_line, control_kind):
                        result.append('\n')
                        changed = True
                i += 1
                continue
            else:
                # Body is on the next line
                pending_control = control_kind
        elif OPEN_BRACE_RE.match(stripped):
            brace_stack.append(pending_control)
            pending_control = None
        elif CLOSE_BRACE_RE.match(stripped):
            control_kind = brace_stack.pop() if brace_stack else None
            result.append(line)
            if control_kind:
                next_i = i + 1
                if next_i < len(lines):
                    next_line = lines[next_i]
                    if should_insert_blank_after_control(next_line, control_kind):
                        result.append('\n')
                        changed = True
            i += 1
            continue
        elif pending_control and stripped.strip() != '':
            # Braceless body: the first non-blank, non-brace line after a control keyword
            control_kind = pending_control
            pending_control = None
            result.append(line)
            next_i = i + 1
            if next_i < len(lines):
                next_line = lines[next_i]
                if should_insert_blank_after_control(next_line, control_kind):
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


def iter_cpp_targets(root: str, paths: list[str]):
    if not paths:
        yield from find_cpp_files(root)
        return

    for path in paths:
        abs_path = os.path.abspath(path)
        if os.path.isdir(abs_path):
            yield from find_cpp_files(abs_path)
        elif os.path.splitext(abs_path)[1] in EXTENSIONS:
            yield abs_path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('paths', nargs='*', help='C++ files or directories to format. Defaults to the repository root.')
    parser.add_argument('--no-clang-format', action='store_true', help='Skip clang-format and only apply custom fixups.')
    args = parser.parse_args()

    root = os.path.dirname(os.path.abspath(__file__))
    targets = list(dict.fromkeys(iter_cpp_targets(root, args.paths)))

    if not targets:
        print('No C++ files found.')
        return

    before = {p: open(p, 'rb').read() for p in targets}

    if not args.no_clang_format:
        print('Running clang-format...')
        subprocess.run(['clang-format', '-i'] + targets, check=True)

    for p in targets:
        process_file(p)

    modified = [p for p in targets if open(p, 'rb').read() != before[p]]

    if modified:
        print(f"Modified {len(modified)} file(s):")
        for p in modified:
            print(f"  {p}")
    else:
        print("No files needed changes.")


if __name__ == '__main__':
    main()
