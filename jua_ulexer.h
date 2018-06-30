/*
Copyright (C) 2018 LdRoW
Do not delete this comment block. Respect others' work!
*/

#pragma once
#include <map>
#include <bitset>
#include "jua_meta.h"


#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#define _CRTDBG_MAP_ALLOC
#endif

namespace jua
{
	enum JUA_TOKEN_TYPE :int
	{
		error = -1,
		match = 0,
		any = 1,
		//!! rvalue
		tchar,
		digit,
		whitespace,
		separator
	};
	struct jua_lexer_rule {
		virtual int parse(char c) {
			return JUA_TOKEN_TYPE::match;
		}
	};
	constexpr float pair(float a, float b)
	{
		return 0.5f*(a + b)*(a + b + 1) + b;
	}
	template<typename T, typename U>
	struct jua_hasher {
	public:
		size_t operator()(std::pair<T, U>x) const throw() {
			size_t h = pair(x.first, x.second);
			return h;
		}
	};


	JUA_TOKEN_TYPE const jua_tchar = tchar;
	JUA_TOKEN_TYPE const jua_separator = separator;
	JUA_TOKEN_TYPE const jua_whspace = whitespace;
	JUA_TOKEN_TYPE const jua_digit = digit;
	JUA_TOKEN_TYPE const jua_error = error;
	JUA_TOKEN_TYPE const jua_any = any;
	JUA_TOKEN_TYPE const jua_match = match;
	JUA_TOKEN_TYPE const jua_number = (JUA_TOKEN_TYPE)7;
	JUA_TOKEN_TYPE const jua_word = (JUA_TOKEN_TYPE)8;

	struct jua_token_t_map {
	private:
		std::map<std::pair<int, int>, int> _map;
		std::bitset<256> special_char;
	public:
		jua_token_t_map() {}
		jua_token_t_map(const jua_token_t_map&other) :_map(other._map), special_char(other.special_char) {}
		jua_token_t_map& operator=(const jua_token_t_map&other)
		{
			this->_map = other._map;
			this->special_char = other.special_char;
			return *this;
		}
		int& getPairRes(int first, int second) {
			return _map[{first, second}];
		}
		int compute(int one, int two) {
			if (one == whitespace)
				return two;

			auto t = _map.find({ one,two });
			if (t != _map.end())
				return t->second;
			return jua_error;
		}
		void addSpecialChar(int d) {
			//if (!isSpecialChar(d))
			special_char[d] = true;
		}
		bool isSpecialChar(const int d) {
			return special_char[d];
		}
	};
	jua_token_t_map* global_map;

	int& operator+(const JUA_TOKEN_TYPE& one, const JUA_TOKEN_TYPE& other) {
		return global_map->getPairRes(one, other);
	}
	int& operator+(const char& one, const JUA_TOKEN_TYPE& other) {
		global_map->addSpecialChar(one);
		return global_map->getPairRes(one, other);
	}
	int& operator+(const JUA_TOKEN_TYPE& one, const char& other) {
		global_map->addSpecialChar(other);
		return global_map->getPairRes(one, other);
	}
	struct jua_token
	{
		std::string_view value;
		int tok_t;
		jua_token(int token_t, char* tok_st, int tok_s) :tok_t(token_t), value(tok_st, tok_s) {
		}
		jua_token(std::string_view str, int token_t) : value(str), tok_t(token_t) {
		}
		jua_token(const jua_token& other) :value(other.value), tok_t(other.tok_t) {
		}
		jua_token& operator=(const jua_token& other) {
			this->value = other.value;
			this->tok_t = other.tok_t;
			return *this;
		}
		std::string get_value() {
			return std::string(value.data(), value.size());
		}
	};
	struct jua_lexer
	{
	private:
		char*ptext = 0;
		size_t tsize = 0;

	public:
		int lastTypedUniqueId = 32;
		jua_token_t_map token_Tmap;
		std::vector<jua_token*> tokens;
		std::vector<std::string> reserved_words;
		std::vector<std::string> separators;
		std::unordered_map<std::string_view, int> unique_id;
		bool TryWorkIfBadLexem = false;
		bool caseSensative = false;

		jua_lexer() :tsize(0), caseSensative(false) {
			global_map = &this->token_Tmap;
		}
		~jua_lexer() {
			if (ptext)
				delete[] ptext;
		}
		inline bool isLnBreak(unsigned char c) {
			return c == '\n' || c == '\r';
		}
		inline bool isWhSpaceOrTab(unsigned char c) {
			return c == ' ' || c == '\t';
		}
		inline bool isDigit(unsigned char d) {
			return ('0' <= d) && (d <= '9');
		};
		inline bool isChar(unsigned char d) {
			return ('a' <= d && d <= 'z') || ('A' <= d && d <= 'Z');
		}
		bool isSeparator(const std::string_view&w)
		{
			for (const auto&it : separators) {
				if (it == w)
					return true;
			}
			return false;
		}
		bool isReservedWord(const std::string_view&w) {
			for (const auto& it : reserved_words) {
				if (it == w)
					return true;
			}
			return false;
		}
		inline bool isSeparator(const char char_t) {
			return !isDigit(char_t) && !isChar(char_t) && !isWhSpaceOrTab(char_t) && !isLnBreak(char_t);
		}
		virtual int getCharType(const char d) {
			if (isDigit(d)) return JUA_TOKEN_TYPE::digit;
			if (isChar(d)) return JUA_TOKEN_TYPE::tchar;
			if (isWhSpaceOrTab(d) || isLnBreak(d))	return JUA_TOKEN_TYPE::whitespace;
			if (token_Tmap.isSpecialChar(d))	return d;
			return JUA_TOKEN_TYPE::separator;
		}
		virtual bool isUncompleteType(const int t)
		{
			return false;
		}
		//!! tok_size is always > 0 , check parse function
		virtual std::string_view performValue(int tok_t, int tok_start, int tok_size)
		{
			return std::string_view(ptext + tok_start, tok_size);
		}
		//!! Produce map item with jua_whspace and jua_separator as jua_match
		template<JUA_TOKEN_TYPE right, JUA_TOKEN_TYPE...leftparam>
		constexpr void process_batch_match()
		{
			((leftparam + right = jua_match), ...);
		}
		template<auto left, JUA_TOKEN_TYPE res, auto...rparam>
		constexpr void produce_res_by_any() {
			((left + rparam = res), ...);
		};
		
			//!! Get pointer to text 
		char* getText()
		{
			return ptext;
		}
		int parse(char* text, size_t size, bool caseSense = false, bool Allow_Single_Lexem = true)
		{
			if (ptext) {
				delete[] ptext;
				ptext = 0;
			}
			ptext = new char[size + 2];
			tsize = size + 2;
			memcpy(ptext, text, size);
			ptext[size] = '\n';
			ptext[size + 1] = '\0';
			char ch;
			size_t c_pos = 0, c_line = 1;
			size_t tok_size = 0, tok_start = 0;
			int cTokenT = whitespace;
			int res_T = -2;
			int cCharT = -2;
			bool useUniqueTokenType = false;
			bool needToken = true;
			while (c_pos < tsize) {
				ch = ptext[c_pos];
				
				cCharT = getCharType(ch);
				res_T = token_Tmap.compute(cTokenT, cCharT);

				if (needToken&& res_T!= jua_whspace) {
					tok_start = c_pos;
					needToken = false;
				}
				if (res_T == jua_match) {
					if (tok_size == 0)
						return -1;
					std::string_view str = std::string_view(ptext + tok_start, tok_size);

					useUniqueTokenType = isReservedWord(str)|| cTokenT == jua_separator|| cTokenT == jua_word;

					if (useUniqueTokenType) {
						auto& pT = unique_id.find(str);
						if (pT != unique_id.end()) {
							cTokenT = pT->second;
						}
						else {
							unique_id[str] = ++lastTypedUniqueId;
							cTokenT = lastTypedUniqueId;
						}
					}
					tokens.emplace_back(new jua_token(str, cTokenT));
					cTokenT = whitespace;
					needToken = true;
					tok_size = 0;
					continue;
				}
				else if (res_T != jua_whspace) {
					cTokenT = res_T;
					tok_size++;
				}
				c_pos++;
			}
			return 0;
		}
	};
	//!! This is light and basic lexer with such defined terminals as:
	// Number,Float , Variable(With '_' char ), Separators
	struct jua_base_lexer :public jua_lexer
	{
		static const JUA_TOKEN_TYPE  jua_variable = (JUA_TOKEN_TYPE)10;
		static const JUA_TOKEN_TYPE  jua_float_number = (JUA_TOKEN_TYPE)9;
		static const JUA_TOKEN_TYPE  jua_str_src = (JUA_TOKEN_TYPE)11;
		static const JUA_TOKEN_TYPE  jua_string = (JUA_TOKEN_TYPE)12;
		jua_base_lexer() :jua_lexer() {

			// a+a=aa
			jua_tchar + jua_tchar = jua_word;
			// aa+b=aab
			jua_word + jua_tchar = jua_word;
			// aa+5=aa5 -> variable
			jua_word + jua_digit = jua_variable;
			// a+5= a5 -> variable
			jua_tchar + jua_digit = jua_variable;
			// a5 +5 = a55 
			jua_variable + jua_number = jua_variable;
			// a5 + t = a5t 
			jua_variable + jua_tchar = jua_variable;
			// |+|=||
			jua_separator + jua_separator = jua_separator;
			// 6+6 = 66
			jua_digit + jua_digit = jua_number;
			// 66+6=666
			jua_number + jua_digit = jua_number;
			// 55 + . = 55. -> float
			jua_number + '.' = jua_float_number;
			// 5 + . = 5. -> float
			jua_digit + '.' = jua_float_number;
			// 55. + 5 = 55.5
			jua_float_number + jua_digit = jua_float_number;
			char sp = '_';
			// 
			jua_tchar + sp = jua_variable;
			jua_variable + sp = jua_variable;
			sp + jua_tchar = jua_variable;

			produce_res_by_any<'\"', jua_str_src, jua_digit, jua_tchar, jua_whspace, jua_separator>();
			produce_res_by_any<jua_str_src, jua_str_src, jua_digit, jua_tchar, jua_whspace, jua_separator>();
			//'\"' + jua_any = jua_str_src;
			//jua_str_src + jua_any = jua_str_src;
			jua_str_src + '\"' = jua_string;

			process_batch_match<jua_whspace, jua_digit, jua_number, jua_string, jua_float_number, jua_variable,jua_word>();
			process_batch_match<jua_separator, jua_digit, jua_number, jua_string, jua_float_number, jua_variable,jua_word>();
			produce_res_by_any<jua_separator,
				jua_match,
				jua_whspace, jua_digit, jua_tchar,'_','\"'>();
		}
	};
}