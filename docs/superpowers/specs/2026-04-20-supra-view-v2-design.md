# Supra View V2 Phase 1 Design

## Objective

Supra View V2 Phase 1 defines the repository's default product direction as a Windows-native application-owned workspace, not as a mirrored-desktop viewer. The default user-facing path is one native Win32 window that owns its own workspace surface, accepts direct pointer and keyboard input, and supports standard windowed and borderless fullscreen presentation.

This direction replaces the old V1 capture-viewer story as the top-level identity for the worktree. The retained Desktop Duplication path remains available only as historical or auxiliary code.

## Current default runtime path

The current executable entry point is `app/main.cpp`, which constructs `supra::app::Window` and initializes `supra::app::V2AppController`.

That default path currently establishes these user-facing behaviors:

- One native Win32 shell window.
- Direct input routing from window-client coordinates into logical workspace coordinates.
- Resize handling tied to the active window-client area.
- Borderless fullscreen entry and exit with `F11`, plus `Esc` restore.
- A product model centered on one application-owned workspace surface.

## Scope boundaries for Phase 1

Phase 1 is intentionally narrower than a full application-hosting environment.

### In scope

- One window.
- One application-owned interactive workspace surface.
- Direct shell-level pointer and keyboard handling.
- Logical-workspace input mapping.
- Borderless fullscreen presentation behavior.
- Repository groundwork for workspace state and placeholder rendering.

### Out of scope for the current Phase 1 story

- Arbitrary Windows desktop application hosting inside the workspace.
- Multi-window or multi-workspace composition.
- Multi-monitor workspace management.
- A remote desktop product.
- A mirrored-desktop viewer as the default path.

## Module roles

### `app`

- `main.cpp` selects the V2 path as the default executable behavior.
- `v2_app_controller.*` owns shell-level input handling, logical workspace mapping, and fullscreen shortcuts.
- `window.*` owns the native Win32 window, message loop, resize callbacks, input event forwarding, and borderless fullscreen shell transitions.
- `app_controller.*` remains in the tree as the older V1 capture-oriented controller.

### `core/input`

- Defines input events, pointer state, keyboard state, and mapping from input-surface coordinates into logical workspace coordinates.
- Supports the V2 requirement that input stays bound to the application-owned workspace surface rather than a mirrored desktop image.

### `core/session`

- Holds the current workspace session and workspace state models.
- Tracks logical and presentation resolutions, debug metadata, and placeholder panel visibility.
- Represents current V2 groundwork for workspace-owned content rather than a desktop capture feed.

### `core/renderer`

- Contains the D3D11 context and workspace rendering groundwork for the V2 direction.
- Includes a placeholder-scene path, logical and presentation render targets, and a downsample pass.
- Also retains `window_renderer.*`, which belongs to the older capture-viewer path.

### `core/shader`

- Provides shared shader infrastructure.
- Holds both legacy full-screen capture-viewer shader sources and V2 workspace downsample shader sources.

### `core/capture`

- Retains the DXGI Desktop Duplication implementation from V1.
- Is historical or auxiliary relative to the Phase 1 product story.
- Is not the default user-facing entry path for this worktree.

## Interaction model

The Phase 1 interaction model is direct and local.

- The user interacts with one native window.
- Pointer events arrive in window-client coordinates, then the V2 controller remaps them into logical workspace coordinates.
- Keyboard events are handled by the same shell path, including fullscreen shortcuts.
- Borderless fullscreen changes shell chrome and presentation bounds, but it does not create a second workspace or a separate desktop shell.

## Product honesty requirements

Documentation for this branch must make these points clear:

- V2 Phase 1 is the default product story.
- The old Desktop Duplication viewer is retained code, not the main identity.
- Current V2 work establishes shell, input, and workspace-direction foundations.
- The repository does not yet claim arbitrary Windows desktop app hosting inside the Phase 1 workspace.

## Validation strategy

### Possible on the current macOS host

- Verify repository structure.
- Verify that README and spec language match the current V2 entry path.
- Review source-level consistency between `app/main.cpp`, `app/v2_app_controller.*`, `core/input/*`, and supporting workspace modules.

### Requires Windows

- Build and link the Win32 and D3D11 application.
- Verify default shell startup.
- Verify focus-sensitive input handling.
- Verify borderless fullscreen transitions with `F11` and `Esc`.
- Verify any future integration between workspace session, renderer, and the default executable path.

## Acceptance criteria

- Repository documentation leads with V2 Phase 1 as the default path.
- Documentation describes one application-owned interactive workspace in one native window.
- Documentation does not present the legacy Desktop Duplication viewer as the product identity.
- Documentation does not imply arbitrary Windows desktop application hosting already exists in Phase 1.
- Retained `core/capture` code is described as historical or auxiliary when referenced.
