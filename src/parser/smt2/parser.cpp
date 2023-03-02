#include "parser/smt2/parser.h"

#include <algorithm>
#include <iostream>

#include "bitwuzla/cpp/bitwuzla.h"

namespace bzla {
namespace parser::smt2 {

/* Parser public ------------------------------------------------------------ */

Parser::Parser(bitwuzla::Options& options,
               std::istream* infile,
               const std::string& infile_name,
               uint64_t log_level,
               uint64_t verbosity)
    : d_options(options),
      d_lexer(new Lexer(infile)),
      d_infile_name(infile_name),
      d_logger(log_level, verbosity),
      d_log_level(log_level),
      d_verbosity(verbosity)
{
  d_token_class_mask = static_cast<uint32_t>(TokenClass::COMMAND)
                       | static_cast<uint32_t>(TokenClass::CORE)
                       | static_cast<uint32_t>(TokenClass::KEYWORD)
                       | static_cast<uint32_t>(TokenClass::RESERVED);
}

std::string
Parser::parse()
{
  util::Timer timer(d_statistics.time_parse);

  while (parse_command() && !d_done && !terminate())
    ;

  // this should be in parser, and parser has to store the msg of lexer
  if (d_lexer->error()) return d_lexer->error_msg();

  if (!terminate() && d_verbosity)
  {
    if (d_statistics.num_commands == 0)
    {
      Msg(1) << "warning: no commands in '" << d_infile_name << "'";
    }
    else
    {
      if (d_statistics.num_set_logic == 0)
      {
        Msg(1) << "warning: no 'set-logic' command in '" << d_infile_name
               << "'";
      }
      if (d_statistics.num_assertions == 0)
      {
        Msg(1) << "warning: no 'assert' command in '" << d_infile_name << "'";
      }
      if (d_statistics.num_check_sat == 0)
      {
        Msg(1) << "warning: no 'check-sat' command in '" << d_infile_name
               << "'";
      }
      if (d_statistics.num_exit == 0)
      {
        Msg(1) << "warning: no 'exit' command in '" << d_infile_name << "'";
      }
    }
  }

  Msg(1) << "parsed " << d_statistics.num_commands << " in "
         << d_statistics.time_parse.elapsed() << " seconds";
}

void
Parser::configure_terminator(bitwuzla::Terminator* terminator)
{
  if (d_bitwuzla)
  {
    d_bitwuzla->configure_terminator(terminator);
  }
  d_terminator = terminator;
}

/* Parser private ----------------------------------------------------------- */

bool
Parser::terminate()
{
  return d_terminator != nullptr && d_terminator->terminate();
}

Token
Parser::next_token()
{
  assert(d_lexer);
  Token token = d_lexer->next_token();
  if (token == Token::SYMBOL || token == Token::ATTRIBUTE)
  {
    assert(d_lexer->has_token());
    SymbolTable::Node* node = d_table.find(d_lexer->token());
    if (!node)
    {
      node = d_table.insert(token, d_lexer->token());
    }
    d_last_node = node;
  }
  return token;
}

bool
Parser::parse_command()
{
  Token token = next_token();

  if (!check_token(token))
  {
    return false;
  }

  if (token != Token::LPAR)
  {
    assert(d_lexer->has_token());
    return error("expected '(' at '" + d_lexer->token() + "'");
  }

  token = next_token();
  if (!check_token(token))
  {
    return false;
  }
  if (!is_token_class(token, TokenClass::COMMAND))
  {
    assert(d_lexer->has_token());
    return error("expected command at '" + d_lexer->token() + "'");
  }

  switch (token)
  {
    case Token::ASSERT: return parse_command_assert();
    case Token::CHECK_SAT: return parse_command_check_sat();
    case Token::CHECK_SAT_ASSUMING: return parse_command_check_sat_assuming();
    case Token::DECLARE_CONST: return parse_command_declare_const();
    case Token::DECLARE_SORT: return parse_command_declare_sort();
    case Token::DECLARE_FUN: return parse_command_declare_fun();
    case Token::DEFINE_FUN: return parse_command_define_fun();
    case Token::DEFINE_SORT: return parse_command_define_sort();
    case Token::ECHO: return parse_command_echo();
    case Token::EXIT: return parse_command_exit();
    case Token::GET_MODEL: return parse_command_get_model();
    case Token::GET_UNSAT_ASSUMPTIONS:
      return parse_command_get_unsat_assumptions();
    case Token::GET_UNSAT_CORE: return parse_command_get_unsat_core();
    case Token::GET_VALUE: return parse_command_get_value();
    case Token::POP: return parse_command_pop();
    case Token::PUSH: return parse_command_push();
    case Token::SET_INFO: return parse_command_set_info();
    case Token::SET_LOGIC: return parse_command_set_logic();
    case Token::SET_OPTION: return parse_command_set_option();

    default:
      assert(d_lexer->has_token());
      return error("unsupported command '" + d_lexer->token() + "'");
  }

  d_statistics.num_commands += 1;
  return true;
}

bool
Parser::parse_command_assert()
{
  //    case BZLA_ASSERT_TAG_SMT2:
  //      if (!parse_term_smt2(parser, &exp, &coo)) return 0;
  //      assert(!parser->error);
  //      if (!bitwuzla_term_is_bool(exp))
  //      {
  //        parser->perrcoo = coo;
  //        return !perr_smt2(parser, "assert argument is not a formula");
  //      }
  //      if (!read_rpar_smt2(parser, " after asserted expression"))
  //      {
  //        return 0;
  //      }
  //      bitwuzla_assert(get_bitwuzla(parser), exp);
  //      assert(!parser->error);
  //      parser->commands.asserts++;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_check_sat()
{
  //    case BZLA_CHECK_SAT_TAG_SMT2:
  //      configure_smt_comp_mode(parser);
  //      if (!read_rpar_smt2(parser, " after 'check-sat'")) return 0;
  //      if (!check_sat(parser, 0, NULL)) return 0;
  //      break;
}

bool
Parser::parse_command_check_sat_assuming()
{
  //    case BZLA_CHECK_SAT_ASSUMING_TAG_SMT2:
  //      configure_smt_comp_mode(parser);
  //      if (!read_lpar_smt2(parser, " after 'check-sat-assuming'")) return 0;
  //      if (false
  //          && !bitwuzla_get_option(parser->options,
  //          BITWUZLA_OPT_INCREMENTAL))
  //        return !perr_smt2(parser, "incremental solving is not enabled");
  //      if (!read_exp_list(parser, &exps, &coo))
  //      {
  //        BZLA_RELEASE_STACK(exps);
  //        return 0;
  //      }
  //      for (i = 0; i < BZLA_COUNT_STACK(exps); i++)
  //      {
  //        exp = BZLA_PEEK_STACK(exps, i);
  //        if (bitwuzla_term_is_array(exp))
  //        {
  //          parser->perrcoo = coo;
  //          BZLA_RELEASE_STACK(exps);
  //          return !perr_smt2(
  //              parser, "assumption argument is an array and not a formula");
  //        }
  //      }
  //      if (!read_rpar_smt2(parser, " after 'check-sat-assuming'"))
  //      {
  //        BZLA_RELEASE_STACK(exps);
  //        return 0;
  //      }
  //      if (!check_sat(parser, BZLA_COUNT_STACK(exps), exps.start)) return 0;
  //      for (i = 0; i < BZLA_COUNT_STACK(exps); i++)
  //      {
  //        exp = BZLA_PEEK_STACK(exps, i);
  //        BZLA_PUSH_STACK(parser->sat_assuming_assumptions, exp);
  //      }
  //      BZLA_RELEASE_STACK(exps);
  //      BZLA_RESET_STACK(parser->tokens);
  //      break;
}

bool
Parser::parse_command_declare_const()
{
  //    case BZLA_DECLARE_CONST_TAG_SMT2:
  //      if (!declare_fun_smt2(parser, true)) return 0;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_declare_fun()
{
  //    case BZLA_DECLARE_FUN_TAG_SMT2:
  //      if (!declare_fun_smt2(parser, false)) return 0;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_declare_sort()
{
  //    case BZLA_DECLARE_SORT_TAG_SMT2:
  //      configure_smt_comp_mode(parser);
  //      if (!declare_sort_smt2(parser)) return 0;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_define_fun()
{
  //    case BZLA_DEFINE_FUN_TAG_SMT2:
  //      if (!define_fun_smt2(parser)) return 0;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_define_sort()
{
  //    case BZLA_DEFINE_SORT_TAG_SMT2:
  //      if (!define_sort_smt2(parser)) return 0;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_echo()
{
  //    case BZLA_ECHO_TAG_SMT2:
  //      if (!echo_smt2(parser)) return 0;
  //      break;
}

bool
Parser::parse_command_exit()
{
  //    case BZLA_EXIT_TAG_SMT2:
  //      if (!read_rpar_smt2(parser, " after 'exit'")) return 0;
  //      assert(!parser->commands.exits);
  //      parser->commands.exits++;
  //      parser->done = true;
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_get_model()
{
  //    case BZLA_GET_MODEL_TAG_SMT2:
  //      if (!read_rpar_smt2(parser, " after 'get-model'")) return 0;
  //      if (!bitwuzla_get_option(parser->options,
  //      BITWUZLA_OPT_PRODUCE_MODELS))
  //        return !perr_smt2(parser, "model generation is not enabled");
  //      if (parser->res->result != BITWUZLA_SAT) break;
  //      // if (bitwuzla_get_option(parser->options,
  //      BITWUZLA_OPT_OUTPUT_FORMAT)
  //      //     == BZLA_OUTPUT_FORMAT_BTOR)
  //      //{
  //      //   // bitwuzla_print_model(bitwuzla, "btor", parser->outfile);
  //      // }
  //      // else
  //      //{
  //      //   // bitwuzla_print_model(bitwuzla, "smt2", parser->outfile);
  //      // }
  //      fflush(parser->outfile);
  //      break;
}

bool
Parser::parse_command_get_unsat_assumptions()
{
  //    case BZLA_GET_UNSAT_ASSUMPTIONS_TAG_SMT2: {
  //      if (!read_rpar_smt2(parser, " after 'get-unsat-assumptions'")) return
  //      0; if (parser->res->result != BITWUZLA_UNSAT) break; fputc('(',
  //      parser->outfile); size_t size; failed_assumptions =
  //          bitwuzla_get_unsat_assumptions(get_bitwuzla(parser), &size);
  //      for (i = 0; i < size; ++i)
  //      {
  //        if (i > 0) fputc(' ', parser->outfile);
  //        const char *symbol =
  //        bitwuzla_term_get_symbol(failed_assumptions[i]); if (symbol)
  //        {
  //          fprintf(parser->outfile, "%s", symbol);
  //        }
  //        else
  //        {
  //          bitwuzla_term_dump(failed_assumptions[i], "smt2",
  //          parser->outfile);
  //        }
  //      }
  //      failed_assumptions = 0;
  //      fputs(")\n", parser->outfile);
  //      fflush(parser->outfile);
  //      break;
  //    }
}

bool
Parser::parse_command_get_unsat_core()
{
  //    case BZLA_GET_UNSAT_CORE_TAG_SMT2: {
  //      if (!read_rpar_smt2(parser, " after 'get-unsat-assumptions'")) return
  //      0; if (parser->res->result != BITWUZLA_UNSAT) break; fputc('(',
  //      parser->outfile); size_t size; unsat_core =
  //      bitwuzla_get_unsat_core(get_bitwuzla(parser), &size); const char *sym;
  //      bool printed_first = false;
  //      for (i = 0; i < size; ++i)
  //      {
  //        sym = bitwuzla_term_get_symbol(unsat_core[i]);
  //        if (!sym) continue;
  //        if (printed_first)
  //        {
  //          fputc(' ', parser->outfile);
  //        }
  //        fprintf(parser->outfile, "%s", sym);
  //        printed_first = true;
  //      }
  //      unsat_core = 0;
  //      fputs(")\n", parser->outfile);
  //      fflush(parser->outfile);
  //      break;
  //    }
}

bool
Parser::parse_command_get_value()
{
  //    case BZLA_GET_VALUE_TAG_SMT2: {
  //      if (!read_lpar_smt2(parser, " after 'get-value'")) return 0;
  //      if (!bitwuzla_get_option(parser->options,
  //      BITWUZLA_OPT_PRODUCE_MODELS))
  //      {
  //        return !perr_smt2(parser, "model generation is not enabled");
  //      }
  //      if (parser->res->result != BITWUZLA_SAT) break;
  //      if (!read_exp_list(parser, &exps, &coo))
  //      {
  //        BZLA_RELEASE_STACK(exps);
  //        return 0;
  //      }
  //      if (!read_rpar_smt2(parser, " after 'get-value'"))
  //      {
  //        BZLA_RELEASE_STACK(exps);
  //        return 0;
  //      }
  //      fputc('(', parser->outfile);
  //      char *symbols = parser->tokens.start;
  //      for (i = 0; i < BZLA_COUNT_STACK(exps); i++)
  //      {
  //        if (BZLA_COUNT_STACK(exps) > 1) fputs("\n ", parser->outfile);
  //        exp = BZLA_PEEK_STACK(exps, i);
  //        bitwuzla_term_print_value_smt2(
  //            parser->bitwuzla, exp, symbols, parser->outfile);
  //        symbols += strlen(symbols) + 1;
  //        assert(symbols <= parser->tokens.top);
  //      }
  //      if (BZLA_COUNT_STACK(exps) > 1) fputc('\n', parser->outfile);
  //      fprintf(parser->outfile, ")\n");
  //      fflush(parser->outfile);
  //      BZLA_RELEASE_STACK(exps);
  //      BZLA_RESET_STACK(parser->tokens);
  //      break;
  //    }
}

bool
Parser::parse_command_pop()
{
  //    case BZLA_POP_TAG_SMT2:
  //      (void) parse_uint64_smt2(parser, true, &level);
  //      if (!read_rpar_smt2(parser, " after 'pop'")) return 0;
  //      if (level > parser->scope_level)
  //      {
  //        return !perr_smt2(parser,
  //                          "popping more scopes (%u) than created via push
  //                          (%u)", level, parser->scope_level);
  //      }
  //      for (i = 0; i < level; i++) close_current_scope(parser);
  //      bitwuzla_pop(get_bitwuzla(parser), level);
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_push()
{
  //    case BZLA_PUSH_TAG_SMT2:
  //      level = 0;
  //      (void) parse_uint64_smt2(parser, true, &level);
  //      if (!read_rpar_smt2(parser, " after 'push'")) return 0;
  //      for (i = 0; i < level; i++) open_new_scope(parser);
  //      configure_smt_comp_mode(parser);
  //      bitwuzla_push(get_bitwuzla(parser), level);
  //      print_success(parser);
  //      break;
}

bool
Parser::parse_command_set_info()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }
  if (token != Token::ATTRIBUTE)
  {
    return error("missing keyword after 'set-info'");
  }
  if (token == Token::STATUS)
  {
    token = next_token();
    if (!check_token(token))
    {
      return false;
    }
    if (token != Token::SYMBOL)
    {
      return error("invalid value for ':status'");
    }
    assert(d_lexer->has_token());
    const std::string& status = d_lexer->token();
    if (status == "sat")
    {
      d_status = bitwuzla::Result::SAT;
    }
    else if (status == "unsat")
    {
      d_status = bitwuzla::Result::UNSAT;
    }
    else if (status == "unknown")
    {
      d_status = bitwuzla::Result::UNKNOWN;
    }
    else
    {
      return error("invalid value '" + status + "' for ':status'");
    }
    Msg(1) << "parsed status '" << d_status << "'";
  }
  if (parse_rpars(1))
  {
    print_success();
    return true;
  }
  return false;
}

bool
Parser::parse_command_set_logic()
{
  if (!parse_symbol(" after 'set-logic'"))
  {
    return false;
  }
  SymbolTable::Node* logic = pop_node_arg();
  assert(logic->has_symbol());
  d_logic = logic->d_symbol;
  if (!is_supported_logic(d_logic))
  {
    return error("unsupported logic '" + d_logic + "'");
  }
  Msg(1) << "logic " << d_logic;
  if (parse_rpars(1))
  {
    if (d_statistics.num_set_logic++)
    {
      Msg(1) << "warning: additional 'set-logic' command";
    }
    print_success();
    return true;
  }
  return false;
}

bool
Parser::parse_command_set_option()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }
  if (token == Token::RPAR)
  {
    return error("missing keyword after 'set-option'");
  }

  if (token == Token::REGULAR_OUTPUT_CHANNEL)
  {
    token = next_token();
    if (!check_token(token))
    {
      return false;
    }
    const std::string& outfile_name = d_lexer->token();
    assert(!outfile_name.empty());
    try
    {
      d_outfile.open(outfile_name, std::ofstream::out);
    }
    catch (...)
    {
      return error("cannot create '" + outfile_name + "'");
    }
    d_out = &d_outfile;
  }
  else if (token == Token::PRINT_SUCCESS)
  {
    token = next_token();
    if (!check_token(token))
    {
      return false;
    }
    if (token == Token::TRUE)
    {
      d_print_success = true;
      // configure_smt_comp_mode();
    }
    else if (token == Token::FALSE)
    {
      d_print_success = false;
    }
    else
    {
      assert(d_lexer->has_token());
      return error("expected Boolean argument at '" + d_lexer->token() + "'");
    }
  }
  else if (token == Token::GLOBAL_DECLARATIONS)
  {
    token = next_token();
    if (!check_token(token))
    {
      return false;
    }
    if (token == Token::TRUE)
    {
      d_global_decl = true;
    }
    else if (token == Token::FALSE)
    {
      d_global_decl = false;
    }
    else
    {
      assert(d_lexer->has_token());
      return error("expected Boolean argument at '" + d_lexer->token() + "'");
    }
  }
  else if (token == Token::PRODUCE_UNSAT_ASSUMPTIONS)
  {
    // nothing to do, always true
  }
  // Bitwuzla options
  else
  {
    assert(d_lexer->has_token());
    std::string opt = d_lexer->token();
    token           = next_token();
    if (!check_token(token))
    {
      return false;
    }
    assert(d_lexer->has_token());
    try
    {
      d_options.set(opt, d_lexer->token());
    }
    catch (bitwuzla::Exception& e)
    {
      return error(e.msg());
    }
  }
  if (parse_rpars(1))
  {
    print_success();
    return true;
  }
  return false;
}

/* -------------------------------------------------------------------------- */

bool
Parser::parse_lpars(uint64_t nlpars)
{
  while (nlpars > 0)
  {
    Token token = next_token();
    if (token != Token::LPAR)
    {
      return error("missing '('");
    }
    nlpars -= 1;
  }
  return true;
}

bool
Parser::parse_rpars(uint64_t nrpars)
{
  while (nrpars > 0)
  {
    Token token = next_token();
    if (token == Token::ENDOFFILE)
    {
      if (nrpars > 0)
      {
        return error("missing ')' at end of file");
      }
      return true;
    }
    if (token != Token::RPAR)
    {
      return error("missing ')'");
    }
    nrpars -= 1;
  }
  return true;
}

bool
Parser::parse_uint64()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }
  if (token == Token::DECIMAL_VALUE)
  {
    assert(d_lexer->has_token());
    try
    {
      uint64_t val = std::stoll(d_lexer->token());
      d_work_args.push_back(val);
      return true;
    }
    catch (...)
    {
      return error("invalid 64 bit integer '" + d_lexer->token() + "'");
    }
  }
  return error("expected decimal value");
}

bool
Parser::parse_symbol(const std::string& error_msg, bool shadow)
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }
  if (token != Token::SYMBOL)
  {
    return error("expected symbol" + error_msg);
  }
  assert(d_last_node->d_token == Token::SYMBOL);
  // shadow previously defined symbols
  if (shadow && d_last_node->d_coo.line)
  {
    d_last_node        = d_table.insert(token, d_lexer->token());
    d_last_node->d_coo = d_lexer->coo();
  }
  d_work_args.push_back(d_last_node);
  return true;
}

/* -------------------------------------------------------------------------- */

bool
Parser::parse_term(bool look_ahead, Token la_char)
{
  /* Note: we need look ahead and tokens string only for get-value
   *       (for parsing a term list and printing the originally parsed,
   *       non-simplified expression) */
  Token token;
  bitwuzla::Term res;
  d_term_open = 0;

  do
  {
    if (look_ahead)
    {
      token      = la_char;
      look_ahead = false;
    }
    else
    {
      token = next_token();
    }
    if (!check_token(token))
    {
      return false;
    }

    if (token == Token::RPAR)
    {
      if (!close_term(token))
      {
        return false;
      }
    }
    else
    {
      if (!parse_open_term(token))
      {
        return false;
      }
    }
  } while (d_term_open > 0);
  assert(d_work_args.size() > 0);
  return true;
}

bool
Parser::parse_open_term(Token token)
{
  d_expect_body = false;

  if (token == Token::LPAR)
  {
    open_term_scope();
    if (d_is_var_binding)
    {
      d_work.emplace_back(Token::LETBIND, d_lexer->coo());
      d_is_var_binding          = false;
      if (!parse_symbol("", true))
      {
        return false;
      }
    }
    else if (d_is_sorted_var)
    {
      // parse <sorted_var>: <symbol> <sort>
      d_work.emplace_back(Token::SORTED_VAR, d_lexer->coo());
      d_is_sorted_var           = false;
      if (!parse_symbol(" in sorted var", true))
      {
        return false;
      }
      if (!parse_sort())
      {
        return false;
      }
      bitwuzla::Sort sort = pop_sort_arg();
      SymbolTable::Node* symbol = pop_node_arg();
      assert(symbol->has_symbol());
      d_work_args.push_back(bitwuzla::mk_var(sort, symbol->d_symbol));
    }
  }
  else if (d_is_var_binding)
  {
    return error("expected var binding");
  }
  else if (d_is_sorted_var)
  {
    return error("expected sorted variable");
  }
  else if (is_symbol(token))
  {
    d_work.emplace_back(token, d_last_node, d_lexer->coo());
    if (!parse_open_term_symbol())
    {
      return false;
    }
  }
  else if (token == Token::NAMED)
  {
    if (!parse_symbol(" in sorted var", true))
    {
      return false;
    }
    SymbolTable::Node* symbol = pop_node_arg();
    assert(symbol->has_symbol());
    if (symbol->d_coo.line)
    {
      return error("symbol '" + symbol->d_symbol + "' already defined at line "
                   + std::to_string(symbol->d_coo.line) + " column "
                   + std::to_string(symbol->d_coo.col));
    }
    symbol->d_coo = d_lexer->coo();
    d_work.emplace_back(Token::SYMBOL, symbol, d_lexer->coo());
  }
  else if (token == Token::BINARY_VALUE)
  {
    assert(d_lexer->has_token());
    std::string val     = d_lexer->token().substr(2);
    bitwuzla::Sort sort = bitwuzla::mk_bv_sort(val.size());
    d_work_args.push_back(bitwuzla::mk_bv_value(sort, val));
  }
  else if (token == Token::HEXADECIMAL_VALUE)
  {
    assert(d_lexer->has_token());
    std::string val     = d_lexer->token().substr(2);
    bitwuzla::Sort sort = bitwuzla::mk_bv_sort(val.size() * 4);
    d_work_args.push_back(bitwuzla::mk_bv_value(sort, val, 16));
  }
  else if (token == Token::DECIMAL_VALUE || token == Token::REAL_VALUE)
  {
    assert(d_lexer->has_token());
    d_work_args.push_back(d_lexer->token().empty());
  }
  else
  {
    return error("unexpected token");
  }
  return true;
}

bool
Parser::parse_open_term_as()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }

  if (token != Token::SYMBOL)
  {
    return error("expected identifier");
  }

  SymbolTable::Node* node = d_last_node;
  assert(node);
  assert(node->has_symbol());
  const std::string& iden = node->d_symbol;

  if (iden == "const" || iden == "const-array")
  {
    if (!parse_sort())
    {
      return false;
    }
    const bitwuzla::Sort& sort = peek_sort_arg();
    if (!sort.is_array())
    {
      return error("expected array sort");
    }
    if (!parse_rpars(1))
    {
      return false;
    }
    assert(d_work_args.size() - d_work_args_control.back() == 1);
    close_term_scope();
    return true;
  }
  return error("invalid identifier '" + iden + "'");
}

bool
Parser::parse_open_term_indexed()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }

  Lexer::Coordinate coo   = d_lexer->coo();
  Token token_kind        = token;
  bitwuzla::Kind kind     = bitwuzla::Kind::VALUE;
  SymbolTable::Node* node = d_last_node;

  bool allow_zero = false;
  uint64_t nidxs  = 1;

  switch (token)
  {
    case Token::BV_REPEAT:
      kind       = bitwuzla::Kind::BV_REPEAT;
      allow_zero = true;
      break;

    case Token::BV_ROTATE_LEFT:
      kind       = bitwuzla::Kind::BV_ROLI;
      allow_zero = true;
      break;

    case Token::BV_ROTATE_RIGHT:
      kind       = bitwuzla::Kind::BV_RORI;
      allow_zero = true;
      break;

    case Token::BV_SIGN_EXTEND:
      kind       = bitwuzla::Kind::BV_SIGN_EXTEND;
      allow_zero = true;
      break;

    case Token::BV_ZERO_EXTEND:
      kind       = bitwuzla::Kind::BV_ZERO_EXTEND;
      allow_zero = true;
      break;

    case Token::BV_EXTRACT:
      kind  = bitwuzla::Kind::BV_EXTRACT;
      nidxs = 2;
      break;

    case Token::FP_NAN:
    case Token::FP_NEG_INF:
    case Token::FP_NEG_ZERO:
    case Token::FP_POS_INF:
    case Token::FP_POS_ZERO:
    case Token::FP_TO_FP:
    case Token::FP_TO_FP_UNSIGNED: nidxs = 2; break;

    case Token::FP_TO_SBV: kind = bitwuzla::Kind::FP_TO_SBV; break;

    case Token::FP_TO_UBV: kind = bitwuzla::Kind::FP_TO_UBV; break;

    case Token::SYMBOL: {
      assert(d_lexer->has_token());
      const std::string& val = d_lexer->token();
      if (val[0] != 'b' || val[1] != 'v')
      {
        return false;
      }
      std::string v = val.substr(2);
      if (!std::all_of(
              v.begin(), v.end(), [](char c) { return std::isdigit(c); }))
      {
        return error("invalid bit-vector value '" + val + "'");
      }
      d_work_args.push_back(v);
    }
    break;

    default: assert(false);
  }

  for (uint64_t i = 0; i < nidxs; ++i)
  {
    if (!parse_uint64())
    {
      return false;
    }
    if (!allow_zero && peek_uint64_arg() == 0)
    {
      return error("expected non-zero index");
    }
  }

  if (!parse_rpars(1))
  {
    return false;
  }

  if (kind == bitwuzla::Kind::VALUE)
  {
#ifndef NDEBUG
    size_t nargs = d_work_args.size() - d_work_args_control.back();
#endif
    close_term_scope();
    switch (token_kind)
    {
      case Token::FP_NAN:
      case Token::FP_NEG_INF:
      case Token::FP_NEG_ZERO:
      case Token::FP_POS_INF:
      case Token::FP_POS_ZERO: {
        assert(nargs == 2);
        uint64_t ssize      = pop_uint64_arg();
        uint64_t esize      = pop_uint64_arg();
        bitwuzla::Sort sort = bitwuzla::mk_fp_sort(esize, ssize);
        if (token_kind == Token::FP_NAN)
        {
          d_work_args.push_back(bitwuzla::mk_fp_nan(sort));
        }
        else if (token_kind == Token::FP_NEG_INF)
        {
          d_work_args.push_back(bitwuzla::mk_fp_neg_inf(sort));
        }
        else if (token_kind == Token::FP_POS_INF)
        {
          d_work_args.push_back(bitwuzla::mk_fp_pos_inf(sort));
        }
        else if (token_kind == Token::FP_NEG_ZERO)
        {
          d_work_args.push_back(bitwuzla::mk_fp_neg_zero(sort));
        }
        else if (token_kind == Token::FP_POS_ZERO)
        {
          d_work_args.push_back(bitwuzla::mk_fp_pos_zero(sort));
        }
      }
      break;

      default: {
        assert(token_kind == Token::SYMBOL);
        assert(nargs == 2);
        uint64_t size       = pop_uint64_arg();
        std::string val     = pop_str_arg();
        bitwuzla::Sort sort = bitwuzla::mk_bv_sort(size);
        d_work_args.push_back(bitwuzla::mk_bv_value(sort, val, 10));
      }
    }
  }
  else
  {
    // ((_ <indexed_op> <idxs) <terms>) -> (<indexed_op> <idxs> <terms>)
    close_term_scope();
    assert(node);
    d_work.emplace_back(token_kind, node, std::move(coo));
  }
  return true;
}

bool
Parser::parse_open_term_quant()
{
  if (!parse_lpars(1))
  {
    return false;
  }
  open_term_scope();
  d_work.emplace_back(Token::SORTED_VARS, d_lexer->coo());
  assert(!d_is_sorted_var);
  d_is_sorted_var = true;
  return true;
}

bool
Parser::parse_open_term_symbol()
{
  ParsedItem& cur         = d_work.back();
  SymbolTable::Node* node = std::get<SymbolTable::Node*>(cur.d_parsed);
  Token token             = node->d_token;

  if (is_token_class(token, TokenClass::COMMAND))
  {
    return error("unexpected command '" + node->d_symbol + "'");
  }
  if (is_token_class(token, TokenClass::KEYWORD))
  {
    return error("unexpected keyword '" + node->d_symbol + "'");
  }
  if (is_token_class(token, TokenClass::RESERVED))
  {
    if (token == Token::LET)
    {
      if (!parse_lpars(1))
      {
        return false;
      }
      open_term_scope();
      d_work.emplace_back(Token::PARLETBIND, d_lexer->coo());
      assert(!d_is_var_binding);
      d_is_var_binding = true;
    }
    else if (token == Token::FORALL || token == Token::EXISTS)
    {
      if (!parse_open_term_quant())
      {
        return false;
      }
    }
    else if (token == Token::UNDERSCORE)
    {
      if (!parse_open_term_indexed())
      {
        return false;
      }
    }
    else if (token == Token::AS)
    {
      if (!parse_open_term_as())
      {
        return false;
      }
    }
    else if (token != Token::BANG)
    {
      assert(node->has_symbol());
      return error("unsupported reserved word '" + node->d_symbol + "'");
    }
  }
  else if (token == Token::SYMBOL)
  {
    if (node->d_term.is_null())
    {
      assert(node->has_symbol());
      return error("undefined symbol '" + node->d_symbol + "'");
    }
    cur.d_token = Token::EXP;
  }
  else if (token == Token::TRUE)
  {
    d_work.pop_back();
    d_work_args.push_back(bitwuzla::mk_true());
  }
  else if (token == Token::FALSE)
  {
    d_work.pop_back();
    d_work_args.push_back(bitwuzla::mk_false());
  }
  else if (token == Token::ATTRIBUTE)
  {
    return error("unexpected attribute '" + std::to_string(token) + "'");
  }
  else if (is_token_class(token, TokenClass::CORE))
  {
    if (token == Token::BOOL)
    {
      return error("unexpected '" + std::to_string(token) + "'");
    }
  }
  else if (d_arrays_enabled && is_token_class(token, TokenClass::ARRAY))
  {
    if (token == Token::ARRAY)
    {
      return error("unexpected '" + std::to_string(token) + "'");
    }
  }
  else if (d_bv_enabled && is_token_class(token, TokenClass::BV))
  {
    if (token == Token::BV_BITVEC)
    {
      return error("unexpected '" + std::to_string(token) + "'");
    }
  }
  else if (d_fp_enabled && is_token_class(token, TokenClass::FP))
  {
    if (token == Token::FP_FLOATINGPOINT || token == Token::FP_FLOAT16
        || token == Token::FP_FLOAT32 || token == Token::FP_FLOAT64
        || token == Token::FP_FLOAT128)
    {
      return error("unexpected '" + std::to_string(token) + "'");
    }

    if (token == Token::FP_RM_RNA_LONG || token == Token::FP_RM_RNA)
    {
      d_work.pop_back();
      d_work_args.push_back(bitwuzla::mk_rm_value(bitwuzla::RoundingMode::RNA));
    }
    else if (token == Token::FP_RM_RNE_LONG || token == Token::FP_RM_RNE)
    {
      d_work.pop_back();
      d_work_args.push_back(bitwuzla::mk_rm_value(bitwuzla::RoundingMode::RNE));
    }
    else if (token == Token::FP_RM_RTN_LONG || token == Token::FP_RM_RTN)
    {
      d_work.pop_back();
      d_work_args.push_back(bitwuzla::mk_rm_value(bitwuzla::RoundingMode::RTN));
    }
    else if (token == Token::FP_RM_RTP_LONG || token == Token::FP_RM_RTP)
    {
      d_work.pop_back();
      d_work_args.push_back(bitwuzla::mk_rm_value(bitwuzla::RoundingMode::RTP));
    }
    else if (token == Token::FP_RM_RTZ_LONG || token == Token::FP_RM_RTZ)
    {
      d_work.pop_back();
      d_work_args.push_back(bitwuzla::mk_rm_value(bitwuzla::RoundingMode::RTZ));
    }
  }
  else if (token != Token::REAL_DIV)
  {
    return error("unexpected token '" + std::to_string(token) + "'");
  }
  return true;
}

bool
Parser::close_term(Token token)
{
  // TODO
}

/* -------------------------------------------------------------------------- */

bool
Parser::parse_sort()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }

  if (token == Token::BOOL)
  {
    d_work_args.push_back(bitwuzla::mk_bool_sort());
    return true;
  }
  if (token == Token::FP_FLOAT16)
  {
    d_work_args.push_back(bitwuzla::mk_fp_sort(5, 11));
    return true;
  }
  if (token == Token::FP_FLOAT32)
  {
    d_work_args.push_back(bitwuzla::mk_fp_sort(8, 24));
    return true;
  }
  if (token == Token::FP_FLOAT64)
  {
    d_work_args.push_back(bitwuzla::mk_fp_sort(11, 53));
    return true;
  }
  if (token == Token::FP_FLOAT128)
  {
    d_work_args.push_back(bitwuzla::mk_fp_sort(15, 113));
    return true;
  }
  if (token == Token::FP_ROUNDINGMODE)
  {
    d_work_args.push_back(bitwuzla::mk_rm_sort());
    return true;
  }
  if (token == Token::LPAR)
  {
    token = next_token();
    if (!check_token(token))
    {
      return false;
    }
    if (token == Token::ARRAY)
    {
      return parse_sort_array();
    }
    if (token != Token::UNDERSCORE)
    {
      return error("expected '_' or 'Array'");
    }
    return parse_sort_bv_fp();
  }
  if (token == Token::SYMBOL)
  {
    assert(d_lexer->has_token());
    SymbolTable::Node* sort = d_table.find(d_lexer->token());
    if (!sort || sort->d_sort.is_null())
    {
      return error("invalid sort '" + d_lexer->token() + "'");
    }
    d_work_args.push_back(sort->d_sort);
    return true;
  }
  return error("expected '(' or sort keyword");
}

bool
Parser::parse_sort_array()
{
  if (!parse_sort())
  {
    return false;
  }
  bitwuzla::Sort index = pop_sort_arg();
  if (!parse_sort())
  {
    return false;
  }
  bitwuzla::Sort element = pop_sort_arg();
  if (!parse_rpars(1))
  {
    return false;
  }
  d_work_args.push_back(bitwuzla::mk_array_sort(index, element));
  return true;
}

bool
Parser::parse_sort_bv_fp()
{
  Token token = next_token();
  if (!check_token(token))
  {
    return false;
  }

  if (token == Token::BV_BITVEC)
  {
    if (!parse_uint64())
    {
      return false;
    }
    uint64_t size = pop_uint64_arg();
    if (size == 0)
    {
      return error("invalid bit-vector size '0'");
    }
    if (!parse_rpars(1))
    {
      return false;
    }
    d_work_args.push_back(bitwuzla::mk_bv_sort(size));
    return true;
  }
  if (token == Token::FP_FLOATINGPOINT)
  {
    if (!parse_uint64())
    {
      return false;
    }
    uint64_t esize = pop_uint64_arg();
    if (esize == 0)
    {
      return error("invalid exponent bit-vector size '0'");
    }
    if (!parse_uint64())
    {
      return false;
    }
    uint64_t ssize = pop_uint64_arg();
    if (ssize == 0)
    {
      return error("invalid significand bit-vector size '0'");
    }
    if (!parse_rpars(1))
    {
      return false;
    }
    d_work_args.push_back(bitwuzla::mk_fp_sort(esize, ssize));
    return true;
  }
  return error("expected '" + std::to_string(Token::BV_BITVEC) + "' or '"
               + std::to_string(Token::FP_FLOATINGPOINT) + "'");
}

/* -------------------------------------------------------------------------- */

void
Parser::open_term_scope()
{
  d_work_args_control.push_back(d_work_args.size());
  d_term_open += 1;
}

void
Parser::close_term_scope()
{
  d_work_args_control.pop_back();
  d_term_open -= 1;
}

/* -------------------------------------------------------------------------- */

bool
Parser::error(const std::string& error_msg, const Lexer::Coordinate* coo)
{
  assert(d_lexer);
  if (!coo) coo = &d_lexer->coo();
  d_error = d_infile_name + ":" + std::to_string(coo->col) + ":"
            + std::to_string(coo->line) + ": " + error_msg;
  return false;
}

bool
Parser::error_invalid()
{
  assert(d_lexer);
  assert(d_lexer->error());
  return error(d_lexer->error_msg());
}

bool
Parser::error_eof(Token token)
{
  return error("unexpected end of file after '" + std::to_string(token) + "'",
               &d_lexer->last_coo());
}

bool
Parser::check_token(Token token)
{
  if (token == Token::ENDOFFILE)
  {
    return error_eof(token);
  }
  if (token == Token::INVALID)
  {
    return error_invalid();
  }
  return true;
}

bool
Parser::is_symbol(Token token)
{
  return token == Token::SYMBOL || token == Token::ATTRIBUTE
         || (static_cast<uint32_t>(token) & d_token_class_mask) > 0;
}
size_t
Parser::enable_theory(const std::string& logic,
                      const std::string& theory,
                      size_t size_prefix)
{
  if (size_prefix < logic.size())
  {
    size_t size_theory = theory.size();
    std::string l      = logic.substr(size_prefix, size_theory);
    if (l == theory)
    {
      if (theory == "A")
      {
        d_table.init_array_symbols();
        d_token_class_mask |= static_cast<uint32_t>(TokenClass::ARRAY);
        d_arrays_enabled = true;
      }
      else if (theory == "BV")
      {
        d_table.init_bv_symbols();
        d_token_class_mask |= static_cast<uint32_t>(TokenClass::BV);
        d_bv_enabled = true;
      }
      else if (theory == "FP" || theory == "FPLRA")
      {
        d_table.init_fp_symbols();
        d_token_class_mask |= static_cast<uint32_t>(TokenClass::FP);
        d_token_class_mask |= static_cast<uint32_t>(TokenClass::REALS);
        d_fp_enabled = true;
      }
      size_prefix += size_theory;
    }
  }
  return size_prefix;
}

bool
Parser::is_supported_logic(const std::string& logic)
{
  size_t size = logic.size(), size_prefix = 0;

  if (size < 3)
  {
    return false;
  }

  if (logic == "ALL")
  {
    d_table.init_array_symbols();
    d_table.init_bv_symbols();
    d_table.init_fp_symbols();
    d_token_class_mask |= static_cast<uint32_t>(TokenClass::ARRAY);
    d_token_class_mask |= static_cast<uint32_t>(TokenClass::BV);
    d_token_class_mask |= static_cast<uint32_t>(TokenClass::FP);
    d_token_class_mask |= static_cast<uint32_t>(TokenClass::REALS);
    d_arrays_enabled = true;
    d_bv_enabled     = true;
    d_fp_enabled     = true;
    return true;
  }
  size_prefix = enable_theory(logic, "QF_", size_prefix);
  size_prefix = enable_theory(logic, "A", size_prefix);
  size_prefix = enable_theory(logic, "UF", size_prefix);
  size_prefix = enable_theory(logic, "BV", size_prefix);
  size_prefix = enable_theory(logic, "FPLRA", size_prefix);
  size_prefix = enable_theory(logic, "FP", size_prefix);
  return size_prefix == size;
}

void
Parser::print_success()
{
  if (d_print_success)
  {
    *d_out << "success" << std::endl;
    d_out->flush();
  }
}

/* -------------------------------------------------------------------------- */

uint64_t
Parser::pop_uint64_arg()
{
  assert(std::holds_alternative<uint64_t>(d_work_args.back()));
  uint64_t res = std::get<uint64_t>(d_work_args.back());
  d_work_args.pop_back();
  return res;
}

bitwuzla::Sort
Parser::pop_sort_arg()
{
  assert(std::holds_alternative<bitwuzla::Sort>(d_work_args.back()));
  bitwuzla::Sort res = std::get<bitwuzla::Sort>(d_work_args.back());
  d_work_args.pop_back();
  return res;
}

bitwuzla::Term
Parser::pop_term_arg()
{
  assert(std::holds_alternative<bitwuzla::Term>(d_work_args.back()));
  bitwuzla::Term res = std::get<bitwuzla::Term>(d_work_args.back());
  d_work_args.pop_back();
  return res;
}

std::string
Parser::pop_str_arg()
{
  assert(std::holds_alternative<std::string>(d_work_args.back()));
  std::string res = std::get<std::string>(d_work_args.back());
  d_work_args.pop_back();
  return res;
}

SymbolTable::Node*
Parser::pop_node_arg()
{
  assert(std::holds_alternative<SymbolTable::Node*>(d_work_args.back()));
  SymbolTable::Node* res = std::get<SymbolTable::Node*>(d_work_args.back());
  d_work_args.pop_back();
  return res;
}

uint64_t
Parser::peek_uint64_arg()
{
  assert(std::holds_alternative<uint64_t>(d_work_args.back()));
  return std::get<uint64_t>(d_work_args.back());
}

const bitwuzla::Sort&
Parser::peek_sort_arg()
{
  assert(std::holds_alternative<bitwuzla::Sort>(d_work_args.back()));
  return std::get<bitwuzla::Sort>(d_work_args.back());
}

/* Parser::ParsedItem ------------------------------------------------------- */

Parser::ParsedItem::ParsedItem(Token token, const Lexer::Coordinate& coo)
    : d_token(token), d_coo(coo)
{
}

Parser::ParsedItem::ParsedItem(Token token,
                               const std::string& str,
                               const Lexer::Coordinate& coo)
    : d_token(token), d_parsed(str), d_coo(coo)
{
}

Parser::ParsedItem::ParsedItem(Token token,
                               SymbolTable::Node* node,
                               const Lexer::Coordinate& coo)
    : d_token(token), d_parsed(node), d_coo(coo)
{
}

/* Parser::Statistics ------------------------------------------------------- */

Parser::Statistics::Statistics()
    : num_assertions(d_stats.new_stat<uint64_t>("parser::smt2:num_assertions")),
      num_check_sat(d_stats.new_stat<uint64_t>("parser::smt2:num_check_sat")),
      num_commands(d_stats.new_stat<uint64_t>("parser::smt2:num_commands")),
      num_exit(d_stats.new_stat<uint64_t>("parser::smt2:num_exit")),
      num_set_logic(d_stats.new_stat<uint64_t>("parser::smt2:num_set_logic")),
      time_parse(
          d_stats.new_stat<util::TimerStatistic>("parser::smt2::time_parse"))
{
}

}  // namespace parser::smt2
}  // namespace bzla