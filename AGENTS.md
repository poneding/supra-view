# Supra View Agent Notes

## Product path and architecture
- The default product path is **V2 Phase 1**, not the old Desktop Duplication viewer. Start from `README.md`, `app/main.cpp`, `app/v2_app_controller.*`, `app/window.*`, `core/input/*`, `core/session/*`, and the V2-side `core/renderer/*` / `core/shader/*` files.
- Treat `core/capture/*`, `app/app_controller.*`, and `core/renderer/window_renderer.*` as **historical or auxiliary V1 code** unless the task explicitly concerns the old capture-viewer path.
- Do not describe the repo as a desktop-capture product by default. The current default story is a **single Win32 window with one application-owned interactive workspace**.

## Build and run
- Preferred Windows build: `./scripts/build.ps1`
- Preferred Windows run: `./scripts/run.ps1`
- Batch wrappers (`scripts/build.bat`, `scripts/run.bat`) only forward to PowerShell with `-ExecutionPolicy Bypass`.
- `scripts/build.ps1` supports:
  - `-Configuration Debug|Release|RelWithDebInfo|MinSizeRel`
  - `-BuildDir <dir>`
  - `-Generator <name>`
  - `-Architecture x64|Win32|ARM64`
- The build script auto-detects Visual Studio with `vswhere.exe` and prefers:
  - `Visual Studio 18 2026` for VS major 18
  - `Visual Studio 17 2022` otherwise

## Build-directory gotcha
- `scripts/build.ps1` reads `CMakeCache.txt` and **refuses to reuse a build directory configured for a different generator**.
- If Visual Studio version or generator changes, use a fresh build directory, e.g.:
  - `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -BuildDir build-vs2026`

## Tests and verification
- The normal app build does **not** build tests.
- Test targets exist only when configuring with:
  - `-DSUPRA_VIEW_BUILD_WORKSPACE_PLACEHOLDER_SCENE_TEST=ON`
- That option enables these standalone test executables:
  - `workspace_placeholder_scene_test`
  - `workspace_renderer_validation_test`
  - `workspace_downsample_pass_test`
  - `workspace_shader_sources_test`
  - `workspace_downsample_shader_contract_test`
- Useful focused test flow:
  - `cmake -S . -B build-test -DSUPRA_VIEW_BUILD_WORKSPACE_PLACEHOLDER_SCENE_TEST=ON`
  - `cmake --build build-test --target <test_target>`
  - `ctest --test-dir build-test --output-on-failure -R <test_target>`
- Non-Windows hosts can configure some standalone tests only because that option bypasses the top-level Windows fatal check. Do **not** assume the full app or all renderer/shader paths are portable.

## Windows-only constraints
- The main executable is intentionally Windows-only because it depends on Win32, DXGI, D3D11, and D3DCompiler.
- On macOS/Linux, limit claims to source review, diagnostics, and the standalone tests that actually configure and run.
- Do not claim Windows runtime behavior is verified unless it was actually run on Windows.

## Worktree workflow
- `.worktrees/` is intentionally ignored in `.gitignore` and is the expected place for local git worktrees.
- Prefer new work in a `.worktrees/<branch-name>` worktree instead of directly on `master`.

## Documentation sources of truth
- High-value docs live under:
  - `docs/superpowers/specs/`
  - `docs/superpowers/plans/`
- The current V2 direction is documented in:
  - `docs/superpowers/specs/2026-04-20-supra-view-v2-design.md`
  - `docs/superpowers/plans/2026-04-20-supra-view-v2-phase1-implementation.md`

## Script pitfalls already hit in this repo
- In PowerShell scripts, keep `param(...)` at the top of the file. Earlier script failures came from putting `Set-StrictMode` before `param(...)`.
- Do not remove `.worktrees/` from `.gitignore`.
- If Windows build errors mention V2 files but unresolved capture helpers, check whether `CMakeLists.txt` still links the required utility `.cpp` files into `supra_view` before attempting larger refactors.
