/*
 * Copyright 2024 Louis Maurin <louis7maurin@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/hls/ir/static-hls.hpp>
#include <jlm/rvsdg/structural-node.hpp>

namespace jlm::static_hls
{

loop_node *
loop_node::create(jlm::rvsdg::region * parent)
{
    auto ln = new loop_node(parent);
    return ln;
}

jlm::rvsdg::structural_output *
loop_node::add_loopvar(jlm::rvsdg::output * origin)
{
    auto input = jlm::rvsdg::structural_input::create(this, origin, origin->type());
    auto output = jlm::rvsdg::structural_output::create(this, origin->type());

    auto argument_in = jlm::rvsdg::argument::create(compute_subregion(), input, origin->type());
    auto reg = reg_op::create(*(static_cast<rvsdg::output *>(argument_in)));
    jlm::rvsdg::result::create(compute_subregion(), reg, output, origin->type());
    return output;
}

// FIXME: This function is not implemented
loop_node *
loop_node::copy(jlm::rvsdg::region * region, jlm::rvsdg::substitution_map & smap) const {
    auto ln = new loop_node(region);
    return ln;
}

} // namespace jlm::static_hls