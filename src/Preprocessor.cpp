#include "pch.h"
#include "Preprocessor.h"
#include <regex>

extern list<string> get_file_content(const string& filename);

bool replace(string& str, const string& from, const string& to) 
{
	size_t start_pos = str.find(from);
	if (start_pos == string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

bool is_valid_var_char(char c)
{
	return isalpha(c) || c == '_' || isdigit(c);
}

DIRECTIVE Preprocessor::get_directive(const strit& str_it, const strit& str_end)
{
	string str = string(str_it, str_end);

	if (str.compare(0, 5, "ifdef") == 0)
		return DIRECTIVE::IFDEF;
	if (str.compare(0, 4, "else") == 0)
		return DIRECTIVE::ELSE;
	if (str.compare(0, 4, "elif") == 0)
		return DIRECTIVE::ELSEIF;
	if (str.compare(0, 5, "endif") == 0)
		return DIRECTIVE::ENDIF;
	if (str.compare(0, 7, "include") == 0)
		return DIRECTIVE::INCLUDE;
	if (str.compare(0, 6, "define") == 0)
	{		
		if (std::regex_match(str, std::regex("define\\s+\\w+\\s*\\((.*)\\)(.*)[\\n]*"))) // define VAR ()...
			return DIRECTIVE::DEFINE_MACROS;
		if (std::regex_match(str, std::regex("define\\s+\\w+\\s+\\w+(.*)[\\n]*"))) // define VAR ...
			return DIRECTIVE::DEFINE_VAR;
		//if (std::regex_match("define ", std::regex("define\\s+\\w+\\s*"))) // define VAR
		return DIRECTIVE::DEFINE;
	}

	return DIRECTIVE::UNKNOWN;
}

void Preprocessor::do_include(list<string>& text, list<string>::iterator& replace_it, const string & filename)
{
	list<string> lines = get_file_content(filename);
	//list<string> lines;
	//lines.push_back("replaced\n");
	replace_it = text.erase(replace_it);
	replace_it = text.insert(replace_it, lines.begin(), lines.end());
}

string Preprocessor::get_next_str(strit& it, const strit& str_end)
{
	while (it != str_end && *it == ' ') it++; //skip whitespace
	while (it != str_end && *it == '\n') it++; //skip EOL

	if (it == str_end) return string();

	if (*it == '&' && *(it + 1) == '&')
	{
		it += 2;
		return string("&&");
	}

	if (*it == '|' && *(it + 1) == '|')
	{
		it += 2;
		return string("||");
	}

	if (*it == '!' || isalpha(*it) || *it == '_')
	{
		strit def_begin = it;
		it++;
		while (it != str_end && (isalnum(*it) || *it == '_')) it++;
		return string(def_begin, it);
	}

	assert(false); // invalid string
	return string();
}

bool Preprocessor::evaluate_define_value(strit& it, const strit& str_end)
{
	bool l_operand = false;
	bool prev_was_operation = false;
	bool op_is_and = false;

	while (true)
	{
		
		string str = get_next_str(it, str_end);
		if (str.empty()) break;

		if (str == "&&")
		{
			prev_was_operation = true;
			op_is_and = true;
		}
		else if (str == "||")
		{
			prev_was_operation = true;
			op_is_and = false;
		}
		else
		{
			bool value;

			if (str[0] == '!')
			{
				string def = string(str.begin() + 1, str.end());
				value = !DefineExist(def);
			}
			else
				value = DefineExist(str);

			if (prev_was_operation)
			{
				if (op_is_and)
					l_operand = l_operand && value;
				else
					l_operand = l_operand || value;
				prev_was_operation = false;
			}
			else
			{
				l_operand = value;
			}
		}
	}

	return l_operand;
}

void Preprocessor::move_it_to_end_dirictive(strit& it, const strit& str_end)
{
	while (it != str_end && *it != ' ') it++;
}

bool Preprocessor::is_first_dericitive(const list<string>::iterator& it, const strit& str_it)
{
	strit begin_str = it->begin();
	while (begin_str != it->end() && *begin_str == ' ') begin_str++; //skip whitespace
	while (begin_str != it->end() && *begin_str == '\n') begin_str++; //skip EOL
	while (begin_str != it->end() && *begin_str == '\t') begin_str++; //skip tabs

	if (begin_str == it->end()) assert(false); // invalid arguments

	return begin_str == str_it;
}

void Preprocessor::Run(list<string>& text)
{
	struct TxtBlock
	{
		list<string>::iterator list_it1;
		strit str_it1;
		list<string>::iterator list_it2;
		strit str_it2;
	};

	bool fisrt_block_finished = false;
	bool second_block_started = false;
	TxtBlock block_tmp;
	vector<TxtBlock> text_to_remove;

	vector<list<string>::iterator> lines_to_delete_tab;

	for (auto line_it = text.begin(); line_it != text.end(); line_it++)
	{
		for (auto it = line_it->begin(); it != line_it->end(); )
		{
			while (it != line_it->end() && *it == ' ') it++; //skip whitespace
			while (it != line_it->end() && *it == '\n') it++; //skip EOL
			while (it != line_it->end() && *it == '\t') it++; //skip tabs
			if (it == line_it->end()) break;

			if (*it != '#')
				break;

			const DIRECTIVE directive = get_directive(it + 1, line_it->end());

			if (directive == DIRECTIVE::UNKNOWN)
			{
				++it;
				continue;
			}
				
			switch (directive)
			{
			// it = "#include <VAR>"
			case DIRECTIVE::INCLUDE:
			{
				string include = get_include_name(it, line_it->end());
				do_include(text, line_it, include);
				it = line_it->begin();
			}
			break;	
				
			case DIRECTIVE::DEFINE:
			{
				it += 8;
				string def = get_next_str(it, line_it->end());
				_defines.push_back(def);
			}
			break;			

			// it = "#define VAR ..."
			case DIRECTIVE::DEFINE_VAR: 
				it += 8;// todo
			
			break;

			// it = "#define MACROS(ARG1, ARG2, ...) ..."
			case DIRECTIVE::DEFINE_MACROS: 
				it += 8;// todo
			
			break;

			// it = "#ifdef ..."
			case DIRECTIVE::IFDEF:
				block_tmp.list_it1 = line_it;

				if (is_first_dericitive(line_it, it))
					block_tmp.str_it1 = line_it->begin();
				else
					block_tmp.str_it1 = it;

				move_it_to_end_dirictive(it, line_it->end());

				if (evaluate_define_value(it, line_it->end()))
				{
					block_tmp.list_it2 = line_it;
					block_tmp.str_it2 = it;
					fisrt_block_finished = true;
					text_to_remove.push_back(block_tmp);
				}
				break;

			// it = "#else ..."
			case DIRECTIVE::ELSE:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != line_it; i++)
						lines_to_delete_tab.push_back(i);

					block_tmp.list_it1 = line_it;
					block_tmp.str_it1 = it;
					second_block_started = true;
					move_it_to_end_dirictive(it, line_it->end());
				}
				else if (fisrt_block_finished && second_block_started)
				{
					move_it_to_end_dirictive(it, line_it->end());
				}
				else
				{
					move_it_to_end_dirictive(it, line_it->end());

					block_tmp.list_it2 = line_it;
					block_tmp.str_it2 = it;
					text_to_remove.push_back(block_tmp);
					fisrt_block_finished = true;
				}
				break;

			case DIRECTIVE::ELSEIF:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != line_it; i++)
						lines_to_delete_tab.push_back(i);

					block_tmp.list_it1 = line_it;
					block_tmp.str_it1 = it;
					second_block_started = true;
					move_it_to_end_dirictive(it, line_it->end());
					evaluate_define_value(it, line_it->end());
				}
				else if (fisrt_block_finished && second_block_started)
				{
					move_it_to_end_dirictive(it, line_it->end());
					evaluate_define_value(it, line_it->end());
				}
				else // fisrt_block_finished == false
				{
					move_it_to_end_dirictive(it, line_it->end());

					if (evaluate_define_value(it, line_it->end()))
					{
						block_tmp.list_it2 = line_it;
						block_tmp.str_it2 = it;
						fisrt_block_finished = true;
						text_to_remove.push_back(block_tmp);
					}
				}
				break;

			case DIRECTIVE::ENDIF:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != line_it; i++)
						lines_to_delete_tab.push_back(i);


					block_tmp.list_it1 = line_it;

					if (is_first_dericitive(line_it, it))
						block_tmp.str_it1 = line_it->begin();
					else
						block_tmp.str_it1 = it;
				}

				move_it_to_end_dirictive(it, line_it->end());

				block_tmp.list_it2 = line_it;
				block_tmp.str_it2 = it;

				text_to_remove.push_back(block_tmp);

				fisrt_block_finished = false;
				second_block_started = false;

				break;

			default: it++; break;
			}
		}
	}

	// remove one tab
	for (auto& it : lines_to_delete_tab)
	{
		if (!it->empty())
			if (it->at(0) == '\t')
				it->erase(0, 1);
	}

	// remove text blocks
	for (auto& txt_block : text_to_remove)
	{
		if (txt_block.list_it1 == txt_block.list_it2) // one line
		{
			if (txt_block.str_it1 == txt_block.list_it1->begin() && txt_block.str_it2 == txt_block.list_it2->end())
				text.erase(txt_block.list_it1);
			else
				txt_block.list_it1->erase(txt_block.str_it1, txt_block.str_it2);
		}
		else
		{
			int lines = (int)distance(txt_block.list_it1, txt_block.list_it2);

			auto it1 = txt_block.list_it1;
			++it1;
			auto it2 = txt_block.list_it2;

			// remove begining of line
			if (txt_block.str_it1 == txt_block.list_it1->begin())
				text.erase(txt_block.list_it1);
			else
				txt_block.list_it1->erase(txt_block.str_it1, txt_block.list_it1->end());

			// delete whole lines
			if (lines > 1)
				text.erase(it1, it2);

			// remove ending of line
			if (txt_block.str_it2 == txt_block.list_it2->end())
				text.erase(txt_block.list_it2);
			else
				txt_block.list_it2->erase(txt_block.list_it2->begin(), txt_block.str_it2);
		}
	}

	// remove empty lines
	//auto is_non_printale_string = [](const string& str) -> bool 
	//{ 
	//	char *s = const_cast<char*>(str.c_str()); 
	//	while (*s && !isgraph((unsigned char)*s)) 
	//		++s; 
	//	return *s == '\0'; 
	//};
	//text.remove_if([&](const string& r) -> bool { return r.empty() || r == "\t" || r == "\n" || is_non_printale_string(r); });

}

string Preprocessor::get_include_name(const strit& it_, const strit& str_end)
{
	string subject = string(it_, str_end);
	try {
		//string subject("#include <language.sh>");
		std::regex re("#include <(\\w|.+)>(.*)[\\n]*");
		std::smatch match;

		if (std::regex_search(subject, match, re))
		{
			std::cout << "include: " << match[1];
			return match[1];
		}
		else
			std::cerr << subject << " did not match\n";
		return "";
	}
	catch (std::regex_error& ) {
		std::cerr << "regex error\n";
		return "";
	}
}


