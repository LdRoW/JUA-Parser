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


using namespace jua_pj;
struct luagrammar : jua_pj::jua_grammar
{
#define R jua_pj::jua_rule&
	static const JUA_TOKEN_TYPE  jua_variable = (JUA_TOKEN_TYPE)10;
	static const JUA_TOKEN_TYPE  jua_float_number = (JUA_TOKEN_TYPE)9;
	static const JUA_TOKEN_TYPE  jua_str_src = (JUA_TOKEN_TYPE)11;
	static const JUA_TOKEN_TYPE  jua_string = (JUA_TOKEN_TYPE)12;

	

	luagrammar(jua_lexer*lex) :jua_grammar(lex)
	{
		jua_pj::jua_rule& variable = createRule(jua_variable);
		jua_pj::jua_rule& float_number = createRule(jua_float_number);
		jua_pj::jua_rule& number = createRule(jua_number);
		jua_pj::jua_rule& string = createRule(jua_string);
		jua_pj::jua_rule& plus = createRule();
		jua_pj::jua_rule& div = createRule();
		jua_pj::jua_rule& minus = createRule();
		jua_pj::jua_rule& mul = createRule();
		jua_pj::jua_rule& assign = createRule();
		jua_pj::jua_rule& not_equal = createRule();
		jua_pj::jua_rule& equal = createRule();
		jua_pj::jua_rule& func_word = createRule();
		jua_pj::jua_rule& local_word = createRule();
		jua_pj::jua_rule& return_word = createRule();
		jua_pj::jua_rule& end_word = createRule();
		jua_pj::jua_rule& if_word = createRule();
		jua_pj::jua_rule& then_word = createRule();
		jua_pj::jua_rule& elseif_word = createRule();
		jua_pj::jua_rule& else_word = createRule();
		//jua_pj::jua_rule& name = createRule(jua_word);
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

		plus = make_word("+");
		div = make_word("/");
		minus = make_word("-");
		mul = make_word("*");
		not_equal = make_word("~=");
		equal = make_word("==");
		assign = make_word("=");
		func_word = make_word("function");
		local_word = make_word("local");
		end_word = make_word("end");
		if_word = make_word("if");
		then_word = make_word("then");
		elseif_word = make_word("elseif");
		else_word = make_word("else");
		return_word = make_word("return");
		lBracket = make_word("[");
		rBracket = make_word("]");
		lParenthesis = make_word("(");
		rParenthesis = make_word(")");
		comma = make_word(",");
		nil = make_word("nil");

		rvalue = nil|variable | float_number | string | number ;
		lvalue = variable ;

		bin_op = plus | minus | div | mul | assign | not_equal | equal;

		rvalue_expr = rvalue + bin_op + rvalue;
		expr = lvalue + bin_op + rvalue;
		return_expr = return_word + rvalue_expr + createStarRule(comma + rvalue_expr);

		fcall_args_add = comma + expr;
		fcall_args_add.Flags[eStar] = true;
		fcall_args_add.Flags[eTransient] = true;
		fcall_args = expr + fcall_args_add;

		fparam_list_add = comma + variable;
		fparam_list_add.Flags[eStar] = true;
		fparam_list_add.Flags[eTransient] = true;
		fparam_list = variable + fparam_list_add;

		func_decl = func_word + variable + lParenthesis+ createOneOrNullRule(fparam_list) + rParenthesis+ createStarRule(fcode_block) + end_word;
		func_call = variable + lParenthesis + createOneOrNullRule( fcall_args )+ rParenthesis;

		local_decl = local_word + variable + assign + rvalue;
		
		fcode_block = local_decl  | expr|return_expr;
		scope_code_block = func_decl | local_decl | expr;
		
		//start rule assign only with op %=
		start %=  createStarRule(scope_code_block);
	
		//set transparent nodes
		lvalue.setTransparent();	
		rvalue.setTransparent();
		bin_op.setTransparent();
		fcode_block.setTransparent();

		//set punctuation -> must be in the code but haven't to be in ast
		setPunctuation(comma.ruleId);
	}
};