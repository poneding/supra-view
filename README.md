# Supra View

Supra View is a Windows-only local rendering pipeline that captures the desktop with DXGI Desktop Duplication, copies the captured framebuffer into a GPU-owned texture, downsamples it with a Direct3D 11 bicubic shader, and displays the processed image in a native Win32 window.

## Goals

- Simulate a high-resolution rendering path similar to a remote-rendering or supersampled viewer.
- Capture the desktop on the GPU with minimal CPU involvement.
- Downsample on the GPU with a high-quality shader.
- Present the processed result in a 2K-class output window.

## Architecture

The project is split into the required modules:

- `core/capture`
  - Owns DXGI Desktop Duplication setup and per-frame capture.
  - Uses the renderer-owned D3D11 device so capture and rendering stay on the same adapter.
- `core/renderer`
  - Owns the D3D11 device, immediate context, swap chain, GPU texture pipeline, and presentation.
  - Copies the duplication surface into a shader-readable texture before releasing the acquired frame.
- `core/shader`
  - Owns embedded HLSL source and shader compilation helpers.
  - Provides a bicubic downsampling shader and a Lanczos-ready interface.
- `app`
  - Owns the Win32 window, application controller, and main loop.

### Frame Pipeline

```text
Desktop output
  -> IDXGIOutputDuplication::AcquireNextFrame
  -> captured ID3D11Texture2D
  -> GPU copy into shader-readable source texture
  -> bicubic fullscreen pixel shader pass
  -> intermediate render target
  -> copy pass to swap-chain backbuffer
  -> Present
```

### Why the project uses one D3D11 device

Desktop Duplication requires `IDXGIOutput1::DuplicateOutput` to receive a device created from the same adapter that owns the target output. To avoid cross-adapter mistakes and extra synchronization, Supra View creates one D3D11 device on the selected adapter and shares it across capture and rendering.

### Initial scope

- Windows 10/11, D3D11, DXGI 1.2 Desktop Duplication
- Single output capture
- Bicubic downsampling as the first production filter
- Lanczos kept as a public scaling mode for a later implementation
- Single-threaded main loop for the first working demo

### Current limitations

- Windows only
- No multi-monitor stitching
- No HDR-specific handling
- No input forwarding or remote control features
- Lanczos is interface-ready but currently falls back to bicubic with a debug log

## Repository Layout

```text
core/
  capture/
  renderer/
  shader/
app/
docs/
  superpowers/
    specs/
    plans/
```

## Validation Matrix

| Validation | macOS host | Windows host |
|---|---|---|
| Repository structure and documentation | Yes | Yes |
| CMake intent and source layout review | Yes | Yes |
| DirectX compile/link verification | No | Yes |
| Desktop Duplication runtime | No | Yes |
| Shader compilation and presentation | No | Yes |
| Resize and access-loss recovery | No | Yes |

## Build Requirements

- Windows 10 or Windows 11
- Visual Studio 2022 or Build Tools with MSVC
- Windows SDK with D3D11, DXGI, and D3DCompiler
- CMake 3.20 or newer

## Build Instructions

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Run Instructions

```powershell
build\Release\supra_view.exe
```

Expected first-run behavior:

- A native Win32 window opens.
- The application captures the selected desktop output.
- The captured frame is copied into a GPU-owned texture.
- The bicubic shader downsamples the source to the current output size.
- The processed image is presented into the window.

## Troubleshooting

- `DuplicateOutput` fails immediately
  - Ensure the device and output come from the same adapter.
  - Ensure the process is running in a normal interactive desktop session.
- `DXGI_ERROR_ACCESS_LOST`
  - The desktop mode changed or the output was invalidated. The application will attempt to recreate duplication.
- Black or stale output
  - Check shader compile errors in the debugger output.
  - Check that the output index exists on the selected adapter.
- Build fails on a non-Windows host
  - This is expected. The CMake project intentionally stops outside Windows because the runtime depends on Win32 and DirectX 11.

## Future Work

- Implement a real Lanczos filter path.
- Add output-selection UI and diagnostics overlay.
- Improve recovery behavior across output changes and secure desktop transitions.
