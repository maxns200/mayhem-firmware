/*
 * activation_app.hpp
 * PortaPack Mayhem App: "Activation"
 *
 * Этапы:
 *   1. Bloom      — расцветающий цветок + прогресс-бар
 *   2. WaitCard   — "ATTACH CARD / PRESS BUTTON"
 *   3. Countdown  — отсчёт 10..0 большими цифрами
 *   4. Explosion  — падающие буквы B O O M + частицы + вспышка
 *   5. Done       — "SYSTEM DESTROYED"
 */

#ifndef __ACTIVATION_APP_HPP__
#define __ACTIVATION_APP_HPP__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_painter.hpp"
#include "ui_text.hpp"
#include "string_format.hpp"
#include "theme.hpp"
#include "timer.hpp"

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace ui {

class ActivationView : public View {
public:
    ActivationView(NavigationView& nav);

    void on_show() override;
    void focus() override;
    std::string title() const override { return "Activation"; }

    void paint(Painter& painter) override;
    bool on_key(const KeyEvent key) override;

private:
    enum class Stage { Bloom, WaitCard, Countdown, Explosion, Done };

    NavigationView& nav_;
    Stage    stage_{Stage::Bloom};
    uint32_t tick_{0};
    uint32_t stage_tick_{0};
    int      count_value_{10};

    // ---------- частицы взрыва ----------
    struct Particle { int16_t x, y; int8_t vx, vy; uint8_t life; };
    static constexpr int MAX_PARTICLES = 150;
    Particle particles_[MAX_PARTICLES]{};

    // ---------- падающие буквы B O O M ----------
    struct FallingLetter {
        char     ch;
        int16_t  x;
        int16_t  y;
        int8_t   vy;
        uint8_t  delay;
        bool     landed;
        uint8_t  land_tick;
    };
    static constexpr int LETTER_COUNT = 4;
    FallingLetter letters_[LETTER_COUNT]{};

    // ---------- методы ----------
    void draw_bloom(Painter& painter);
    void draw_wait(Painter& painter);
    void draw_countdown(Painter& painter);
    void draw_explosion(Painter& painter);

    void init_letters();
    void spawn_particles();
    void spawn_land_burst(int16_t x, int16_t y);
    void step_particles();

    Timer timer_{
        50,
        [this]() {
            tick_++;
            stage_tick_++;

            switch (stage_) {
                case Stage::Bloom:
                    if (stage_tick_ > 80) {
                        stage_ = Stage::WaitCard;
                        stage_tick_ = 0;
                    }
                    break;

                case Stage::WaitCard:
                    break;

                case Stage::Countdown:
                    if (stage_tick_ % 20 == 0) {
                        if (count_value_ > 0) {
                            count_value_--;
                        } else {
                            stage_ = Stage::Explosion;
                            stage_tick_ = 0;
                            spawn_particles();
                            init_letters();
                        }
                    }
                    break;

                case Stage::Explosion: {
                    for (auto& L : letters_) {
                        if (stage_tick_ < L.delay) continue;
                        if (!L.landed) {
                            L.y += L.vy;
                            if (L.y >= (int16_t)(screen_height - 40)) {
                                L.landed = true;
                                L.land_tick = 0;
                                spawn_land_burst(L.x, L.y);
                            }
                        } else {
                            L.land_tick++;
                        }
                    }
                    step_particles();

                    if (stage_tick_ > 140) {
                        stage_ = Stage::Done;
                        stage_tick_ = 0;
                    }
                    break;
                }

                case Stage::Done:
                    break;
            }
            set_dirty();
        }
    };
};

} // namespace ui

#endif /*__ACTIVATION_APP_HPP__*/
