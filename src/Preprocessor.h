#pragma once

using std::string;
using std::map;
using std::list;
using std::vector;
using strit = string::iterator;


enum class DIRECTIVE
{
	UNKNOWN,
	DEFINE,
	DEFINE_VAR,
	DEFINE_MACROS,
	IFDEF,
	ELSE,
	ELSEIF,
	ENDIF,
	INCLUDE
};

class Preprocessor
{
	list<string> _defines;

	struct Macros
	{
		string fullText;
		vector<string> arguments;
	};
	map<string, Macros> _macros; // name -> macros

	DIRECTIVE get_directive(const strit& str_it, const strit& str_end);

	void do_include(list<string>& text, list<string>::iterator& replace_it, const string &filename);

	//
	// Parse include name
	// Example: "#include <some.h>"
	// Returns "some.h"
	//
	string get_include_name(const strit& it, const strit& str_end);
	
	string get_next_str(strit& it, const strit& str_end);
	bool evaluate_define_value(strit& it, const strit& str_end);
	void move_it_to_end_dirictive(strit& it,const strit& str_end);
	bool is_first_dericitive(const list<string>::iterator& line, const strit& str_it);

public:
	void SetDefine(const string& def) { _defines.push_back(def); }
	void EraseDefine(const string& def) { _defines.remove(def); }
	bool DefineExist(const string& def) { return find(_defines.begin(), _defines.end(), def) != _defines.end(); }
	void Run(list<string>& text);
};
