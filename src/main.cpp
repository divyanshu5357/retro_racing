#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
constexpr int width = 160;
constexpr int height = 50;
constexpr int horizon = 17;
constexpr float max_speed = 85.0F;
constexpr float track_length = 2200.0F;

struct Console {
    HANDLE handle{CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr)};
    std::vector<CHAR_INFO> buffer{static_cast<std::size_t>(width * height)};

    Console() {
        SetConsoleActiveScreenBuffer(handle);
        SetConsoleTitleW(L"Retro Racer - C++ Pseudo 3D");
        const COORD size{static_cast<SHORT>(width), static_cast<SHORT>(height)};
        SetConsoleScreenBufferSize(handle, size);
        SMALL_RECT area{0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        SetConsoleWindowInfo(handle, TRUE, &area);
        CONSOLE_CURSOR_INFO cursor{1, FALSE};
        SetConsoleCursorInfo(handle, &cursor);
    }

    ~Console() { CloseHandle(handle); }

    void pixel(int x, int y, wchar_t symbol, WORD color) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        auto& cell = buffer[static_cast<std::size_t>(y * width + x)];
        cell.Char.UnicodeChar = symbol;
        cell.Attributes = color;
    }

    void text(int x, int y, const std::wstring& value, WORD color) {
        for (std::size_t i = 0; i < value.size(); ++i) pixel(x + static_cast<int>(i), y, value[i], color);
    }

    void present() {
        const COORD size{static_cast<SHORT>(width), static_cast<SHORT>(height)};
        const COORD origin{0, 0};
        SMALL_RECT target{0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        WriteConsoleOutputW(handle, buffer.data(), size, origin, &target);
    }
};

bool down(int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; }
float clamp(float value, float low, float high) { return std::max(low, std::min(value, high)); }

void fill(Console& console, int y, WORD color) {
    for (int x = 0; x < width; ++x) console.pixel(x, y, L' ', color);
}

void draw_car(Console& console, float player_x) {
    const int x = static_cast<int>((0.5F + player_x * 0.34F) * static_cast<float>(width));
    constexpr WORD body = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD tyre = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    console.text(x - 7, 42, L"  _________  ", body);
    console.text(x - 9, 43, L" /|       |\\ ", body);
    console.text(x - 10, 44, L"/ |__O__|   ", body);
    console.text(x - 11, 45, L"|  _   _    |", body);
    console.text(x - 11, 46, L"O_/     \\_O", tyre);
}
} // namespace

int main() {
    Console console;
    float distance = 0.0F;
    float speed = 0.0F;
    float player_x = 0.0F;
    float elapsed = 0.0F;
    auto previous = std::chrono::steady_clock::now();

    while (!down(VK_ESCAPE)) {
        const auto now = std::chrono::steady_clock::now();
        const float delta = std::chrono::duration<float>(now - previous).count();
        previous = now;
        elapsed += delta;

        const bool accelerate = down(VK_UP) || down(L'W');
        const bool brake = down(VK_DOWN) || down(L'S');
        const float steer = (down(VK_LEFT) || down(L'A') ? -1.0F : 0.0F) +
                            (down(VK_RIGHT) || down(L'D') ? 1.0F : 0.0F);
        speed += (accelerate ? 42.0F : -12.0F) * delta;
        if (brake) speed -= 70.0F * delta;
        speed = clamp(speed, 0.0F, max_speed);
        distance = std::fmod(distance + speed * delta, track_length);

        const float curve = std::sin(distance * 0.004F) * 0.75F + std::sin(distance * 0.011F) * 0.30F;
        player_x += steer * delta * (0.62F + speed / max_speed);
        player_x -= curve * speed * delta * 0.010F;
        player_x = clamp(player_x, -1.15F, 1.15F);
        if (std::abs(player_x) > 0.92F) speed = std::max(0.0F, speed - 38.0F * delta);

        for (int y = 0; y < horizon; ++y) fill(console, y, BACKGROUND_BLUE | BACKGROUND_INTENSITY);
        for (int y = horizon; y < height; ++y) {
            const float perspective = static_cast<float>(y - horizon) / static_cast<float>(height - horizon);
            const float road_half = 0.06F + perspective * 0.48F;
            const float bend = curve * (1.0F - perspective) * (1.0F - perspective) * 0.42F;
            const float centre = 0.5F + bend;
            const float left_road = centre - road_half;
            const float right_road = centre + road_half;
            const float kerb = 0.028F + perspective * 0.018F;
            const bool stripe = (static_cast<int>(distance * 0.18F + 1.0F / (perspective + 0.03F)) / 3) % 2 == 0;
            for (int x = 0; x < width; ++x) {
                const float px = static_cast<float>(x) / static_cast<float>(width);
                WORD color = (stripe ? BACKGROUND_GREEN : BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                if (px >= left_road - kerb && px < left_road) color = stripe ? BACKGROUND_RED : BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
                else if (px > right_road && px <= right_road + kerb) color = stripe ? BACKGROUND_RED : BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
                else if (px >= left_road && px <= right_road) color = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
                console.pixel(x, y, L' ', color);
            }
        }

        // A distant rival helps show motion and perspective.
        const float rival_phase = std::fmod(elapsed * 0.15F, 1.0F);
        const int rival_y = horizon + 4 + static_cast<int>(rival_phase * 22.0F);
        const int rival_x = width / 2 + static_cast<int>(curve * 18.0F);
        console.text(rival_x - 2, rival_y, L"[=]", FOREGROUND_RED | FOREGROUND_INTENSITY);
        draw_car(console, player_x);

        wchar_t hud[100]{};
        std::swprintf(hud, 100, L"RETRO RACER     SPEED %03d km/h     DISTANCE %04d m", static_cast<int>(speed * 2.4F), static_cast<int>(distance));
        console.text(3, 2, hud, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE);
        console.text(3, 4, L"W / UP: ACCELERATE   S / DOWN: BRAKE   A,D / ARROWS: STEER   ESC: QUIT", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_BLUE);
        console.present();
        Sleep(16);
    }
    return 0;
}
