/*
 * Copyright 2024 Louis Maurin <louis7maurin@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/llvm/ir/types.hpp>
#include <jlm/rvsdg/control.hpp>
#include <jlm/rvsdg/operation.hpp>
#include <jlm/rvsdg/structural-node.hpp>

namespace jlm::static_hls
{

class mux_op final : public jlm::rvsdg::simple_op
{
public:
  virtual ~mux_op()
  {}

  mux_op(size_t nalternatives, const jlm::rvsdg::type & type)
      : jlm::rvsdg::simple_op(create_portvector(nalternatives, type), { type })
  {}

  std::string
  debug_string() const override
  {
    return "SHLS_MUX";
  }

  bool
  operator==(const jlm::rvsdg::operation & other) const noexcept override
  {
    auto ot = dynamic_cast<const mux_op *>(&other);
    // check predicate and value
    return ot && ot->argument(0).type() == argument(0).type()
        && ot->result(0).type() == result(0).type();
  }

  std::unique_ptr<jlm::rvsdg::operation>
  copy() const override
  {
    return std::unique_ptr<jlm::rvsdg::operation>(new mux_op(*this));
  }

  static std::vector<jlm::rvsdg::output *>
  create(
      jlm::rvsdg::output & predicate,
      const std::vector<jlm::rvsdg::output *> & alternatives)
  {
    if (alternatives.empty())
      throw util::error("Insufficient number of operands.");
    auto ctl = dynamic_cast<const jlm::rvsdg::ctltype *>(&predicate.type());
    if (!ctl)
      throw util::error("Predicate needs to be a ctltype.");
    if (alternatives.size() != ctl->nalternatives())
      throw util::error("Alternatives and predicate do not match.");
    
    auto region = predicate.region();
    auto operands = std::vector<jlm::rvsdg::output *>();
    operands.push_back(&predicate);
    operands.insert(operands.end(), alternatives.begin(), alternatives.end());
    mux_op op(alternatives.size(), alternatives.front()->type());
    return jlm::rvsdg::simple_node::create_normalized(region, op, operands);
  }

private:
  static std::vector<jlm::rvsdg::port>
  create_portvector(size_t nalternatives, const jlm::rvsdg::type & type)
  {
    auto vec = std::vector<jlm::rvsdg::port>(nalternatives + 1, type);
    vec[0] = jlm::rvsdg::ctltype(nalternatives);
    return vec;
  }
};

class reg_op final : public jlm::rvsdg::simple_op
{
public:
  virtual ~reg_op()
  {}

  reg_op(const jlm::rvsdg::type & type)
      : jlm::rvsdg::simple_op(std::vector<jlm::rvsdg::port>(1, type), { type })
  {}

  std::string
  debug_string() const override
  {
    return "SHLS_REG";
  }

  bool
  operator==(const jlm::rvsdg::operation & other) const noexcept override
  {
    return true; //TODO check if that's how to do it
  }

  std::unique_ptr<jlm::rvsdg::operation>
  copy() const override
  {
    return std::unique_ptr<jlm::rvsdg::operation>(new reg_op(*this));
  }

  static jlm::rvsdg::output *
  create(
      jlm::rvsdg::output & input)
  {
    reg_op op(input.type());
    return jlm::rvsdg::simple_node::create_normalized(input.region(), op, { &input })[0];
  }

private:
  static std::vector<jlm::rvsdg::port>
  create_portvector(size_t nalternatives, const jlm::rvsdg::type & type)
  {
    auto vec = std::vector<jlm::rvsdg::port>(nalternatives + 1, type);
    vec[0] = jlm::rvsdg::ctltype(nalternatives);
    return vec;
  }
};

class loop_op final : public jlm::rvsdg::structural_op
{
public:
  virtual ~loop_op() noexcept
  {}

  std::string
  debug_string() const override
  {
    return "SHLS_LOOP";
  }

  std::unique_ptr<jlm::rvsdg::operation>
  copy() const override
  {
    return std::unique_ptr<jlm::rvsdg::operation>(new loop_op(*this));
  }

private:
  std::vector<jlm::rvsdg::output *> inputs_regs_;
};

class loop_node final : public jlm::rvsdg::structural_node
{
public:
  virtual ~loop_node()
  {}

private:
  inline loop_node(jlm::rvsdg::region * parent)
      : structural_node(loop_op(), parent, 2)
  {}

public:
  static loop_node *
  create(jlm::rvsdg::region * parent);

  jlm::rvsdg::structural_output *
  add_loopvar(jlm::rvsdg::output * origin);
  
  inline jlm::rvsdg::region *
  control_subregion() const noexcept
  {
    return structural_node::subregion(0);
  }

  inline jlm::rvsdg::region *
  compute_subregion() const noexcept
  {
    return structural_node::subregion(1);
  }

  virtual loop_node *
  copy(jlm::rvsdg::region * region, jlm::rvsdg::substitution_map & smap) const override;

};

} // namespace jlm::static_hls