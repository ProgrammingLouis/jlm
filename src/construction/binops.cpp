/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/common.hpp>
#include <jlm/construction/binops.hpp>
#include <jlm/construction/context.hpp>
#include <jlm/construction/instruction.hpp>
#include <jlm/IR/basic_block.hpp>
#include <jlm/IR/tac.hpp>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/comparison.h>

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>

#include <map>

namespace jlm {

static inline const variable *
convert_int_binary_operator(
	const llvm::BinaryOperator * i,
	basic_block * bb,
	const context & ctx)
{
	static std::map<
		const llvm::Instruction::BinaryOps,
		std::unique_ptr<jive::operation>(*)(size_t)> map({
			{llvm::Instruction::Add,	[](size_t nbits){jive::bits::add_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::And,	[](size_t nbits){jive::bits::and_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::AShr,	[](size_t nbits){jive::bits::ashr_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::Sub,	[](size_t nbits){jive::bits::sub_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::UDiv,	[](size_t nbits){jive::bits::udiv_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::SDiv,	[](size_t nbits){jive::bits::sdiv_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::URem,	[](size_t nbits){jive::bits::umod_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::SRem,	[](size_t nbits){jive::bits::smod_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::Shl,	[](size_t nbits){jive::bits::shl_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::LShr,	[](size_t nbits){jive::bits::shr_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::Or,		[](size_t nbits){jive::bits::or_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::Xor,	[](size_t nbits){jive::bits::xor_op op(nbits); return op.copy();}}
		,	{llvm::Instruction::Mul,	[](size_t nbits){jive::bits::mul_op op(nbits); return op.copy();}}
	});

	const jlm::variable * op1 = convert_value(i->getOperand(0), ctx);
	const jlm::variable * op2 = convert_value(i->getOperand(1), ctx);
	size_t nbits = static_cast<const llvm::IntegerType*>(i->getType())->getBitWidth();
	return bb->append(*map[i->getOpcode()](nbits), {op1, op2}, {ctx.lookup_value(i)})->output(0);
}

void
convert_binary_operator(
	const llvm::BinaryOperator * i,
	basic_block * bb,
	const context & ctx)
{
	const jlm::variable * result = nullptr;
	switch (i->getType()->getTypeID()) {
		case llvm::Type::IntegerTyID:
			result = convert_int_binary_operator(i, bb, ctx);
			break;
		default:
			JLM_DEBUG_ASSERT(0);
	}

	JLM_DEBUG_ASSERT(result);
}

}
