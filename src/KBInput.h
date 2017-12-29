/*
 * Input.h
 *
 *  Created on: 28 Dec 2017
 *      Author: rkfg
 */

#ifndef KBINPUT_H_
#define KBINPUT_H_

#include "Minefield.h"
#include <caf/all.hpp>

using input_error_atom = caf::atom_constant<caf::atom("ierr")>;

enum class input_error
    : uint8_t {
        invalid = 1
};

using kb_input_atom = caf::atom_constant<caf::atom("inp")>;
using kb_input_actor_t = caf::typed_actor<caf::replies_to<kb_input_atom, std::string>::with<minefield_result>>;

class KBInput: public kb_input_actor_t::base {
public:
    KBInput(caf::actor_config& cfg, minefield_actor_t field);

protected:
    virtual behavior_type make_behavior() override;
private:
    minefield_actor_t m_field;
};

#endif /* KBINPUT_H_ */
