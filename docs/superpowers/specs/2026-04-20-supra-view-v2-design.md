# Supra View V2 Design

## Objective

Redesign Supra View from a desktop-capture viewer into a local interactive supersampled rendering environment that can later evolve into a virtual-display-backed system. V2 must stop treating desktop duplication as the product architecture and instead treat it, at most, as a diagnostic side path.

## Why V1 Is Not the Product Target

V1 duplicates the already composed Windows desktop, downsamples it on the GPU, and presents that processed image into a normal Win32 window. That is sufficient for proving a capture-plus-shader pipeline, but it cannot become a VMware-like local rendering environment because:

- it mirrors an existing desktop rather than owning a separate interactive rendering space;
- it structurally suffers from self-reference on a single monitor;
- it cannot provide an environment that the user can directly "enter" and interact with as a first-class workspace;
- hiding the viewer window from capture does not create a virtual display, compositor, or alternate desktop target.

V2 therefore changes the product definition rather than iterating further on the V1 viewer architecture.

## Selected V2 Strategy

Supra View V2 uses a phased design.

### Phase 1: Interactive supersampled workspace

Build a Windows-native application that owns its own high-resolution rendering space, receives user input directly, and presents a downsampled result to the real monitor. This phase does not attempt to impersonate the full Windows desktop and does not depend on Desktop Duplication.

### Phase 2: Virtual display / IDD expansion

Extend the Phase 1 architecture toward a true virtual-display-backed system using a Windows virtual display strategy such as an Indirect Display Driver (IDD) or an equivalent virtual presentation backend. This phase aims to make the rendering environment feel closer to a separate monitor or remoted desktop target.

This phased strategy was selected because it solves the user's immediate requirement for a directly interactive supersampled environment without forcing the first milestone into driver-level complexity.

## Product Definition for Phase 1

Phase 1 is not a desktop mirror. It is a standalone interactive rendering environment with these properties:

- it owns a virtual workspace with a high internal rendering resolution, such as 3840x2160;
- it downsamples the final image to the actual output presentation resolution, such as 2560x1440;
- it accepts keyboard and mouse input directly and maps physical input coordinates into the virtual workspace;
- it supports fullscreen or borderless fullscreen presentation;
- it has no recursive hall-of-mirrors capture behavior because it no longer captures the host desktop as its primary content source.

## Phase 1 Architecture

### Core system model

```text
Input events
  -> workspace/session state
  -> high-resolution offscreen render target
  -> GPU downsample pass
  -> native presentation surface
  -> screen
```

This data flow is the key architectural correction. The source of pixels is the workspace owned by Supra View itself, not the already composed host desktop.

## Module Boundaries

### `core/session`

- Own the virtual workspace lifecycle.
- Define the internal render resolution and logical workspace dimensions.
- Hold scene or workspace state.
- Expose update and render contracts to the renderer.

### `core/renderer`

- Create the high-resolution offscreen render target.
- Own the graphics device, command submission path, and presentation backend.
- Dispatch downsample passes.
- Support windowed and borderless fullscreen presentation.

### `core/shader`

- Provide downsampling filters such as bicubic first and Lanczos later.
- Provide optional sharpen or debug overlay passes.
- Keep filter and presentation parameters isolated from session logic.

### `core/input`

- Translate physical mouse coordinates to virtual workspace coordinates.
- Manage keyboard and pointer routing.
- Provide hit-testing and focus abstractions that are independent from presentation resolution.

### `app`

- Own the Win32 application shell.
- Manage the main loop, window mode changes, and lifecycle.
- Wire together session, renderer, shader, and input components.

## Phase 1 Interaction Model

The user interacts with the virtual workspace directly. Supra View receives native input events and remaps them into the logical high-resolution workspace. This makes the environment feel like a supersampled local rendering target rather than a passive video surface.

Minimum interaction requirements:

- mouse move, click, drag, and hover support;
- keyboard input routing;
- borderless fullscreen mode;
- a clean exit path and mode switching;
- an optional debug overlay showing internal resolution, output resolution, and filter mode.

## Phase 1 Content Model

Phase 1 must own its content model explicitly. It does not inherit content by capturing the host desktop. The initial content scope should therefore be constrained to content that Supra View itself can render and manage. This may include:

- a custom rendered workspace or scene;
- a UI shell or demonstrator surface;
- synthetic or application-managed panels used to validate interaction and scaling behavior.

This is a deliberate scope choice that keeps Phase 1 honest. Arbitrary existing Windows desktop applications are out of scope for the first milestone unless a separate hosting model is introduced deliberately.

## Phase 2 Direction: Virtual Display Expansion

Once the Phase 1 core is stable, the architecture can evolve toward a virtual display backend.

### What stays stable

- `core/session`
- `core/input`
- `core/shader`
- most high-resolution render and downsample logic in `core/renderer`

### What changes

- presentation moves from a simple app-controlled window shell toward a virtual display or equivalent backend;
- the system gains a virtual display abstraction layer, likely under `platform/display` or `core/display`;
- installation, device management, and system integration complexity increase significantly.

### What Phase 2 is expected to unlock

- a rendering target that behaves more like a separate display or remote-rendered workspace;
- a product experience closer to the original VMware-like expectation;
- removal of the conceptual mismatch between a host-window viewer and an independent working environment.

## Risks and Trade-Offs

### Phase 1 trade-off

Phase 1 solves the interaction and supersampling problem but does not yet create a system-visible virtual monitor. This is intentional. It provides a usable milestone while preserving the path to a more ambitious system-level design.

### Phase 2 trade-off

Virtual display work is significantly more complex. It involves system integration, installation friction, and a deeper Windows display-stack understanding. It should only begin after the core workspace, input mapping, and rendering model are stable.

### Non-goal clarification

V2 Phase 1 is not a stealth continuation of the V1 viewer. Desktop Duplication is no longer the architectural center of the product. Any viewer or capture path retained from V1 should be documented as auxiliary tooling only.

## Verification Strategy

### Phase 1 verification

- verify that rendering originates from an application-owned offscreen workspace rather than from Desktop Duplication;
- verify coordinate mapping between physical output and logical workspace;
- verify borderless fullscreen interaction;
- verify that no recursive self-capture behavior exists because no desktop mirror path is used for the main product loop.

### Phase 2 verification

- verify virtual display creation and lifecycle;
- verify that the virtual target can be presented and interacted with as a distinct environment;
- verify stability across display-mode changes and multi-display configurations.

## Acceptance Criteria

### V2 Phase 1

- Supra View no longer depends on Desktop Duplication for the main user-facing path.
- Supra View owns a high-resolution interactive workspace.
- User input is mapped correctly into that workspace.
- The final image is downsampled on the GPU before presentation.
- The app supports fullscreen or borderless fullscreen interaction.
- Recursive hall-of-mirrors behavior is absent by architecture, not by masking hacks.

### V2 Phase 2

- Supra View can target a virtual display or equivalent system-level display path.
- The environment behaves like a distinct workspace rather than a viewer window.
- The system architecture matches the original VMware-like product expectation more closely than V1 ever could.
