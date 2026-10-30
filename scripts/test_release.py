"""Exercise release boundaries using a disposable Git repository."""
import hashlib
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

import release


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.previous = Path.cwd()
        self.directory = tempfile.TemporaryDirectory()
        os.chdir(self.directory.name)
        self.addCleanup(self.restore)
        self.git('init', '-q')
        self.git('config', 'user.email', 'test@example.com')
        self.git('config', 'user.name', 'Release test')
        path = Path('include/zeroerr/internal/config.h')
        path.parent.mkdir(parents=True)
        self.header = '#define ZEROERR_VERSION_MAJOR 0\n#define ZEROERR_VERSION_MINOR 4\n#define ZEROERR_VERSION_PATCH 2\n'
        path.write_text(self.header)
        Path('zeroerr.hpp').write_text(self.header)
        self.git('add', '.')
        self.git('commit', '-qm', 'release')
        self.sha = self.git('rev-parse', 'HEAD')
        self.git('update-ref', 'refs/remotes/origin/master', self.sha)
        self.git('tag', 'v0.4.2')

    def restore(self):
        os.chdir(self.previous)
        self.directory.cleanup()

    def git(self, *args):
        return subprocess.check_output(['git', *args], text=True).strip()

    def test_valid_release_assets_and_checksums(self):
        self.assertEqual(release.validate('v0.4.2'), self.sha)
        release.package('v0.4.2', self.sha, Path('dist'))
        for line in Path('dist/SHA256SUMS').read_text().splitlines():
            digest, name = line.split('  ')
            self.assertEqual(hashlib.sha256(Path('dist', name).read_bytes()).hexdigest(), digest)
        self.assertEqual(len(list(Path('dist').iterdir())), 4)
        with self.assertRaises(FileExistsError):
            release.package('v0.4.2', self.sha, Path('dist'))

    def test_annotated_tag(self):
        self.git('tag', '-d', 'v0.4.2')
        self.git('tag', '-a', 'v0.4.2', '-m', 'release')
        self.assertEqual(release.validate('v0.4.2'), self.sha)

    def test_invalid_tags(self):
        for tag in ['main', 'v01.4.2', 'v0.4.2-rc1', 'v0.4.2\nsha=bad', '--help']:
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                release.validate(tag)

    def test_mismatched_version(self):
        self.git('tag', 'v0.4.3')
        with self.assertRaisesRegex(ValueError, 'version'):
            release.validate('v0.4.3')

    def test_unmerged_commit_and_moved_tag(self):
        Path('new').touch()
        self.git('add', 'new')
        self.git('commit', '-qm', 'not promoted')
        self.git('tag', '-f', 'v0.4.2')
        with self.assertRaisesRegex(ValueError, 'moved'):
            release.validate('v0.4.2', self.sha)
        with self.assertRaisesRegex(ValueError, 'master'):
            release.validate('v0.4.2')

    def test_stale_single_header(self):
        Path('zeroerr.hpp').write_text(self.header.replace('PATCH 2', 'PATCH 1'))
        self.git('add', 'zeroerr.hpp')
        self.git('commit', '-qm', 'stale header')
        sha = self.git('rev-parse', 'HEAD')
        self.git('update-ref', 'refs/remotes/origin/master', sha)
        self.git('tag', '-f', 'v0.4.2')
        with self.assertRaisesRegex(ValueError, 'zeroerr.hpp'):
            release.validate('v0.4.2')

    def test_dirty_header(self):
        Path('zeroerr.hpp').write_text('modified')
        with self.assertRaisesRegex(ValueError, 'clean checkout'):
            release.package('v0.4.2', self.sha, Path('dist'))


if __name__ == '__main__':
    unittest.main()
