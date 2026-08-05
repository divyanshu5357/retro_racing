#include <ncurses.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <locale.h>

namespace {
enum ColorPairs {
    PAIR_SKY = 1,
    PAIR_GRASS_DARK,
    PAIR_GRASS_LIGHT,
    PAIR_KERB_RED,
    PAIR_ROAD,
    PAIR_CAR,
    PAIR_TYRE,
    PAIR_RIVAL,
    PAIR_HUD_BG,
    PAIR_TEXT_NEON,
    PAIR_HEART
};

constexpr float max_speed = 85.0F;
constexpr float track_length = 2200.0F;
constexpr int max_lives = 3;

struct Console {
    int width{160};  
    int height{50}; 

    Console() {
        setlocale(LC_ALL, ""); // Enables wide-character support for macOS terminal symbols
        initscr();             
        cbreak();              
        noecho();              
        keypad(stdscr, TRUE);  
        nodelay(stdscr, TRUE); 
        curs_set(0);           
        
        getmaxyx(stdscr, height, width);
        
        if (has_colors()) {
            start_color();
            init_pair(PAIR_SKY, COLOR_BLUE, COLOR_BLUE);
            init_pair(PAIR_GRASS_DARK, COLOR_GREEN, COLOR_BLACK);
            init_pair(PAIR_GRASS_LIGHT, COLOR_GREEN, COLOR_GREEN); 
            init_pair(PAIR_KERB_RED, COLOR_RED, COLOR_RED);
            init_pair(PAIR_ROAD, COLOR_WHITE, COLOR_WHITE); 
            init_pair(PAIR_CAR, COLOR_YELLOW, COLOR_BLACK);
            init_pair(PAIR_TYRE, COLOR_WHITE, COLOR_BLACK);
            init_pair(PAIR_RIVAL, COLOR_RED, COLOR_WHITE);
            init_pair(PAIR_HUD_BG, COLOR_WHITE, COLOR_BLACK);
            init_pair(PAIR_TEXT_NEON, COLOR_CYAN, COLOR_BLACK);
            init_pair(PAIR_HEART, COLOR_RED, COLOR_BLACK);
        }
    }

    ~Console() { endwin(); }

    void pixel(int x, int y, char symbol, int color_pair, bool bold = false) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        attron(COLOR_PAIR(color_pair));
        if (bold) attron(A_BOLD);
        mvaddch(y, x, symbol);
        if (bold) attroff(A_BOLD);
        attroff(COLOR_PAIR(color_pair));
    }

    // Transparent text rendering: Space characters ' ' are ignored so background shines through!
    void transparent_text(int x, int y, const std::string& value, int color_pair, bool bold = false) {
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (value[i] != ' ') {
                pixel(x + static_cast<int>(i), y, value[i], color_pair, bold);
            }
        }
    }

    void text(int x, int y, const std::string& value, int color_pair, bool bold = false) {
        for (std::size_t i = 0; i < value.size(); ++i) {
            pixel(x + static_cast<int>(i), y, value[i], color_pair, bold);
        }
    }

    void present() { refresh(); }
};

struct InputState {
    float up_timer = 0.0f;
    float down_timer = 0.0f;
    float left_timer = 0.0f;
    float right_timer = 0.0f;
    bool escape = false;
};

void update_input(InputState& input, float delta) {
    int ch = getch();
    
    if (input.up_timer > 0.0f) input.up_timer -= delta;
    if (input.down_timer > 0.0f) input.down_timer -= delta;
    if (input.left_timer > 0.0f) input.left_timer -= delta;
    if (input.right_timer > 0.0f) input.right_timer -= delta;

    while (ch != ERR) {
        switch (ch) {
            case KEY_UP:    case 'w': case 'W': input.up_timer = 0.12f;    break;
            case KEY_DOWN:  case 's': case 'S': input.down_timer = 0.12f;  break;
            case KEY_LEFT:  case 'a': case 'A': input.left_timer = 0.12f;  break;
            case KEY_RIGHT: case 'd': case 'D': input.right_timer = 0.12f; break;
            case 27: input.escape = true; break;
        }
        ch = getch(); 
    }
}

float clamp(float value, float low, float high) { return std::max(low, std::min(value, high)); }

void fill(Console& console, int y, int color_pair, bool bold = false) {
    for (int x = 0; x < console.width; ++x) console.pixel(x, y, ' ', color_pair, bold);
}

// Fixed: Aligned character grid + transparent drawing rules
void draw_car(Console& console, float player_x, bool flashing) {
    if (flashing && (std::chrono::system_clock::now().time_since_epoch().count() / 100000000) % 2 == 0) {
        return; 
    }
    
    const int x = static_cast<int>((0.5F + player_x * 0.34F) * static_cast<float>(console.width));
    const int car_y = console.height - 6; 
    
    // Grid alignment fixed by building a uniform width matrix bounding box
    console.transparent_text(x - 8, car_y,     "   ||#####||     ", PAIR_CAR, true);
    console.transparent_text(x - 8, car_y + 1, "      #####      ", PAIR_CAR, true);
    console.transparent_text(x - 8, car_y + 2, "      #####      ", PAIR_CAR, true);
    console.transparent_text(x - 8, car_y + 3, "||| ######### |||", PAIR_CAR, true);
    console.transparent_text(x - 8, car_y + 4, "||| ######### |||", PAIR_TYRE, false);
}
} 

int main() {
    Console console;
    InputState input;
    
    float distance = 0.0F;
    float speed = 0.0F;
    float player_x = 0.0F;
    float elapsed = 0.0F;
    int lives = max_lives;
    float invulnerability_timer = 0.0F;
    
    auto previous = std::chrono::steady_clock::now();

    while (!input.escape && lives > 0) {
        const auto now = std::chrono::steady_clock::now();
        const float delta = std::chrono::duration<float>(now - previous).count();
        previous = now;
        elapsed += delta;

        update_input(input, delta);

        if (invulnerability_timer > 0.0F) {
            invulnerability_timer -= delta;
        }

        
        if (input.up_timer > 0.0f) {
            speed += 45.0F * delta; 
        } else {
            speed -= 18.0F * delta; 
        }

        if (input.down_timer > 0.0f) speed -= 75.0F * delta;
        
        speed = clamp(speed, 0.0F, max_speed);
        
        if (speed > 0.0F) {
            distance = std::fmod(distance + speed * delta * 15.0F, track_length); 
        }

        const float steer = (input.left_timer > 0.0f ? -1.0F : 0.0F) + (input.right_timer > 0.0f ? 1.0F : 0.0f);
        const float curve = std::sin(distance * 0.004F) * 0.75F + std::sin(distance * 0.011F) * 0.30F;
        
        if (speed > 1.0F) {
            player_x += steer * delta * (0.75F + (speed / max_speed));
            player_x -= curve * speed * delta * 0.012F;
        }
        player_x = clamp(player_x, -1.15F, 1.15F);
        
        if (std::abs(player_x) > 0.92F) {
            speed = std::max(0.0F, speed - 42.0F * delta);
            if (std::abs(player_x) > 1.05F && invulnerability_timer <= 0.0F && speed > 10.0F) {
                lives--;
                invulnerability_timer = 1.4F; 
                speed *= 0.15F; 
            }
        }

        clear();
        const int horizon = console.height * 0.40f;

        // 1. Render Sky
        for (int y = 0; y < horizon; ++y) fill(console, y, PAIR_SKY, true);
        
        // 2. Render Road
        for (int y = horizon; y < console.height; ++y) {
            const float perspective = static_cast<float>(y - horizon) / static_cast<float>(console.height - horizon);
            const float road_half = 0.06F + perspective * 0.42F;
            const float bend = curve * (1.0F - perspective) * (1.0F - perspective) * 0.42F;
            const float centre = 0.5F + bend;
            const float left_road = centre - road_half;
            const float right_road = centre + road_half;
            const float kerb = 0.025F + perspective * 0.018F;
            const bool stripe = (static_cast<int>(distance * 0.22F + 1.0F / (perspective + 0.03F)) / 3) % 2 == 0;
            
            for (int x = 0; x < console.width; ++x) {
                const float px = static_cast<float>(x) / static_cast<float>(console.width);
                int color = stripe ? PAIR_GRASS_LIGHT : PAIR_GRASS_DARK;
                bool bold = (color == PAIR_GRASS_LIGHT);
                
                if (px >= left_road - kerb && px < left_road) {
                    color = stripe ? PAIR_KERB_RED : PAIR_ROAD;
                    bold = false;
                } else if (px > right_road && px <= right_road + kerb) {
                    color = stripe ? PAIR_KERB_RED : PAIR_ROAD;
                    bold = false;
                } else if (px >= left_road && px <= right_road) {
                    color = PAIR_ROAD;
                    bold = false;
                }
                console.pixel(x, y, ' ', color, bold);
            }
        }


        const float rival_phase = std::fmod(elapsed * 0.15F, 1.0F);
        const int rival_y = horizon + 2 + static_cast<int>(rival_phase * (console.height * 0.40f));
        const float rival_perspective = static_cast<float>(rival_y - horizon) / static_cast<float>(console.height - horizon);
        const float rival_bend = curve * (1.0F - rival_perspective) * (1.0F - rival_perspective) * 0.42F;
        const int rival_x = static_cast<int>((0.5F + rival_bend) * console.width);
        
        if (rival_y < console.height - 8) {
            console.text(rival_x - 1, rival_y, "[=]", PAIR_RIVAL, true);
        }
        

        draw_car(console, player_x, invulnerability_timer > 0.0F);


        for (int h_y = 1; h_y <= 4; ++h_y) {
            for (int h_x = 2; h_x < console.width - 2; ++h_x) {
                console.pixel(h_x, h_y, ' ', PAIR_HUD_BG);
            }
        }
        
        char hud_stats[120]{};
        std::snprintf(hud_stats, sizeof(hud_stats), "⚙ SPEED: %03d km/h   🏁 DISTANCE: %04d / %04d m", 
                     static_cast<int>(speed * 2.4F), static_cast<int>(distance), static_cast<int>(track_length));
        console.text(5, 2, hud_stats, PAIR_TEXT_NEON, true);

        console.text(console.width - 30, 2, "LIVES: ", PAIR_HUD_BG, true);
        for(int l = 0; l < max_lives; l++) {
            if(l < lives) {
                console.text(console.width - 23 + (l * 2), 2, "HP", PAIR_HEART, true); // Cross-platform safe health tracking indicator
            } else {
                console.text(console.width - 23 + (l * 2), 2, "..", PAIR_HUD_BG, false);
            }
        }

        console.text(5, 3, "PROGRESS: [", PAIR_HUD_BG, false);
        int bar_length = console.width - 32;
        float progress_pct = distance / track_length;
        int filled_chars = static_cast<int>(progress_pct * bar_length);
        for (int p = 0; p < bar_length; ++p) {
            if (p < filled_chars) console.pixel(16 + p, 3, '=', PAIR_TEXT_NEON, true);
            else console.pixel(16 + p, 3, '-', PAIR_HUD_BG, false);
        }
        console.text(16 + bar_length, 3, "]", PAIR_HUD_BG, false);

        console.present();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (lives <= 0) {
        clear();
        int center_y = console.height / 2;
        int center_x = (console.width / 2) - 10;
        console.text(center_x, center_y,     "####################", PAIR_HEART, true);
        console.text(center_x, center_y + 1, "#    GAME OVER     #", PAIR_HEART, true);
        console.text(center_x, center_y + 2, "#   CRASHED OUT    #", PAIR_HEART, true);
        console.text(center_x, center_y + 3, "####################", PAIR_HEART, true);
        console.present();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}