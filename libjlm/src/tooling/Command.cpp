/*
 * Copyright 2019 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/tooling/Command.hpp>
#include <jlm/tooling/LlvmPaths.hpp>
#include <jlm/util/strfmt.hpp>

#include <unordered_map>

namespace jlm {

Command::~Command()
= default;

PrintCommandsCommand::~PrintCommandsCommand()
= default;

std::string
PrintCommandsCommand::ToString() const
{
  return "PrintCommands";
}

void
PrintCommandsCommand::Run() const
{
  for (auto & node : CommandGraph::SortNodesTopological(*CommandGraph_)) {
    if (node != &CommandGraph_->GetEntryNode() && node != &CommandGraph_->GetExitNode())
      std::cout << node->GetCommand().ToString() << "\n";
  }
}

std::unique_ptr<CommandGraph>
PrintCommandsCommand::Create(std::unique_ptr<CommandGraph> commandGraph)
{
  std::unique_ptr<CommandGraph> newCommandGraph(new CommandGraph());
  auto & printCommandsNode = PrintCommandsCommand::Create(*newCommandGraph, std::move(commandGraph));
  newCommandGraph->GetEntryNode().AddEdge(printCommandsNode);
  printCommandsNode.AddEdge(newCommandGraph->GetExitNode());
  return newCommandGraph;
}

ClangCommand::~ClangCommand()
= default;

std::string
ClangCommand::ToString() const
{
  std::string inputFiles;
  for (auto & inputFile : InputFiles_)
    inputFiles += inputFile.to_str() + " ";

  std::string libraryPaths;
  for (auto & libraryPath : LibraryPaths_)
    libraryPaths += "-L" + libraryPath + " ";

  std::string libraries;
  for (const auto & library : Libraries_)
    libraries += "-l" + library + " ";

  std::string arguments;
  if (UsePthreads_)
    arguments += "-pthread ";

  return strfmt(
    clangpath.to_str() + " "
    , "-no-pie -O0 "
    , arguments
    , inputFiles
    , "-o ", OutputFile_.to_str(), " "
    , libraryPaths
    , libraries);
}

void
ClangCommand::Run() const
{
  if (system(ToString().c_str()))
    exit(EXIT_FAILURE);
}

cgencmd::~cgencmd()
{}

std::string
cgencmd::ToString() const
{
  return strfmt(
    llcpath.to_str() + " "
    , "-", ToString(ol_), " "
    , "--relocation-model=", ToString(RelocationModel_), " "
    , "-filetype=obj "
    , "-o ", ofile_.to_str(), " "
    , ifile_.to_str()
  );
}

void
cgencmd::Run() const
{
  if (system(ToString().c_str()))
    exit(EXIT_FAILURE);
}

std::string
cgencmd::ToString(const OptimizationLevel & optimizationLevel)
{
  static std::unordered_map<OptimizationLevel, const char*>
    map({
          {OptimizationLevel::O0, "O0"},
          {OptimizationLevel::O1, "O1"},
          {OptimizationLevel::O2, "O2"},
          {OptimizationLevel::O3, "O3"}
        });

  JLM_ASSERT(map.find(optimizationLevel) != map.end());
  return map[optimizationLevel];
}

std::string
cgencmd::ToString(const RelocationModel & relocationModel)
{
  static std::unordered_map<RelocationModel, const char*>
    map({
          {RelocationModel::Static, "static"},
          {RelocationModel::Pic, "pic"},
        });

  JLM_ASSERT(map.find(relocationModel) != map.end());
  return map[relocationModel];
}

}
