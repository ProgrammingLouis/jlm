/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <test-registry.hpp>
#include <test-types.hpp>

#include <jive/arch/addresstype.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/view.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/statemux.hpp>

#include <jlm/ir/operators/alloca.hpp>
#include <jlm/ir/operators/operators.hpp>
#include <jlm/ir/operators/store.hpp>
#include <jlm/ir/types.hpp>

static inline void
test_store_mux_reduction()
{
	using namespace jlm;

	jlm::valuetype vt;
	jlm::ptrtype pt(vt);
	jive::memtype mt;

	jive::graph graph;
	auto nf = graph.node_normal_form(typeid(jlm::store_op));
	auto snf = static_cast<jlm::store_normal_form*>(nf);
	snf->set_mutable(false);
	snf->set_store_mux_reducible(false);

	auto a = graph.add_import({pt, "a"});
	auto v = graph.add_import({vt, "v"});
	auto s1 = graph.add_import({mt, "s1"});
	auto s2 = graph.add_import({mt, "s2"});
	auto s3 = graph.add_import({mt, "s3"});

	auto mux = MemStateMergeOperator::Create({s1, s2, s3});
	auto state = store_op::create(a, v, {mux}, 4);

	auto ex = graph.add_export(state[0], {state[0]->type(), "s"});

//	jive::view(graph.root(), stdout);

	snf->set_mutable(true);
	snf->set_store_mux_reducible(true);
	graph.normalize();
	graph.prune();

//	jive::view(graph.root(), stdout);

	auto muxnode= jive::node_output::node(ex->origin());
	assert(is<MemStateMergeOperator>(muxnode));
	assert(muxnode->ninputs() == 3);
	auto n0 = jive::node_output::node(muxnode->input(0)->origin());
	auto n1 = jive::node_output::node(muxnode->input(1)->origin());
	auto n2 = jive::node_output::node(muxnode->input(2)->origin());
	assert(jive::is<jlm::store_op>(n0->operation()));
	assert(jive::is<jlm::store_op>(n1->operation()));
	assert(jive::is<jlm::store_op>(n2->operation()));
}

static inline void
test_multiple_origin_reduction()
{
	using namespace jlm;

	jlm::valuetype vt;
	jlm::ptrtype pt(vt);
	jive::memtype mt;

	jive::graph graph;
	auto nf = graph.node_normal_form(typeid(jlm::store_op));
	auto snf = static_cast<jlm::store_normal_form*>(nf);
	snf->set_mutable(false);
	snf->set_multiple_origin_reducible(false);

	auto a = graph.add_import({pt, "a"});
	auto v = graph.add_import({vt, "v"});
	auto s = graph.add_import({mt, "s"});

	auto states = store_op::create(a, v, {s, s, s, s}, 4);

	auto ex = graph.add_export(states[0], {states[0]->type(), "s"});

//	jive::view(graph.root(), stdout);

	snf->set_mutable(true);
	snf->set_multiple_origin_reducible(true);
	graph.normalize();
	graph.prune();

//	jive::view(graph.root(), stdout);

	auto node = jive::node_output::node(ex->origin());
	assert(jive::is<jlm::store_op>(node->operation()) && node->ninputs() == 3);
}

static inline void
test_store_alloca_reduction()
{
	using namespace jlm;

	jlm::valuetype vt;
	jive::memtype mt;
	jive::bittype bt(32);

	jive::graph graph;
	auto nf = graph.node_normal_form(typeid(jlm::store_op));
	auto snf = static_cast<jlm::store_normal_form*>(nf);
	snf->set_mutable(false);
	snf->set_store_alloca_reducible(false);

	auto size = graph.add_import({bt, "size"});
	auto value = graph.add_import({vt, "value"});
	auto s = graph.add_import({mt, "s"});

	auto alloca1 = alloca_op::create(vt, size, 4);
	auto alloca2 = alloca_op::create(vt, size, 4);
	auto states1 = store_op::create(alloca1[0], value, {alloca1[1], alloca2[1], s}, 4);
	auto states2 = store_op::create(alloca2[0], value, states1, 4);

	graph.add_export(states2[0], {states2[0]->type(), "s1"});
	graph.add_export(states2[1], {states2[1]->type(), "s2"});
	graph.add_export(states2[2], {states2[2]->type(), "s3"});

//	jive::view(graph.root(), stdout);

	snf->set_mutable(true);
	snf->set_store_alloca_reducible(true);
	graph.normalize();
	graph.prune();

//	jive::view(graph.root(), stdout);

	bool has_add_import = false;
	for (size_t n = 0; n < graph.root()->nresults(); n++) {
		if (graph.root()->result(n)->origin() == s)
			has_add_import = true;
	}
	assert(has_add_import);
}

static inline void
test_store_store_reduction()
{
	using namespace jlm;

	valuetype vt;
	jlm::ptrtype pt(vt);
	jive::memtype mt;

	jive::graph graph;
	auto a = graph.add_import({pt, "address"});
	auto v1 = graph.add_import({vt, "value"});
	auto v2 = graph.add_import({vt, "value"});
	auto s = graph.add_import({mt, "state"});

	auto s1 = store_op::create(a, v1, {s}, 4)[0];
	auto s2 = store_op::create(a, v2, {s1}, 4)[0];

	auto ex = graph.add_export(s2, {s2->type(), "state"});

	jive::view(graph.root(), stdout);

	auto nf = store_op::normal_form(&graph);
	nf->set_store_store_reducible(true);
	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stdout);

	assert(graph.root()->nnodes() == 1);
	assert(jive::node_output::node(ex->origin())->input(1)->origin() == v2);
}

static int
test()
{
	test_store_mux_reduction();
	test_store_alloca_reduction();
	test_multiple_origin_reduction();
	test_store_store_reduction();

	return 0;
}

JLM_UNIT_TEST_REGISTER("libjlm/test-store", test)
