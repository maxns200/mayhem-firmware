/*
 * activation_app.cpp
 * PortaPack Mayhem App: "Activation"
 */

#include "activation_app.hpp"

using namespace ui;

// ============================================================
//                       КОНСТРУКТОР
// ============================================================
ActivationView::ActivationView(NavigationView& nav)
    : nav_{nav}
{
    set_style(Theme::getInstance()->bg_darkest);
    add_children({});
    timer_.start();
}

void ActivationView::on_show() {
    focus();
}

void ActivationView::focus() {
    // весь ввод — через on_key()
}

// ============================================================
//              ЭТАП 1: РАСЦВЕТАНИЕ
// ============================================================
void ActivationView::draw_bloom(Painter& painter) {
    auto r = screen_rect();
    painter.fill_rectangle(r, Theme::getInstance()->bg_darkest->background);

    painter.draw_string(
        {6, 6},
        style().font_fixed_8x16,
        "SYSTEM READY",
        Theme::getInstance()->fg_green->foreground);

    painter.draw_string(
        {6, 26},
        style().font_fixed_8x16,
        "TO ACTIVATE",
        Theme::getInstance()->fg_green->foreground);

    int cx = r.width()  / 2;
    int cy = r.height() / 2 + 6;

    float t = stage_tick_ / 80.0f;
    if (t > 1.0f) t = 1.0f;

    int size = 5 + (int)(t * 34.0f);

    // лепестки
    for (int a = 0; a < 10; a++) {
        float ang = a * (2.0f * 3.14159f / 10.0f) + (tick_ * 0.02f);
        int px = cx + (int)(std::cos(ang) * size);
        int py = cy + (int)(std::sin(ang) * size);

        Color c = (a % 2)
            ? Theme::getInstance()->fg_magenta->foreground
            : Theme::getInstance()->fg_yellow->foreground;

        painter.fill_rectangle({px - 2, py - 2, 4, 4}, c);
    }

    // сердцевина
    painter.fill_rectangle(
        {cx - 4, cy - 4, 8, 8},
        Theme::getInstance()->fg_white->foreground);

    // прогресс-бар
    int bar_y = r.height() - 20;
    painter.fill_rectangle(
        {6, bar_y, r.width() - 12, 10},
        Theme::getInstance()->bg_dark->background);
    painter.fill_rectangle(
        {6, bar_y, (int)((r.width() - 12) * t), 10},
        Theme::getInstance()->fg_cyan->foreground);
}

// ============================================================
//              ЭТАП 2: ЖДЁМ КАРТУ
// ============================================================
void ActivationView::draw_wait(Painter& painter) {
    auto r = screen_rect();
    painter.fill_rectangle(r, Theme::getInstance()->bg_darkest->background);

    painter.draw_string(
        {8, 30},
        style().font_fixed_8x16,
        "ATTACH CARD",
        Theme::getInstance()->fg_cyan->foreground);

    if ((tick_ / 10) % 2 == 0) {
        painter.draw_string(
            {8, 70},
            style().font_fixed_8x16,
            "PRESS BUTTON",
            Theme::getInstance()->fg_yellow->foreground);
    }

    painter.draw_string(
        {8, 130},
        style().font_fixed_5x8,
        "Place card, then press OK/Select.",
        Theme::getInstance()->fg_white->foreground);
}

// ============================================================
//              ЭТАП 3: ОТСЧЁТ 10..0
// ============================================================
void ActivationView::draw_countdown(Painter& painter) {
    auto r = screen_rect();
    painter.fill_rectangle(r, Theme::getInstance()->bg_darkest->background);

    painter.draw_string(
        {6, 6},
        style().font_fixed_8x16,
        "ACTIVATION...",
        Theme::getInstance()->fg_red->foreground);

    char buf[4];
    std::snprintf(buf, sizeof(buf), "%d", count_value_);

    Color c = count_value_ > 5
        ? Theme::getInstance()->fg_green->foreground
        : count_value_ > 2
            ? Theme::getInstance()->fg_yellow->foreground
            : Theme::getInstance()->fg_red->foreground;

    int cx = r.width() / 2 - 12;
    int cy = r.height() / 2 - 16;

    // рисуем цифру трижды со смещением — получается жирная
    painter.draw_string({cx,     cy}, style().font_fixed_16x32, buf, c);
    painter.draw_string({cx - 1, cy}, style().font_fixed_16x32, buf, c);
    painter.draw_string({cx + 1, cy}, style().font_fixed_16x32, buf, c);
}

// ============================================================
//         ПОДГОТОВКА ПАДАЮЩИХ БУКВ B O O M
// ============================================================
void ActivationView::init_letters() {
    const char chars[LETTER_COUNT] = {'B', 'O', 'O', 'M'};

    int step = screen_width / (LETTER_COUNT + 1);

    for (int i = 0; i < LETTER_COUNT; i++) {
        letters_[i].ch        = chars[i];
        letters_[i].x         = step * (i + 1) - 8;
        letters_[i].y         = -30 - (i * 15);
        letters_[i].vy        = 4 + i;
        letters_[i].delay     = i * 3;
        letters_[i].landed    = false;
        letters_[i].land_tick = 0;
    }
}

// ============================================================
//         ВСПЛЕСК ЧАСТИЦ ПРИ ПАДЕНИИ БУКВЫ
// ============================================================
void ActivationView::spawn_land_burst(int16_t x, int16_t y) {
    for (int i = 0; i < 15; i++) {
        float ang = (std::rand() % 360) * 3.14159f / 180.0f;
        float spd = 0.5f + (std::rand() % 20) / 10.0f;

        for (auto& p : particles_) {
            if (p.life == 0) {
                p.x    = x;
                p.y    = y;
                p.vx   = (int8_t)(std::cos(ang) * spd * 2.0f);
                p.vy   = (int8_t)(std::sin(ang) * spd * 2.0f - 1);
                p.life = 25 + (std::rand() % 25);
                break;
            }
        }
    }
}

// ============================================================
//         СТАРТОВЫЙ ВЗРЫВ ЧАСТИЦ
// ============================================================
void ActivationView::spawn_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        float ang = (std::rand() % 360) * 3.14159f / 180.0f;
        float spd = 1.0f + (std::rand() % 30) / 10.0f;

        particles_[i] = {
            (int16_t)(screen_width / 2),
            (int16_t)(screen_height / 2),
            (int8_t)(std::cos(ang) * spd * 2.0f),
            (int8_t)(std::sin(ang) * spd * 2.0f),
            (uint8_t)(30 + std::rand() % 50)
        };
    }
}

void ActivationView::step_particles() {
    for (auto& p : particles_) {
        if (p.life == 0) continue;
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 1;               // гравитация
        p.life--;
    }
}

// ============================================================
//         ЭТАП 4: ВЗРЫВ С ПАДАЮЩИМИ БУКВАМИ
// ============================================================
void ActivationView::draw_explosion(Painter& painter) {
    auto r = screen_rect();
    painter.fill_rectangle(r, Theme::getInstance()->bg_darkest->background);

    // короткая вспышка в начале
    if (stage_tick_ < 10) {
        Color flash = (stage_tick_ % 2 == 0)
            ? Theme::getInstance()->fg_red->foreground
            : Theme::getInstance()->fg_yellow->foreground;
        painter.fill_rectangle(
            {0, (int)(r.height() / 2 - 20), r.width(), 40},
            flash);
    }

    // падающие буквы
    for (auto& L : letters_) {
        if (stage_tick_ < L.delay) continue;

        int16_t draw_y = L.y;
        if (L.landed && L.land_tick < 4) {
            draw_y = L.y - (4 - L.land_tick) * 2;   // подскок
        }

        char buf[2] = { L.ch, '\0' };

        Color c = L.landed
            ? Theme::getInstance()->fg_red->foreground
            : Theme::getInstance()->fg_yellow->foreground;

        painter.draw_string({L.x,     draw_y}, style().font_fixed_16x32, buf, c);
        painter.draw_string({L.x + 1, draw_y}, style().font_fixed_16x32, buf, c);
    }

    // частицы поверх
    for (auto& p : particles_) {
        if (p.life == 0) continue;
        Color c = (p.life > 40)
            ? Theme::getInstance()->fg_yellow->foreground
            : (p.life > 20)
                ? Theme::getInstance()->fg_red->foreground
                : Theme::getInstance()->fg_magenta->foreground;
        painter.fill_rectangle({p.x, p.y, 2, 2}, c);
    }

    // когда все буквы упали — мигающая надпись снизу
    bool all_landed = true;
    for (auto& L : letters_) {
        if (!L.landed) { all_landed = false; break; }
    }
    if (all_landed && (stage_tick_ / 5) % 2 == 0) {
        painter.draw_string(
            {10, (int)(r.height() - 16)},
            style().font_fixed_8x16,
            "*** BOOM ***",
            Theme::getInstance()->fg_red->foreground);
    }
}

// ============================================================
//         ОБЩИЙ PAINT (диспетчер по этапам)
// ============================================================
void ActivationView::paint(Painter& painter) {
    switch (stage_) {
        case Stage::Bloom:     draw_bloom(painter);     break;
        case Stage::WaitCard:  draw_wait(painter);      break;
        case Stage::Countdown: draw_countdown(painter); break;
        case Stage::Explosion: draw_explosion(painter); break;
        case Stage::Done:
            painter.fill_rectangle(
                screen_rect(),
                Theme::getInstance()->bg_darkest->background);
            painter.draw_string(
                {8, 90},
                style().font_fixed_8x16,
                "SYSTEM",
                Theme::getInstance()->fg_red->foreground);
            painter.draw_string(
                {8, 110},
                style().font_fixed_8x16,
                "DESTROYED",
                Theme::getInstance()->fg_red->foreground);
            break;
    }
}

// ============================================================
//                       ВВОД
// ============================================================
bool ActivationView::on_key(const KeyEvent key) {
    if (key == KeyEvent::Select) {
        if (stage_ == Stage::WaitCard) {
            stage_       = Stage::Countdown;
            stage_tick_  = 0;
            count_value_ = 10;
            set_dirty();
            return true;
        }
    }
    if (key == KeyEvent::Back) {
        nav_.pop();
        return true;
    }
    return false;
}
