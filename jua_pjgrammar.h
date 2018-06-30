/*
Copyright (C) 2018 LdRoW
Do not delete this comment block. Respect others' work!
*/

#pragma once
#include "jua.h"
#include <functional>
#include <map>
#include <iostream>
#include <bitset>

namespace jua_pj
{
	using namespace jua;
	//! template function for ast node creation 
	template<class T>
	jua::jua_ast_node* create_ast()
	{
		if constexpr(std::is_base_of_v<jua::jua_ast_node, T>)
		{
			return new T();
		}
		else static_assert(false, "Type is not derived from jua_ast_node");
	}
	struct jua_rule;

	struct jua_grammar_impl
	{
		virtual	jua::jua_token* get_token() = 0;
		virtual	jua::jua_token* get_token(size_t pos) = 0;
		virtual	jua::jua_token* pop_token() = 0;
		virtual unsigned int get_token_count() = 0;
		virtual bool can_get_token() = 0;
		virtual void set_used(unsigned int used) {};
		virtual unsigned int get_used() = 0;
		virtual int get_trans(int one, int two) = 0;
		virtual std::string_view getTokenByUniqueID(int uid) = 0;
		virtual jua_rule& createRule(int lexemType = -1) = 0;
		virtual jua_rule& getRule(unsigned int id) = 0;
		virtual bool isPunctuation(int uid) = 0;
		virtual jua_rule* getRuleByLexT(int lex_t) = 0;
	};
	jua_grammar_impl* global_lexer = 0;

	enum EFlags
	{
		eRuleNone,
		eRuleFollow,
		eRuleOr,
		eRuleNotFollow,
		eStar,
		ePlus,
		eOneOrNull,
		eTransient,
		eTryRenew,
		ePushChildsUp,
		eHideAstNode,
		ePunctuation
	};

	struct jua_rule
	{
	public:
		std::vector<unsigned int> childs;
		int ruleId = -1;
		int lexemType = -1;
		std::bitset<16> Flags;
		jua::jua_ast_node* (*ast_creation_fn)() = &create_ast<jua::jua_ast_node>;

#pragma region FlagsCheckFunctions


		inline	bool isStar()const
		{
			return Flags.test(eStar);
		}
		inline	bool isPlus()const
		{
			return Flags.test(ePlus);
		}
		inline	bool isOneOrNull()const
		{
			return Flags.test(eOneOrNull);
		}
		inline	bool isTransient()const
		{
			return Flags.test(eTransient);
		}
		inline	bool isTryRenew() const
		{
			return Flags.test(eTryRenew);
		}
		inline	bool isPushChildsUp() const
		{
			return Flags.test(ePushChildsUp);
		}
		inline	bool isHideAstNode() const
		{
			return Flags.test(eHideAstNode);
		}
		inline	bool isPunctuation() const
		{
			return Flags.test(ePunctuation);
		}

#pragma endregion
		jua_rule() {};
		jua_rule(unsigned int lextype) :lexemType(lextype), Flags(0) {
		}
		jua_rule(const jua_rule& other) :lexemType(other.lexemType), ruleId(other.ruleId), childs(other.childs), Flags(other.Flags),
			ast_creation_fn(other.ast_creation_fn) {
		}
		~jua_rule() {}
		jua_ast_node* parse_follow() {
			bool isStarOrOneNull = false;
			bool isStarOrPlus = false;
			jua::jua_ast_node* node = ast_creation_fn();
			jua::jua_ast_node* sres = 0;
			for (size_t n = 0; n < childs.size(); ++n) {

				auto& rule = global_lexer->getRule(childs[n]);
				isStarOrOneNull = rule.isOneOrNull() || rule.isStar();
				isStarOrPlus = rule.isStar() || rule.isPlus();
				if (isStarOrPlus) {
					size_t used = 0, count = 0;
					jua::jua_ast_node* snode = new jua::jua_ast_node();
					for (;; ++count) {
						used = global_lexer->get_used();
						sres = rule.parse();
						if (!sres) {
							global_lexer->set_used(used);
							break;
						}
						else {
							if (rule.isPushChildsUp() || rule.isTransient()) {
								node->child_nodes.insert(node->child_nodes.cend(), sres->child_nodes.begin(), sres->child_nodes.end());
								sres->child_nodes.clear();
								delete sres;
								continue;
							}
							snode->child_nodes.emplace_back(sres);
						}
					}
					if (!snode->child_nodes.empty()) {
						node->child_nodes.emplace_back(snode);
					}
					else {
						delete snode;
						snode = 0;
					}
					if (rule.isPlus() && count == 0) {
						if (snode)
							delete snode;
						delete node;
						return 0;
					}
					continue;
				}
				else
					sres = rule.parse();
				if (sres != 0) {
					if (rule.isPunctuation()) {
						delete sres;
						continue;
					}
					else if (rule.isPushChildsUp() || rule.isTransient()) {
						node->child_nodes.insert(node->child_nodes.cend(), sres->child_nodes.begin(), sres->child_nodes.end());
						sres->child_nodes.clear();
						delete sres;
						continue;
					}
					node->child_nodes.emplace_back(sres);
				}
				else {
					if (isStarOrOneNull)
						continue;
					else {
						delete node;
						return 0;
					}
				}

			}
			if (node->child_nodes.empty()) {
				delete node;
				node = 0;
			}
			return node;
		}
		jua_ast_node* parse_or() {
			if (!global_lexer->can_get_token())
				return 0;
			auto tk = global_lexer->pop_token()->tok_t;
			int ch = global_lexer->get_trans(ruleId, tk) - 1;
			if (ch < 0)
				return 0;
			auto& rule = global_lexer->getRule(childs[ch]);
			jua::jua_ast_node* sres = 0;
			jua::jua_ast_node* node = ast_creation_fn();
			bool isStarOrPlus = rule.isStar() || rule.isPlus();
			if (isStarOrPlus) {
				size_t count = 0, used = 0;
				for (;; ++count) {
					used = global_lexer->get_used();
					sres = rule.parse();
					if (sres) {
						if (rule.isPushChildsUp() || rule.isTransient()) {
							node->child_nodes.insert(node->child_nodes.cbegin(), sres->child_nodes.begin(), sres->child_nodes.end());
							sres->child_nodes.clear();
							delete sres;
						}
						else
							node->child_nodes.emplace_back(sres);
					}
					else {
						if (count == 0 && rule.isPlus()) {
							delete node;
							return 0;
						}
						global_lexer->set_used(used);
						return node;
					}
				}
				//return node;
			}
			else {
				sres = rule.parse();
				if (!sres)
					return 0;
				if (rule.isTransient()) {
					*node = *sres;
					delete sres;
					return node;
				}
				else {
					node->child_nodes.emplace_back(sres);
					return node;
				}
			}
		}
		jua_ast_node* parse() {
			if (Flags[eRuleFollow])
				return parse_follow();
			else if (Flags[eRuleOr])
				return parse_or();
			else {
				if (!global_lexer->can_get_token())
					return 0;
				auto tok = global_lexer->get_token();
				if (tok->tok_t == lexemType) {
					jua::jua_ast_node* node = ast_creation_fn();
					*node->Token = *tok;
					return node;
				}
			}
			return 0;
		};
		jua_rule operator+(jua_rule& other) {
			if (Flags[eRuleFollow]) {
				jua_rule &r = jua_rule();
				r.Flags[eRuleFollow] = true;
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
			else if (Flags[eRuleOr]) {
				jua_rule& r = jua_rule();
				r.Flags[eRuleFollow] = true;
				if (this->ruleId == -1) {
					auto& lval = global_lexer->createRule(-1);
					lval.childs = this->childs;
					lval.Flags[eRuleOr] = true;
					this->ruleId = lval.ruleId;
				}
				r.childs.emplace_back(this->ruleId);
				if (other.ruleId == -1) {
					auto& ss = global_lexer->createRule(-1);
					ss.Flags = other.Flags;;
					ss.childs = other.childs;
					other.ruleId = ss.ruleId;
				}
				//	else
				r.childs.emplace_back(other.ruleId);
				return r;
			}
			else {
				jua_rule r = jua_rule();
				r.Flags[eRuleFollow] = true;
				r.childs.emplace_back(this->ruleId);
				if (other.ruleId == -1) {
					auto& ss = global_lexer->createRule(-1);
					ss.Flags = other.Flags;
					ss.childs = other.childs;
					other.ruleId = ss.ruleId;
				}
				r.childs.emplace_back(other.ruleId);
				return r;
			}
			return *this;
		}
		jua_rule operator|(jua_rule& other) {
			if (Flags[eRuleFollow]) {
				// if this ruleId != -1 we just need to explore child indices
				jua_rule &r = jua_rule();
				r.Flags[eRuleOr] = true;
				if (this->ruleId != -1)
					r.childs.emplace_back(this->ruleId);
				else {
					jua_rule& lf = global_lexer->createRule(-1);
					lf = *this;
					r.childs.emplace_back(lf.ruleId);
				}
				if (other.ruleId != -1) {
					r.childs.emplace_back(other.ruleId);
				}
				else {
					jua_rule& rt = global_lexer->createRule(-1);
					rt = other;
					r.childs.emplace_back(rt.ruleId);
				}
				return r;
			}
			else if (Flags[eRuleOr]) {
				jua_rule &r = jua_rule();
				r.Flags[eRuleOr] = true;
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
			else {
				jua_rule& r = jua_rule();
				r.Flags[eRuleOr] = true;
				r.childs.emplace_back(this->ruleId);
				if (other.ruleId != -1) {
					r.childs.emplace_back(other.ruleId);
				}
				else {
					jua_rule& rt = global_lexer->createRule(-1);
					rt = other;
					r.childs.emplace_back(rt.ruleId);
				}
				return r;
			}
			return *this;
		}
		jua_rule& operator=(const jua_rule& other)
		{
			this->childs = other.childs;
			this->Flags = other.Flags;
			this->lexemType = other.lexemType;
			this->ast_creation_fn = other.ast_creation_fn;
			if (this->ruleId == -1)
				this->ruleId = other.ruleId;
			return *this;
		}
		jua_rule& operator%=(const jua_rule& other)
		{
			//this->childs = other.childs;
			this->Flags[eRuleFollow] = true;
			this->Flags = other.Flags;
			this->ast_creation_fn = other.ast_creation_fn;
			this->lexemType = other.lexemType;
			if (this->ruleId != -1)
			{
				if (other.ruleId != -1)
					this->childs.emplace_back(other.ruleId);
			}
			return *this;
		}
		jua_rule& operator*()
		{
			this->Flags.set(eStar, true);
			this->Flags.set(eOneOrNull, false);
			this->Flags.set(ePlus, false);
			return *this;
		}
		jua_rule& operator+()
		{
			this->Flags.set(ePlus, true);
			this->Flags.set(eStar, false);
			this->Flags.set(eOneOrNull, false);
			return *this;
		}
		jua_rule& operator-() {
			this->Flags.set(eOneOrNull);
			this->Flags.set(eStar, false);
			this->Flags.set(ePlus, false);
		}
		/*jua_rule& operator[](int s)
		{
			return *this;
		}*/
	};
	//static_assert(sizeof(jua_rule) == 32, "Size is incorrect");
	struct jua_grammar :jua_grammar_impl
	{
	private:
		std::unordered_map<std::string_view, int> unique_ids;
		std::vector<jua::jua_token*> tokens;
		std::vector<int> _punctuation;
		unsigned int used_tok = 0, last_used = 0;
		jua::jua_ast_node* root = 0;
		int last_uid = 0;
		std::map<int, std::map<int, int>> fsm;

		void get_first_term(std::vector<int>& s, int rule) {
			auto rl = getRule(rule);
			if (rl.lexemType != -1) {
				s.emplace_back(rl.lexemType);
			}
			else {
				if (rl.Flags[eRuleFollow]) {
					get_first_term(s, rl.childs[0]);
				}
				else if (rl.Flags[eRuleOr]) {
					for (size_t n = 0; n < rl.childs.size(); ++n) {
						get_first_term(s, rl.childs[n]);
					}
				}
			}
		}
	public:
		std::vector<jua_rule*> _rules;
		jua_rule& start = createRule();
		//!! Attention jua_grammar will handle all created tokens and delete it
		// Be carefull
		jua_grammar(jua::jua_lexer*pr) :jua_grammar_impl(), unique_ids(std::move(pr->unique_id)),
			tokens(std::move(pr->tokens)), last_uid(pr->lastTypedUniqueId) {

			global_lexer = this;
		}
		void construct_grammar() {

			std::vector<int> terms;
			for (auto& rule : _rules) {
				if (rule->Flags[eRuleOr]) {
					if (rule->childs.size() == 1)
						continue;
					for (size_t n = 0; n < rule->childs.size(); ++n) {

						get_first_term(terms, rule->childs[n]);
						for (size_t i = 0; i < terms.size(); ++i) {

							if (fsm[rule->ruleId][terms[i]] == 0)
								fsm[rule->ruleId][terms[i]] = n + 1;
							else {
								std::cout << "Conflict in rule " << rule->childs[n] << " : " << rule->ruleId << " by term " << terms[i] << " has " << fsm[rule->ruleId][terms[i]] << " \n";
							}
						}
						terms.clear();
					}
				}
			}
		}

		jua::jua_ast_node* parse() {
			if (root) {
				delete root;
				root = 0;
			}
			root = start.parse();
			return root;
		}

		virtual int get_trans(int ruleiD, int tok_t) {
			return fsm[ruleiD][tok_t];
		}
		//!! Creates rule in database
		jua_rule& createRule(int lexemType = -1) {
			jua_rule* rl = _rules.emplace_back(new jua_rule(lexemType));
			rl->ruleId = 127 + _rules.size();
			return  *rl;
		}
		jua_rule& createStarRule(jua_rule& rule) {
			jua_rule* rl = _rules.emplace_back(new jua_rule(-1));
			rl->ruleId = 127 + _rules.size();
			if (rule.ruleId != -1)
				rl->childs.emplace_back(rule.ruleId);
			else {
				auto& ri = createRule();
				ri = rule;
				rl->childs.emplace_back(ri.ruleId);
			}
			rl->Flags[eStar] = true;
			return  *rl;
		}
		jua_rule& createPlusRule(jua_rule& rule) {
			jua_rule* rl = _rules.emplace_back(new jua_rule(-1));
			rl->ruleId = 127 + _rules.size(); 
			if (rule.ruleId != -1)
				rl->childs.emplace_back(rule.ruleId);
			else {
				auto& ri = createRule();
				ri = rule;
				rl->childs.emplace_back(ri.ruleId);
			}
			rl->Flags[ePlus] = true;
			return  *rl;
		}
		jua_rule& createOneOrNullRule(jua_rule& rule) {
			jua_rule* rl = _rules.emplace_back(new jua_rule(-1));
			rl->ruleId = 127 + _rules.size();
			if (rule.ruleId != -1)
				rl->childs.emplace_back(rule.ruleId);
			else {
				auto& ri = createRule();
				ri = rule;
				rl->childs.emplace_back(ri.ruleId);
			}
			rl->Flags[eOneOrNull] = true;
			return  *rl;
		}

		//!! Get rule by ruleid
		jua_rule& getRule(unsigned int ruleid) {
			return *_rules[ruleid - 128];
		}
		jua_rule* getRuleByLexT(int lex_t) {
			for (auto& n : _rules) {
				if (n->lexemType == lex_t)
					return n;
			}
			return 0;
		}
		virtual jua::jua_token* get_token() {
			used_tok++;
			return tokens[last_used = used_tok - 1];
		};

		virtual	jua::jua_token* get_token(size_t pos) {
			return tokens[pos];
		}

		virtual	jua::jua_token* pop_token() {
			return tokens[used_tok];
		}

		virtual bool can_get_token() {
			return used_tok < tokens.size();
		}
		//! sets punctuation ruleid 
		void setPunctuation(const size_t uid) {
			getRule(uid).Flags.set(ePunctuation, true);
		}
		virtual bool isPunctuation(const int uid) {
			for (int n : _punctuation) {
				if (n == uid)
					return true;
			}
			return false;
		}
		//!! creates unique id for word or if word is already presented
		// try to find it in info from lexer
		int make_word(const std::string& g) {
			auto fd = unique_ids.find(g);
			if (fd != unique_ids.end()) {
				return  fd->second;
			}
			else {
				unique_ids[g] = last_uid += 1;
				return  unique_ids[g];
			}
		}

		virtual std::string_view getTokenByUniqueID(int uid) {
			for (auto[tk, id] : unique_ids) {
				if (id == uid) {
					return const_cast<std::string_view&>(tk);
				}
			}
			return std::to_string(uid);
		}

		virtual unsigned int get_token_count() { return tokens.size(); }
		virtual unsigned int get_used() { return used_tok; };

		virtual void set_used(unsigned int used) { used_tok = used; };

		~jua_grammar() {
			delete root;
			root = 0;
			for (auto&n : tokens) {
				delete n;
				n = 0;
			}
			tokens.clear();
			for (auto& n : _rules) {
				delete n;
				n = 0;
			}
			_rules.clear();
		}
	};
}
