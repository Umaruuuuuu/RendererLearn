#pragma once
#include "RenderData.h"
#include <algorithm>
#include <SDL3/SDL.h>
#include "Model.h"

void Render(int width, int height, Model* model, Mat4 model_mat, SDL_Surface* texture, Camera* camera, SDL_Surface* normal_map, Uint32* frame_buffer, vector<float>& z_buffer, Material material, std::vector<Light>& lights);
void RasterizeTriangle(const Triangle& tri, int width, int height, SDL_Surface* texture, Camera* camera, SDL_Surface* normal_map, Uint32* frame_buffer, vector<float>& z_buffer, Material material, std::vector<Light>& lights);
float EdgeFunction(const Vec3& p1, const Vec3& p2, const Vec3& p3);
inline Vec3 ComputeBarycentric(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c);
Vec3 GetPixelFromSurface(SDL_Surface* surface, float u, float v);
Vec3 reflect(const Vec3& I, const Vec3& N);
Vertex intersect(const Vertex& a, const Vertex& b, float w_near = 0.1f);
void TransformToScreen(Triangle& tri, int width, int height);