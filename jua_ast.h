/*
		Copyright (C) 2019 LdRoW
		Do not delete this comment block. 
*/

#pragma once
#include "jua_ulexer.h"

namespace jua
{
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
		jua_ast_node(const jua_ast_node& other):child_nodes(other.child_nodes)
		{
			*this->Token = *other.Token;
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
					n = 0;
				}
				this->child_nodes.clear();
			}
			if (Token) {
				delete Token; 
				Token = 0;
			};
		}
	};
}
