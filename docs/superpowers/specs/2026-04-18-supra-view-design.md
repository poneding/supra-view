# Supra View Design

## Objective

Build a Windows-native local rendering pipeline that captures the desktop framebuffer, feeds the captured image through a GPU-only texture path, downsamples it with a high-quality shader, and presents the result in a window with low latency.

## Selected Baseline

- Language: C++17
- Build system: CMake
- Graphics API: Direct3D 11
- Capture API: DXGI Desktop Duplication
- Shader model: HLSL compiled with D3DCompiler

This baseline was chosen because Desktop Duplication integrates naturally with D3D11 resources and allows a direct GPU pipeline with lower implementation risk than a DX12 or Rust-first version.

## Module Boundaries

### `core/capture`

- Initialize duplication from the renderer-owned `ID3D11Device`
- Enumerate outputs on the device adapter
- Acquire and release desktop frames
- Surface duplication errors such as timeout and access loss

### `core/renderer`

- Create the D3D11 device, context, and swap chain
- Create GPU-owned source-copy and intermediate render targets
- Dispatch downsampling and presentation passes
- Keep the steady-state frame path on the GPU

### `core/shader`

- Provide embedded shader source for the fullscreen vertex shader, copy shader, and bicubic shader
- Compile shaders with readable diagnostics
- Keep a stable filter-selection interface that can later host a Lanczos implementation

### `app`

- Create the native Win32 window
- Run the message pump and frame loop
- Coordinate capture, rendering, resize handling, and shutdown

## Data Flow

```text
DXGI output
  -> Desktop Duplication acquire
  -> duplication texture
  -> GPU copy into shader-readable texture
  -> bicubic shader to intermediate texture
  -> copy shader to swap-chain backbuffer
  -> Present
```

## Design Decisions

### One-device model

The renderer owns the D3D11 device. Capture is initialized from that device so `DuplicateOutput` always receives a device created from the same adapter as the selected output.

### GPU copy before release

The duplication surface is copied into a GPU-owned texture before the duplication frame is released. This avoids depending on duplication-surface lifetime after `ReleaseFrame` and keeps sampling stable in later passes.

### Bicubic-first filter strategy

Bicubic is the first implemented filter because it provides a strong quality-per-cost balance for desktop imagery while staying simpler and more robust than a first-pass Lanczos implementation. The public renderer interface still exposes a Lanczos mode so the architecture does not need to change later.

### Scope control

The first version supports one output, one window, and a single-threaded frame loop. This keeps the system small enough to validate before adding multi-output synchronization or richer quality modes.

## Error Handling

- `DXGI_ERROR_WAIT_TIMEOUT` is treated as a normal no-new-frame condition.
- `DXGI_ERROR_ACCESS_LOST` triggers duplication recreation.
- Shader compile errors are surfaced as readable strings.
- Window resize recreates swap-chain-dependent resources while preserving capture state.

## Verification Strategy

### Possible on current macOS host

- Verify repository layout and module ownership
- Verify documentation and CMake intent
- Review source-level consistency and API boundaries

### Requires Windows

- Actual build and link against DirectX libraries
- Desktop Duplication initialization and frame acquisition
- Shader compilation, GPU rendering, present loop, and resize behavior

## Acceptance Criteria

- README explains the architecture clearly
- The project structure follows `/core/{capture,renderer,shader}` and `/app`
- The application builds on Windows with MSVC and CMake
- The application captures the desktop, downsamples it on the GPU, and presents the result in a native window
