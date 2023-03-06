#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "parser.hh"
#include "ast.hh"
#include "llvmcodegen.hh"

extern FILE *yyin;
extern int yylex();
extern char *yytext;

NodeStmts *final_values;

#define ARG_OPTION_L 0
#define ARG_OPTION_P 1
#define ARG_OPTION_S 2
#define ARG_OPTION_O 3
#define ARG_FAIL -1

/*void expand_macro(string &str)
{
	istringstream ss(str);
	string line;
	string expanded_line;
	while (getline(ss, line))
	{
		if (!in_comment)
		{
			if (line.find("#def") == 0)
			{
				// handle macro definition
				size_t first_space = line.find(" ");
				size_t second_space = line.find(" ", first_space + 1);
				if (second_space == string::npos)
				{
					// macro without a body
					macros[line.substr(first_space + 1)] = "1";
				}
				else
				{
					string key = line.substr(first_space + 1, second_space - first_space - 1);
					string value = line.substr(second_space + 1);
					macros[key] = value;
				}
				in_macro = true;
			}
			else if (line.find("#undef") == 0)
			{
				// handle macro undefinition
				size_t first_space = line.find(" ");
				string key = line.substr(first_space + 1);
				macros.erase(key);
				in_macro = true;
			}
			else
			{
				// expand macros in line
				expanded_line = line;
				for (auto const &macro : macros)
				{
					size_t pos = 0;
					while ((pos = expanded_line.find(macro.first, pos)) != string::npos)
					{
						expanded_line.replace(pos, macro.first.length(), macro.second);
						pos += macro.second.length();
					}
				}
				// handle comments
				size_t comment_start = expanded_line.find("//");
				if (comment_start != string::npos)
				{
					expanded_line = expanded_line.substr(0, comment_start);
				}
				size_t comment_start_multi = expanded_line.find("");
				size_t comment_end_multi = expanded_line.find("");
				if (comment_start_multi != string::npos && comment_end_multi == string::npos)
				{
					in_comment = true;
					expanded_line = expanded_line.sub
				}
			}
		}
	}
} */

int parse_arguments(int argc, char *argv[])
{
	if (argc == 3 || argc == 4)
	{
		if (strlen(argv[2]) == 2 && argv[2][0] == '-')
		{
			if (argc == 3)
			{
				switch (argv[2][1])
				{
				case 'l':
					return ARG_OPTION_L;

				case 'p':
					return ARG_OPTION_P;

				case 's':
					return ARG_OPTION_S;
				}
			}
			else if (argv[2][1] == 'o')
			{
				return ARG_OPTION_O;
			}
		}
	}

	std::cerr << "Usage:\nEach of the following options halts the compilation process at the corresponding stage and prints the intermediate output:\n\n";
	std::cerr << "\t`./bin/base <file_name> -l`, to tokenize the input and print the token stream to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -p`, to parse the input and print the abstract syntax tree (AST) to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -s`, to compile the file to LLVM assembly and print it to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -o <output>`, to compile the file to LLVM bitcode and write to <output>\n";
	return ARG_FAIL;
}

int main(int argc, char *argv[])
{
	int arg_option = parse_arguments(argc, argv);
	if (arg_option == ARG_FAIL)
	{
		exit(1);
	}

	std::string file_name(argv[1]);
	FILE *source = fopen(argv[1], "r");

	if (!source)
	{
		std::cerr << "File does not exists.\n";
		exit(1);
	}

	yyin = source;

	if (arg_option == ARG_OPTION_L)
	{
		extern std::string token_to_string(int token, const char *lexeme);

		while (true)
		{
			int token = yylex();
			if (token == 0)
			{
				break;
			}

			std::cout << token_to_string(token, yytext) << "\n";
		}
		fclose(yyin);
		return 0;
	}

	final_values = nullptr;
	yyparse();

	fclose(yyin);

	if (final_values)
	{
		if (arg_option == ARG_OPTION_P)
		{
			std::cout << final_values->to_string() << std::endl;
			return 0;
		}

		llvm::LLVMContext context;
		LLVMCompiler compiler(&context, "base");
		compiler.compile(final_values);
		if (arg_option == ARG_OPTION_S)
		{
			compiler.dump();
		}
		else
		{
			compiler.write(std::string(argv[3]));
		}
	}
	else
	{
		std::cerr << "empty program";
	}

	return 0;
}