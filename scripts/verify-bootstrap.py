#!/usr/bin/env python3
"""Check documentation integrity only; no SDK, network, or device access."""
import hashlib
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = (
    'AGENTS.md', 'README.md', 'docs/PROJECT.md', 'docs/WORK.md',
    'docs/TESTING.md', 'docs/bootstrap/PROPOSAL.md',
    'docs/reference/PRODUCT-BRIEF.md', 'scripts/verify-bootstrap.py', '.gitignore',
)
BRIEF_SHA = '1b642e5a0d7adde3bacde3382ff00bca1b155c824e09160e615d6fb6afdfa203'
DISPOSITIONS = {'adopted', 'next', 'deferred', 'excluded-v1'}


def main():
    errors = []
    for name in REQUIRED:
        path = ROOT / name
        if not path.is_file() or not path.stat().st_size:
            errors.append(f'missing or empty artifact: {name}')
    readme = ROOT / 'README.md'
    linked = set()
    if readme.is_file():
        for target in re.findall(r'\[[^\]]+\]\(([^)]+)\)', readme.read_text()):
            url = urlsplit(target)
            if url.scheme or url.netloc or not url.path:
                continue
            path = (ROOT / unquote(url.path)).resolve()
            if not path.is_relative_to(ROOT) or not path.is_file():
                errors.append(f'invalid local index target: {target}')
            else:
                linked.add(path.relative_to(ROOT).as_posix())
    for name in REQUIRED:
        if name not in {'README.md', '.gitignore'} and name not in linked:
            errors.append(f'artifact absent from README index: {name}')
    brief = ROOT / 'docs/reference/PRODUCT-BRIEF.md'
    if brief.is_file() and hashlib.sha256(brief.read_bytes()).hexdigest() != BRIEF_SHA:
        errors.append('archived product brief SHA-256 mismatch')
    project = ROOT / 'docs/PROJECT.md'
    covered = set()
    if project.is_file():
        for line in project.read_text().splitlines():
            match = re.fullmatch(r'\| (\d+)(?:-(\d+))? \| ([\w-]+) \| (.+) \|', line)
            if not match:
                continue
            start, end, disposition, owner = match.groups()
            start, end = int(start), int(end or start)
            if not (1 <= start <= end <= 27) or disposition not in DISPOSITIONS or not owner.strip():
                errors.append(f'invalid coverage row: {line}')
                continue
            sections = set(range(start, end + 1))
            if covered & sections:
                errors.append(f'duplicate source coverage: {line}')
            covered.update(sections)
    if covered != set(range(1, 28)):
        errors.append(f'incomplete source coverage: missing {sorted(set(range(1, 28)) - covered)}')
    if errors:
        for error in errors:
            print(f'FAIL: {error}', file=sys.stderr)
        return 1
    print('PASS: documentation verification only; artifacts, index, source hash and sections 1-27.')
    print('Firmware build, device behavior and fresh-session discovery are not verified by this check.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
