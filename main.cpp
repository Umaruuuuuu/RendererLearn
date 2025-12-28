using namespace std;

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <cmath>

#include "Math.h"
#include "Renderer.h"
#include "Model.h"

// 窗口大小
int width = 800;    
int height = 600;   

// 鼠标控制相关
float lastX = 400, lastY = 300;
bool firstMouse = true;

// 多光源系统
std::vector<Light> lights =
{
    Light::Directional(Vec3(1, -1, -1), Vec3(1, 1, 0.8f), 0.8f),    // 平行光（模拟太阳光）
    Light::Point(Vec3(0, 2, 0), Vec3(0.2f, 0.2f, 0.5f), 10.0f)      // 点光源
};

SDL_Surface* LoadTexture(const char* filename);
void MoveCamera(Camera* camera);
void MoveLight();
void CalculateFPS();

// 帧间隔时间计算
Uint64 last_time;
Uint64 current_time;
float deltaTime;

int main(int argc, char* argv[]) {

    // 加载模型、纹理、法线贴图
    Model* plant = new Model("indoor plant_02.obj");
    SDL_Surface* texture = LoadTexture("indoor plant_2_COL.jpg");
    SDL_Surface* normal_map = LoadTexture("indoor plant_2_NOR.jpg");
    Material plant_Mat = { 0.1f, 0.8f, 0.1f, 5.0f };
    Mat4 plant_model_mat1 = CreateScale(Vec3(1.0f, 1.0f, 1.0f));

    Model* ground = new Model("ground.obj");
    Material ground_Mat = { 0.1f, 0.7f, 0.5f, 32.0f };
    Mat4 ground_model_mat = CreateTranslation(Vec3(0, 0, 0));

    // 创建窗口、渲染器
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("RendererLearn", width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0);

    // 开启相对鼠标模式，隐藏鼠标指针
    SDL_SetWindowRelativeMouseMode(window, true);

    // 创建相机
    Camera* camera = Camera::CreateCamera(Vec3(0, 2, 20), 5.0f, -90.0f, 0.0f);
    last_time = SDL_GetTicks();

    // 创建临时帧缓冲区
    Uint32* frame_buffer = new Uint32[width * height];
    SDL_Texture* texture_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    // 创建深度缓冲区
    std::vector<float> z_buffer(width * height, 1.0f);

    while (true)
    {
        // 清空渲染器与深度缓冲区
        SDL_RenderClear(renderer);
        std::fill(z_buffer.begin(), z_buffer.end(), 1.0f);

        // 绘制一个蓝色渐变天空盒
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float t = (float)y / height;
                Vec3 skyColor = Vec3(0.5f, 0.7f, 1.0f) * (1 - t) + Vec3(0.f, 0.2f, 0.4f) * t;
                frame_buffer[y * width + x] = Vec3ToUint32(skyColor);
            }
        }

        // 鼠标控制相机角度
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                exit(0);
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                float xoffset = event.motion.xrel;
                float yoffset = -event.motion.yrel;

                float sensitivity = 0.1;
                xoffset *= sensitivity;
                yoffset *= sensitivity;

                camera->yaw += xoffset;
                camera->pitch += yoffset;

                if (camera->pitch > 89.0f)
                {
                    camera->pitch = 89.0f;
                }
                if (camera->pitch < -89.0f)
                {
                    camera->pitch = -89.0f;
                }
            }
        }

        // 键盘控制相机移动
        MoveCamera(camera);

        // 点光源随时间旋转
        MoveLight();

        // 绘制地面和植物
        Render(width, height, ground, ground_model_mat, NULL, camera, NULL, frame_buffer, z_buffer, ground_Mat, lights);
        Render(width, height, plant, plant_model_mat1, texture, camera, normal_map, frame_buffer, z_buffer, plant_Mat, lights);

        // 将绘制的内容显示到窗口上
        SDL_UpdateTexture(texture_buffer, NULL, frame_buffer, width * sizeof(Uint32));
        SDL_RenderTexture(renderer, texture_buffer, NULL, NULL);
        SDL_RenderPresent(renderer);

        // 计算帧间隔时间
        current_time = SDL_GetTicks();
        deltaTime = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // 计算帧数
        CalculateFPS();
    }

    // 销毁渲染器与窗口
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

SDL_Surface* LoadTexture(const char* filename)
{
    // 加载图片
    SDL_Surface* loadedSurface = IMG_Load(filename);
    if (!loadedSurface)
    {
        SDL_Log("无法加载图片 %s: %s", filename, SDL_GetError());
        return nullptr;
    }

    // 将图片转成 RGBA32 格式方便读取像素
    SDL_Surface* optimizedSurface = SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_RGBA32);

    // 释放原本表面
    SDL_DestroySurface(loadedSurface);

    return optimizedSurface;
}

void MoveCamera(Camera* camera)
{
    Vec3 front;
    front.x = cos(to_radians(camera->yaw)) * cos(to_radians(camera->pitch));
    front.y = sin(to_radians(camera->pitch));
    front.z = sin(to_radians(camera->yaw)) * cos(to_radians(camera->pitch));
    front = normalize(front);

    Vec3 worldUp(0, 1, 0);
    Vec3 right = normalize(cross(front, worldUp));
    Vec3 up = normalize(cross(right, worldUp));

    const bool* state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_W]) camera->position += front * camera->speed * deltaTime; // 前进
    if (state[SDL_SCANCODE_S]) camera->position -= front * camera->speed * deltaTime; // 后退
    if (state[SDL_SCANCODE_A]) camera->position -= right * camera->speed * deltaTime; // 左移
    if (state[SDL_SCANCODE_D]) camera->position += right * camera->speed * deltaTime; // 右移
    if (state[SDL_SCANCODE_Q]) camera->position.y += camera->speed * deltaTime;       // 上升
    if (state[SDL_SCANCODE_E]) camera->position.y -= camera->speed * deltaTime;       // 下降

    camera->target = camera->position + front;
}

void MoveLight()
{
    float time = SDL_GetTicks() / 1000.0f;
    lights[1].position = Vec3(sin(time) * 2.0f, 1.5f + cos(time * 0.5f), cos(time) * 2.0f);
}

void CalculateFPS()
{
    static int frame_count = 0;
    static float last_fps_time = SDL_GetTicks() / 1000.0f;
    frame_count++;
    float current_fps_time = SDL_GetTicks() / 1000.0f;
    if (current_fps_time - last_fps_time >= 1.0f) {
        cout << "FPS: " << frame_count << endl;
        frame_count = 0;
        last_fps_time = current_fps_time;
    }
}