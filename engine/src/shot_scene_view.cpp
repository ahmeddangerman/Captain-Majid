#include "captain/shot_scene.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace {
using Result = captain::CompletePreparedShotResult;
constexpr SDL_Color white{234, 240, 233, 255}, gold{242, 202, 105, 255};
constexpr SDL_Color blue{99, 181, 255, 255}, red{229, 108, 99, 255};

std::array<unsigned, 7> glyph(char character) {
    // Simple original placeholder lettering; no game font/art is included.
    static constexpr std::array<std::array<unsigned, 7>, 36> shapes{{
        {{14,17,19,21,25,17,14}}, {{4,12,4,4,4,4,14}}, {{14,17,1,2,4,8,31}},
        {{30,1,1,14,1,1,30}}, {{2,6,10,18,31,2,2}}, {{31,16,16,30,1,1,30}},
        {{14,16,16,30,17,17,14}}, {{31,1,2,4,8,8,8}}, {{14,17,17,14,17,17,14}},
        {{14,17,17,15,1,1,14}}, {{14,17,17,31,17,17,17}}, {{30,17,17,30,17,17,30}},
        {{14,17,16,16,16,17,14}}, {{30,17,17,17,17,17,30}}, {{31,16,16,30,16,16,31}},
        {{31,16,16,30,16,16,16}}, {{14,17,16,23,17,17,15}}, {{17,17,17,31,17,17,17}},
        {{14,4,4,4,4,4,14}}, {{7,2,2,2,2,18,12}}, {{17,18,20,24,20,18,17}},
        {{16,16,16,16,16,16,31}}, {{17,27,21,21,17,17,17}}, {{17,25,25,21,19,19,17}},
        {{14,17,17,17,17,17,14}}, {{30,17,17,30,16,16,16}}, {{14,17,17,17,21,18,13}},
        {{30,17,17,30,20,18,17}}, {{15,16,16,14,1,1,30}}, {{31,4,4,4,4,4,4}},
        {{17,17,17,17,17,17,14}}, {{17,17,17,17,17,10,4}}, {{17,17,17,21,21,27,17}},
        {{17,17,10,4,10,17,17}}, {{17,17,10,4,4,4,4}}, {{31,1,2,4,8,16,31}}
    }};
    const auto c = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    if (c >= '0' && c <= '9') return shapes[static_cast<unsigned>(c - '0')];
    if (c >= 'A' && c <= 'Z') return shapes[10 + static_cast<unsigned>(c - 'A')];
    if (c == ':') return {0,4,4,0,4,4,0};
    if (c == '-') return {0,0,0,31,0,0,0};
    if (c == '/') return {1,2,2,4,8,8,16};
    if (c == '.') return {0,0,0,0,0,4,4};
    return {};
}
void fill(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const SDL_Rect rectangle{x, y, width, height};
    SDL_RenderFillRect(renderer, &rectangle);
}
void box(SDL_Renderer* renderer, int x, int y, int width, int height) {
    fill(renderer, x, y, width, height, white);
    fill(renderer, x + 2, y + 2, width - 4, height - 4, {9, 17, 25, 255});
}
void text(SDL_Renderer* renderer, const std::string& value, int x, int y,
          int scale = 2, SDL_Color color = white) {
    for (const char c : value) {
        const auto rows = glyph(c);
        for (int row = 0; row < 7; ++row)
            for (int column = 0; column < 5; ++column)
                if (rows[row] & (1u << (4 - column)))
                    fill(renderer, x + column * scale, y + row * scale, scale, scale, color);
        x += 6 * scale;
    }
}
void center(SDL_Renderer* renderer, const std::string& value, int y, int scale, SDL_Color color) {
    text(renderer, value, 320 - static_cast<int>(value.size()) * 3 * scale, y, scale, color);
}
void player(SDL_Renderer* renderer, int x, int y, SDL_Color jersey) {
    fill(renderer, x - 7, y - 15, 14, 10, {241, 188, 139, 255});
    fill(renderer, x - 7, y - 17, 14, 4, {19, 23, 30, 255});
    fill(renderer, x - 10, y - 5, 20, 15, jersey);
    fill(renderer, x - 8, y + 10, 6, 8, white);
    fill(renderer, x + 2, y + 10, 6, 8, white);
    fill(renderer, x - 10, y + 17, 8, 4, {19, 23, 30, 255});
    fill(renderer, x + 2, y + 17, 8, 4, {19, 23, 30, 255});
}
const char* event_name(captain::ShotEvent event) {
    switch (event) {
    case captain::ShotEvent::goal: return "GOAL";
    case captain::ShotEvent::special_turnover: return "TURNOVER";
    case captain::ShotEvent::rebound: return "REBOUND";
    case captain::ShotEvent::keeper_catch: return "KEEPER CATCH";
    case captain::ShotEvent::keeper_punch: return "KEEPER PUNCH";
    }
    return "UNKNOWN";
}
void draw(SDL_Renderer* renderer, const captain::ShotSceneOptions& options,
          const std::optional<Result>& result, double progress, const std::string& error) {
    SDL_SetRenderDrawColor(renderer, 9, 17, 25, 255);
    SDL_RenderClear(renderer);
    center(renderer, "CAPTAIN MAJID", 18, 3, white);
    center(renderer, "NATIVE DEVELOPMENT SCENE", 51, 1, gold);
    fill(renderer, 24, 80, 592, 212, white);
    fill(renderer, 26, 82, 588, 208, {25, 74, 58, 255});
    for (int x = 26; x < 614; x += 84)
        fill(renderer, x, 82, 42, 208, {29, 82, 62, 255});
    fill(renderer, 319, 82, 2, 208, white);
    for (int y = -26; y <= 26; ++y) {
        for (int x = -26; x <= 26; ++x) {
            const auto radius = x * x + y * y;
            if (radius >= 24 * 24 && radius <= 26 * 26)
                fill(renderer, 320 + x, 186 + y, 1, 1, white);
        }
    }
    fill(renderer, 536, 120, 2, 132, white);
    fill(renderer, 536, 120, 78, 2, white);
    fill(renderer, 536, 250, 78, 2, white);
    fill(renderer, 601, 151, 13, 70, white);
    fill(renderer, 603, 153, 9, 66, {50, 68, 72, 255});
    player(renderer, 220, 184, blue);
    for (unsigned i = 0; i < options.defenders; ++i)
        player(renderer, 360 + static_cast<int>(i) * 28,
               i % 2 ? 210 : 162, red);
    player(renderer, 565, 184, gold);
    int target_x = 565, target_y = 198;
    const captain::ShotOutcomeResult* outcome = nullptr;
    if (result && result->flow.continuation && result->flow.continuation->outcome)
        outcome = &*result->flow.continuation->outcome;
    if (outcome) {
        if (outcome->event == captain::ShotEvent::goal) target_x = 606;
        else if (outcome->event == captain::ShotEvent::keeper_punch) { target_x = 440; target_y = 130; }
        else if (outcome->event == captain::ShotEvent::rebound) { target_x = 410; target_y = 224; }
    } else if (result) target_x = 370;
    const int ball_x = result ? 235 + static_cast<int>((target_x - 235) * progress) : 235;
    const int ball_y = result ? 200 + static_cast<int>((target_y - 200) * progress) : 200;
    fill(renderer, ball_x - 4, ball_y - 4, 8, 8, white);
    fill(renderer, ball_x - 1, ball_y - 1, 3, 3, {19, 23, 30, 255});
    box(renderer, 24, 310, 180, 110);
    box(renderer, 220, 310, 396, 110);
    text(renderer, "ATTACK " + std::to_string(options.level), 36, 326, 2, blue);
    text(renderer, "KEEPER " + std::to_string(options.keeper_level), 36, 356, 2, gold);
    text(renderer, "DEFEND " + std::to_string(options.defenders), 36, 386);
    if (!error.empty()) {
        text(renderer, "SCENE REQUEST REJECTED", 234, 326, 2, red);
        text(renderer, error.substr(0, 59), 234, 362, 1, white);
    } else if (result) {
        text(renderer, outcome ? event_name(outcome->event) : "FIELD HANDOFF", 234, 326, 2, gold);
        text(renderer, "POWER " + std::to_string(result->attack.action_power.power) +
             "  KEEPER " + (result->keeper ? std::to_string(result->keeper->comparison.strength) : "NONE"),
             234, 356, 2);
        if (outcome)
            text(renderer, "SCORE " + std::to_string(outcome->left_goals) + ":" +
                 std::to_string(outcome->right_goals), 234, 386);
    } else {
        text(renderer, "SPACE TO SHOOT", 234, 326, 2, gold);
        text(renderer, "D  CHANGE DEFENDERS", 234, 356);
        text(renderer, "R  CLEAR RESULT", 234, 386);
    }
    center(renderer, "UP/DOWN ATTACK  LEFT/RIGHT KEEPER  ESC EXIT", 436, 1, white);
    center(renderer, "PLACEHOLDER ART - EACH SHOT RESETS", 458, 1, gold);
    SDL_RenderPresent(renderer);
}
void sdl_error(const std::string& operation) {
    throw std::runtime_error(operation + ": " + SDL_GetError());
}
struct Shutdown { ~Shutdown() { SDL_Quit(); } };
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: captain-shot-scene PACK [--shoot] [--capture NEW.bmp]\n";
        return 2;
    }
    try {
        std::string capture;
        bool initial_shot = false;
        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--shoot") initial_shot = true;
            else if (argument == "--capture" && i + 1 < argc) capture = argv[++i];
            else throw std::invalid_argument("Unknown or incomplete option");
        }
        if (!capture.empty() && (std::filesystem::path(capture).extension() != ".bmp" ||
                                 std::filesystem::exists(capture)))
            throw std::invalid_argument("Capture requires a new .bmp output path");
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) throw std::invalid_argument("Unable to open local content pack");
        const auto content = captain::load_shot_content(input);
        captain::ShotSceneOptions options;
        std::optional<Result> result;
        if (initial_shot) result = captain::calculate_shot_scene(content, options);
        SDL_SetMainReady();
        Shutdown shutdown;
        if (SDL_Init(capture.empty() ? SDL_INIT_VIDEO | SDL_INIT_TIMER : 0) != 0) sdl_error("SDL init");
        if (!capture.empty()) {
            std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
                SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32), SDL_FreeSurface);
            if (!surface) sdl_error("Capture surface");
            std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
                SDL_CreateSoftwareRenderer(surface.get()), SDL_DestroyRenderer);
            if (!renderer) sdl_error("Capture renderer");
            draw(renderer.get(), options, result, 1.0, {});
            auto* output = std::fopen(capture.c_str(), "wbx");
            if (!output) throw std::runtime_error("Unable to create new capture exclusively");
            auto* stream = SDL_RWFromFP(output, SDL_TRUE);
            if (!stream) { std::fclose(output); sdl_error("Capture stream"); }
            if (SDL_SaveBMP_RW(surface.get(), stream, 1) != 0) sdl_error("Save capture");
            std::cout << "Saved development scene to " << capture << '\n';
            return 0;
        }
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("Captain Majid - development shot scene", SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, 960, 720, SDL_WINDOW_RESIZABLE), SDL_DestroyWindow);
        if (!window) sdl_error("Window");
        SDL_SetWindowMinimumSize(window.get(), 640, 480);
        auto* raw_renderer = SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!raw_renderer) raw_renderer = SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE);
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(raw_renderer, SDL_DestroyRenderer);
        if (!renderer) sdl_error("Renderer");
        if (SDL_RenderSetLogicalSize(renderer.get(), 640, 480) != 0) sdl_error("Logical size");
        if (SDL_RenderSetIntegerScale(renderer.get(), SDL_TRUE) != 0) sdl_error("Integer scaling");
        bool running = true;
        std::string error;
        auto shot_started = SDL_GetTicks64();
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
                if (event.type != SDL_KEYDOWN || event.key.repeat) continue;
                const auto key = event.key.keysym.sym;
                bool changed = false;
                if (key == SDLK_ESCAPE) running = false;
                else if (key == SDLK_UP) { options.level = std::min(255u, options.level + 1); changed = true; }
                else if (key == SDLK_DOWN) { if (options.level) --options.level; changed = true; }
                else if (key == SDLK_RIGHT) { options.keeper_level = std::min(255u, options.keeper_level + 1); changed = true; }
                else if (key == SDLK_LEFT) { if (options.keeper_level) --options.keeper_level; changed = true; }
                else if (key == SDLK_d) { options.defenders = (options.defenders + 1) % 6; changed = true; }
                else if (key == SDLK_r) changed = true;
                else if (key == SDLK_SPACE) {
                    result.reset();
                    error.clear();
                    try { result = captain::calculate_shot_scene(content, options); shot_started = SDL_GetTicks64(); }
                    catch (const std::exception& failure) { error = failure.what(); }
                }
                if (changed) { result.reset(); error.clear(); }
            }
            const auto progress = std::min(1.0, (SDL_GetTicks64() - shot_started) / 800.0);
            draw(renderer.get(), options, result, progress, error);
            SDL_Delay(8);
        }
    } catch (const std::exception& error) {
        std::cerr << "Shot scene stopped: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
