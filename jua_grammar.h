
#pragma once

#include "jua.h"

namespace jua
{
	//! template function for ast node creation 
	template<class T>
	jua_ast_node* create_ast()
	{
		return new T();
	}
	struct jua_rule_b;

	struct jua_grammar_impl
	{
		std::vector<jua_ast_node*> deletion_list;
		virtual	jua_token* get_token() = 0;
		virtual unsigned int get_token_count() = 0;
		virtual bool can_get_token() = 0;
		virtual void set_used(unsigned int used) {};
		virtual unsigned int get_used() = 0;
		virtual std::string getTokenByUniqueID(int uid) = 0;
		virtual jua_rule_b& createRule(int lexemType = -1) = 0;
		virtual jua_rule_b& getRule(unsigned int id) = 0;
		virtual bool isPunctuation(int uid) = 0;
	};
	jua_grammar_impl* global_lexer = 0;

	int _ast_depth = 0;
	struct jua_rule_b
	{
	private:
	public:
		std::vector<unsigned int> childs;
		int ruleId = -1;
		int lexemType = -1;
		enum EFlags
		{
			eStar = 0x01,
			ePlus = 0x02,
			eOneOrNull = 0x04,
			eTransient = 0x08,
			eTryRenew = 0x10,
			ePushCifStart = 0x20,
			eHideAstNode = 0x40
		};
		unsigned int Flags = 0;
		inline	bool isStar()const
		{
			return Flags&eStar;
		}
		inline	bool isPlus()const
		{
			return Flags&ePlus;
		}
		inline	bool isOneOrNull()const
		{
			return Flags&eOneOrNull;
		}
		inline	bool isTransient()const
		{
			return Flags&eTransient;
		}
		inline	bool isTryRenew() const
		{
			return Flags&eTryRenew;
		}
		inline	bool isPushChildsUp() const
		{
			return Flags&ePushCifStart;
		}
		inline bool isHideAstNode() const
		{
			return Flags&eHideAstNode;
		}
		//jua_expected expected = { -1, 0 };
		enum lex_type
		{
			follow_r,
			or_r,
			none_r,
			dont_follow_r
		} ruleType = none_r;

		jua_rule_b() {  };
		jua_rule_b(unsigned int lextype) {
			this->lexemType = lextype;
			this->ruleType = none_r;
		}
		jua_rule_b(const jua_rule_b& other)
		{
			this->lexemType = other.lexemType;
			this->ruleType = other.ruleType;
			this->ruleId = other.ruleId;
			this->childs = other.childs;
			this->Flags = other.Flags;
			this->func_c = other.func_c;
		}
		~jua_rule_b()
		{
		}

		jua_ast_node* (*func_c)() = &create_ast<jua::jua_ast_node>;


		virtual jua_ast_node* scan_x3()
		{
			_ast_depth += 1;
			jua_ast_node* res = 0;
			if (this->ruleType == follow_r)
			{
				auto used = global_lexer->get_used();
				auto node = func_c();
				bool isrSt = false; jua_rule_b rule;
				int sused = -1;
				for (unsigned int n = 0; n < childs.size(); ++n)
				{
					rule = global_lexer->getRule(childs[n]);
					isrSt = rule.isStar() || rule.isOneOrNull();
					if (rule.isStar() || rule.isPlus())
					{
						while (true)
						{
							sused = global_lexer->get_used();
							res = rule.scan_x3();
							if (res)
							{
								if (rule.isPushChildsUp())
								{
									node->child_nodes = res->child_nodes;
									res->child_nodes.clear();
									delete res;
								}
								else if (!global_lexer->isPunctuation(rule.lexemType))
								{
									node->child_nodes.emplace_back(res);
									continue;
								}
								delete res;
								res = 0;
							}
							else
							{
								if (isrSt)
								{
									global_lexer->set_used(sused);
									break;
								}
								delete node;
								node = 0;
								global_lexer->set_used(used);
								_ast_depth -= 1;
								return 0;
							}
						}
					}
					else
					{
						res = rule.scan_x3();
						if (!res)
						{
							if (isrSt)
							{
								continue;
							}
							delete node;
							node = 0;
							global_lexer->set_used(used);
							_ast_depth -= 1;
							return 0;
						}
						else
						{
							if (rule.isPushChildsUp())
							{
								node->child_nodes = res->child_nodes;
								res->child_nodes.clear();
								delete res;
								res = 0;
							}
							else if (!global_lexer->isPunctuation(rule.lexemType))
							{
								node->child_nodes.emplace_back(res);
								continue;
							}
							delete res;
							res = 0;
							continue;
						}
					}
				}
				_ast_depth -= 1;
				return node;
			}
			else if (this->ruleType == or_r)
			{
				unsigned int used = 0;
				auto node = func_c();
				bool isrSt = false; jua_rule_b rule; int sused = -1;
				for (unsigned int n = 0; n < childs.size(); ++n)
				{
					used = global_lexer->get_used();
					rule = global_lexer->getRule(childs[n]);
					isrSt = rule.isStar() || rule.isOneOrNull();
					if (rule.isStar() || rule.isPlus())
					{
						while (true)
						{
							sused = global_lexer->get_used();
							res = rule.scan_x3();
							if (res)
							{
								if (rule.isPushChildsUp())
								{
									node->child_nodes = res->child_nodes;
									res->child_nodes.clear();
								}
								else if (isTransient()) {
									//std::cout << "Node transient deleting safe from star rule or\n";
									*node = *res;
									delete res;
								}
								else {
									node->child_nodes.emplace_back(res);
									//res = 0;
								}
								continue;
							}
							else
							{
								if (isrSt) {
									global_lexer->set_used(sused);
									break;
								}
								else {
									delete node;
									node = 0;
									global_lexer->set_used(used);
									_ast_depth -= 1;
									return 0;
								}

							}
						}
					}
					else
					{
						res = rule.scan_x3();
						if (res)
						{
							if (rule.isPushChildsUp())
							{
								node->child_nodes = res->child_nodes;
								res->child_nodes.clear();
								delete res;
								res = 0;
							}
							else if (isTransient()) {
							//	std::cout << "Node transient deleting safe\n";
								*node = *res;
								delete res;
							}
							else {
								node->child_nodes.emplace_back(res);
							}

							_ast_depth -= 1;
							return node;
						}
						else
						{
							global_lexer->set_used(used);
						}
					}
				}
				delete node;
				node = 0;
				_ast_depth -= 1;
				return 0;

			}
			else
			{
				auto tok = global_lexer->can_get_token() ? global_lexer->get_token() : 0;
				if (tok != 0 && this->lexemType == tok->tok_t) {
					jua_ast_node* node = new jua_ast_node();
					*node->Token = *tok;
					_ast_depth -= 1;
					return node;
				}
				else {
					_ast_depth -= 1;
					return 0;
				}
			}
			_ast_depth -= 1;
			return 0;
		};

		jua_rule_b operator+(jua_rule_b& other)
		{
			if (ruleType == follow_r)
			{
				jua_rule_b &r = jua_rule_b();
				r.ruleType = follow_r;
				if (this->ruleId == -1)
					r.childs = this->childs;
				else
					r.childs.emplace_back(this->ruleId);
				if (other.ruleId == -1)
					r.childs.insert(r.childs.end(), other.childs.begin(), other.childs.end());
				else
					r.childs.emplace_back(other.ruleId);
				return r;
			}
			else if (ruleType == or_r)
			{
				jua_rule_b& r = jua_rule_b();
				r.ruleType = follow_r;
				if (this->ruleId == -1)
				{
					auto& lval = global_lexer->createRule(-1);
					lval.childs = this->childs;
					lval.ruleType = lval.or_r;
					this->ruleId = lval.ruleId;
				}
				r.childs.emplace_back(this->ruleId);
				if (other.ruleId == -1)
				{
					auto& ss = global_lexer->createRule(-1);
					ss.ruleType = ss.or_r;
					ss.childs = other.childs;
					r.childs.emplace_back(ss.ruleId);
				}
				else
					r.childs.emplace_back(other.ruleId);
				return r;
			}
			else
			{
				jua_rule_b r = jua_rule_b();
				r.ruleType = follow_r;
				r.childs.emplace_back(this->ruleId);
				r.childs.emplace_back(other.ruleId);
				return r;
			}
			return *this;
		}
		jua_rule_b operator|(jua_rule_b& other)
		{
			if (ruleType == follow_r)
			{
				if (this->ruleId != -1)
				{
					// if this ruleId != -1 we just need to explore child indices
					jua_rule_b &r = jua_rule_b();
					r.ruleType = or_r;
					r.childs.emplace_back(this->ruleId);
					if (other.ruleId != -1)
					{
						r.childs.emplace_back(other.ruleId);
					}
					else
					{
						jua_rule_b& rt = global_lexer->createRule(-1);
						rt = other;
						r.childs.emplace_back(rt.ruleId);
					}
					return r;
				}
				else
				{
					//this ruleId is undefined in case
					//a= (rule+rule)|(rule+rule)
					// the operator = will be called in last order and we need to get new rule but
					// dont change the lvalue ruleId
					jua_rule_b& r = jua_rule_b();
					r.ruleType = or_r;
					jua_rule_b& lf = global_lexer->createRule(-1);
					lf = *this;
					r.childs.emplace_back(lf.ruleId);
					if (other.ruleId != -1)
					{
						r.childs.emplace_back(other.ruleId);
					}
					else
					{
						jua_rule_b& rt = global_lexer->createRule(-1);
						rt = other;
						r.childs.emplace_back(rt.ruleId);
					}
					return r;
				}
			}
			else if (ruleType == or_r)
			{
				jua_rule_b &r = jua_rule_b();
				r.ruleType = or_r;
				if (this->ruleId == -1)
					r.childs = this->childs;
				else
					r.childs.emplace_back(this->ruleId);
				if (other.ruleId == -1)
					r.childs.insert(r.childs.end(), other.childs.begin(), other.childs.end());
				else
					r.childs.emplace_back(other.ruleId);
				return r;
			}
			else
			{
				jua_rule_b& r = jua_rule_b();
				r.ruleType = or_r;
				r.childs.emplace_back(this->ruleId);
				if (other.ruleId != -1)
				{
					r.childs.emplace_back(other.ruleId);
				}
				else
				{
					jua_rule_b& rt = global_lexer->createRule(-1);
					rt = other;
					r.childs.emplace_back(rt.ruleId);
				}
				return r;
			}
			return *this;
		}
		jua_rule_b& operator=(const jua_rule_b& other)
		{
			this->childs = other.childs;
			this->ruleType = other.ruleType;
			this->Flags = other.Flags;
			this->lexemType = other.lexemType;
			if (this->ruleId == -1)
				this->ruleId = other.ruleId;
			return *this;
		}
		jua_rule_b& operator%=(const jua_rule_b& other)
		{
			this->childs = other.childs;
			this->ruleType = other.ruleType;
			this->Flags = other.Flags;
			this->lexemType = other.lexemType;
			return *this;
		}
		jua_rule_b& operator*()
		{
			this->Flags |= EFlags::eStar;
			this->Flags ^= EFlags::ePlus;
			return *this;
		}
		jua_rule_b& operator ++()
		{
			this->Flags |= EFlags::ePlus;
			this->Flags ^= EFlags::eStar;
			return *this;
		}
	};
	_declspec(align(32)) struct jua_grammar :jua_grammar_impl
	{
	private:
		std::unordered_map<std::string, int> unique_ids;
		std::vector<jua_token*> tokens;
		std::vector<int> _punctuation;
		unsigned int used_tok = 0, last_used = 0;
		jua_ast_node* root = 0;
		int last_uid = 0;
	public:
		jua_rule_b start;
		std::vector<jua_rule_b*> _rules;
		//!! Attention jua_grammar will handle all created tokens and delete it
		// Be carefull
		jua_grammar(jua_lexer*pr) :jua_grammar_impl(), unique_ids(std::move(pr->unique_id)),
			tokens(std::move(pr->tokens)), last_uid(pr->lastTypedUniqueId)
		{
			global_lexer = this;
		}

		jua_ast_node* scan_x3()
		{
			if (root)
			{
				delete root;
				root = 0;
			}
			root = start.scan_x3();
			return root;
		}
		//!! Creates rule in database
		jua_rule_b& createRule(int lexemType = -1)
		{
			_rules.emplace_back(new jua_rule_b(lexemType));
			_rules[_rules.size() - 1]->ruleId = 128 + _rules.size() - 1;
			return  *(_rules[_rules.size() - 1]);
		}
		//!! Get rule by ruleid
		jua_rule_b& getRule(unsigned int ruleid)
		{
			return *_rules[ruleid - 128];
		}
		virtual jua_token* get_token() {

			used_tok++;
			return tokens[last_used = used_tok - 1];
		};

		virtual bool can_get_token() {
			return used_tok < tokens.size();
		}
		//! sets punctuation ruleid 
		void setPunctuation(int uid)
		{
			if (uid != -1)
				_punctuation.emplace_back(uid);
		}
		virtual bool isPunctuation(int uid) {
			for (int n : _punctuation)
			{
				if (n == uid)return true;
			}
			return false;
		}
		//!! creates unique id for word or if word is already presented
		// try to find it in info from lexer
		int make_word(const std::string& g)
		{
			if (unique_ids.find(g) != unique_ids.end()) {
				return  unique_ids[g];
			}
			else {
				unique_ids[g] = last_uid += 1;
				return  unique_ids[g];
			}
		}

		virtual std::string getTokenByUniqueID(int uid)
		{
			for (auto it = unique_ids.begin(); it != unique_ids.end(); it++)
			{
				if ((*it).second == uid)
				{
					return const_cast<std::string&>((*it).first);
				}
			}
			return std::to_string(uid);
		}

		virtual unsigned int get_token_count() { return tokens.size(); }
		virtual unsigned int get_used() { return used_tok; };

		virtual void set_used(unsigned int used) { used_tok = used; };

		~jua_grammar()
		{
			delete root; root = 0;
			for (auto&n : tokens)
			{
				delete n;
			}
			tokens.clear();
			for (auto n : _rules)
			{
				delete n;
			}
			_rules.clear();
		}
	};
}
