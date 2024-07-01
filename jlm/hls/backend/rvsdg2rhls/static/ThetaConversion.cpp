/*
 * Copyright 2024 Louis Maurin <louis7maurin@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/hls/backend/rvsdg2rhls/static/ThetaConversion.hpp>
#include <jlm/rvsdg/traverser.hpp>
#include <jlm/hls/ir/static-hls.hpp>
#include <jlm/rvsdg/substitution.hpp>

namespace jlm::static_hls
{

void
CopyOperations(jlm::rvsdg::region & region, jlm::static_hls::loop_node & loop)
{
  rvsdg::substitution_map reg_smap;
  std::unordered_map<const rvsdg::node *, const rvsdg::node *> nullary_node_map;
  std::vector<const jlm::rvsdg::operation *> operations;
  for (auto & node : jlm::rvsdg::topdown_traverser(&region))
  {
    if (dynamic_cast<const jlm::rvsdg::simple_node *>(node))
    {
      std::cout << "copying operation: " << node->operation().debug_string() << std::endl;

      if (dynamic_cast<const jlm::rvsdg::nullary_op *>(&node->operation()))
      {
        auto new_node = node->copy(loop.compute_subregion(), {});
        nullary_node_map[node] = new_node;
        continue;
      }


      //FIXME this is not really efficient
      // push operation if not already in the list
      //FIXME this is only comparing pointers ?
      if (std::find(operations.begin(), operations.end(), &(node->operation())) == operations.end())
      {
        std::vector<jlm::rvsdg::output *> inputs_args;
        for (size_t i = 0; i < node->ninputs(); i++)
        {
          if (dynamic_cast<const jlm::rvsdg::nullary_op *>(&node->input(i)->origin()->node()->operation())) {
            auto output_index = node->input(i)->origin()->index();
            inputs_args.push_back(nullary_node_map[node]->output(output_index));
          }
          else if (auto arg = dynamic_cast<jlm::rvsdg::argument*>(node->input(i)))
          {
            arg->index();
            auto reg = reg_smap.lookup(arg->origin());
            inputs_args.push_back(reg);
            continue;
          }
          // auto input = jlm::rvsdg::structural_input::create(&loop, loop.input(0), origin->type());
          inputs_args.push_back(rvsdg::argument::create(loop.compute_subregion(), nullptr, node->input(i)->type()));
        }
        // Copy the node into the compute subregion of the loop
        node->copy(loop.compute_subregion(), inputs_args);

        //FIXME this only works for nodes with one output
        // Create a register for the output of the operation
        // auto output = jlm::rvsdg::output(loop.compute_subregion(), node->output(0)->type())
        auto reg = reg_op::create(*node->output(0));
        reg_smap.insert(node->output(0), reg);

        operations.push_back(&(node->operation()));
      }
    }
  }
}

static void
ConvertThetaNode(jlm::rvsdg::theta_node & theta)
{
  std::cout << "Converting theta node" << std::endl;
  std::cout << "Theta node has " << theta.ninputs() << " inputs" << std::endl;
  std::cout << "Theta node has " << theta.noutputs() << " outputs" << std::endl;
  jlm::rvsdg::substitution_map smap;

  auto loop = static_hls::loop_node::create(theta.region());

  // add loopvars and populate the smap
  for (size_t i = 0; i < theta.ninputs(); i++)
  {
    loop->add_loopvar(theta.input(i)->origin());
    smap.insert(theta.input(i)->argument(), loop->compute_subregion()->argument(i));
    // divert theta outputs
    theta.output(0)->divert_users(loop->output(i));
  }

  // copy contents of theta
  CopyOperations(*theta.subregion(), *loop);
  // theta.subregion()->copy(loop->compute_subregion(), smap, false, false);

  // // copy contents of theta
  // theta.subregion()->copy(loop->subregion(), smap, false, false);
  remove(&theta);
}

static void
ConvertThetaNodesInRegion(jlm::rvsdg::region & region);

static void
ConvertThetaNodesInStructuralNode(jlm::rvsdg::structural_node & structuralNode)
{
  for (size_t n = 0; n < structuralNode.nsubregions(); n++)
  {
    ConvertThetaNodesInRegion(*structuralNode.subregion(n));
  }

  if (auto thetaNode = dynamic_cast<jlm::rvsdg::theta_node *>(&structuralNode))
  {
    ConvertThetaNode(*thetaNode);
  }
}

static void
ConvertThetaNodesInRegion(jlm::rvsdg::region & region)
{
  for (auto & node : jlm::rvsdg::topdown_traverser(&region))
  {
    if (auto structuralNode = dynamic_cast<jlm::rvsdg::structural_node *>(node))
    {
      ConvertThetaNodesInStructuralNode(*structuralNode);
    }
  }
}

void
ConvertThetaNodes(jlm::llvm::RvsdgModule & rvsdgModule)
{
  ConvertThetaNodesInRegion(*rvsdgModule.Rvsdg().root());
}

}