# RendererLearn: A Lightweight C++ CPU Software Rasterizer

## Introduction

RendererLearn is a 3D software renderer built from scratch, independent of modern graphics APIs like OpenGL or DirectX. By simulating the entire graphics pipeline on the CPU, this project explores the fundamental principles of transforming 3D scenes into 2D pixels.



## Effect demonstration

![1766493303844](https://s2.loli.net/2025/12/31/msu1c3EBOaQJX4V.gif)

![1766889301734](https://s2.loli.net/2025/12/31/q7aeFBO1grdwYty.gif)

![1766925699406](https://s2.loli.net/2025/12/31/ikzwNqfj5HygXSO.gif)



## Core Features

- **OBJ Model Loader**: Integrated a custom parser to load and render 3D meshes in `.obj` format, including vertex positions, texture coordinates (UVs), and surface normals.
- **Complete Graphics Pipeline**: Fully implemented Model, View (Camera), Projection (Perspective), and Viewport transformations.
- **Barycentric Rasterization**: Triangle filling based on barycentric coordinate algorithms with support for **Sub-pixel Precision**.
- **Blinn-Phong Shading**: Optimized shading using the **Half-way Vector**, resolving highlight artifacts (cutoff) seen in the classic Phong model while improving performance.
- **Perspective Correct Interpolation**: Correctly interpolates attributes such as $1/w$, $u/w$, and $v/w$ to eliminate texture warping under extreme perspective angles.
- **Advanced Mapping Support**:
  - **Diffuse Mapping**: Renders high-resolution surface texture details.
  - **Normal Mapping**: Simulates complex surface geometry details in **Tangent Space**.
- **Depth Testing (Z-Buffer)**: Implemented with a 1D contiguous memory layout to efficiently handle occlusion and visibility.

## Performance Optimizations

This project focuses on execution efficiency, utilizing several CPU-specific optimizations:

1. **Reduced Division Overhead**: Converted expensive per-pixel division operations (e.g., perspective division) into **"one reciprocal + multiple multiplications,"** significantly boosting fill rate.
2. **Cache-Friendly Design**:
   - **1D Contiguous Memory**: Frame Buffer and Z-Buffer are stored in 1D arrays, dramatically increasing **CPU L1/L2 Cache hit rates**.
   - **Spatial Locality**: Processed pixels in **row-major order** to align with CPU hardware prefetchers.

## Technical Notes

- **Why Blinn-Phong?** The traditional Phong model requires calculating the reflection vector $R$, which is computationally expensive and produces unnatural edges. Blinn-Phong uses the half-vector $H = \text{normalize}(L + V)$, which is faster to compute and provides a smoother specular falloff.
- **Significance of Tangent Space**: By transforming lighting calculations into tangent space, normal maps become independent of the model's world coordinates, enabling texture reuse and support for skeletal animations.

##  Build & Run

- **Environment**: C++17，Windows，Visual Studio2022
- **Dependencies**: SDL3 (Used strictly for window management and pixel buffer display)
-  **Instructions**:
  1. Ensure the `.obj` files and texture maps are placed in project root directory.
  2. Open the `.sln` solution file in Visual Studio.
  3. Ensure the **SDL3** library paths are correctly configured in the Project Properties.
  4. Set the build configuration to **Release** / **x64** (Highly recommended for performance).
  5. Press **F5** to compile and run.

