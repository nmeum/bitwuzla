#include "check/check_model.h"

#include <unordered_set>

#include "node/node_manager.h"

namespace bzla::check {

using namespace node;

CheckModel::CheckModel(SolvingContext& ctx)
    : d_ctx(ctx), d_logger(ctx.env().logger())
{
}

bool
CheckModel::check()
{
  if (!d_ctx.options().dbg_check_model())
  {
    return true;
  }

  Log(1) << "\n*** check model\n";

  option::Options opts;
  opts.dbg_check_model.set(false);
  SolvingContext check_ctx(opts);
  for (const Node& assertion : d_ctx.original_assertions())
  {
    check_ctx.assert_formula(assertion);
  }

  collect_consts();
  NodeManager& nm = NodeManager::get();
  for (const Node& input : d_consts)
  {
    Node value = d_ctx.get_value(input);
    // Special handling until equality over constant arrays supported
    if (input.type().is_array())
    {
      assert_array_model(check_ctx, input, value);
    }
    // Special handling until equality over lambda supported
    else if (input.type().is_fun())
    {
      assert_fun_model(check_ctx, input, value);
    }
    else
    {
      check_ctx.assert_formula(nm.mk_node(Kind::EQUAL, {input, value}));
    }
    Log(2) << "check: " << input << " = " << value;
  }

  return check_ctx.solve() == Result::SAT;
}

void
CheckModel::collect_consts()
{
  std::unordered_set<Node> cache;
  std::vector<Node> visit;
  for (const Node& assertion : d_ctx.original_assertions())
  {
    visit.push_back(assertion);
    do
    {
      Node cur = visit.back();
      visit.pop_back();
      if (cache.insert(cur).second)
      {
        if (cur.is_const())
        {
          d_consts.push_back(cur);
        }
        visit.insert(visit.end(), cur.begin(), cur.end());
      }
    } while (!visit.empty());
  }

  // Special handling until equality over lambdas supported
  cache.clear();
  for (const Node& assertion : d_ctx.original_assertions())
  {
    visit.push_back(assertion);
    do
    {
      Node cur = visit.back();
      visit.pop_back();
      if (cache.insert(cur).second)
      {
        if (cur.kind() == Kind::APPLY && cur[0].is_const())
        {
          d_fun_apps[cur[0]].push_back(cur);
        }
        // Do not collect applications below quantifiers.
        else if (cur.kind() == Kind::FORALL || cur.kind() == Kind::EXISTS)
        {
          continue;
        }
        visit.insert(visit.end(), cur.begin(), cur.end());
      }
    } while (!visit.empty());
  }
}

void
CheckModel::assert_array_model(SolvingContext& ctx,
                               const Node& input,
                               const Node& value) const
{
  NodeManager& nm = NodeManager::get();
  Node cur        = value;
  while (cur.kind() == Kind::STORE)
  {
    // Special handling until equality over constant arrays supported
    if (!cur[2].type().is_array())
    {
      Node read = nm.mk_node(Kind::SELECT, {input, cur[1]});
      ctx.assert_formula(nm.mk_node(Kind::EQUAL, {read, cur[2]}));
    }
    cur = cur[0];
  }
}

void
CheckModel::assert_fun_model(SolvingContext& ctx,
                             const Node& input,
                             const Node& value) const
{
  auto it = d_fun_apps.find(input);
  if (it == d_fun_apps.end())
  {
    return;
  }
  NodeManager& nm = NodeManager::get();
  for (const Node& app : it->second)
  {
    std::vector<Node> args;
    args.push_back(value);
    args.insert(args.end(), app.begin() + 1, app.end());
    Node val_app = nm.mk_node(Kind::APPLY, args);
    ctx.assert_formula(nm.mk_node(Kind::EQUAL, {app, val_app}));
  }
}

}  // namespace bzla::check