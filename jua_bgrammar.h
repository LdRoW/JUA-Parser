
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
		virtual jua_rule_b& createRule(int lexemType=-1) = 0;
		virtual jua_rule_b& getRule(unsigned int id) = 0;
		virtual bool isPunctuation(int uid) = 0;
	};
	jua_grammar_impl* global_lexer = 0;
	/*struct jua_expected
	{
		jua_expected(int str, unsigned int d)
		{
			what = str;
			depth = d;
		}
		jua_expected() {};
		int what = -1;
		int depth = -1;
	};
	*/
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
			eCreateAstNode = 0x40
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
		inline	bool isPushCifStar() const
		{
			return Flags&ePushCifStart;
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
		~jua_rule_b()
		{
		}

		jua_ast_node* (*func_c)() = &create_ast<jua_ast_node>;


		virtual jua_ast_node* scan_x3()
		{
			_ast_depth += 1;
			jua_ast_node* res = 0;
			if (this->ruleType == follow_r)
			{
				if (this->isStar() || this->isPlus())
				{
					int num = 0;
					unsigned int used = global_lexer->get_used();
					auto node = func_c(); bool isrSt = false;
					while (true)
					{
						for (unsigned int n = 0; n < childs.size(); ++n)
						{
							used = global_lexer->get_used();
							jua_rule_b& rule = global_lexer->getRule(childs[n]);
							res = rule.scan_x3();
							isrSt = rule.isStar() || rule.isOneOrNull();
							if (res)
							{
								if (!global_lexer->isPunctuation(rule.lexemType)) {
									node->child_nodes.emplace_back(res);
								}
								else {
									delete res;
									res = 0;
								}
							}
							else
							{
								if (isrSt)
								{
									continue;
								}
								global_lexer->set_used(used);
								if (num == 0)
								{
									delete node;
									node = 0;
									_ast_depth -= 1;
									return 0;
								}
								else if (num > 0)
								{
									_ast_depth -= 1;
									return node;
								}
							}
						}
						num++;
					}
					_ast_depth -= 1;
					return node;
				}
				else
				{
					auto used = global_lexer->get_used();
					auto node = func_c();
					bool isrSt = false;
					for (unsigned int n = 0; n < childs.size(); ++n)
					{
						jua_rule_b& rule = global_lexer->getRule(childs[n]);
						res = rule.scan_x3();
						isrSt = rule.isStar() || rule.isOneOrNull();
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
							if (rule.isStar() && rule.isPushCifStar())
							{
								for (auto&n : res->child_nodes)
								{
									node->child_nodes.emplace_back(n);
								}
								res->child_nodes.clear();
							}
							else if (!global_lexer->isPunctuation(rule.lexemType))
							{
								node->child_nodes.emplace_back(res);
								continue;
							}
							delete res;
							res = 0;
						}
					}
					_ast_depth -= 1;
					return node;
				}
			}
			else if (this->ruleType == or_r)
			{
				if (this->isStar() || this->isPlus())
				{
					int num = 0;
					unsigned int used = global_lexer->get_used();
					auto node = func_c();
					bool isrSt = false;
					while (true)
					{
						for (unsigned int n = 0; n < childs.size(); ++n)
						{
							used = global_lexer->get_used();
							jua_rule_b& rule = global_lexer->getRule(childs[n]);
							res = rule.scan_x3();
							isrSt = rule.isStar() || rule.isOneOrNull();
							if (res)
							{
								node->child_nodes.emplace_back(res);
								res = 0;
								num++;
								break;
							}
							else
							{
								if (isrSt)
								{
									num++;
									break;
								}
								global_lexer->set_used(used);
								if (n + 1 == childs.size())
								{
									if (num == 0)
									{
										delete node;
										node = 0;
										_ast_depth -= 1;
										return 0;
									}
									else if (num > 0)
									{
										_ast_depth -= 1;
										return node;
									}
								}
							}
						}
					}
					_ast_depth -= 1;
					return node;
				}
				else
				{
					unsigned int used = 0;
					auto node = func_c();
					bool isrSt = false;
					for (unsigned int n = 0; n < childs.size(); ++n)
					{
						used = global_lexer->get_used();
						jua_rule_b& rule = global_lexer->getRule(childs[n]);
						res = rule.scan_x3();
						isrSt = rule.isStar() || rule.isOneOrNull();
						if (res)
						{
							if (!isTransient()) {
								node->child_nodes.emplace_back(res);
								res = 0;
							}
							else {
								delete node;
								node = res;
							}
							_ast_depth -= 1;
							return node;
						}
						else
						{
							global_lexer->set_used(used);
						}
					}

					delete node;
					node = 0;
					_ast_depth -= 1;
					return 0;
				}
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
		jua_rule_b& operator=(jua_rule_b& other)
		{
			this->childs = std::move(other.childs);
			this->ruleType = other.ruleType;
			this->Flags = other.Flags;
			this->lexemType = other.lexemType;
			if (this->ruleId == -1)
				this->ruleId = other.ruleId;
			return *this;
		}
		jua_rule_b& operator%=(jua_rule_b& other)
		{
			this->childs = std::move(other.childs);
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
	struct jua_grammar :jua_grammar_impl
	{
	private:
		std::unordered_map<std::string, int> unique_ids;
		std::vector<jua_token*> tokens;
		std::vector<int> _punctuation;
		unsigned int used_tok = 0, last_used;
		jua_ast_node* root = 0;
		int last_uid = 0;
	public:
		jua_rule_b start;
		std::vector<jua_rule_b*> _rules;
		//!! Attention jua_grammar will handle all created tokens and delete it
		// Be carefull
		jua_grammar(jua_lexer*pr) :jua_grammar_impl(),unique_ids(std::move(pr->unique_id)),
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
		jua_rule_b& createRule(int lexemType=-1)
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
		virtual	jua_token* get_token() {

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
		int make_word(std::string g)
		{
			if (unique_ids.find(g) != unique_ids.end()) {
				
				return  unique_ids[g];
			}
			else
			{
				unique_ids[g] =  last_uid += 1;

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
			delete root;
			for (auto&n : tokens)
			{
				delete n;
			}
			for (auto n : _rules)
			{
				delete n;
			}
		}
	};
}
