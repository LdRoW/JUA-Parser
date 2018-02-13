

#pragma once

namespace jua
{
	template<typename T, typename U>
	struct jua_hasher {
	public:
		size_t operator()(std::pair<T, U>x) const throw() {
			size_t h = x.first^x.second;
			return h;
		}
	};
	enum jua_token_t :int
	{
		none = -1,
		//!! can be rvalue and lvalue
		tchar,
		digit,
		whitespace,
		//!! under is lvalue
		number,
		word,
		variable,
		separator
	};
	struct jua_token_t_map {
	private:
		std::unordered_map<std::pair<int, int>, int, jua_hasher<int, int>> _map;
		std::vector<int> special_char;
	public:
		int& getPairRes(std::pair<int, int> pair) {
			return _map[pair];
		}
		int operator()(int one, int two) {
			if (one == none || one == whitespace)
				return two;
			std::pair<int, int> pr{ one, two };
			if (_map.find(pr) != _map.end())
				return _map[pr];
			return -1;
		}
		void addSpecialChar(int d) {
			if (!isSpecialChar(d))
				special_char.emplace_back(d);
		}
		bool isSpecialChar(int d) {
			for (int b : special_char)
			{
				if (b == d)
					return true;
			}
			return false;
		}
	};
	jua_token_t_map* global_map;
	static jua_token_t jua_number = number;
	static jua_token_t jua_tchar = tchar;
	static jua_token_t jua_word = word;
	static jua_token_t jua_variable = variable;
	static jua_token_t jua_separator = separator;
	static jua_token_t jua_whspace = whitespace;
	static jua_token_t jua_digit = digit;

	int& operator+(const jua_token_t& one,const jua_token_t& other) {
		return global_map->getPairRes(std::pair<int, int>((int)one, (int)other));
	}
	int& operator+(const char& one,const jua_token_t& other) {
		global_map->addSpecialChar(one);
		return global_map->getPairRes(std::pair<int, int>((int)one, (int)other));
	}
	int& operator+(const jua_token_t& one,const char& other) {
		global_map->addSpecialChar(other);
		return global_map->getPairRes(std::pair<int, int>((int)one, (int)other));
	}
	struct jua_token_pos
	{
		unsigned int start;
		unsigned int size;
		jua_token_pos()
		{
			start = 0;
			size = 0;
		}
		jua_token_pos(unsigned int tstart, unsigned int tsize) :start(tstart), size(tsize)
		{
		}
		jua_token_pos(const jua_token_pos& other)
		{
			start = other.start;
			size = other.size;
		}
	};
	struct jua_token
	{
		std::string value;
		int tok_t;
		int tok_start_pos;
		int tok_sz;
		jua_token(int token_t, int tok_st, int tok_s) :tok_t(token_t), tok_start_pos(tok_st), tok_sz(tok_s) {

		}
		jua_token(char* t, int token_t, int tok_st, int tok_s) :tok_t(token_t), tok_start_pos(tok_st), tok_sz(tok_s) {
			value = std::string(t + tok_st, tok_s);
		}
		jua_token(const jua_token& other)
		{
			this->value = other.value;
			this->tok_t = other.tok_t;
			this->tok_sz = other.tok_sz;
			this->tok_start_pos = other.tok_start_pos;
		}
	};
	struct jua_lexer
	{
	private:
		char*ptext;
		int tsize;
		bool _caseSensative;

	public:
		int lastTypedUniqueId = 32;
		jua_token_t_map token_Tmap;
		std::vector<jua_token*> tokens;
		std::vector<std::string> reserved_words;
		std::unordered_map<std::string, int> unique_id;
		jua_lexer(char* text, int size, bool caseSense = false) :tsize(size), _caseSensative(caseSense) {
			ptext = new char[size + 2];
			memcpy(ptext, text, size);
			ptext[size] = '\n';
			ptext[size + 1] = '\0';
			global_map = &this->token_Tmap;
		}
		~jua_lexer() {
			delete ptext;
			for (auto&it : tokens)
			{
				delete it;
			}
		}
		inline bool isLnBreak(char c) {
			return c == '\n' || c == '\r';
		}
		inline bool isLnBreakWIn(char c1, char c) {
			return c1 == '\r'&&c == '\n';
		}
		inline bool isWhSpaceOrTab(char c) {
			return c == ' ' || c == '\t';
		}
		inline bool isDigit(char d) {
			return ('0' <= d) && (d <= '9');
		};
		inline bool isChar(char d) {
			return ('a' <= d&&d <= 'z') || ('A' <= d&&d <= 'Z');
		}
		bool isReservedWord(std::string&w) {
			for (const auto& it : reserved_words) {
				if (it == w)
					return true;
			}
			return false;
		}
		bool isSeparator(int char_t) {
			return !isDigit(char_t) && !isChar(char_t);
		}
		int getCharType(char d) {
			if (token_Tmap.isSpecialChar(d))
				return d;
			else if (isDigit(d))
				return jua_token_t::digit;
			else if (isChar(d)) {
				return jua_token_t::tchar;
			}
			else if (isWhSpaceOrTab(d) || isLnBreak(d)) {
				return jua_token_t::whitespace;
			}
			{
				return jua_token_t::separator;
			}
			return jua_token_t::none;
		}
		int parse(bool Allow_Single_Lexem=true)
		{
			char ch;
			int c_pos = 0, c_line = 1;
			int tok_size = 0, tok_start = -1;
			int cTokenT = none; int tmp_t = none;
			int cCharT = none;
			bool useUniqueTokenType = false;

			std::string tmp = "";
			while (c_pos < tsize) {
				ch = ptext[c_pos];
				if (tok_size == 0) {
					cTokenT = none;
					tok_start = c_pos;
				}

				cCharT = getCharType(ch);
				tmp_t = token_Tmap(cTokenT, cCharT);

				if (tmp_t == -1 && tok_size > 0) {
					tmp.append(ptext + tok_start, tok_size);
					if (!Allow_Single_Lexem)
					{
						if (cTokenT == word || cTokenT == tchar)cTokenT = variable;
						else if (cTokenT == digit)cTokenT = number;
					}
					useUniqueTokenType = isReservedWord(tmp) || cTokenT == separator ||  cTokenT == word;
					if (useUniqueTokenType) {

						lastTypedUniqueId++;
						if (unique_id.find(tmp) != unique_id.end()) {
							lastTypedUniqueId = unique_id[tmp];
						}
						else {
							unique_id[tmp] = lastTypedUniqueId;
						}
					}
					tokens.emplace_back(new jua_token(ptext, useUniqueTokenType ? lastTypedUniqueId : cTokenT, tok_start, tok_size));
					tmp.clear();
					tok_start = 0;
					tok_size = 0;
					cTokenT = -1;
					cCharT = -1;
				}
				else if (tmp_t != jua_token_t::none&&tmp_t != jua_token_t::whitespace) {
					++tok_size;
					cTokenT = tmp_t;
				}
				c_pos++;
			}
			return 0;
		}
	};
	//!! This is light and basic lexer with such terminals as:
	// Number , Variable(With '_' char ), Separators
	struct jua_base_lexer :public jua_lexer
	{
		jua_base_lexer(char* text, unsigned int size, bool caseSense = false) :jua_lexer(text, size, caseSense) {
			jua_tchar + jua_tchar = jua_token_t::word;
			jua_word + jua_digit = jua_variable;
			jua_word + jua_tchar = jua_word;
			jua_tchar + jua_digit = jua_variable;
			jua_variable + jua_number = jua_variable;
			jua_variable + jua_word = jua_variable;
			jua_separator + jua_separator = jua_separator;
			char sp = '_';
			jua_tchar + sp = jua_variable;
			jua_variable + sp = jua_variable;
			sp + jua_tchar = jua_variable;
		}
	};
}