/*
		Copyright (C) 2018 Michailo Yarush
		Do not delete this comment block. Respect others' work!
*/

#pragma once
#include "jua_ulexer.h"

namespace jua
{
	//!! Provide your visit virtual functions here
	struct jua_ast_visitor_base
	{

	};

	struct jua_ast_node
	{
		std::vector<jua_ast_node*> child_nodes;
		jua_token* Token = 0;
		virtual void visit(jua_ast_visitor_base*visitor) {};
		jua_ast_node()
		{
			Token = new jua_token(0, 0, 0);
		}
		jua_ast_node(const jua_ast_node& other)
		{
			*this->Token = *other.Token;
			this->child_nodes = other.child_nodes;
		}
		jua_ast_node& operator=(const jua_ast_node& other)
		{
			*this->Token = *other.Token;
			this->child_nodes = other.child_nodes;
			return *this;
		}
		virtual	 ~jua_ast_node()
		{
			if (this->child_nodes.size())
			{
				for (auto&n : child_nodes)
				{
					delete n;
				}
				this->child_nodes.clear();
			}
			if (Token) {
				std::cout << "Delete " << Token->value << " \n";
				delete Token; Token = 0; };
		}
	};
}