/*
 * Input.cpp
 *
 *  Created on: 28 Dec 2017
 *      Author: rkfg
 */

#include "KBInput.h"

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace caf;

error make_error(input_error x) {
    return {static_cast<uint8_t>(x), input_error_atom::value};
}

KBInput::KBInput(caf::actor_config& cfg, minefield_actor_t field) :
        kb_input_actor_t::base(cfg), m_field(field) {
}

kb_input_actor_t::behavior_type KBInput::make_behavior() {
    return {
        [=](kb_input_atom, string s) -> result<minefield_result> {
            if (s.empty()) {
                return input_error::invalid;
            }
            boost::algorithm::to_lower(s);
            bool flag = false;
            if (s[0] == '-') {
                flag = true;
                s = s.substr(1);
            }
            if (s[0] < 'a' || s[0] > 'z') {
                return input_error::invalid;
            }
            uint8_t x = s[0] - 'a';
            try {
                uint8_t y = stoi(s.substr(1)) - 1;
                if (flag) {
                    send(m_field, flag_atom::value, x, y);
                    return minefield_result::ok;
                } else {
                    return delegate(m_field, open_atom::value, x, y);
                }
            } catch (const invalid_argument& e) {
                aout(this) << "Error parsing input." << endl;
                return input_error::invalid;
            }
        }
    };
}
