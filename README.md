# Supra View

Supra View V2 Phase 1 is a Windows-native application shell for one application-owned interactive workspace. The default product path on this branch is a single Win32 window that owns its own workspace surface, maps pointer and keyboard input directly into that surface, and supports standard windowed and borderless fullscreen presentation.

The repository still contains the earlier V1 Desktop Duplication viewer pipeline under `core/capture` and the older app controller path. That capture-first flow is now historical or auxiliary code, not the main user-facing story for this worktree.

## V2 Phase 1 product direction

- One native Win32 shell window.
- One application-owned interactive workspace surface.
- Pointer and keyboard input mapped from window-client coordinates into logical workspace coordinates.
- Borderless fullscreen toggled with `F11`, restored with `F11` or `Esc`.
- No mirrored desktop window and no second desktop shell.

### What the default path does today

- `app/main.cpp` launches `V2AppController` as the default executable path.
- `app/window.*` owns the native window, message pump, resize notifications, and borderless fullscreen shell behavior.
- `core/input/*` defines input events and maps the window input surface into logical workspace coordinates.
- `core/session/*` and `core/renderer/*` hold the current V2 workspace-state and placeholder-rendering groundwork.
- Windows remains the only supported runtime target.

### What V2 Phase 1 does not claim yet

- Arbitrary Windows desktop application hosting inside the workspace.
- Multi-window or multi-workspace composition.
- Multi-monitor workspace management.
- A remote desktop or mirrored-desktop product identity.

## Historical V1 capture path

The repository still keeps the original Desktop Duplication capture and bicubic downsampling pipeline for reference and auxiliary development work. That path is not the default product flow for this branch.

- `core/capture/*` owns DXGI Desktop Duplication setup and per-frame desktop acquisition.
- `app/app_controller.*` is the earlier controller that wires capture into the legacy renderer path.
- `core/renderer/window_renderer.*` and related legacy shader helpers support that older viewer flow.

If you are reading this repository to understand current product intent, start with the V2 path above, not the retained V1 capture viewer.

## Repository layout

```text
app/
  main.cpp
  v2_app_controller.*
  window.*
  app_controller.*
core/
  capture/
  input/
  session/
  renderer/
  shader/
docs/
  superpowers/
    specs/
    plans/
```

## Validation matrix

| Validation | macOS host | Windows host |
|---|---|---|
| Repository structure and documentation | Yes | Yes |
| V2 shell source layout and input-mapping review | Yes | Yes |
| Win32 and D3D11 compile or link verification | No | Yes |
| Default V2 shell runtime behavior | No | Yes |
| Historical V1 Desktop Duplication runtime | No | Yes |

## Build requirements

- Windows 10 or Windows 11
- Visual Studio 2022, Visual Studio 2026, or Build Tools with MSVC
- Windows SDK with D3D11, DXGI, and D3DCompiler
- CMake 3.20 or newer

## Build instructions

Preferred scripted workflow:

```powershell
.\scripts\build.ps1
```

If you prefer Command Prompt or double-click-friendly wrappers:

```bat
scripts\build.bat
```

Useful options:

```powershell
.\scripts\build.ps1 -Configuration Debug
.\scripts\build.ps1 -BuildDir build-debug -Configuration Debug
```

Manual CMake workflow:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

If you are using Visual Studio 2026 instead, switch the generator name:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

## Run instructions

Preferred scripted workflow:

```powershell
.\scripts\run.ps1
```

If you prefer Command Prompt or batch wrappers:

```bat
scripts\run.bat
```

To run a different build configuration:

```powershell
.\scripts\run.ps1 -Configuration Debug
```

Manual launch:

```powershell
build\Release\supra_view.exe
```

### Expected current V2 Phase 1 behavior

- A native Win32 window opens.
- The process initializes the V2 shell controller for one application-owned workspace surface.
- Pointer and keyboard input are interpreted against the current window-client bounds and logical workspace mapping.
- Resizing the window updates the active presentation bounds and input mapping.
- Press `F11` to enter or leave borderless fullscreen on the active monitor.
- Press `Esc` to restore the standard windowed mode when borderless fullscreen is active.

The default executable on this branch does not start the old Desktop Duplication viewer path.

## Current limitations

- Windows only.
- One shell window and one workspace surface.
- The default executable establishes the V2 shell, input, and presentation model. It does not yet claim arbitrary hosted Windows applications inside the workspace.
- Workspace session and placeholder-rendering groundwork exist in the repository, but the branch is still in the Phase 1 application-owned workspace transition.
- Legacy capture code remains in the tree for historical or auxiliary use, not as the default product flow.

## Troubleshooting

- `F11` or `Esc` has no visible effect
  - Ensure the Supra View window has input focus. The shell shortcut path is handled through the active V2 window.
- Build fails on a non-Windows host
  - This is expected. The CMake project intentionally stops outside Windows because the runtime depends on Win32 and DirectX 11.
- `scripts\build.ps1` says CMake was not found
  - Install Visual Studio C++ tools with CMake support, or install CMake separately and ensure `cmake.exe` is on `PATH`.
- PowerShell blocks the script
  - Start PowerShell with a policy that allows local scripts for your user, for example:
    `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`
- Visual Studio 2026 is installed but the build directory was previously configured for Visual Studio 2022
  - Remove `build`, or use a fresh directory such as:
    `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -BuildDir build-vs2026`
- `build.bat` or `run.bat` fails immediately
  - The batch wrappers forward to PowerShell. Ensure `powershell.exe` is available and that your Windows installation has not disabled it.

### Historical V1 capture troubleshooting

These notes apply only if you are inspecting or reviving the retained Desktop Duplication path.

- `DuplicateOutput` fails immediately
  - Ensure the device and output come from the same adapter.
  - Ensure the process is running in a normal interactive desktop session.
- `DXGI_ERROR_ACCESS_LOST`
  - The desktop mode changed or the output was invalidated. The capture path must recreate duplication.
- Black or stale output
  - Check shader compile errors in the debugger output.
  - Check that the output index exists on the selected adapter.

## Future work

- Continue wiring the V2 workspace session and renderer into a fuller default application path.
- Replace placeholder workspace content with richer application-owned workspace behavior.
- Decide the long-term disposition of the retained V1 capture modules.
