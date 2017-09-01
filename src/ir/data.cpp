/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/ir/data.hpp>

namespace jlm {

/* data operator */

std::string
data_op::debug_string() const
{
	return "DATA";
}

std::unique_ptr<jive::operation>
data_op::copy() const
{
	return std::unique_ptr<jive::operation>(new data_op(*this));
}

bool
data_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const data_op*>(&other);
	return op && op->linkage_ == linkage_ && op->constant_ == constant_;
}

}
