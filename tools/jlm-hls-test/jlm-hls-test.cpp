/*
Copyright 2024 Louis Maurin <louis7maurin@gmail.com>
See COPYING for terms of redistribution.
*/

// #include <tests/TestRvsdgs.hpp>
#include <jlm/rvsdg/view.hpp>

#include <jlm/hls/backend/rhls2firrtl/dot-hls.hpp>
#include <jlm/hls/backend/rhls2firrtl/firrtl-hls.hpp>
#include <jlm/hls/backend/rhls2firrtl/RhlsToFirrtlConverter.hpp>
// #include <jlm/hls/backend/rvsdg2rhls/rvsdg2rhls.hpp>
#include <jlm/hls/backend/rvsdg2rhls/static/rvsdg2rhls.hpp>

#include <jlm/llvm/ir/operators.hpp>
#include <jlm/rvsdg/theta.hpp>


// static void
// stringToFile(std::string output, std::string fileName)
// {
//   std::ofstream outputFile;
//   outputFile.open(fileName);
//   outputFile << output;
//   outputFile.close();
// }

std::unique_ptr<jlm::llvm::RvsdgModule>
CreateTestModule() {
  using namespace jlm::llvm;

  auto module = jlm::llvm::RvsdgModule::Create(jlm::util::filepath(""), "", "");
  auto graph = &module->Rvsdg();

  auto nf = graph->node_normal_form(typeid(jlm::rvsdg::operation));
  nf->set_mutable(false);

  MemoryStateType mt;
  PointerType pointerType;
  FunctionType fcttype(
        { jlm::rvsdg::bittype::Create(32)},
        { jlm::rvsdg::bittype::Create(32) });
  auto fct = lambda::node::create(graph->root(), fcttype, "f", linkage::external_linkage);

  auto thetanode = jlm::rvsdg::theta_node::create(fct->subregion());

  auto sum_loop_var = thetanode->add_loopvar(fct->fctargument(0));

  auto one = jlm::rvsdg::create_bitconstant(thetanode->subregion(), 32, 1);
  auto five = jlm::rvsdg::create_bitconstant(thetanode->subregion(), 32, 5);
  auto sum = jlm::rvsdg::bitadd_op::create(32, sum_loop_var->argument(), one);
  auto cmp = jlm::rvsdg::bitult_op::create(32, sum, five);
  auto predicate = jlm::rvsdg::match(1, { { 1, 1 } }, 0, 2, cmp);

  // change to loop_var result origin to the ouput of sum
  // (by default the loop_var result origin is connected to the loop_var argument (to itself))
  sum_loop_var->result()->divert_to(sum);

  thetanode->set_predicate(predicate);

  fct->finalize({ sum_loop_var });

  // Make the function external func
  graph->add_export(fct->output(), { PointerType(), "f" });

  return module;
}

int main() {
    // jlm::tests::ThetaTest test;
    auto rvsdgModule = CreateTestModule();


    FILE * file1 = fopen("My_module_rvsdg.txt", "w");
    jlm::rvsdg::view(*rvsdgModule->Rvsdg().root()->graph(), file1);

    std::cout << "Running static rvsdg2rhls" << std::endl;
    jlm::static_hls::rvsdg2rhls(*rvsdgModule);

    FILE * file2 = fopen("My_module_rhls.txt", "w");
    jlm::rvsdg::view(*rvsdgModule->Rvsdg().root()->graph(), file2);

    // jlm::hls::RhlsToFirrtlConverter hls;
    // auto output = hls.ToString(*rvsdgModule);

    // stringToFile(output, "PERSO.fir");

    // jlm::hls::DotHLS dhls;
    // stringToFile(dhls.run(*rvsdgModule), "PERSO.dot");

    return 0;
}