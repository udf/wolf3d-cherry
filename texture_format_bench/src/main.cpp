#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <cstring>

#include <SDL2/SDL.h>

std::unordered_map<uint32_t, std::string> get_fmt_names() {
    std::unordered_map<uint32_t, std::string> names;

    names[SDL_PIXELFORMAT_UNKNOWN] += "SDL_PIXELFORMAT_UNKNOWN ";
    names[SDL_PIXELFORMAT_INDEX1LSB] += "SDL_PIXELFORMAT_INDEX1LSB ";
    names[SDL_PIXELFORMAT_INDEX1MSB] += "SDL_PIXELFORMAT_INDEX1MSB ";
    names[SDL_PIXELFORMAT_INDEX4LSB] += "SDL_PIXELFORMAT_INDEX4LSB ";
    names[SDL_PIXELFORMAT_INDEX4MSB] += "SDL_PIXELFORMAT_INDEX4MSB ";
    names[SDL_PIXELFORMAT_INDEX8] += "SDL_PIXELFORMAT_INDEX8 ";
    names[SDL_PIXELFORMAT_RGB332] += "SDL_PIXELFORMAT_RGB332 ";
    names[SDL_PIXELFORMAT_RGB444] += "SDL_PIXELFORMAT_RGB444 ";
    names[SDL_PIXELFORMAT_RGB555] += "SDL_PIXELFORMAT_RGB555 ";
    names[SDL_PIXELFORMAT_BGR555] += "SDL_PIXELFORMAT_BGR555 ";
    names[SDL_PIXELFORMAT_ARGB4444] += "SDL_PIXELFORMAT_ARGB4444 ";
    names[SDL_PIXELFORMAT_RGBA4444] += "SDL_PIXELFORMAT_RGBA4444 ";
    names[SDL_PIXELFORMAT_ABGR4444] += "SDL_PIXELFORMAT_ABGR4444 ";
    names[SDL_PIXELFORMAT_BGRA4444] += "SDL_PIXELFORMAT_BGRA4444 ";
    names[SDL_PIXELFORMAT_ARGB1555] += "SDL_PIXELFORMAT_ARGB1555 ";
    names[SDL_PIXELFORMAT_RGBA5551] += "SDL_PIXELFORMAT_RGBA5551 ";
    names[SDL_PIXELFORMAT_ABGR1555] += "SDL_PIXELFORMAT_ABGR1555 ";
    names[SDL_PIXELFORMAT_BGRA5551] += "SDL_PIXELFORMAT_BGRA5551 ";
    names[SDL_PIXELFORMAT_RGB565] += "SDL_PIXELFORMAT_RGB565 ";
    names[SDL_PIXELFORMAT_BGR565] += "SDL_PIXELFORMAT_BGR565 ";
    names[SDL_PIXELFORMAT_RGB24] += "SDL_PIXELFORMAT_RGB24 ";
    names[SDL_PIXELFORMAT_BGR24] += "SDL_PIXELFORMAT_BGR24 ";
    names[SDL_PIXELFORMAT_RGB888] += "SDL_PIXELFORMAT_RGB888 ";
    names[SDL_PIXELFORMAT_RGBX8888] += "SDL_PIXELFORMAT_RGBX8888 ";
    names[SDL_PIXELFORMAT_BGR888] += "SDL_PIXELFORMAT_BGR888 ";
    names[SDL_PIXELFORMAT_BGRX8888] += "SDL_PIXELFORMAT_BGRX8888 ";
    names[SDL_PIXELFORMAT_ARGB8888] += "SDL_PIXELFORMAT_ARGB8888 ";
    names[SDL_PIXELFORMAT_RGBA8888] += "SDL_PIXELFORMAT_RGBA8888 ";
    names[SDL_PIXELFORMAT_ABGR8888] += "SDL_PIXELFORMAT_ABGR8888 ";
    names[SDL_PIXELFORMAT_BGRA8888] += "SDL_PIXELFORMAT_BGRA8888 ";
    names[SDL_PIXELFORMAT_ARGB2101010] += "SDL_PIXELFORMAT_ARGB2101010 ";
    names[SDL_PIXELFORMAT_RGBA32] += "SDL_PIXELFORMAT_RGBA32 ";
    names[SDL_PIXELFORMAT_ARGB32] += "SDL_PIXELFORMAT_ARGB32 ";
    names[SDL_PIXELFORMAT_BGRA32] += "SDL_PIXELFORMAT_BGRA32 ";
    names[SDL_PIXELFORMAT_ABGR32] += "SDL_PIXELFORMAT_ABGR32 ";
    names[SDL_PIXELFORMAT_YV12] += "SDL_PIXELFORMAT_YV12 ";
    names[SDL_PIXELFORMAT_IYUV] += "SDL_PIXELFORMAT_IYUV ";
    names[SDL_PIXELFORMAT_YUY2] += "SDL_PIXELFORMAT_YUY2 ";
    names[SDL_PIXELFORMAT_UYVY] += "SDL_PIXELFORMAT_UYVY ";
    names[SDL_PIXELFORMAT_YVYU] += "SDL_PIXELFORMAT_YVYU ";
    names[SDL_PIXELFORMAT_NV12] += "SDL_PIXELFORMAT_NV12 ";
    names[SDL_PIXELFORMAT_NV21] += "SDL_PIXELFORMAT_NV21 ";

    return names;
}

struct Pixel {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

std::unordered_map<uint32_t, std::string> fmt_names = get_fmt_names();

struct H {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *buffer = nullptr;
    SDL_PixelFormat *buffer_fmt = nullptr;
    std::unique_ptr<Pixel[]> test_data;

    size_t width = 1366;
    size_t height = 768;

    void h() {
        if (SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error("Failed to initialize SDL video subsystem");

        window = SDL_CreateWindow(
            "SDL texture format benchmark",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(width),
            static_cast<int>(height),
            SDL_WINDOW_SHOWN
        );
        if (!window)
            throw std::runtime_error("Failed to create SDL window");

        renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED// | SDL_RENDERER_PRESENTVSYNC
        );
        if (!renderer)
            throw std::runtime_error("Failed to create renderer");

        // create test data
        test_data = std::make_unique<Pixel[]>(width * height);
        auto data = test_data.get();
        for (size_t x = 0; x < width; x++) {
            for (size_t y = 0; y < height; y++) {
                auto &p = data[y * width + x];
                if ((x % 2 == 0) ^ (y % 2 == 0)) {
                    p = {0, 0, 0, 0};
                } else {
                    p = {0, 255, 255, 255};
                }
            }
        }

        // print supported formats
        std::cout << "Supported texture formats:" << std::endl;
        SDL_RendererInfo info;
        if (SDL_GetRendererInfo(renderer, &info) < 0)
            throw std::runtime_error("Failed to get renderer info");
        for (size_t i = 0; i < info.num_texture_formats; i++) {
            std::cout << fmt_names[info.texture_formats[i]] << std::endl;
        }

        // test each format
        std::cout << "Testing formats..." << std::endl;
        for (size_t i = 0; i < info.num_texture_formats; i++) {
            auto format = info.texture_formats[i];
            std::cout << fmt_names[format];
            test_format(format);
            std::cout << std::endl;
        }
    }

    void test_format(uint32_t format) {
        buffer_fmt = SDL_AllocFormat(format);
        if (!buffer_fmt) {
            std::cout << "Failed to get buffer format struct: " << SDL_GetError();
            return;
        }

        if (buffer_fmt->BytesPerPixel != 4) {
            std::cout << "oh no this texure has a bpp of " << (int)buffer_fmt->BytesPerPixel << std::endl;
            throw std::runtime_error("pls implement render_frame for non 4-bpp textures");
        }

        buffer = SDL_CreateTexture(
            renderer,
            format,
            SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(width),
            static_cast<int>(height)
        );

        uint32_t start = SDL_GetTicks();
        const int32_t time = 5000;
        float num_frames = 0;
        while (SDL_GetTicks() - start <= time) {
            render_frame();
            num_frames++;
        }
        float fps = num_frames / (static_cast<float>(time) / 1000.f);
        std::cout << fps << " FPS";
        std::cout << " (" << num_frames << " frames)";

        SDL_FreeFormat(buffer_fmt);
        SDL_DestroyTexture(buffer);
    }

    void render_frame() {
        uint32_t *pixels;
        int pitch;

        SDL_LockTexture(
            buffer,
            NULL,
            reinterpret_cast<void **>(&pixels),
            &pitch
        );

        std::memset(
            static_cast<void *>(pixels),
            0,
            sizeof(uint32_t) * width * height
        );

        // auto data = test_data.get();
        // for (size_t x = 0; x < width; x++) {
        //     for (size_t y = 0; y < height; y++) {
        //         const auto i = y * width + x;
        //         auto p = data[i];
        //         pixels[i] = SDL_MapRGB(buffer_fmt, p.r, p.g, p.b);
        //     }
        // }

        SDL_UnlockTexture(buffer);
        SDL_RenderCopy(renderer, buffer, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    ~H() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main() {
    try {
        H h;
        h.h();
    } catch (std::exception &e) {
        std::cerr << "SDL error: " << SDL_GetError() << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}