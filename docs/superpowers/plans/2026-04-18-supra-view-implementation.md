# Supra View Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Windows-only Direct3D 11 desktop-capture viewer that downsamples desktop frames on the GPU and presents them in a native window.

**Architecture:** The renderer creates one D3D11 device on the adapter used for presentation, and the capture module initializes Desktop Duplication from that same device to satisfy `DuplicateOutput` constraints. Captured frames are copied into a shader-readable texture, processed by a bicubic fullscreen shader, and then presented through a swap chain.

**Tech Stack:** C++17, CMake, Win32 API, DXGI Desktop Duplication, Direct3D 11, HLSL, D3DCompiler, WRL `ComPtr`

---

### Task 1: Write architecture documentation

**Files:**
- Create: `README.md`
- Create: `docs/superpowers/specs/2026-04-18-supra-view-design.md`

- [x] **Step 1: Describe the approved architecture**
- [x] **Step 2: Document module boundaries and data flow**
- [x] **Step 3: Record validation limits for macOS vs Windows**

### Task 2: Initialize project structure

**Files:**
- Create: `CMakeLists.txt`
- Create: `core/capture/*`
- Create: `core/renderer/*`
- Create: `core/shader/*`
- Create: `app/*`

- [x] **Step 1: Add Windows-only CMake target definition**
- [x] **Step 2: Add source files in required directories**
- [x] **Step 3: Keep the skeleton internally consistent**

### Task 3: Implement desktop capture module

**Files:**
- Modify: `core/capture/desktop_capture.h`
- Modify: `core/capture/desktop_capture.cpp`
- Modify: `core/capture/capture_types.h`
- Modify: `core/capture/capture_utils.*`

- [x] **Step 1: Define capture config, result, and frame types**
- [x] **Step 2: Implement duplication initialization from renderer-owned device**
- [x] **Step 3: Implement frame acquire and release flow**
- [x] **Step 4: Handle timeout and access-loss conditions explicitly**

### Task 4: Implement GPU texture pipeline

**Files:**
- Modify: `core/renderer/d3d_context.*`
- Modify: `core/renderer/texture_pipeline.*`
- Modify: `core/renderer/window_renderer.*`

- [x] **Step 1: Create D3D11 device, context, swap chain, and backbuffer RTV**
- [x] **Step 2: Copy acquired frame into a shader-readable GPU texture**
- [x] **Step 3: Create intermediate render target for downsampled output**

### Task 5: Implement downsampling shader

**Files:**
- Modify: `core/shader/*`
- Modify: `core/renderer/renderer_types.h`
- Modify: `core/renderer/texture_pipeline.*`

- [x] **Step 1: Add fullscreen vertex and copy shaders**
- [x] **Step 2: Add bicubic pixel shader**
- [x] **Step 3: Keep a Lanczos-ready filter-selection interface**

### Task 6: Render to window

**Files:**
- Modify: `app/*`
- Modify: `core/renderer/window_renderer.*`

- [x] **Step 1: Create native window and message loop**
- [x] **Step 2: Wire capture, renderer, and presentation into the frame loop**
- [x] **Step 3: Handle resize and shutdown cleanly**

### Task 7: Build and run instructions

**Files:**
- Modify: `README.md`

- [x] **Step 1: Document Windows prerequisites**
- [x] **Step 2: Document configure/build/run commands**
- [x] **Step 3: Document expected runtime behavior and troubleshooting**

## Verification Notes

- macOS host can verify layout, documentation, and source-level consistency only.
- Windows host must be used for actual compilation, Desktop Duplication runtime validation, shader compilation, and windowed presentation.

## Remaining manual verification on Windows

- Configure and build with Visual Studio 2022
- Run the executable and confirm live desktop output
- Verify resize behavior
- Trigger duplication recreation by changing desktop state
