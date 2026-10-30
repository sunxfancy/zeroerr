# CI and releases

Ordinary changes enter `dev` through pull requests. `master` contains promoted,
tested release candidates. Do not send fixes directly to `master` or overwrite
`dev` with `master`: merge both histories if they have diverged.

## Repository setup (administrator)

After this workflow is merged into `dev`:

1. Set the repository's default branch to `dev` in Settings → General. New PRs
   will then select `dev` by default.
2. Protect both `dev` and `master` with required pull requests and the status
   checks `PR target` and `CI result`. Select the checks after their first run.
   Require branches to be up to date, block force pushes/deletion, and disable
   bypasses if the policy must apply to administrators too.
3. Merge a promotion PR from `dev` into `master` to install the release workflow
   there. Use a merge commit for promotions so branch ancestry is preserved.

Workflow files cannot themselves change the repository's default branch or
branch-protection rules. Without required checks, a failing PR policy is advisory.
The `PR target` check rejects every ordinary PR whose base is not `dev`; the only
exception is the same repository's `dev` → `master` promotion.

## Development CI

`ci.yml` runs for pushes to `dev`, PRs to `dev`/`master`, and manual runs. It is
also reusable by `release.yml` with an immutable commit SHA. It checks:

- C++17 builds, examples, and tests on Linux, Windows, and macOS;
- GCC/Clang C++11, C++14, and C++20 compatibility;
- standalone headers, Clang warning hygiene, generated-header consistency;
- domain/serialization ASan and UBSan checks with leak detection;
- real Clang per-test coverage integration;
- all 12 color/thread-safety/auto-init combinations on Linux;
- release validation and packaging regression tests.

Every configuration explicitly enables tests, and `CI result` fails if any
required job fails, is cancelled, or is skipped. Normal runs use Debug for the
platform matrix; release runs test both Debug and Release. The old serial
`build-all.sh` matrix is no longer used by CI.

## Publish a version

1. Open a PR to `dev` updating `ZEROERR_VERSION_MAJOR/MINOR/PATCH` in
   `include/zeroerr/internal/config.h`, regenerate `zeroerr.hpp`, and merge after CI.
2. Open a `dev` → `master` promotion PR. Wait for `PR target` and `CI result`,
   then merge it using a merge commit.
3. Tag the promoted commit (replace `0.4.2` with the actual version):

   ```sh
   git fetch origin master
   git tag -a v0.4.2 origin/master -m 'zeroerr v0.4.2'
   git push origin v0.4.2
   ```

4. The Release workflow verifies the stable `vMAJOR.MINOR.PATCH` tag, both version
   declarations, and membership in `master` history. It pins the resolved commit
   and runs the full reusable CI including Debug and Release on all three OSes.
5. Only after every test succeeds, it revalidates the tag and creates a GitHub
   Release with `zeroerr.hpp`, source `.tar.gz`/`.zip`, and `SHA256SUMS`. Only the
   publishing job has write permission. Existing releases are not overwritten.

For a transient failure, rerun the failed workflow, or manually dispatch Release
with the existing tag. Select `master` as the workflow branch. Manual dispatch
also requires an existing, matching tag on `master`; it never creates or moves
one. A failed release leaves the tag in place and publishes no new release.
If a GitHub upload fails partway through publication, inspect any draft before
retrying; the workflow deliberately does not overwrite an existing release.
Prereleases and automatic version bumps are not part of this workflow.
