/*
 * Minefield.cpp
 *
 *  Created on: 28 Dec 2017
 *      Author: rkfg
 */

#include "Minefield.h"
#include <algorithm>
#include <ctime>

using namespace caf;
using namespace std;

error make_error(minefield_error x) {
    return {static_cast<uint8_t>(x), minefield_error_atom::value};
}

std::string to_string(minefield_error x) {
    switch (x) {
    case minefield_error::too_many_mines:
        return "too many mines";
    case minefield_error::invalid_size:
        return "invalid field size";
    case minefield_error::invalid_coords:
        return "invalid coords entered";
    case minefield_error::already_opened:
        return "this cell is already opened";
    case minefield_error::has_flag:
        return "this cell has a flag and can't be opened";
    default:
        return "<unknown>";
    }
}

Minefield::Minefield(actor_config& cfg) :
        minefield_actor_t::stateful_base<field_t>(cfg) {
    rnd.seed(time(0));
}

minefield_actor_t::behavior_type Minefield::make_behavior() {
    return {
        [=](init_atom, uint8_t width, uint8_t height, uint8_t mines_count) -> result<void> {
            if (mines_count >= width*height) {
                return minefield_error::too_many_mines;
            }
            if (width < 1 || width > 26 || height < 1 || height > 26) {
                return minefield_error::invalid_size;
            }
            state.width = width;
            state.height = height;
            state.field.clear();
            state.field.resize(width*height);
            state.cnt = width*height - mines_count;
            for (int i = 0; i < mines_count; ++i) {
                state.field[i].has_mine = true;
            }
            shuffle(state.field.begin(), state.field.end(), rnd);
            aout(this) << "Created a minefield " << width << "x" << height << " with " << mines_count << " mines." << endl;
            return unit;
        }, [=](open_atom, uint8_t x, uint8_t y) -> result<minefield_result> {
            if (!coords_valid(x, y)) {
                return minefield_error::invalid_coords;
            }
            auto& mine = get_mine(x, y);
            if (mine.is_opened) {
                return minefield_error::already_opened;
            }
            if (mine.has_flag) {
                return minefield_error::has_flag;
            }
            if (mine.has_mine) {
                aout(this) << "K A B O O M" << endl;
                anon_send(this, dump_atom::value, true);
                return minefield_result::kaboom;
            }
            mine.is_opened = true;
            auto a = mines_around(x, y);
            if (!a) {
                open_around(x, y);
            } else {
                mine.mines_around = a;
            }
            --state.cnt;
            if (!state.cnt) {
                aout(this) << "V I C T O R Y" << endl;
                send(this, dump_atom::value, true);
                return minefield_result::victory;
            }
            send(this, dump_atom::value, false);
            return minefield_result::ok;
        }, [=](dump_atom, bool full) -> void {
            dump_minefield(full);
        }, [=](flag_atom, uint8_t x, uint8_t y) -> result<void> {
            if (!coords_valid(x, y)) {
                return minefield_error::invalid_coords;
            }
            auto& m = get_mine(x, y);
            m.has_flag = !m.has_flag;
            send(this, dump_atom::value, false);
            return unit;
        }
    };
}

Mine& Minefield::get_mine(uint8_t x, uint8_t y) {
    return state.field[y * state.width + x];
}

void Minefield::dump_minefield(const bool full) {
    for (int y = 0; y < state.height; ++y) {
        for (int x = 0; x < state.width; ++x) {
            auto& elem = get_mine(x, y);
            if (elem.has_mine && full) {
                aout(this) << "\uff38";
            } else if (elem.mines_around > 0) {
                aout(this) << m_mnums[elem.mines_around - 1];
            } else if (elem.is_opened) {
                aout(this) << "\uff0e";
            } else if (elem.has_flag) {
                aout(this) << "\uff26";
            } else {
                aout(this) << "\uff3f";
            }
        }
        aout(this) << " " << y + 1 << endl;
    }
    for (int x = 0; x < state.width; ++x) {
        aout(this) << m_xaxis[x];
    }
    aout(this) << endl << endl;
}

void Minefield::do_around(uint8_t x, uint8_t y, std::function<void(uint8_t, uint8_t)> f) {
    for (int8_t yc = y - 1; yc <= y + 1; ++yc) {
        for (int8_t xc = x - 1; xc <= x + 1; ++xc) {
            if ((xc != x || yc != y) && xc < state.width && yc < state.height && xc >= 0 && yc >= 0) {
                f(xc, yc);
            }
        }
    }
}

uint8_t Minefield::mines_around(uint8_t x, uint8_t y) {
    uint8_t result = 0;
    do_around(x, y, [&](uint8_t xc, uint8_t yc) {
        if (get_mine(xc, yc).has_mine) {
            ++result;
        }
    });
    return result;
}

void Minefield::open_around(uint8_t x, uint8_t y) {
    do_around(x, y, [&](uint8_t xc, uint8_t yc) {
        anon_send(this, open_atom::value, xc, yc);
    });
}

bool Minefield::coords_valid(uint8_t x, uint8_t y) {
    return (x >= 0 && x < state.width && y >= 0 && y <= state.height);
}
