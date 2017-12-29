/*
 * Minefield.h
 *
 *  Created on: 28 Dec 2017
 *      Author: rkfg
 */

#ifndef MINEFIELD_H_
#define MINEFIELD_H_

#include <caf/all.hpp>
#include <array>
#include <vector>
#include <random>

enum class minefield_error
    : uint8_t {
        too_many_mines = 1, invalid_size, invalid_coords, already_opened, has_flag
};

std::string to_string(minefield_error x);

enum class minefield_result {
    ok, kaboom, victory
};

using init_atom = caf::atom_constant<caf::atom("init")>;
using open_atom = caf::atom_constant<caf::atom("open")>;
using dump_atom = caf::atom_constant<caf::atom("dump")>;
using flag_atom = caf::atom_constant<caf::atom("flag")>;
using minefield_error_atom = caf::atom_constant<caf::atom("mferr")>;

using minefield_actor_t = caf::typed_actor<caf::reacts_to<init_atom, uint8_t, uint8_t, uint8_t>,
caf::replies_to<open_atom, bool, uint8_t, uint8_t>::with<minefield_result>,
caf::reacts_to<dump_atom, bool>,
caf::reacts_to<flag_atom, uint8_t, uint8_t>>;

struct Mine {
    bool has_mine;
    bool has_flag;
    bool is_opened;
    uint8_t mines_around;
};

struct field_t {
    std::vector<Mine> field;
    uint8_t width = 0;
    uint8_t height = 0;
    uint16_t empty_cells = 0;
    uint16_t deferred_opening = 0;
    bool first_move = false;
};

class Minefield: public minefield_actor_t::stateful_base<field_t> {
public:
    Minefield(caf::actor_config& cfg);
protected:
    virtual behavior_type make_behavior() override;
private:
    std::mt19937_64 rnd;
    const std::string m_mnums[8] { "\uff11", "\uff12", "\uff13", "\uff14", "\uff15", "\uff16", "\uff17", "\uff18" };
    const std::string m_xaxis[26] { "\uff21", "\uff22", "\uff23", "\uff24", "\uff25", "\uff26", "\uff27", "\uff28",
            "\uff29", "\uff2a", "\uff2b", "\uff2c", "\uff2d", "\uff2e", "\uff2f", "\uff30", "\uff31", "\uff32",
            "\uff33", "\uff34", "\uff35", "\uff36", "\uff37", "\uff38", "\uff39", "\uff3a" };

    Mine& get_mine(uint8_t x, uint8_t y);
    void dump_minefield(const bool full = false);
    void do_around(uint8_t x, uint8_t y, std::function<void(uint8_t, uint8_t)> f);
    uint8_t mines_around(uint8_t x, uint8_t y);
    void open_around(uint8_t x, uint8_t y);
    inline bool coords_valid(uint8_t x, uint8_t y);
    void dec_deferred_opening();
};

#endif /* MINEFIELD_H_ */
