# Tooling

## Tooling goals
- fast iteration
- explicit build steps
- easy emulator execution
- low-friction debugging
- portable contributor setup

## Accepted direction
Primary stack: `llvm-mos`
Secondary path: handwritten assembly for isolated critical modules only when profiling clearly justifies it.

## Suggested workflow qualities
- one-command build
- one-command run in emulator
- debug/release separation
- source of truth documented in repo

## C64 build
- **Compiler:** llvm-mos; `make c64` expects `mos-c64-clang` on PATH (set `MOS_CC` to override).
- **Output:** `build/quattro.prg`.
- **Run:** `make c64_run` launches VICE `x64sc` if installed; otherwise load the PRG in your C64 emulator manually.

## compile_commands (clangd)

**Purpose:** Generate compilation databases so clangd (and similar tools) can index host and/or C64 sources accurately. The build system stays Makefile-based; [Bear](https://github.com/rizsotto/Bear) intercepts compiler invocations.

**Targets:**

| Target | Output | What is captured |
|--------|--------|------------------|
| `make compdb` | `compile_commands.json` | Host only (default: copy of `.host.json`) |
| `make compdb-host` | `compile_commands.host.json` | Core + host_debug + tests (host `cc`) |
| `make compdb-c64` | `compile_commands.c64.json` | Core + C64 platform (llvm-mos) |
| `make compdb-all` | `compile_commands.json` | Merged host + C64 into one file |

**Dependencies:** Bear is required for all compdb targets. `compdb-c64` and `compdb-all` also require llvm-mos (`mos-c64-clang` on PATH). If targets are already up to date, Bear may capture nothing; run `make clean && make compdb-host` (or the relevant build) to force a full capture.

**Recommendation:** For core-heavy work, prefer `make compdb` (host-only). For simultaneous host and C64 work, use `make compdb-all`.

**Default workflow:** Run `make compdb` for day-to-day work. That updates `compile_commands.json` from the host build, so clangd indexes core, host_debug, and tests. No C64 toolchain needed.

**C64 indexing:** Run `make compdb-c64` to produce `compile_commands.c64.json`. Then either:
- Use the merged database: `make compdb-all` so `compile_commands.json` contains both host and C64 entries; clangd will use the correct compiler and flags per file. Recommended if you work on both and have llvm-mos installed. Note: core sources appear twice in the merged file (once from host, once from C64); clangd picks one entry per file, so this is the one subtle point to be aware of when using the merged DB.
- Use C64 only: point your editor/clangd at `compile_commands.c64.json` (e.g. in VS Code: `"clangd.arguments": ["--compile-commands-dir=${workspaceFolder}", "--compile-commands-filename=compile_commands.c64.json"]` or equivalent). Switch back to default by using `compile_commands.json` (host) again.

**Commit:** All generated `compile_commands*.json` files are in `.gitignore` (machine-generated, may contain absolute paths).

## Important rule
The toolchain exists to support clarity and iteration speed.
Do not choose novelty over maintainability.
