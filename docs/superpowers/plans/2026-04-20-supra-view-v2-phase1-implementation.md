# Supra View V2 Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Windows-native interactive supersampled workspace that renders into a high-resolution offscreen target, downsamples on the GPU, and presents a directly interactive result without relying on Desktop Duplication.

**Architecture:** Phase 1 replaces the desktop-mirror viewer path with an application-owned workspace/session model. User input is mapped into a logical high-resolution workspace, rendering occurs in an offscreen render target, and the final image is downsampled to the actual presentation size before being shown in a window or borderless fullscreen surface.

**Tech Stack:** C++17, CMake, Win32 API, Direct3D 11, HLSL, D3DCompiler, WRL `ComPtr`

---

## Planned File Structure

### Reused files
- `CMakeLists.txt`
- `app/main.cpp`
- `app/window.h`
- `app/window.cpp`
- `core/renderer/d3d_context.h`
- `core/renderer/d3d_context.cpp`
- `core/shader/shader_compiler.h`
- `core/shader/shader_compiler.cpp`

### New files
- `core/session/workspace_state.h`
- `core/session/workspace_state.cpp`
- `core/session/workspace_session.h`
- `core/session/workspace_session.cpp`
- `core/input/input_state.h`
- `core/input/input_state.cpp`
- `core/input/input_mapper.h`
- `core/input/input_mapper.cpp`
- `core/renderer/workspace_renderer.h`
- `core/renderer/workspace_renderer.cpp`
- `core/shader/workspace_shader_sources.h`
- `core/shader/workspace_shader_sources.cpp`
- `app/v2_app_controller.h`
- `app/v2_app_controller.cpp`

### Files to retire or isolate from the Phase 1 main path
- `core/capture/*` remains in the repository but is removed from the default app path for Phase 1
- `core/renderer/texture_pipeline.*` is either adapted for workspace rendering or replaced by `workspace_renderer.*`
- `README.md` updated to distinguish V1 viewer history from V2 Phase 1 architecture

---

### Task 1: Reframe the application entry point around V2 Phase 1

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `app/main.cpp`
- Create: `app/v2_app_controller.h`
- Create: `app/v2_app_controller.cpp`

- [ ] **Step 1: Write the failing build-level expectation in the plan notes**

Expected behavior to introduce: the app starts without initializing Desktop Duplication and instead boots a V2 controller that owns a workspace session.

- [ ] **Step 2: Run the current source-level verification before editing**

Run: `lsp_diagnostics /Users/dp/supra-view/app error`
Expected: no pre-change errors in the current app sources.

- [ ] **Step 3: Add the V2 controller files to the build target**

Create `app/v2_app_controller.h` and `app/v2_app_controller.cpp`, and update `CMakeLists.txt` so the executable compiles against the V2 controller instead of the capture-driven controller path.

- [ ] **Step 4: Update `app/main.cpp` to construct the V2 app controller**

Keep the Win32 shell and message loop model, but route startup, per-frame update, input, resize, and shutdown through the V2 controller.

- [ ] **Step 5: Run diagnostics after the controller swap**

Run: `lsp_diagnostics /Users/dp/supra-view/app error`
Expected: zero errors.

### Task 2: Introduce the workspace/session model

**Files:**
- Create: `core/session/workspace_state.h`
- Create: `core/session/workspace_state.cpp`
- Create: `core/session/workspace_session.h`
- Create: `core/session/workspace_session.cpp`

- [ ] **Step 1: Write the failing interface expectation**

Expected behavior: the application can own a logical workspace resolution independently from the presentation resolution.

- [ ] **Step 2: Define the minimal workspace state model**

Create `workspace_state.*` with a focused structure that stores logical size, background style, simple interactive surface state, and per-frame update values.

- [ ] **Step 3: Define the session lifecycle object**

Create `workspace_session.*` to expose `Initialize`, `Update`, `Render`, and `OnResize` style entry points for the app controller and renderer.

- [ ] **Step 4: Keep the first milestone content intentionally simple**

Do not attempt arbitrary Windows app hosting. The workspace should initially manage only application-owned content such as a simple interactive panel, background, cursor marker, or debug layer.

- [ ] **Step 5: Run diagnostics on the session module**

Run: `lsp_diagnostics /Users/dp/supra-view/core/session error`
Expected: zero errors.

### Task 3: Add input capture and coordinate remapping

**Files:**
- Create: `core/input/input_state.h`
- Create: `core/input/input_state.cpp`
- Create: `core/input/input_mapper.h`
- Create: `core/input/input_mapper.cpp`
- Modify: `app/window.h`
- Modify: `app/window.cpp`
- Modify: `app/v2_app_controller.cpp`

- [ ] **Step 1: Write the failing behavior expectation**

Expected behavior: mouse and keyboard events are received from the Win32 shell and mapped into the logical workspace coordinate space.

- [ ] **Step 2: Extend the window shell with focused input callbacks**

Add callback hooks for mouse movement, mouse buttons, and keyboard events without bloating `window.*` into app logic.

- [ ] **Step 3: Define input state and mapping helpers**

Create `input_state.*` and `input_mapper.*` so physical output coordinates can be translated into logical workspace coordinates using the current output size and internal workspace size.

- [ ] **Step 4: Wire input into the V2 app controller**

The V2 controller should update workspace state based on mapped input rather than raw screen pixels.

- [ ] **Step 5: Run diagnostics after the input path is added**

Run: `lsp_diagnostics /Users/dp/supra-view/app error` and `lsp_diagnostics /Users/dp/supra-view/core/input error`
Expected: zero errors.

### Task 4: Create the workspace renderer and offscreen render target path

**Files:**
- Create: `core/renderer/workspace_renderer.h`
- Create: `core/renderer/workspace_renderer.cpp`
- Modify: `core/renderer/d3d_context.h`
- Modify: `core/renderer/d3d_context.cpp`

- [ ] **Step 1: Write the failing behavior expectation**

Expected behavior: rendering occurs first into an application-owned high-resolution offscreen target rather than into a texture copied from Desktop Duplication.

- [ ] **Step 2: Define the workspace renderer interface**

Create `workspace_renderer.*` so it can own a logical render resolution, workspace render target, intermediate resources, and final present pass.

- [ ] **Step 3: Add offscreen target creation and resize handling**

Use the existing D3D context as the device owner, but create distinct render targets for the logical workspace and the final presentation pass.

- [ ] **Step 4: Keep the first pass content minimal**

The renderer should be able to draw a simple application-owned scene or panel tree into the high-resolution target before downsampling.

- [ ] **Step 5: Run diagnostics on renderer sources**

Run: `lsp_diagnostics /Users/dp/supra-view/core/renderer error`
Expected: zero errors.

### Task 5: Add the V2 shader path for workspace downsampling

**Files:**
- Create: `core/shader/workspace_shader_sources.h`
- Create: `core/shader/workspace_shader_sources.cpp`
- Modify: `core/shader/shader_compiler.*`
- Modify: `core/renderer/workspace_renderer.*`

- [ ] **Step 1: Write the failing behavior expectation**

Expected behavior: the final presentation image is produced by downsampling the high-resolution workspace target on the GPU.

- [ ] **Step 2: Add dedicated workspace shader sources**

Create shader source files for the workspace present pass, including a fullscreen vertex shader and bicubic pixel shader tuned for offscreen-to-output downsampling.

- [ ] **Step 3: Compile and bind the workspace shader set through the existing shader compiler utilities**

Do not mix V2 workspace shader source names into the old viewer path. Keep the V2 shader path explicit.

- [ ] **Step 4: Connect the shader pass to the workspace renderer**

The workspace renderer should render application-owned content to the logical target, then execute the downsample pass into the swap-chain backbuffer.

- [ ] **Step 5: Run diagnostics for shader-facing sources**

Run: `lsp_diagnostics /Users/dp/supra-view/core/shader error`
Expected: zero errors.

### Task 6: Support borderless fullscreen and direct interaction mode

**Files:**
- Modify: `app/window.h`
- Modify: `app/window.cpp`
- Modify: `app/v2_app_controller.*`
- Modify: `README.md`

- [ ] **Step 1: Write the failing behavior expectation**

Expected behavior: the user can place Supra View into a borderless fullscreen presentation mode and interact directly with the workspace.

- [ ] **Step 2: Add a focused window-mode toggle path**

Implement a clean borderless fullscreen mode with a reversible transition back to normal windowed mode.

- [ ] **Step 3: Document the interaction model in the controller and README**

Make it explicit that V2 Phase 1 is an interactive workspace, not a mirrored desktop window.

- [ ] **Step 4: Keep input and presentation synchronization simple**

Do not add complex mode managers or multiple display-target abstractions in Phase 1. One window, one workspace, one presentation target.

- [ ] **Step 5: Run diagnostics after mode support is added**

Run: `lsp_diagnostics /Users/dp/supra-view/app error`
Expected: zero errors.

### Task 7: Retire the V1 viewer path from the default product flow

**Files:**
- Modify: `README.md`
- Modify: `docs/superpowers/specs/2026-04-20-supra-view-v2-design.md`
- Modify: `core/capture/*` comments or documentation only if needed

- [ ] **Step 1: Write the failing documentation expectation**

Expected behavior: repository docs no longer describe Desktop Duplication as the primary user-facing product path for V2.

- [ ] **Step 2: Update README to distinguish V1 history from V2 Phase 1**

Keep V1 capture code documented only as historical or auxiliary implementation material.

- [ ] **Step 3: Remove ambiguous claims about "desktop capture" from V2-facing docs**

The user should be able to read the docs and immediately understand that V2 Phase 1 is an application-owned workspace.

- [ ] **Step 4: Review docs for honesty and scope control**

Make sure the docs do not imply that arbitrary Windows desktop apps are already hosted in the first Phase 1 milestone.

- [ ] **Step 5: Re-read the changed docs and confirm wording is consistent**

Read: `README.md` and `docs/superpowers/specs/2026-04-20-supra-view-v2-design.md`
Expected: no contradictory wording about V1 versus V2.

## Verification Notes

- The current macOS host can verify file structure, source consistency, and diagnostics only.
- Actual Windows compilation, rendering, input, and fullscreen validation must occur on a Windows host.
- Because Phase 1 introduces a new interactive path, manual Windows-side validation is mandatory and cannot be replaced by source inspection alone.

## Required Windows Validation After Implementation

- Build the project with Visual Studio and CMake.
- Launch the V2 app and confirm it does not initialize Desktop Duplication in the main product path.
- Verify a visible interactive workspace exists.
- Verify mouse and keyboard input affect the workspace using logical workspace coordinates.
- Verify the workspace is rendered internally at a higher resolution than the presentation surface.
- Verify borderless fullscreen mode works and exits cleanly.
- Verify there is no hall-of-mirrors recursion because no desktop capture viewer is in the main loop.
