#pragma once
#include "jua.h"
#include "jua_pjgrammar.h"

using namespace jua;

struct calc :jua_pj::jua_grammar
{
	jua_pj::jua_rule& numb = createRule(jua_number);
	jua_pj::jua_rule& float_n = createRule(jua_base_lexer::jua_float_number);
	jua_pj::jua_rule& var = createRule(jua_base_lexer::jua_variable);
	jua_pj::jua_rule& val = createRule();
	jua_pj::jua_rule& expr = createRule();
	jua_pj::jua_rule& expr_impl = createRule();
	jua_pj::jua_rule& unary = createRule();
	jua_pj::jua_rule& plus = createRule();
	jua_pj::jua_rule& div = createRule();
	jua_pj::jua_rule& minus = createRule();
	jua_pj::jua_rule& mul = createRule();
	calc(jua_lexer*lex) :jua_grammar(lex)
	{
		plus.lexemType = make_word("+");
		div.lexemType = make_word("/");
		minus.lexemType = make_word("-");
		mul.lexemType = make_word("*");
		unary = plus | div | minus | mul;
		val = numb | var | float_n;
		expr_impl = unary + val;
		expr = val + *expr_impl;
		this->start = expr;
	}
};

struct gtalexer :jua_lexer
{
	static const JUA_TOKEN_TYPE jua_variable = (JUA_TOKEN_TYPE)9;
	static const JUA_TOKEN_TYPE jua_gl_var = (JUA_TOKEN_TYPE)12;
	static const JUA_TOKEN_TYPE jua_loc_var = (JUA_TOKEN_TYPE)13;
	static const JUA_TOKEN_TYPE jua_locstr_var = (JUA_TOKEN_TYPE)15;
	static const JUA_TOKEN_TYPE jua_name_var = (JUA_TOKEN_TYPE)14;
	static const JUA_TOKEN_TYPE jua_label_name = (JUA_TOKEN_TYPE)16;
	static const JUA_TOKEN_TYPE jua_label_ref = (JUA_TOKEN_TYPE)17;
	static const JUA_TOKEN_TYPE jua_string_source = (JUA_TOKEN_TYPE)18;
	static const JUA_TOKEN_TYPE jua_string_t = (JUA_TOKEN_TYPE)19;
	static const JUA_TOKEN_TYPE jua_float_number = (JUA_TOKEN_TYPE)11;
	gtalexer() :jua_lexer()
	{
		'\"' + jua_any = jua_string_source;
		jua_string_source + jua_any = jua_string_source;
		jua_string_source + '\"' = jua_string_t;

		jua_separator + jua_digit = jua_match;
		jua_separator + jua_tchar = jua_match;
		jua_separator + jua_whspace = jua_match;
		jua_separator + jua_separator = jua_separator;

		process_batch_match<jua_whspace,jua_digit,jua_number, jua_label_name, jua_label_ref, 
			jua_string_t, jua_gl_var, jua_loc_var, jua_name_var,jua_tchar>();
		process_batch_match<jua_separator, jua_digit, jua_number, jua_label_name, jua_label_ref,
			jua_string_t, jua_gl_var, jua_loc_var, jua_name_var, jua_tchar>();
	}
	virtual bool isUncompleteType(const int t)
	{
		return t == jua_string_source || t == jua_variable ;
	}
	virtual std::string_view performValue(int tok_t,int start, int size)
	{
		if (tok_t == jua_label_name)
			return std::string_view(getText() + start, size - 1);
		else jua_lexer::performValue(tok_t, start, size);
	}
};
struct gtagrammar: jua_pj::jua_grammar
{
#define d ::jua_pj::
	d jua_rule& numb = createRule(jua_number); 
	jua_pj::jua_rule& float_n = createRule(gtalexer::jua_float_number);
	jua_pj::jua_rule& lvar = createRule(gtalexer::jua_loc_var);
	jua_pj::jua_rule& gvar = createRule(gtalexer::jua_gl_var);
	jua_pj::jua_rule& name = createRule(gtalexer::jua_name_var);
	jua_pj::jua_rule& val = createRule();
	jua_pj::jua_rule& rval = createRule();
	jua_pj::jua_rule& lval = createRule();
	jua_pj::jua_rule& parameters = createRule();
	jua_pj::jua_rule& expr = createRule();
	jua_pj::jua_rule& expr_impl = createRule();
	jua_pj::jua_rule& binop = createRule();
	jua_pj::jua_rule& plus = createRule();
	jua_pj::jua_rule& div = createRule();
	jua_pj::jua_rule& minus = createRule();
	jua_pj::jua_rule& mul = createRule();
	d jua_rule& hex = createRule(), &end = createRule();
	d jua_rule& instr = createRule();
	d jua_rule& code_block = createRule();
	d jua_rule& label_name = createRule(gtalexer::jua_label_name);
	d jua_rule& jua_label_rf = createRule(gtalexer::jua_label_ref);
	gtagrammar(jua_lexer*lex) :jua_grammar(lex)
	{
		plus.lexemType = make_word("+");
		div.lexemType = make_word("/");
		minus.lexemType = make_word("-");
		mul.lexemType = make_word("*");
		hex.lexemType = make_word("hex");
		end.lexemType = make_word("end");

		binop = plus | div | minus | mul;

		val = numb | lvar|gvar | float_n;
		lval = lvar | gvar;
		rval = lvar | gvar | numb | float_n | name|jua_label_rf;
		expr_impl = binop + val;
		expr = val + *expr_impl;
		instr = expr;
		code_block = label_name + *instr|hex + end;
		
	}
};

using namespace jua_pj;
struct luagrammar : jua_pj::jua_grammar
{
#define R jua_pj::jua_rule&
	static const JUA_TOKEN_TYPE  jua_variable = (JUA_TOKEN_TYPE)10;
	static const JUA_TOKEN_TYPE  jua_float_number = (JUA_TOKEN_TYPE)9;
	static const JUA_TOKEN_TYPE  jua_str_src = (JUA_TOKEN_TYPE)11;
	static const JUA_TOKEN_TYPE  jua_string = (JUA_TOKEN_TYPE)12;

	jua_pj::jua_rule& variable = createRule(jua_variable);
	jua_pj::jua_rule& float_number = createRule(jua_float_number);
	jua_pj::jua_rule& number = createRule(jua_number);
	jua_pj::jua_rule& string = createRule(jua_string);
	jua_pj::jua_rule& plus = createRule();
	jua_pj::jua_rule& div = createRule();
	jua_pj::jua_rule& minus = createRule();
	jua_pj::jua_rule& mul = createRule();
	jua_pj::jua_rule& assign = createRule();
	jua_pj::jua_rule& func_word = createRule();
	jua_pj::jua_rule& local_word = createRule();
	jua_pj::jua_rule& return_word = createRule();
	jua_pj::jua_rule& end_word = createRule();
	jua_pj::jua_rule& if_word = createRule();
	jua_pj::jua_rule& then_word = createRule();
	jua_pj::jua_rule& elseif_word = createRule();
	jua_pj::jua_rule& else_word = createRule();
	jua_pj::jua_rule& name = createRule(jua_word);
	jua_pj::jua_rule& lBracket = createRule();
	jua_pj::jua_rule& rBracket = createRule();
	jua_pj::jua_rule& lParenthesis = createRule();
	jua_pj::jua_rule& rParenthesis = createRule();

	R rvalue = createRule();
	R lvalue = createRule();
	R expr = createRule();
	R rvalue_expr = createRule();
	R bin_op = createRule();
	R fcode_block = createRule();
	R scope_code_block = createRule();
	R local_decl = createRule();
	R func_decl = createRule();
	R func_call = createRule();
	R array_decl = createRule();
	R fparam_list_add = createRule();
	R fparam_list = createRule();
	R fcall_args_add = createRule();
	R fcall_args = createRule();
	R comma = createRule();
	R nil = createRule();
	R return_expr = createRule();

	luagrammar(jua_lexer*lex) :jua_grammar(lex)
	{
		plus.lexemType = make_word("+");
		div.lexemType = make_word("/");
		minus.lexemType = make_word("-");
		mul.lexemType = make_word("*");
		assign = make_word("=");
		func_word.lexemType = make_word("function");
		local_word.lexemType = make_word("local");
		end_word.lexemType = make_word("end");
		if_word.lexemType = make_word("if");
		then_word.lexemType = make_word("then");
		elseif_word.lexemType = make_word("elseif");
		else_word.lexemType = make_word("else");
		return_word.lexemType = make_word("return");
		lBracket.lexemType = make_word("[");
		rBracket.lexemType = make_word("]");
		lParenthesis.lexemType = make_word("(");
		rParenthesis.lexemType = make_word(")");
		comma.lexemType = make_word(",");
		nil.lexemType = make_word("nil");


		rvalue = nil|variable | float_number | string | number | func_call;
		lvalue = variable | func_call;

		bin_op = plus | minus | div | mul | assign;

		rvalue_expr = rvalue + bin_op + rvalue|rvalue;
		expr = lvalue + bin_op + rvalue_expr | rvalue_expr;
		return_expr = expr + createStarRule(comma + expr);

		fcall_args_add = comma + rvalue;
		fcall_args_add.Flags[eStar] = true;
		fcall_args_add.Flags[eTransient] = true;
		fcall_args = rvalue + fcall_args_add;

		fparam_list_add = comma + variable;
		fparam_list_add.Flags[eStar] = true;
		fparam_list_add.Flags[eTransient] = true;
		fparam_list = variable + fparam_list_add;

		func_decl = func_word + variable + lParenthesis+ createOneOrNullRule(fparam_list) + rParenthesis+ createStarRule(fcode_block) + end_word;
		func_call = variable + lParenthesis + createOneOrNullRule( fcall_args )+ rParenthesis;

		local_decl = local_word + variable + assign + rvalue;
		
		fcode_block = local_decl | func_call | expr|return_expr;
		scope_code_block = local_decl | func_call | func_decl | expr;

		setPunctuation(comma.ruleId);
	}
};