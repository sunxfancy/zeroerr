#!/usr/bin/env python3
"""Validate an immutable release tag and assemble source/header release assets."""
import argparse
import hashlib
import re
import shutil
import subprocess
from pathlib import Path


def git(*args):
    return subprocess.check_output(['git', *args], text=True).strip()


def version(text):
    numbers = []
    for part in ('MAJOR', 'MINOR', 'PATCH'):
        match = re.search(r'^#define ZEROERR_VERSION_' + part + r'\s+(\d+)\s*$', text, re.M)
        if not match:
            raise ValueError('Missing version macro: ' + part)
        numbers.append(str(int(match.group(1))))
    return 'v' + '.'.join(numbers)


def validate(tag, expected_sha=None):
    if not re.fullmatch(r'v(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)', tag):
        raise ValueError('Use a stable version tag such as v0.4.2')
    sha = git('rev-parse', '--verify', 'refs/tags/' + tag + '^{commit}')
    if expected_sha and sha != expected_sha:
        raise ValueError('Release tag moved after validation')
    if subprocess.call(['git', 'merge-base', '--is-ancestor', sha, 'refs/remotes/origin/master']):
        raise ValueError('Release commit must already be merged into master')
    for path in ('include/zeroerr/internal/config.h', 'zeroerr.hpp'):
        if version(git('show', sha + ':' + path)) != tag:
            raise ValueError('Tag does not match version in ' + path)
    return sha


def package(tag, sha, output):
    # Refuse dirty/different checkouts and existing directories; never overwrite assets.
    if git('rev-parse', 'HEAD') != sha or git('status', '--porcelain', '--untracked-files=no'):
        raise ValueError('Packaging requires a clean checkout of the validated commit')
    output.mkdir(parents=True, exist_ok=False)
    shutil.copyfile('zeroerr.hpp', output / 'zeroerr.hpp')
    for fmt, suffix in [('tar.gz', 'tar.gz'), ('zip', 'zip')]:
        subprocess.run(['git', 'archive', '--format=' + fmt, '--prefix=zeroerr-' + tag[1:] + '/',
                        '--output=' + str(output / ('zeroerr-' + tag[1:] + '.' + suffix)), sha], check=True)
    checksums = [hashlib.sha256(p.read_bytes()).hexdigest() + '  ' + p.name
                 for p in sorted(output.iterdir())]
    (output / 'SHA256SUMS').write_text('\n'.join(checksums) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('tag')
    parser.add_argument('--expected-sha')
    parser.add_argument('--github-output', type=Path)
    parser.add_argument('--package', type=Path)
    args = parser.parse_args()
    sha = validate(args.tag, args.expected_sha)
    if args.github_output:
        with args.github_output.open('a', encoding='utf-8') as stream:
            stream.write('sha=' + sha + '\ntag=' + args.tag + '\n')
    if args.package:
        package(args.tag, sha, args.package)
    print(args.tag + ': ' + sha)


if __name__ == '__main__':
    main()
