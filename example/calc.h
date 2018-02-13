#pragma once
#include "jua.h"

using namespace jua;
struct calc :public jua_grammar
{
	//Create terminal rule from lexer type
	jua_rule_b& number = createRule(jua_token_t::number);
	//Create nonterminal rule
	jua_rule_b& expr_impl = createRule();
	jua_rule_b& expr = createRule();
	jua_rule_b& plus = createRule();
	jua_rule_b& minus = createRule();
	jua_rule_b& div = createRule();
	jua_rule_b& mul = createRule();
	jua_rule_b& ops = createRule();
	jua_rule_b& val = createRule();
	jua_rule_b& var = createRule(jua_token_t::variable);
	//Constructor
	calc(jua_lexer*lex) :jua_grammar(lex)
	{
		//Get unique id for "+"
		plus.lexemType = make_word("+");
		div.lexemType = make_word("/");
		mul.lexemType = make_word("*");
		minus.lexemType = make_word("-");
		ops = (plus | div | minus | mul);
		val = (number | var);
		expr_impl = ops + val;
		ops.Flags |= ops.eTransient;
		val.Flags |= ops.eTransient;
		//! Make all nodes linear
		expr_impl.Flags |= expr_impl.ePushCifStart;
		//Make main rule and set expr_impl as star rule
		expr = number + *expr_impl;
		this->start = expr;
	}
};