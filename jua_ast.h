

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
		virtual	 ~jua_ast_node()
		{
			if (this->child_nodes.size())
			{
				for (auto&n : child_nodes)
				{
					delete n;
				}
			}
			if (Token) { delete Token; Token = 0; };
		}
	};
}
