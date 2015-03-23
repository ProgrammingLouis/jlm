/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JLM_IR_BITSTRING_H
#define JLM_IR_BITSTRING_H

#include <jlm/IR/tac.hpp>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/comparison.h>
#include <jive/types/bitstring/concat.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>

namespace jlm {

JIVE_EXPORTED_INLINE const variable *
bitconstant_tac(
	jlm::basic_block * basic_block,
	const jive::bits::value_repr & v,
	const jlm::variable * result)
{
	jive::bits::constant_op op(v);
	const jlm::tac * tac = basic_block->append(op, {}, {result});
	return tac->output(0);
}

JIVE_EXPORTED_INLINE const variable *
bitconcat_tac(jlm::basic_block * basic_block,
	const std::vector<const jlm::variable*> & ops,
	const jlm::variable * result)
{
	std::vector<jive::bits::type> types;
	for (auto v : ops)
		types.push_back(static_cast<const jive::bits::type&>(v->type()));

	jive::bits::concat_op op(types);
	const jlm::tac * tac = basic_block->append(op, ops, {result});
	return tac->output(0);
}

JIVE_EXPORTED_INLINE const variable *
bitslice_tac(
	jlm::basic_block * basic_block,
	const jlm::variable* operand,
	size_t low, size_t high,
	const jlm::variable * result)
{
	jive::bits::slice_op op(dynamic_cast<const jive::bits::type&>(operand->type()), low, high);
	const jlm::tac * tac = basic_block->append(op, {operand}, {result});
	return tac->output(0);
}

#define MAKE_BINOP_TAC(NAME, OP) \
JIVE_EXPORTED_INLINE const variable * \
bit##NAME##_tac( \
	jlm::basic_block * basic_block, \
	size_t nbits, \
	const jlm::variable * op1, \
	const jlm::variable * op2, \
	const jlm::variable * result) \
{ \
	jive::bits::type type(nbits); \
	jive::bits::OP  op(type); \
	const jlm::tac * tac = basic_block->append(op, {op1, op2}, {result}); \
	return tac->output(0); \
} \

MAKE_BINOP_TAC(add, add_op);
MAKE_BINOP_TAC(and, and_op);
MAKE_BINOP_TAC(ashr, ashr_op);
MAKE_BINOP_TAC(mul, mul_op);
MAKE_BINOP_TAC(or, or_op);
MAKE_BINOP_TAC(sdiv, sdiv_op);
MAKE_BINOP_TAC(shl, shl_op);
MAKE_BINOP_TAC(shr, shr_op);
MAKE_BINOP_TAC(smod, smod_op);
MAKE_BINOP_TAC(smulh, smulh_op);
MAKE_BINOP_TAC(sub, sub_op);
MAKE_BINOP_TAC(udiv, udiv_op);
MAKE_BINOP_TAC(umod, umod_op);
MAKE_BINOP_TAC(umulh, umulh_op);
MAKE_BINOP_TAC(xor, xor_op);

MAKE_BINOP_TAC(slt, slt_op);
MAKE_BINOP_TAC(ult, ult_op);
MAKE_BINOP_TAC(sle, sle_op);
MAKE_BINOP_TAC(ule, ule_op);
MAKE_BINOP_TAC(eq, eq_op);
MAKE_BINOP_TAC(ne, ne_op);
MAKE_BINOP_TAC(sge, sge_op);
MAKE_BINOP_TAC(uge, uge_op);
MAKE_BINOP_TAC(sgt, sgt_op);
MAKE_BINOP_TAC(ugt, ugt_op);

#define MAKE_UNOP_TAC(NAME, OP) \
JIVE_EXPORTED_INLINE const jlm::variable * \
bit##NAME##_tac( \
	jlm::basic_block * basic_block, \
	size_t nbits, \
	const jlm::variable * op1, \
	const jlm::variable * result) \
{ \
	jive::bits::type type(nbits); \
	jive::bits::OP  op(type); \
	const jlm::tac * tac = basic_block->append(op, {op1}, {result}); \
	return tac->output(0); \
} \

MAKE_UNOP_TAC(neg, neg_op);
MAKE_UNOP_TAC(not, not_op);

}

#endif
