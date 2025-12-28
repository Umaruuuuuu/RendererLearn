using namespace std;

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "Renderer.h"

void Render(int width, int height, Model* model, Mat4 model_mat, SDL_Surface* texture, Camera* camera, SDL_Surface* normal_map, Uint32* frame_buffer, vector<float>& z_buffer, Material material, std::vector<Light>& lights)
{
	// 计算mvp矩阵
	Mat4 projection = CreatePerspective((45.0f * atan(1.0f) * 4) / 180, (float)(width) / (float)(height), 0.1f, 100.0f);
	Mat4 view = CreateView(camera->position, camera->target, Vec3(0, 1, 0));
	Mat4 mvp = projection * view * model_mat;

	// 遍历所有三角形
	for (int i = 0; i < model->nfaces(); i++)
	{
		Face face = model->face(i);

		// 计算三角形法向量，用于背面剔除
		Vec3 v[3];
		v[0] = model->vert(face.v[0]);
		v[1] = model->vert(face.v[1]);
		v[2] = model->vert(face.v[2]);

		Vec3 world_v[3];
		world_v[0] = model_mat * v[0];
		world_v[1] = model_mat * v[1];
		world_v[2] = model_mat * v[2];

		Vec3 edge1 = world_v[1] - world_v[0];
		Vec3 edge2 = world_v[2] - world_v[0];
		Vec3 normal = normalize(cross(edge1, edge2));

		Vec3 view_dir = normalize((world_v[0] - camera->position));
		float intensity = dot(normal, view_dir);

		if (intensity >= 0)
		{
			continue;
		}

		// 近平面裁剪
		Vertex verts[3];
		Vertex* inside_verts[3];
		Vertex* outside_verts[3];
		int in_n = 0, out_n = 0;

		// 遍历三角形的三个顶点
		for (int j = 0; j < 3; j++)
		{
			// 应用mvp变换，将顶点从世界坐标转移到屏幕坐标
			Vec4 pos_clip = mvp * v[j];

			verts[j].position.x = pos_clip.x;
			verts[j].position.y = pos_clip.y;
			verts[j].position.z = pos_clip.z;

			// 读取顶点参数
			verts[j].texcoord = Vec2(model->vertTex(face.vt[j]));
			verts[j].normal = normalize(Vec3(model_mat * model->vertNor(face.vn[j])));
			verts[j].world_pos = world_v[j];
			verts[j].pos_clip_w = pos_clip.w;
			//verts[j].color = Vec3(1.0f, 1.0f, 1.0f) * dot(normalize(model->vertNor(face.vn[j])), normalize(light_dir * -1.0f));// 早期通过 Gouraud Shading 计算光照

			// 根据 w 判断顶点是否在视野内
			if (pos_clip.w >= 0.1f)
			{
				inside_verts[in_n++] = &verts[j];
			}
			else
			{
				outside_verts[out_n++] = &verts[j];
			}

		}

		if (in_n == 3)
		{
			// 三个顶点都在里面，直接绘制出来
			Triangle tri;
			tri.v[0] = *inside_verts[0];
			tri.v[1] = *inside_verts[1];
			tri.v[2] = *inside_verts[2];
			TransformToScreen(tri, width, height);
			RasterizeTriangle(tri, width, height, texture, camera, normal_map, frame_buffer, z_buffer, material, lights);
		}
		else if(in_n == 1)
		{
			// 只有一个顶点在里面，拆成一个小三角形
			Triangle new_tri;
			new_tri.v[0] = *inside_verts[0];
			new_tri.v[1] = intersect(*inside_verts[0], *outside_verts[0]);
			new_tri.v[2] = intersect(*inside_verts[0], *outside_verts[1]);
			TransformToScreen(new_tri, width, height);
			RasterizeTriangle(new_tri, width, height, texture, camera, normal_map, frame_buffer, z_buffer, material, lights);
		}
		else if (in_n == 2)
		{
			// 两个顶点在里面，拆成两个三角形
			Vertex A = intersect(*inside_verts[0], *outside_verts[0]);
			Vertex B = intersect(*inside_verts[1], *outside_verts[0]);

			Triangle tri1, tri2;

			tri1.v[0] = *inside_verts[0]; tri1.v[1] = *inside_verts[1]; tri1.v[2] = A;
			tri2.v[0] = *inside_verts[1]; tri2.v[1] = B; tri2.v[2] = A;

			TransformToScreen(tri1, width, height);
			TransformToScreen(tri2, width, height);
			RasterizeTriangle(tri1, width, height, texture, camera, normal_map, frame_buffer, z_buffer, material, lights);
			RasterizeTriangle(tri2, width, height, texture, camera, normal_map, frame_buffer, z_buffer, material, lights);
		}
	}
}

void RasterizeTriangle(const Triangle& tri, int width, int height, SDL_Surface* texture, Camera* camera, SDL_Surface* normal_map, Uint32* frame_buffer, vector<float>& z_buffer, Material material, std::vector<Light>& lights)
{
	// 找到能框柱三角形的最小矩形
	int x_min = (int)std::floor(std::min({ tri.v[0].position.x,tri.v[1].position.x, tri.v[2].position.x }));
	int x_max = (int)std::ceil(std::max({ tri.v[0].position.x,tri.v[1].position.x, tri.v[2].position.x }));
	int y_min = (int)std::floor(std::min({ tri.v[0].position.y,tri.v[1].position.y, tri.v[2].position.y }));
	int y_max = (int)std::ceil(std::max({ tri.v[0].position.y,tri.v[1].position.y, tri.v[2].position.y }));

	// 取整的时候可能会轻微溢出，要钳制一下
	x_min = std::max(0, x_min);
	x_max = std::min(width - 1, x_max);
	y_min = std::max(0, y_min);
	y_max = std::min(height - 1, y_max);

	// 遍历这个最小矩形
	for (int y = y_min; y <= y_max; y++)
	{
		for (int x = x_min; x <= x_max; x++)
		{
			// 计算该像素的重心坐标
			Vec3 curPoint((float)x + 0.5f, (float)y + 0.5f, 0.0f);
			Vec3 barycentric = ComputeBarycentric(curPoint, tri.v[0].position, tri.v[1].position, tri.v[2].position);

			// 重心坐标三个分量都大于0时，点在三角形内
			if (barycentric.x >= 0.0f && barycentric.y >= 0.0f && barycentric.z >= 0.0f)
			{
				// 对三个顶点的深度值进行插值，进行深度测试
				int index = y * width + x;
				float z = tri.v[0].position.z * barycentric.x + tri.v[1].position.z * barycentric.y + tri.v[2].position.z * barycentric.z;
				if (z < z_buffer[index])
				{
					z_buffer[index] = z;
				}
				else
				{
					continue;
				}

				// 透视矫正插值，计算出符合透视原理的 uv 坐标
				float interpolated_u_over_w = (tri.v[0].texcoord.x * tri.v[0].inv_w) * barycentric.x +
											  (tri.v[1].texcoord.x * tri.v[1].inv_w) * barycentric.y +
											  (tri.v[2].texcoord.x * tri.v[2].inv_w) * barycentric.z;

				float interpolated_v_over_w = (tri.v[0].texcoord.y * tri.v[0].inv_w) * barycentric.x +
											  (tri.v[1].texcoord.y * tri.v[1].inv_w) * barycentric.y +
											  (tri.v[2].texcoord.y * tri.v[2].inv_w) * barycentric.z;

				float interpolated_inv_w = tri.v[0].inv_w * barycentric.x +
										   tri.v[1].inv_w * barycentric.y +
										   tri.v[2].inv_w * barycentric.z;

				float true_u = interpolated_u_over_w / interpolated_inv_w;
				float true_v = interpolated_v_over_w / interpolated_inv_w;

				// 计算像素世界坐标
				float interp_pixel_x = (tri.v[0].world_pos.x * tri.v[0].inv_w) * barycentric.x +
									   (tri.v[1].world_pos.x * tri.v[1].inv_w) * barycentric.y +
									   (tri.v[2].world_pos.x * tri.v[2].inv_w) * barycentric.z;

				float interp_pixel_y = (tri.v[0].world_pos.y * tri.v[0].inv_w) * barycentric.x +
									   (tri.v[1].world_pos.y * tri.v[1].inv_w) * barycentric.y +
									   (tri.v[2].world_pos.y * tri.v[2].inv_w) * barycentric.z;

				float interp_pixel_z = (tri.v[0].world_pos.z * tri.v[0].inv_w) * barycentric.x +
									   (tri.v[1].world_pos.z * tri.v[1].inv_w) * barycentric.y +
									   (tri.v[2].world_pos.z * tri.v[2].inv_w) * barycentric.z;

				Vec3 pixel_world_pos(interp_pixel_x / interpolated_inv_w, interp_pixel_y / interpolated_inv_w, interp_pixel_z / interpolated_inv_w);

				// 获取纹理贴图
				Vec3 texColor;
				if (texture != NULL)
				{
					texColor = GetPixelFromSurface(texture, true_u, true_v);
				}
				else
				{
					int check = (int)(floor(true_u * 10.0f)) + (int)(floor(true_v * 10.0f));
					texColor = (check % 2 == 0) ? Vec3(0.2f, 0.2f, 0.2f) : Vec3(0.3f, 0.3f, 0.3f);
				}

				// 冯氏光照模型
				
				// 环境光
				Vec3 ambient = Vec3(1, 1, 1) * material.ambient;

				// 计算法线贴图
				Vec3 normal;
				Vec3 N = normalize(tri.v[0].normal * barycentric.x +
					tri.v[1].normal * barycentric.y +
					tri.v[2].normal * barycentric.z);

				if (normal_map != NULL)
				{
					Vec3 worldUp = (abs(N.y) > 0.99f) ? Vec3(0, 0, 1) : Vec3(0, 1, 0);// 防止 T 的叉乘结果为零向量，无法 normalize
					Vec3 T = normalize(cross(worldUp, N));
					Vec3 B = cross(N, T);

					Vec3 normalColor = GetPixelFromSurface(normal_map, true_u, true_v);
					Vec3 tangentNormal;
					tangentNormal.x = (normalColor.x * 2.0f) - 1.0f;
					tangentNormal.y = (normalColor.y * 2.0f) - 1.0f;
					tangentNormal.z = (normalColor.z * 2.0f) - 1.0f;

					normal = normalize(T * tangentNormal.x + B * tangentNormal.y + N * tangentNormal.z);
				}
				else
				{
					normal = N;
				}

				Vec3 total_diffuse(0, 0, 0);
				Vec3 total_specular(0, 0, 0);
				Vec3 V = normalize(camera->position - pixel_world_pos);

				// 菲涅尔近似
				float cosTheta = std::clamp(dot(V, N), 0.0f, 1.0f);
				float F0 = 0.04f;
				float fresnel = F0 + (1.0f - F0) * std::pow(1.0f - cosTheta, 5.0f);

				// 遍历所有光源
				for (const auto& light : lights)
				{
					if (light.type == LightType::Directional)
					{
						// 计算漫反射光
						float diff = std::max(dot(normal, normalize(light.dir_inv)), 0.0f);
						total_diffuse = total_diffuse + light.color * (diff * light.intensity * material.diffuse);//先算float提升性能

						// 计算高光
						Vec3 R = normalize(reflect(light.direction, normal));
						float spec = std::pow(std::max(dot(R, V), 0.0f), material.shininess);
						total_specular = total_specular + light.color * (spec * light.intensity * material.specular * fresnel);
					}
					else if (light.type == LightType::Point)
					{
						// 计算光源和当前像素的位置关系
						Vec3 light_vector = light.position - pixel_world_pos;
						float distance = length(light_vector);
						Vec3 L = normalize(light_vector);

						// 计算衰减系数
						float attenuation = 1.0f / (light.Kc + light.Kl * distance + light.Kq * distance * distance);

						// 计算漫反射光
						float diff = std::max(dot(normal, L), 0.0f);
						total_diffuse = total_diffuse + light.color * (diff * light.intensity * material.diffuse * attenuation);

						// 计算高光
						Vec3 R = normalize(reflect(L * -1.0f, normal));
						float spec = std::pow(std::max(dot(R, V), 0.0f), material.shininess);
						total_specular = total_specular + light.color * (spec * light.intensity * material.specular * attenuation * fresnel);
					}

				}

				// 最终颜色 = 纹理颜色 * （环境光 + 漫反射光） + 高光
				Vec3 final_color = texColor * (ambient + total_diffuse) + total_specular;

				frame_buffer[y * width + x] = Vec3ToUint32(final_color);
			}
		}
	}
}

float EdgeFunction(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
	Vec3 m = p2 - p1;
	Vec3 n = p3 - p1;

	return (m.y * n.x) - (m.x * n.y);
}

//计算重心坐标
Vec3 ComputeBarycentric(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c)
{
	//两个向量的叉乘等于三角形的面积的两倍
	float area_reciprocal = EdgeFunction(a, b, c);

	if (std::abs(area_reciprocal) < 1e-6)
	{
		return Vec3(-1.0f, -1.0f, -1.0f);
	}

	area_reciprocal = 1.0f / area_reciprocal;

	float alpha = EdgeFunction(b, c, p) * area_reciprocal;
	float beta = EdgeFunction(c, a, p) * area_reciprocal;
	float gamma = EdgeFunction(a, b, p) * area_reciprocal;

	return Vec3(alpha, beta, gamma);
}

//像素抓取
Vec3 GetPixelFromSurface(SDL_Surface* surface, float u, float v)
{
	//将uv坐标转换成像素坐标
	int x = (int)(u * surface->w);
	int y = (int)((1.0f - v) * surface->h);

	//防止越界
	x = std::clamp(x, 0, surface->w - 1);
	y = std::clamp(y, 0, surface->h - 1);

	Uint32* pixels = (Uint32*)surface->pixels;
	Uint32 pixel = pixels[y * (surface->pitch / 4) + x];

	Uint8 r, g, b, a;
	SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(surface->format), NULL, &r, &g, &b, &a);

	return Vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

Vec3 reflect(const Vec3& I, const Vec3& N)
{
	return I - N * (2.0f * dot(N, I));
}

Vertex intersect(const Vertex& a, const Vertex& b, float w_near)
{
	float t = (w_near - a.pos_clip_w) / (b.pos_clip_w - a.pos_clip_w);
	return Vertex::lerp(a, b, t);
}

void TransformToScreen(Triangle& tri, int width, int height)
{
	for (int j = 0; j < 3; j++)
	{
		//在这里透视除法，从3d裁剪空间到2d ndc空间！
		tri.v[j].inv_w = 1.0f / tri.v[j].pos_clip_w;

		float nx = tri.v[j].position.x * tri.v[j].inv_w;
		float ny = tri.v[j].position.y * tri.v[j].inv_w;
		float nz = tri.v[j].position.z * tri.v[j].inv_w;

		tri.v[j].position.x = (nx + 1.0f) * 0.5f * width;
		tri.v[j].position.y = (1.0f - ny) * 0.5f * height;
		tri.v[j].position.z = nz;
	}
}