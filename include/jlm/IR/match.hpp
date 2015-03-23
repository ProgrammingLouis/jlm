/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JLM_IR_MATCH_HPP
#define JLM_IR_MATCH_HPP

#include <jive/vsdg/operators/match.h>

namespace jlm {

JIVE_EXPORTED_INLINE const jlm::variable *
match_tac(
	jlm::basic_block * basic_block,
	const jlm::variable * operand,
	const std::vector<size_t> & constants)
{
	if (!dynamic_cast<const jive::bits::type*>(&operand->type()))
		throw jive::type_error("bits<N>", operand->type().debug_string());

	jive::match_op op(static_cast<const jive::bits::type&>(operand->type()), constants);
	const jlm::variable * result = basic_block->cfg()->create_variable(op.result_type(0));
	const jlm::tac * tac = basic_block->append(op, {operand}, {result});

	JLM_DEBUG_ASSERT(op.narguments() == 1);
	return tac->output(0);
}

}

#endif
