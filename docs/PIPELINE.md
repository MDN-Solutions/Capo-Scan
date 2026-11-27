# MDN Scan — CI/CD pipeline

| Workflow | Trigger | What it does |
|---|---|---|
| `ci.yml` | PR, push to `main`, merge queue, weekly | rustfmt, clang-format, typos, actionlint + zizmor, ruff + gdtoolkit → clippy, build + tests on Windows (MSVC x64 and x86) and Linux (GCC, Clang) + check that the binaries have the exploit mitigations → ASan+UBSan, TSan, clang-tidy, libFuzzer, OBD-II emulator tests → `CI OK` gate |
| `security.yml` | PR, `main`, weekly | cargo-deny (RustSec, licenses, sources), dependency review (blocks PRs adding vulnerable deps), CodeQL `security-and-quality` for C++, Rust and the workflows, gitleaks (secrets in the git history) |
| `scorecard.yml` | `main`, weekly | OpenSSF Scorecard (public score + README badge) |
| `pr-title.yml` | PR | Enforces Conventional Commits in PR titles |
| `release.yml` | push to `main` | release-please Release PR → on merge: draft release → Windows ZIP/NSIS, Linux tar.gz/AppImage (tested, hardening-checked, smoke-tested) → SHA256SUMS + signed SLSA build provenance → publish |
| `claude.yml`, `claude-review.yml` | PR, comments | Claude review and `@claude` (unchanged, now pinned by SHA) |

## C++ security layers

The diagnostic core parses bytes that come from outside (the adapter, the car's
ECUs, data packages), so the C++ side is checked at several levels:

| Layer | Catches | Where |
|---|---|---|
| Strict warnings as errors (`-Wconversion -Wsign-conversion -Wshadow -Wold-style-cast -Wnull-dereference …`, MSVC `/W4 /permissive-`) | truncation, sign bugs, shadowing, suspicious code | every CI build (`MDN_SCAN_WARNINGS_AS_ERRORS`) |
| Hardening flags ([OpenSSF guide](https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++)): `_FORTIFY_SOURCE=3`, stack protector, stack clash protection, CET (`-fcf-protection`), zero-initialised locals, full RELRO, NX, PIE; MSVC `/sdl /guard:cf /CETCOMPAT`, ASLR, DEP | makes memory bugs much harder to exploit | all builds (`MDN_SCAN_HARDENING`, `cmake/MdnScanCompilerOptions.cmake`) |
| Binary verification (`scripts/check-hardening.sh`, `scripts/check-hardening-pe.py`) | a flag silently dropped by a toolchain change | CI build matrix and every release package |
| Standard library assertions (`_GLIBCXX_ASSERTIONS`, `_MSVC_STL_HARDENING`; `_GLIBCXX_DEBUG` under ASan) | out-of-bounds `operator[]`, invalid iterators | all builds |
| ASan + UBSan, TSan | buffer overflows, use-after-free, leaks, undefined behaviour, data races | `asan`, `tsan` presets |
| libFuzzer | crashes on malformed or hostile input | `fuzz` preset, 1 min per fuzzer on PRs, 15 min weekly |
| clang-tidy (`bugprone`, `cert`, `clang-analyzer` incl. `security.*`, `cppcoreguidelines`, …) | unsafe APIs (`strcpy`, CWE-119), C arrays, uninitialised variables, … | `.clang-tidy`, every finding is an error |
| CodeQL `security-and-quality` | taint flows, injection, integer overflows, … | Security tab |
| 32-bit build (MSVC x86) | 32-bit `size_t` truncation (the J2534 bridge is 32-bit) | CI build matrix |

### Supply chain and the pipeline itself

- **Everything is pinned and verified:** actions by commit SHA; googletest by
  SHA-256 and Corrosion by commit (`cmake/`); Cargo deps by `Cargo.lock` with
  `--locked`; Python CI tools with `pip --require-hashes`
  (`.github/requirements/*.txt`, lint tools from wheels only); typos, actionlint, cargo-deny, gitleaks
  and linuxdeploy by SHA-256 (`.github/actions/install-tool`).
- **Updates:** Dependabot (actions, Cargo, Python CI tools) with a 7-day
  cooldown, so a freshly published malicious version is not picked up at once.
  Conventional prefixes (`ci(deps):`, `fix(deps):`) keep its PR titles valid.
- **Workflows:** `permissions: {}` by default and the minimum per job;
  `persist-credentials: false`; caches are only saved from `main` and the release
  workflow uses none; zizmor and CodeQL audit the workflows; zizmor's online
  audits reject pinned SHAs that do not belong to the action's repository.
- **Releases:** built from the tagged commit in a clean job, then SHA256SUMS and
  a Sigstore-signed SLSA provenance attestation. The release stays a draft until
  every asset is uploaded, which works with GitHub *immutable releases*.

## Project layout

- `CMakeLists.txt` + `CMakePresets.json` at the root (CMake ≥ 3.25, Ninja).
  `project(MdnScan VERSION 0.1.0) # x-release-please-version` and
  `[workspace.package] version` in `Cargo.toml` (plus `Cargo.lock`) are bumped by
  release-please.
- `core/` — `capo_core` static library (C++20). Add sources in
  `core/CMakeLists.txt`, unit tests (GoogleTest) in `core/tests/`.
- `app/` — `mdn-scan` command-line executable. **Placeholder** so the release
  pipeline (packages, AppImage, provenance) is exercised end to end until the
  Godot export is packaged.
- `ext-data/` — Rust GDExtension, a member of the root Cargo workspace, built by
  CMake through Corrosion (`MDN_SCAN_BUILD_RUST`).
- `ext-diag/` and `bridge-j2534/` are added automatically once they have a
  `CMakeLists.txt` (the bridge only in 32-bit Windows builds). Call
  `mdn_scan_target_defaults(<target>)` on every new target to get the warnings
  and hardening flags.
- `CPP_DIRS` in `ci.yml` lists the C++ directories for clang-format/clang-tidy.
- `include(CPack)` + `install()` rules in `cmake/MdnScanPackaging.cmake`;
  `packaging/linux/mdn-scan.desktop` and `mdn-scan.png` for the AppImage.

### Fuzzers

Every parser that reads external bytes (ELM327 responses, ISO-TP frames, UDS,
data packages) should get one, in `core/fuzz/`:

```cmake
mdn_scan_add_fuzzer(elm327_response
  SOURCES elm327_response_fuzzer.cpp
  LIBRARIES capo::core)
```

```cpp
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    capo::elm327::parse_response(std::span<const std::uint8_t>(data, size));
    return 0;
}
```

Seed inputs go in `core/fuzz/corpus/<name>/`. Crashing inputs found in CI are
uploaded as the `fuzz-crashes` artifact.

### OBD-II integration tests

Put them in a separate test executable registered with `LABELS integration`.
They read the adapter endpoint (`host:port`) from `MDN_SCAN_OBD_ENDPOINT` and
should call `GTEST_SKIP()` when it is not set. In CI they run against
[ELM327-emulator](https://github.com/Ircama/ELM327-emulator) (scenario `car`,
TCP port 35000), which also produces `SEARCHING...`, multi-frame responses and
`NO DATA`.

## One-time repository setup

1. **Ruleset / branch protection on `main`:** require a pull request, require
   the status check **`CI OK`** only, require linear history, block force
   pushes. Settings → General → Pull Requests: allow **squash merging only**,
   with "Default commit message" = **Pull request title**.
2. **Settings → Code security:** enable the **Dependency graph** (the
   "Dependency review" job fails with `Forbidden` without it), Dependabot alerts
   and security updates, secret scanning and push protection, private
   vulnerability reporting.
   Code scanning must use this workflow ("advanced setup"): if CodeQL "default
   setup" is enabled, turn it off.
3. **Settings → Actions → General:** workflow permissions "Read repository
   contents" (the default). Turn on "Require actions to be pinned to a
   full-length commit SHA".
4. **Release PR CI:** create a fine-grained PAT (only this repository;
   Contents, Pull requests and Issues: read and write) or a GitHub App token,
   and save it as the secret `RELEASE_PLEASE_TOKEN`. Without it, the Release PR
   is opened with `GITHUB_TOKEN`, CI does not run on it and it can never get
   `CI OK` (and "Allow GitHub Actions to create and approve pull requests" must
   be enabled for release-please to open it at all).
5. **Settings → General → Releases:** enable **immutable releases**.
6. **OBD tests:** once integration tests exist, set the repository variable
   `OBD_EMULATOR=true`.
7. **License:** add a `LICENSE` file (the Scorecard "License" check, the NSIS
   installer and SignPath all use it).
8. **Code signing (later):** apply to SignPath Foundation (free for OSS) and add
   the signing step in `release.yml`, before the SHA256SUMS step.

## Local usage

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cargo fmt --all && cargo clippy --workspace --all-targets -- -D warnings && cargo test --workspace
```

Same checks as CI (Linux, LLVM 20: `apt install clang-20 clang-tidy-20 libclang-rt-20-dev`):

```bash
cmake --preset asan -DCMAKE_CXX_COMPILER=clang++-20 && cmake --build --preset asan && ctest --preset asan
cmake --preset tidy -DCMAKE_CXX_COMPILER=clang++-20 && run-clang-tidy-20 -p build/tidy -quiet "^$PWD/(core|app)/"
cmake --preset fuzz -DCMAKE_CXX_COMPILER=clang++-20 && cmake --build --preset fuzz && ctest --preset fuzz
pip install --require-hashes --no-deps -r .github/requirements/ci-tools.txt  # clang-format, ruff, zizmor, ...
```

On Windows, open a "Developer PowerShell for VS" (or let Visual Studio/VS Code
load `CMakePresets.json`). Without a Rust toolchain, add `-DMDN_SCAN_BUILD_RUST=OFF`.

## Maintenance

- Dependabot updates the actions, the Cargo dependencies and
  `.github/requirements/*.txt`.
- The tools downloaded by `install-tool` (typos, actionlint, cargo-deny,
  gitleaks, linuxdeploy), googletest and Corrosion are updated by hand: change
  the version and the SHA-256 (`sha256sum <file>`) together.
- Each `.github/requirements/*.txt` is generated from its `.in` file with the
  `uv pip compile` command written at the top of `ci-tools.in`.
- ELM327-emulator only ships an sdist whose `setup.py` rewrites its version
  when `GITHUB_RUN_NUMBER` is set; the OBD job installs it without that
  variable and with the build backend from `obd-emulator-build.txt`.

## Verifying a release

```bash
sha256sum --check SHA256SUMS
gh attestation verify mdn-scan-<version>-linux-x86_64.tar.gz --repo MDN-Solutions/MDN-Scan
```
