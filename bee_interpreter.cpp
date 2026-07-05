#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif
#ifndef _WIN32
#include <ncurses.h>
#endif

using namespace std;
std::mt19937 rng(
    (unsigned)std::chrono::steady_clock::now()
    .time_since_epoch()
    .count()
);
enum TokenType
{
	//condition
	TK_IF,
	TK_ELSEIF,
	TK_ELSE,
	//[]
	TK_LBRACKET,
	TK_RBRACKET,
	//loops
	TK_WHILE,
	TK_FOR,
	TK_IN,
	TK_BREAK,
	TK_CONTINUE,
	//fun
	TK_FUN,
	TK_RETURN,
	//block
	TK_LBRACE,
	TK_RBRACE,
	//outs
	TK_OUTL,
	TK_OUT,
	//dot
	TK_DOT,
	//=> book
	TK_BOOK,
	TK_LIBCALL,
	//comma
	TK_COMMA,
	//vars types
	TK_INT,
	TK_STR,
	TK_CHAR,
	TK_LIST,
	TK_BOOL,
	TK_FLOAT,
	TK_STRUCT,
	//id
	TK_IDENT,
	//number
	TK_NUMBER,
	//string
	TK_STRING,
	//''for 'line1\nline2 ...'
	TK_LSTRING,
	//true and false
	TK_TRUE,
	TK_FALSE,
	//assign
	TK_ASSIGN,
	//ops
	TK_ADD,
	TK_MIN,
	TK_MUL,
	TK_DIV,
	TK_POW,
	TK_LPAREN,
	TK_RPAREN,
	// mod
	TK_MOD,

// compare
	TK_EQ,
	TK_NE,
	TK_LT,
	TK_GT,
	TK_LE,
	TK_GE,

// logical
	TK_AND,
	TK_OR,
	TK_NOT,
	//new line and EOF
	TK_NEWLINE,
	TK_EOF,
	//default
	TK_UNKNOWN
};
void EnableANSI()
{
#ifdef _WIN32
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD mode=0;
	GetConsoleMode(h,&mode);

	mode|=ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	SetConsoleMode(h,mode);
#endif
}
struct Token
{
	TokenType type;
	string value;
	int line;
};

struct Var
{
	TokenType type;
	string value;
};
struct Value
{
	TokenType type;
	string value;
};

vector<unordered_map<string,Var>> scopes;

unordered_map<string, TokenType> KEYS =
{
	{"if", TK_IF},
	{"elseif",TK_ELSEIF},
	{"else",TK_ELSE},
	{"while",TK_WHILE},
	{"for",TK_FOR},
	{"fun",TK_FUN},
	{"return",TK_RETURN},
	{"break",TK_BREAK},
	{"continue",TK_CONTINUE},
	{"outl", TK_OUTL},
	{"out", TK_OUT},
	{"int",TK_INT},
	{"str",TK_STR},
	{"char",TK_CHAR},
	{"list",TK_LIST},
	{"bool",TK_BOOL},
	{"float",TK_FLOAT},
	{"struct",TK_STRUCT},
	{"book",TK_BOOK}
};
//source code************

//source code************
//scope
void push_scope()
{
	scopes.push_back({});
}

void pop_scope()
{
	if(scopes.size() > 1)
	{
		scopes.pop_back();
	}
}
struct List
{
	vector<Value> items;
};
unordered_map<int,List> list_heap;
int next_list_id=1;
struct StructObject
{
    unordered_map<string,Value> fields;
};

unordered_map<int,StructObject> struct_heap;
int next_struct_id=1;
struct StructDef
{
    unordered_map<string,TokenType> fields;
    vector<string> order;
};

unordered_map<string,StructDef> struct_defs;
//write var
void define_var(
	const string& name,
	TokenType type,
	const string& value
)
{
	if(scopes.back().count(name))
	{
		cout<<"Variable already exists\n";
		exit(1);
	}

	scopes.back()[name] =
	{
		type,
		value
	};
}
//find var
Var* find_var(const string& name)
{
	for(
		int i = scopes.size()-1;
		i >= 0;
		i--
	)
	{
		auto it =
			scopes[i].find(name);

		if(
			it != scopes[i].end()
		)
		{
			return &it->second;
		}
	}

	return nullptr;
}
//تغییر متغییر
bool set_var(const string& name,const string& value)
{
	for(
		int i = scopes.size()-1;
		i >= 0;
		i--
	)
	{
		auto it =
			scopes[i].find(name);

		if(
			it != scopes[i].end()
		)
		{
			it->second.value =
				value;

			return true;
		}
	}

	return false;
}
//lexer
vector<Token> lexer(const string& src)
{
	//list tokens
	vector<Token> tokens;
	
	size_t i = 0;
	int line = 1;
	while (i < src.size())
	{
		char ch = src[i];
		if (ch == '\n')
		{
			tokens.push_back({TK_NEWLINE,"\\n",line});

			line++;
			i++;
			continue;
		}
		if (isspace(ch))
		{
			i++;
			continue;
		}
		
		if(ch=='#')
		{
			while(i<src.size() && src[i]!='\n')
			{
				i++;
			}
			continue;
		}
		if (isalpha(ch) || ch == '_')
		{
			string word;

			while (i < src.size() &&(isalnum(src[i]) || src[i] == '_'))
			{
				word += src[i];
				i++;
			}

			if(word=="true")
			{
				tokens.push_back({TK_TRUE,"true",line});
			}
			else if(word=="false")
			{
				tokens.push_back({TK_FALSE,"false",line});
			}
			else if(KEYS.count(word))
			{
				tokens.push_back({KEYS[word],word,line});
			}
			else
			{
				tokens.push_back({TK_IDENT,word,line});
			}

			continue;
		}
		if (ch == '"')
		{
			string str;

			i++;

			while (i < src.size() && src[i] != '"')
			{
				str += src[i];
				i++;
			}

			if (i < src.size())
			{
				i++;
			}

			tokens.push_back({TK_STRING,str,line});

			continue;
		}
		if(ch=='"' &&
   i+2<src.size() &&
   src[i+1]=='"' &&
   src[i+2]=='"')
{
    i+=3;

    string str;

    while(i+2<src.size())
    {
        if(src[i]=='"' &&
           src[i+1]=='"' &&
           src[i+2]=='"')
        {
            i+=3;
            break;
        }

        if(src[i]=='\n')
            line++;

        str+=src[i];
        i++;
    }

    tokens.push_back({
        TK_LSTRING,
        str,
        line
    });

    continue;
}
		
		if(ch=='=' && i+1<src.size() && src[i+1]=='>')
		{
			tokens.push_back({TK_LIBCALL,"=>",line});
			i+=2;
			continue;
		}
		
		if(ch=='=' && i+1<src.size() && src[i+1]=='=')
		{
			tokens.push_back({TK_EQ,"==",line});
			i+=2;
			continue;
		}

		if(ch=='!' && i+1<src.size() && src[i+1]=='=')
		{
			tokens.push_back({TK_NE,"!=",line});
			i+=2;
			continue;
		}

		if(ch=='<' && i+1<src.size() && src[i+1]=='=')
		{
			tokens.push_back({TK_LE,"<=",line});
			i+=2;
			continue;
		}

		if(ch=='>' && i+1<src.size() && src[i+1]=='=')
		{
			tokens.push_back({TK_GE,">=",line});
			i+=2;
			continue;
		}

		if(ch=='&' && i+1<src.size() && src[i+1]=='&')
		{
			tokens.push_back({TK_AND,"&&",line});
			i+=2;
			continue;
		}

		if(ch=='|' && i+1<src.size() && src[i+1]=='|')
		{
			tokens.push_back({TK_OR,"||",line});
			i+=2;
			continue;
		}
		if(ch=='%')
		{
			tokens.push_back({TK_MOD,"%",line});
			i++;
			continue;
		}

		if(ch=='<')
		{
			tokens.push_back({TK_LT,"<",line});
			i++;
			continue;
		}
		if(ch=='.')
		{
   		 tokens.push_back({TK_DOT,".",line});
   		 i++;
  		  continue;
		}

		if(ch=='>')
		{
			tokens.push_back({TK_GT,">",line});
			i++;
			continue;
		}

		if(ch=='!')
		{
			tokens.push_back({TK_NOT,"!",line});
			i++;
			continue;
		}
		if(isdigit(ch))
		{
			string num;
			bool has_dot=false;

			while(
				i<src.size() &&
				(isdigit(src[i]) || src[i]=='.')
				)
			{
				if(src[i]=='.')
				{
					if(has_dot)
						break;

					has_dot=true;
				}

				num+=src[i];
				i++;
			}

			tokens.push_back({
				TK_NUMBER,
				num,
			line
			});

			continue;
		}
		if (ch == '{')
		{
			tokens.push_back({TK_LBRACE, "{",line});
			i++;
			continue;
		}
		if (ch == '}')
		{
			tokens.push_back({TK_RBRACE, "}",line});
			i++;
			continue;
		}
		if(ch=='[')
		{
   		 tokens.push_back({TK_LBRACKET,"[",line});
   		 i++;
  		  continue;
		}

		if(ch==']')
		{
   		 tokens.push_back({TK_RBRACKET,"]",line});
 		   i++;
   		 continue;
		}
		if(ch==':')
		{
   		 tokens.push_back({TK_IN,":",line});
 		   i++;
   		 continue;
		}

		if (ch == '=')
		{
			tokens.push_back({TK_ASSIGN, "=",line});
			i++;
			continue;
		}
		if (ch == '+')
		{
			tokens.push_back({TK_ADD, "+",line});
			i++;
			continue;
		}

		if (ch == '-')
		{
			tokens.push_back({TK_MIN, "-",line});
			i++;
			continue;
		}

		if (ch == '*')
		{
			tokens.push_back({TK_MUL, "*",line});
			i++;
			continue;
		}
		if(ch == ',')
		{
			tokens.push_back({TK_COMMA,",",line});

			i++;
			continue;
		}

		if (ch == '/')
		{
			tokens.push_back({TK_DIV, "/",line});
			i++;
			continue;
		}

		if (ch == '^')
		{
			tokens.push_back({TK_POW, "^",line});
			i++;
			continue;
		}
		if(ch=='(')
		{
			tokens.push_back({TK_LPAREN,"(",line});
			i++;
			continue;
		}

		if(ch==')')
		{
			tokens.push_back({TK_RPAREN,")",line});
			i++;
			continue;
		}
		

		tokens.push_back({TK_UNKNOWN, string(1, ch),line});
		i++;
	}
	tokens.push_back({TK_EOF,"",line});
	return tokens;
}

string token_name(TokenType t)
{
	switch (t)
	{
		case TK_IF: return "TK_IF";
		case TK_ELSEIF: return "TK_ELSEIF";
		case TK_ELSE: return "TK_ELSE";
		case TK_WHILE: return "TK_WHILE";
		case TK_FOR: return "TK_FOR";
		case TK_IN: return "TK_IN";
		case TK_BREAK: return "TK_BREAK";
		case TK_CONTINUE: return "TK_CONTINUE";
		case TK_FUN: return "TK_FUN";
		case TK_RETURN: return "TK_RETURN";
		case TK_LBRACE: return "TK_LBRACE";
		case TK_RBRACE: return "TK_RBRACE";
		case TK_LBRACKET: return "TK_LBRACKET";
		case TK_RBRACKET: return "TK_RBRACKET";
		case TK_OUTL: return "TK_OUTL";
		case TK_OUT: return "TK_OUT";
		case TK_COMMA: return "TK_COMMA";
		case TK_INT: return "TK_INT";
		case TK_STR: return "TK_STR";
		case TK_BOOL: return "TK_BOOL";
		case TK_LIST: return "TK_LIST";
		case TK_FLOAT: return "TK_FLOAT";
		case TK_STRUCT: return "TK_STRUCT";
		case TK_IDENT: return "TK_IDENT";
		case TK_NUMBER: return "TK_NUMBER";
		case TK_ASSIGN: return "TK_ASSIGN";
		case TK_STRING: return "TK_STRING";
		case TK_LSTRING: return "TK_LSTRING";
		case TK_ADD: return "TK_ADD";
		case TK_MIN: return "TK_MIN";
		case TK_MUL: return "TK_MUL";
		case TK_DIV: return "TK_DIV";
		case TK_POW: return "TK_POW";
		case TK_DOT:return "TK_DOT";
		case TK_TRUE: return "TK_TRUE";
		case TK_FALSE: return "TK_FALSE";
		case TK_LPAREN: return "TK_LPAREN";
		case TK_RPAREN: return "TK_RPAREN";
		case TK_MOD: return "TK_MOD";
		case TK_EQ: return "TK_EQ";
		case TK_NE: return "TK_NE";
		case TK_LT: return "TK_LT";
		case TK_GT: return "TK_GT";
		case TK_LE: return "TK_LE";
		case TK_GE: return "TK_GE";
		case TK_AND: return "TK_AND";
		case TK_OR: return "TK_OR";
		case TK_BOOK: return "TK_BOOK";
		case TK_LIBCALL: return "TK_LIBCALL";
		case TK_NOT: return "TK_NOT";
		case TK_NEWLINE: return "TK_NEWLINE";
		case TK_EOF: return "TK_EOF";
		default: return "TK_UNKNOWN";
	}
}
struct Parameter
{
    TokenType type;
    string name;
};

struct Function
{
    TokenType returnType;
    vector<Parameter> params;
    vector<Token> body;
};
struct Book
{
	unordered_map<string,Function> funcs;
};

unordered_map<string,Book> books;
unordered_map<string, Function> functions;
struct BreakException{};
struct ContinueException{};
struct ReturnException
{
	Value value;
};
//section parsers
class Parser
{
	vector<Token> tokens;
	size_t pos;
	string currentBook="";
	unordered_map<string,Value (Parser::*)(const vector<Value>&)> builtins;

public:

	Parser(const vector<Token>& t)
	{
		tokens=t;
		pos=0;

		builtins["in"]=&Parser::builtin_in;
		builtins["len"]=&Parser::builtin_len;
		builtins["sqrt"]=&Parser::builtin_sqrt;
		builtins["pow"]=&Parser::builtin_pow;
		builtins["type"]=&Parser::builtin_type;
		builtins["abs"]=&Parser::builtin_abs;
		builtins["floor"]=&Parser::builtin_floor;
		builtins["ceil"]=&Parser::builtin_ceil;
		builtins["round"]=&Parser::builtin_round;
		builtins["sin"]=&Parser::builtin_sin;
		builtins["cos"]=&Parser::builtin_cos;
		builtins["tan"]=&Parser::builtin_tan;
		builtins["log"]=&Parser::builtin_log;
		builtins["log10"]=&Parser::builtin_log10;
		builtins["exp"]=&Parser::builtin_exp;
		builtins["min"]=&Parser::builtin_min;
		builtins["max"]=&Parser::builtin_max;
		//str
		builtins["upper"]=&Parser::builtin_upper;
		builtins["lower"]=&Parser::builtin_lower;
		builtins["trim"]=&Parser::builtin_trim;
		builtins["ltrim"]=&Parser::builtin_ltrim;
		builtins["rtrim"]=&Parser::builtin_rtrim;
		builtins["left"]=&Parser::builtin_left;
		builtins["right"]=&Parser::builtin_right;
		builtins["mid"]=&Parser::builtin_mid;
		builtins["reverse"]=&Parser::builtin_reverse;
		builtins["repeat"]=&Parser::builtin_repeat;
		builtins["replace"]=&Parser::builtin_replace;
		builtins["startswith"]=&Parser::builtin_startswith;
		builtins["endswith"]=&Parser::builtin_endswith;
		builtins["contains"]=&Parser::builtin_contains;
		builtins["find"]=&Parser::builtin_find;
		builtins["count"]=&Parser::builtin_count;
		builtins["capitalize"]=&Parser::builtin_capitalize;
		builtins["slice"]=&Parser::builtin_slice;
		builtins["split"]=&Parser::builtin_split;
		builtins["join"]=&Parser::builtin_join;
		//parse to
		builtins["Int"]=&Parser::builtin_int;
		builtins["Float"]=&Parser::builtin_float;
		builtins["Bool"]=&Parser::builtin_bool;
		builtins["Str"]=&Parser::builtin_str;
		//beep
		builtins["beep"] = &Parser::builtin_beep;
		//color clear
		builtins["color"]=&Parser::builtin_color;
		builtins["cls"]=&Parser::builtin_cls;
		//range
		builtins["range"]=&Parser::builtin_range;
		//list
		builtins["push"]=&Parser::builtin_push;
		builtins["pop"]=&Parser::builtin_pop;
		builtins["insert"]=&Parser::builtin_insert;
		builtins["remove"]=&Parser::builtin_remove;
		builtins["clear"]=&Parser::builtin_clear;
		builtins["size"]=&Parser::builtin_size;
		builtins["index"]=&Parser::builtin_index;
		builtins["count"]=&Parser::builtin_count_list;
		builtins["reverse_list"]=&Parser::builtin_reverse_list;
		builtins["copy"]=&Parser::builtin_copy;
		//file
		builtins["read"]=&Parser::builtin_read;
		builtins["write"]=&Parser::builtin_write;
		builtins["append"]=&Parser::builtin_append;
		builtins["exists"]=&Parser::builtin_exists;
		builtins["remove_file"]=&Parser::builtin_remove_file;
		builtins["rename_file"]=&Parser::builtin_rename_file;
		//terminal
		builtins["screen"]=&Parser::builtin_screen;
		builtins["endscreen"]=&Parser::builtin_endscreen;
		builtins["refresh"]=&Parser::builtin_refresh;
		builtins["move"]=&Parser::builtin_move;
		builtins["printxy"]=&Parser::builtin_printxy;
		builtins["getkey"]=&Parser::builtin_getkey;
		builtins["cursor"]=&Parser::builtin_cursor;
		builtins["box"]=&Parser::builtin_box;
		builtins["width"]=&Parser::builtin_width;
		builtins["height"]=&Parser::builtin_height;
		builtins["clear_screen"]=&Parser::builtin_clear_screen;
		builtins["clear_line"]=&Parser::builtin_clear_line;
		builtins["clear_bottom"]=&Parser::builtin_clear_bottom;
		builtins["goto"]=&Parser::builtin_goto;
		builtins["putch"]=&Parser::builtin_putch;
		builtins["hline"]=&Parser::builtin_hline;
		builtins["vline"]=&Parser::builtin_vline;
		builtins["resize"]=&Parser::builtin_resize;
		builtins["is_resize"]=&Parser::builtin_is_resize;
		builtins["echo"]=&Parser::builtin_echo;
		builtins["init_color"]=&Parser::builtin_init_color;
		builtins["color_pair"]=&Parser::builtin_color_pair;
		builtins["color_off"]=&Parser::builtin_color_off;
		builtins["delay"]=&Parser::builtin_delay;
		builtins["nodelay"]=&Parser::builtin_nodelay;
		builtins["border"]=&Parser::builtin_border;
		builtins["line"]=&Parser::builtin_line;
		builtins["fill"]=&Parser::builtin_fill;
		builtins["rand"]=&Parser::builtin_rand;
		builtins["randint"]=&Parser::builtin_randint;
		builtins["randfloat"]=&Parser::builtin_randfloat;
		builtins["seed"]=&Parser::builtin_seed;
		builtins["choice"]=&Parser::builtin_choice;
	}
	Value builtin_split(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"split(string,string)\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
	   args[1].type!=TK_STR)
	{
		cout<<"split only supports string\n";
		exit(1);
	}

	string s=args[0].value;
	string sep=args[1].value;

	vector<Value> out;

	if(sep.empty())
	{
		for(char c:s)
			out.push_back(make_str(string(1,c)));

		return make_list(out);
	}

	size_t pos=0;
	size_t last=0;

	while((pos=s.find(sep,last))!=string::npos)
	{
		out.push_back(
			make_str(
				s.substr(last,pos-last)
			)
		);

		last=pos+sep.size();
	}

	out.push_back(
		make_str(
			s.substr(last)
		)
	);

	return make_list(out);
}	
	Value builtin_join(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"join(list,string)\n";
		exit(1);
	}

	if(args[0].type!=TK_LIST ||
	   args[1].type!=TK_STR)
	{
		cout<<"join only supports list,string\n";
		exit(1);
	}

	auto &list=as_list(args[0]);

	string sep=args[1].value;
	string result;

	for(size_t i=0;i<list.items.size();i++)
	{
		if(list.items[i].type!=TK_STR)
		{
			cout<<"join list must contain strings\n";
			exit(1);
		}

		if(i)
			result+=sep;

		result+=list.items[i].value;
	}

	return make_str(result);
}
	Value builtin_choice(const vector<Value>& args)
{
    if(args.size()!=1 ||
       args[0].type!=TK_LIST)
    {
        cout<<"choice(list)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    if(l.items.empty())
    {
        cout<<"Empty list\n";
        exit(1);
    }

    uniform_int_distribution<long> d(
        0,
        l.items.size()-1
    );

    return l.items[d(rng)];
}
	Value builtin_randfloat(const vector<Value>& args)
{
    if(args.size()!=2)
    {
        cout<<"randfloat(min,max)\n";
        exit(1);
    }

    double a=
        args[0].type==TK_FLOAT ?
        as_float(args[0]) :
        as_int(args[0]);

    double b=
        args[1].type==TK_FLOAT ?
        as_float(args[1]) :
        as_int(args[1]);

    if(a>b)
        swap(a,b);

    uniform_real_distribution<double> d(a,b);

    return make_float(d(rng));
}
	Value builtin_randint(const vector<Value>& args)
{
    if(args.size()!=2)
    {
        cout<<"randint(min,max)\n";
        exit(1);
    }

    if(args[0].type!=TK_INT ||
       args[1].type!=TK_INT)
    {
        cout<<"randint(int,int)\n";
        exit(1);
    }

    long a=as_int(args[0]);
    long b=as_int(args[1]);

    if(a>b)
        swap(a,b);

    uniform_int_distribution<long> d(a,b);

    return make_int(d(rng));
}
	Value builtin_rand(const vector<Value>& args)
{
    if(!args.empty())
    {
        cout<<"rand()\n";
        exit(1);
    }

    uniform_real_distribution<double> d(0.0,1.0);

    return make_float(d(rng));
}
	Value builtin_seed(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_INT)
    {
        cout<<"seed(int)\n";
        exit(1);
    }

    rng.seed(as_int(args[0]));

    return make_bool(true);
}
	Value builtin_fill(const vector<Value>& args)
{
#ifndef _WIN32

    if(args.size()!=1)
    {
        cout<<"fill(char)\n";
        exit(1);
    }

    int h,w;

    getmaxyx(stdscr,h,w);

    char c=args[0].value[0];

    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
        {
            mvaddch(y,x,c);
        }
    }

#endif

    return make_bool(true);
}
	Value builtin_line(const vector<Value>& args)
{
#ifndef _WIN32

    if(args.size()!=4)
    {
        cout<<"line(y1,x1,y2,x2)\n";
        exit(1);
    }

    int y1=as_int(args[0]);
    int x1=as_int(args[1]);
    int y2=as_int(args[2]);
    int x2=as_int(args[3]);

    if(y1==y2)
    {
        if(x1>x2) swap(x1,x2);

        mvhline(
            y1,
            x1,
            ACS_HLINE,
            x2-x1+1
        );
    }
    else if(x1==x2)
    {
        if(y1>y2) swap(y1,y2);

        mvvline(
            y1,
            x1,
            ACS_VLINE,
            y2-y1+1
        );
    }

#endif

    return make_bool(true);
}
	Value builtin_border(const vector<Value>& args)
{
#ifndef _WIN32
    border(
        '|',
        '|',
        '-',
        '-',
        '+',
        '+',
        '+',
        '+'
    );
#endif

    return make_bool(true);
}
	Value builtin_nodelay(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"nodelay(bool)\n";
        exit(1);
    }

    nodelay(
        stdscr,
        as_bool(args[0])
    );
#endif

    return make_bool(true);
}
	Value builtin_delay(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"delay(ms)\n";
        exit(1);
    }

    napms(as_int(args[0]));
#endif
    return make_bool(true);
}
	Value builtin_color_off(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"color_off(pair)\n";
        exit(1);
    }

    attroff(COLOR_PAIR(as_int(args[0])));
#endif
    return make_bool(true);
}
	Value builtin_color_pair(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"color_pair(pair)\n";
        exit(1);
    }

    attron(COLOR_PAIR(as_int(args[0])));
#endif
    return make_bool(true);
}
	Value builtin_init_color(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=3)
    {
        cout<<"init_color(pair,fg,bg)\n";
        exit(1);
    }

    init_pair(
        as_int(args[0]),
        as_int(args[1]),
        as_int(args[2])
    );
#endif
    return make_bool(true);
}
	Token current()
	{
		return tokens[pos];
	}
	Value builtin_echo(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"echo(bool)\n";
        exit(1);
    }

    if(as_bool(args[0]))
        echo();
    else
        noecho();
#endif

    return make_bool(true);
}
	Value builtin_is_resize(const vector<Value>& args)
{
#ifndef _WIN32
    return make_bool(getch()==KEY_RESIZE);
#else
    return make_bool(false);
#endif
}
	Value builtin_resize(const vector<Value>& args)
{
#ifndef _WIN32
    resize_term(0,0);
#endif
    return make_bool(true);
}
	Value builtin_vline(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=3)
    {
        cout<<"vline(y,x,len)\n";
        exit(1);
    }

    mvvline(
        as_int(args[0]),
        as_int(args[1]),
        ACS_VLINE,
        as_int(args[2])
    );
#endif
    return make_bool(true);
}
	Value builtin_hline(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=3)
    {
        cout<<"hline(y,x,len)\n";
        exit(1);
    }

    mvhline(
        as_int(args[0]),
        as_int(args[1]),
        ACS_HLINE,
        as_int(args[2])
    );
#endif
    return make_bool(true);
}
	Value builtin_putch(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=3)
    {
        cout<<"putch(y,x,char)\n";
        exit(1);
    }

    mvaddch(
        as_int(args[0]),
        as_int(args[1]),
        args[2].value[0]
    );
#endif
    return make_bool(true);
}
	Value builtin_goto(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=2)
    {
        cout<<"goto(y,x)\n";
        exit(1);
    }

    move(as_int(args[0]),as_int(args[1]));
#endif
    return make_bool(true);
}
	Value builtin_clear_bottom(const vector<Value>& args)
{
#ifndef _WIN32
    clrtobot();
#endif
    return make_bool(true);
}
	Value builtin_clear_line(const vector<Value>& args)
{
#ifndef _WIN32
    clrtoeol();
#endif
    return make_bool(true);
}
	Value builtin_clear_screen(const vector<Value>& args)
{
#ifndef _WIN32
    clear();
#else
    system("cls");
#endif
    return make_bool(true);
}
	Value builtin_screen(const vector<Value>& args)
{
#ifdef _WIN32
    cout<<"screen() is not supported on Windows yet\n";
#else
    initscr();
    keypad(stdscr,true);
    noecho();
    cbreak();
    start_color();
#endif
    return make_bool(true);
}
Value builtin_endscreen(const vector<Value>& args)
{
#ifndef _WIN32
    endwin();
#endif
    return make_bool(true);
}
Value builtin_refresh(const vector<Value>& args)
{
#ifndef _WIN32
    refresh();
#endif
    return make_bool(true);
}
	Value builtin_move(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=2)
    {
        cout<<"move(y,x)\n";
        exit(1);
    }

    move(as_int(args[0]),as_int(args[1]));
#endif

    return make_bool(true);
}
	Value builtin_printxy(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=3)
    {
        cout<<"printxy(y,x,text)\n";
        exit(1);
    }

    mvprintw(
        as_int(args[0]),
        as_int(args[1]),
        "%s",
        args[2].value.c_str()
    );
#endif

    return make_bool(true);
}
	Value builtin_getkey(const vector<Value>& args)
{
#ifndef _WIN32
    int ch=getch();
    return make_int(ch);
#else
    return make_int(0);
#endif
}
	Value builtin_cursor(const vector<Value>& args)
{
#ifndef _WIN32
    if(args.size()!=1)
    {
        cout<<"cursor(bool)\n";
        exit(1);
    }

    curs_set(as_bool(args[0])?1:0);
#endif

    return make_bool(true);
}
	Value builtin_box(const vector<Value>& args)
{
#ifndef _WIN32
    box(stdscr,0,0);
#endif

    return make_bool(true);
}
	Value builtin_width(const vector<Value>& args)
{
#ifndef _WIN32
    int h,w;
    getmaxyx(stdscr,h,w);
    return make_int(w);
#else
    return make_int(0);
#endif
}
	Value builtin_height(const vector<Value>& args)
{
#ifndef _WIN32
    int h,w;
    getmaxyx(stdscr,h,w);
    return make_int(h);
#else
    return make_int(0);
#endif
}
	
	Value builtin_read(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_STR)
    {
        cout<<"read(path)\n";
        exit(1);
    }

    ifstream file(args[0].value);

    if(!file)
    {
        cout<<"Cannot open file\n";
        exit(1);
    }

    string text(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    return make_str(text);
}
	Value builtin_write(const vector<Value>& args)
{
    if(args.size()!=2 ||
       args[0].type!=TK_STR ||
       args[1].type!=TK_STR)
    {
        cout<<"write(path,text)\n";
        exit(1);
    }

    ofstream file(args[0].value);

    if(!file)
    {
        cout<<"Cannot open file\n";
        exit(1);
    }

    file<<args[1].value;

    return make_bool(true);
}
	Value builtin_append(const vector<Value>& args)
{
    if(args.size()!=2 ||
       args[0].type!=TK_STR ||
       args[1].type!=TK_STR)
    {
        cout<<"append(path,text)\n";
        exit(1);
    }

    ofstream file(args[0].value,ios::app);

    if(!file)
    {
        cout<<"Cannot open file\n";
        exit(1);
    }

    file<<args[1].value;

    return make_bool(true);
}
	Value builtin_exists(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_STR)
    {
        cout<<"exists(path)\n";
        exit(1);
    }

    return make_bool(
        filesystem::exists(args[0].value)
    );
}
	Value builtin_remove_file(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_STR)
    {
        cout<<"remove_file(path)\n";
        exit(1);
    }

    return make_bool(
        filesystem::remove(args[0].value)
    );
}
	Value builtin_rename_file(const vector<Value>& args)
{
    if(args.size()!=2 ||
       args[0].type!=TK_STR ||
       args[1].type!=TK_STR)
    {
        cout<<"rename_file(old,new)\n";
        exit(1);
    }

    filesystem::rename(
        args[0].value,
        args[1].value
    );

    return make_bool(true);
}
	bool match(TokenType type)
	{
		if(current().type == type)
		{
			next();
			return true;
		}

		return false;
	}
	Value builtin_range(const vector<Value>& args)
{
    long start=0;
    long stop=0;
    long step=1;

    if(args.size()==1)
    {
        if(args[0].type!=TK_INT)
        {
            cout<<"range(int)\n";
            exit(1);
        }

        stop=as_int(args[0]);
    }
    else if(args.size()==2)
    {
        if(args[0].type!=TK_INT || args[1].type!=TK_INT)
        {
            cout<<"range(int,int)\n";
            exit(1);
        }

        start=as_int(args[0]);
        stop=as_int(args[1]);
    }
    else if(args.size()==3)
    {
        if(args[0].type!=TK_INT ||
           args[1].type!=TK_INT ||
           args[2].type!=TK_INT)
        {
            cout<<"range(int,int,int)\n";
            exit(1);
        }

        start=as_int(args[0]);
        stop=as_int(args[1]);
        step=as_int(args[2]);

        if(step==0)
        {
            cout<<"range step cannot be zero\n";
            exit(1);
        }
    }
    else
    {
        cout<<"range(stop)\n";
        cout<<"range(start,stop)\n";
        cout<<"range(start,stop,step)\n";
        exit(1);
    }

    vector<Value> values;

    if(step>0)
    {
        for(long i=start;i<stop;i+=step)
            values.push_back(make_int(i));
    }
    else
    {
        for(long i=start;i>stop;i+=step)
            values.push_back(make_int(i));
    }

    return make_list(values);
}
	bool check(TokenType type)
	{
		return current().type == type;
	}
	Token expect(TokenType type)
	{
		if(current().type != type)
		{
			cout<< "Syntax Error at line "<< current().line<< '\n';

			exit(1);
		}

		Token t = current();

		next();

		return t;
	}
	
	void parse_book()
{
	expect(TK_BOOK);

	string name=expect(TK_IDENT).value;

	expect(TK_LBRACE);

	currentBook=name;

	Book book;

	while(
		!check(TK_RBRACE) &&
		!check(TK_EOF)
	)
	{
		if(check(TK_NEWLINE))
		{
			next();
			continue;
		}

		if(check(TK_FUN))
		{
			parse_book_function(book);
			continue;
		}

		cout<<"Only functions allowed inside book\n";
		exit(1);
	}

	expect(TK_RBRACE);

	books[name]=book;

	currentBook="";
}
void parse_for()
{
	expect(TK_FOR);

	string item_name = expect(TK_IDENT).value;

	expect(TK_IN);

	Value iterable = parse_or();

	size_t expr_end = pos;

	if(iterable.type!=TK_LIST && iterable.type!=TK_STR)
	{
		cout<<"for only supports list and string\n";
		exit(1);
	}

	size_t block_start = pos;

	expect(TK_LBRACE);

	int depth=1;

	size_t body_begin=pos;

	while(depth)
	{
		if(check(TK_LBRACE))
			depth++;
		else if(check(TK_RBRACE))
			depth--;

		next();
	}

	size_t body_end=pos-1;

	vector<Token> body(
    tokens.begin()+body_begin,
    tokens.begin()+body_end);

	body.push_back({TK_EOF,"",tokens[body_end].line});

	pos=block_start;

	skip_block();

	vector<Value> items;

	if(iterable.type==TK_LIST)
	{
		items=as_list(iterable).items;
	}
	else
	{
		for(char c:iterable.value)
			items.push_back(make_str(string(1,c)));
	}

	for(auto &item:items)
	{
		push_scope();

		define_var(
			item_name,
			item.type,
			item.value
		);

		Parser p(body);

		try
		{
			p.parse();
		}
		catch(ContinueException&)
		{
			pop_scope();
			continue;
		}
		catch(BreakException&)
		{
			pop_scope();
			break;
		}
		catch(ReturnException&)
		{
			pop_scope();
			throw;
		}

		pop_scope();
	}
}
void parse_book_function(Book &book)
{
	expect(TK_FUN);

	TokenType returnType=TK_BOOL;

	string name=expect(TK_IDENT).value;

	expect(TK_LPAREN);

	vector<Parameter> params;

	if(!check(TK_RPAREN))
	{
		while(true)
		{
			TokenType type;

			if(match(TK_INT))
				type=TK_INT;
			else if(match(TK_FLOAT))
				type=TK_FLOAT;
			else if(match(TK_STR))
				type=TK_STR;
			else if(match(TK_BOOL))
				type=TK_BOOL;
			else
			{
				cout<<"parameter type\n";
				exit(1);
			}

			string pname=expect(TK_IDENT).value;

			params.push_back({type,pname});

			if(check(TK_COMMA))
			{
				next();
				continue;
			}

			break;
		}
	}

	expect(TK_RPAREN);

	expect(TK_LBRACE);

	int depth=1;

	vector<Token> body;

	while(depth)
	{
		if(check(TK_LBRACE))
			depth++;

		if(check(TK_RBRACE))
			depth--;

		if(depth)
			body.push_back(current());

		next();
	}

	book.funcs[name]=
	{
		returnType,
		params,
		body
	};
}
Value call_book_function()
{
	string bookName=expect(TK_IDENT).value;

	expect(TK_LIBCALL);

	string funcName=expect(TK_IDENT).value;

	expect(TK_LPAREN);

	vector<Value> args;

	if(!check(TK_RPAREN))
	{
		while(true)
		{
			args.push_back(parse_or());

			if(check(TK_COMMA))
			{
				next();
				continue;
			}

			break;
		}
	}

	expect(TK_RPAREN);

	auto bit=books.find(bookName);

	if(bit==books.end())
	{
		cout<<"Unknown book\n";
		exit(1);
	}

	auto fit=bit->second.funcs.find(funcName);

	if(fit==bit->second.funcs.end())
	{
		cout<<"Unknown function\n";
		exit(1);
	}

	Function &f=fit->second;

	push_scope();

	for(size_t i=0;i<f.params.size();i++)
	{
		define_var(
			f.params[i].name,
			f.params[i].type,
			args[i].value
		);
	}

	Parser p(f.body);

	try
	{
		p.parse();

		pop_scope();

		return make_bool(true);
	}
	catch(ReturnException &r)
	{
		pop_scope();

		return r.value;
	}
}
	void parse()
	{
		while(!check(TK_EOF))
		{
			if(check(TK_NEWLINE))
			{
				next();
				continue;
			}
			if(
    check(TK_STRUCT) &&
    pos+2<tokens.size() &&
    tokens[pos+2].type==TK_LBRACE
)
{
    parse_struct_def();
    continue;
}
			if(
				check(TK_INT)||
				check(TK_STR)||
				check(TK_BOOL)||
				check(TK_FLOAT)||
				check(TK_LIST)||
				check(TK_STRUCT)
			)
			{
				parse_var_decl();
				continue;
			}
			if(check(TK_FOR))
			{
				parse_for();
				continue;
			}
			if(check(TK_BOOK))
{
	parse_book();
	continue;
}
			if(check(TK_FUN))
			{
    			parse_function();
    			continue;
			}
			if(check(TK_RETURN))
			{
				parse_return();
				continue;
			}

			if(check(TK_OUTL))
			{
				parse_outl();
				continue;
			}

			if(check(TK_OUT))
			{
				parse_out();
				continue;
			}

			if(check(TK_IF))
			{
				parse_if();
				continue;
			}
			if(check(TK_WHILE))
			{
				parse_while();
				continue;
			}
			if(check(TK_BREAK))
			{
				parse_break();
				continue;
			}
			if(check(TK_CONTINUE))
			{
				parse_continue();
				continue;
			}
			if(
	check(TK_IDENT) &&
	pos+1<tokens.size() &&
	tokens[pos+1].type==TK_LPAREN
			)
			{
				parse_call_statement();
				continue;
			}
			if(
	check(TK_IDENT) &&
	pos+1<tokens.size() &&
	tokens[pos+1].type==TK_LBRACKET
)
{
	parse_list_assignment();
	continue;
}
			if(
    check(TK_IDENT) &&
    pos+1<tokens.size() &&
    tokens[pos+1].type==TK_DOT
)
{
    parse_struct_assignment();
    continue;
}
			if(
    check(TK_IDENT) &&
    struct_defs.count(current().value)
)
{
    parse_struct_instance();
    continue;
}
			if(
				check(TK_IDENT) &&
				pos+1<tokens.size() &&
				tokens[pos+1].type==TK_ASSIGN
			)
			{
				parse_assignment();
				continue;
			}

			cout<<"Unexpected token: "
				<<current().value
				<<" at line "
				<<current().line
				<<'\n';

			exit(1);
		}
	}
	void parse_continue()
{
	expect(TK_CONTINUE);

	if(check(TK_NEWLINE))
		next();

	throw ContinueException();
}
	void parse_break()
{
	expect(TK_BREAK);

	if(check(TK_NEWLINE))
		next();

	throw BreakException();
}
	void parse_assignment()
	{
		Token name=current();

		next();

		expect(TK_ASSIGN);

		Var* var=find_var(name.value);

		if(var==nullptr)
		{
			cout<<"Undefined variable\n";
			exit(1);
		}

		Value value=parse_or();

		if(
			var->type==TK_INT &&
			value.type!=TK_INT
		)
		{
			cout<<"Expected int\n";
			exit(1);
		}

		if(
			var->type==TK_STR &&
			value.type!=TK_STR
		)
		{
			cout<<"Expected string\n";
			exit(1);
		}
		if(
			var->type==TK_FLOAT &&
			value.type!=TK_FLOAT
		)
		{
			cout<<"Expected float\n";
			exit(1);
		}

		if(
			var->type==TK_BOOL &&
			value.type!=TK_BOOL
		)
		{
			cout<<"Expected bool\n";
			exit(1);
		}

		set_var(
			name.value,
			value.value
		);

		if(check(TK_NEWLINE))
		{
			next();
		}
	}
	void parse_while()
{
	expect(TK_WHILE);

	// محل شروع شرط
	size_t cond_pos=pos;

	// شرط اولیه
	Value cond=parse_or();

	if(cond.type!=TK_BOOL)
	{
		cout<<"Condition must be bool\n";
		exit(1);
	}

	// محل شروع بلاک
	size_t block_pos=pos;

	// اگر بار اول شرط false بود فقط از بلاک عبور کن
	if(!as_bool(cond))
	{
		skip_block();
		return;
	}

	while(true)
	{
		// اجرای بلاک
		pos=block_pos;

		try
		{
			parse_block();
		}
		catch(ContinueException&)
		{
			// هیچ کاری نکن، فقط شرط دوباره بررسی می‌شود
		}
		catch(BreakException&)
		{
			// از حلقه خارج شو
			pos=block_pos;
			skip_block();
			break;
		}

		// بررسی دوباره شرط
		pos=cond_pos;

		cond=parse_or();

		if(cond.type!=TK_BOOL)
		{
			cout<<"Condition must be bool\n";
			exit(1);
		}

		if(!as_bool(cond))
		{
			pos=block_pos;
			skip_block();
			break;
		}
	}
}
	void next()
	{
		if(pos < tokens.size())
		{
			pos++;
		}
	}
	//out
	void parse_out()
	{
		next();

		while(
			!check(TK_NEWLINE) &&
			!check(TK_EOF)
		)
		{
			Value result=parse_or();


			if(result.type==TK_LIST)
  			  print_list(result);
			else if(result.type==TK_STRUCT)
 			   print_struct(result);
			else
  			  cout<<result.value;

			if(check(TK_COMMA))
			{
				cout<<" ";

				next();

				continue;
			}

			break;
		}

		if(check(TK_NEWLINE))
			next();
	}
	//outl
	void parse_outl()
	{
		next(); // رد شدن از outl

		while(
			!check(TK_NEWLINE) &&
			!check(TK_EOF)
		)
		{
			Value result=parse_or();

			if(result.type==TK_LIST)
  			  print_list(result);
			else if(result.type==TK_STRUCT)
			    print_struct(result);
			else
			    cout<<result.value;

			if(check(TK_COMMA))
			{
				cout << " ";

				next();

				continue;
			}

			break;
		}

		cout << '\n';

		if(check(TK_NEWLINE))
		{
			next();
		}
	}
	//run variables
	void parse_var_decl()
	{
		TokenType var_type =current().type;

		next();

		Token name =expect(TK_IDENT);

		expect(TK_ASSIGN);

		if(var_type==TK_LIST)
{
    Value value;

    if(check(TK_LBRACKET))
    {
        vector<Value> values=parse_list();
        value=make_list(values);
    }
    else
    {
        value=parse_or();

        if(value.type!=TK_LIST)
        {
            cout<<"Expected list\n";
            exit(1);
        }
    }

    define_var(
        name.value,
        TK_LIST,
        value.value
    );

    if(check(TK_NEWLINE))
        next();

    return;
}
		if(var_type==TK_STRUCT)
{
    expect(TK_LBRACE);
    expect(TK_RBRACE);

    Value value=make_struct();

    define_var(
        name.value,
        TK_STRUCT,
        value.value
    );

    if(check(TK_NEWLINE))
        next();

    return;
}
		Value value=parse_or();

		if(
			var_type == TK_INT &&
			value.type != TK_INT
		)
		{
			cout<<"Expected int\n";
			exit(1);
		}

		if(
			var_type == TK_STR &&
			value.type != TK_STR
		)
		{
			cout<<"Expected string\n";
			exit(1);
		}
		if(
			var_type==TK_FLOAT &&
			value.type!=TK_FLOAT
		)
		{
			cout<<"Expected float\n";
			exit(1);
		}

		if(
			var_type==TK_BOOL &&
			value.type!=TK_BOOL
		)
		{
			cout<<"Expected bool\n";
			exit(1);
		}

		define_var(
	name.value,
	var_type,
	value.value
		);


		if(check(TK_NEWLINE))
		{
			next();
		}
	}
	//***************
	void parse_block()
{
	expect(TK_LBRACE);

	push_scope();

	while(check(TK_NEWLINE))
		next();

	while(
		!check(TK_RBRACE) &&
		!check(TK_EOF)
	)
	{
		parse_statement();

		while(check(TK_NEWLINE))
			next();
	}

	expect(TK_RBRACE);

	pop_scope();
}
void skip_block()
{
	expect(TK_LBRACE);

	int depth=1;

	while(depth>0)
	{
		if(check(TK_EOF))
		{
			cout<<"Missing }\n";
			exit(1);
		}

		if(check(TK_LBRACE))
			depth++;
		else if(check(TK_RBRACE))
			depth--;

		next();
	}

	while(check(TK_NEWLINE))
		next();
}
void parse_if()
{
	expect(TK_IF);

	bool executed=false;

	Value cond=parse_or();

	if(cond.type!=TK_BOOL)
	{
		cout<<"Condition must be bool\n";
		exit(1);
	}

	if(as_bool(cond))
	{
		parse_block();
		executed=true;
	}
	else
	{
		skip_block();
	}

	while(check(TK_NEWLINE))
		next();

	while(check(TK_ELSEIF))
	{
		next();

		Value cond=parse_or();

		if(cond.type!=TK_BOOL)
		{
			cout<<"Condition must be bool\n";
			exit(1);
		}

		if(!executed && as_bool(cond))
		{
			parse_block();
			executed=true;
		}
		else
		{
			skip_block();
		}

		while(check(TK_NEWLINE))
			next();
	}

	if(check(TK_ELSE))
	{
		next();

		if(!executed)
			parse_block();
		else
			skip_block();
	}

	while(check(TK_NEWLINE))
		next();
}
	void parse_struct_assignment()
{
    string objName=expect(TK_IDENT).value;

    expect(TK_DOT);

    string field=expect(TK_IDENT).value;

    expect(TK_ASSIGN);

    Var *v=find_var(objName);

    if(v==nullptr || v->type!=TK_STRUCT)
    {
        cout<<"Not a struct\n";
        exit(1);
    }

    Value value=parse_or();

    Value obj=
    {
        TK_STRUCT,
        v->value
    };

    struct_set(
        obj,
        field,
        value
    );

    if(check(TK_NEWLINE))
        next();
}
	void parse_struct_def()
{
    expect(TK_STRUCT);

    string name=expect(TK_IDENT).value;

    expect(TK_LBRACE);

    StructDef def;

    while(!check(TK_RBRACE))
    {
        while(check(TK_NEWLINE))
    		next();
        TokenType t;

        if(match(TK_INT))
            t=TK_INT;
        else if(match(TK_FLOAT))
            t=TK_FLOAT;
        else if(match(TK_STR))
            t=TK_STR;
        else if(match(TK_BOOL))
            t=TK_BOOL;
        else if(match(TK_LIST))
            t=TK_LIST;
        else
        {
            cout<<"Unknown field type\n";
            exit(1);
        }

        string field=expect(TK_IDENT).value;

        def.fields[field]=t;
        def.order.push_back(field);

        if(check(TK_NEWLINE))
            next();
    }

    expect(TK_RBRACE);

    struct_defs[name]=def;
}
	Value struct_get_index(Value obj,long index)
{
    auto &s=as_struct(obj);

    for(auto &d:struct_defs)
    {
        if(d.second.fields.size()!=s.fields.size())
            continue;

        if(index<0 || index>=d.second.order.size())
        {
            cout<<"Struct index out of range\n";
            exit(1);
        }

        return s.fields[d.second.order[index]];
    }

    cout<<"Unknown struct type\n";
    exit(1);
}
	void parse_struct_instance()
{
    string typeName=expect(TK_IDENT).value;

    string varName=expect(TK_IDENT).value;

    expect(TK_ASSIGN);

    expect(TK_LBRACE);
    expect(TK_RBRACE);

    Value obj=make_struct();

    for(auto &f:struct_defs[typeName].fields)
    {
        switch(f.second)
        {
        case TK_INT:
            struct_set(obj,f.first,make_int(0));
            break;

        case TK_FLOAT:
            struct_set(obj,f.first,make_float(0));
            break;

        case TK_STR:
            struct_set(obj,f.first,make_str(""));
            break;

        case TK_BOOL:
            struct_set(obj,f.first,make_bool(false));
            break;

        case TK_LIST:
            struct_set(obj,f.first,make_list({}));
            break;

        default:
            break;
        }
    }

    define_var(
        varName,
        TK_STRUCT,
        obj.value
    );

    if(check(TK_NEWLINE))
        next();
}
	void parse_statement()
{
	if(
    check(TK_STRUCT) &&
    pos+2<tokens.size() &&
    tokens[pos+2].type==TK_LBRACE
)
{
    parse_struct_def();
    return;
}
	if(
		check(TK_INT)||
		check(TK_STR)||
		check(TK_BOOL)||
		check(TK_FLOAT)||
		check(TK_LIST)||
		check(TK_STRUCT)
	)
	{
		parse_var_decl();
		return;
	}
	if(check(TK_FOR))
{
	parse_for();
	return;
}
	if(check(TK_BOOK))
{
	parse_book();
	return;
}
	if(check(TK_FUN))
{
    parse_function();
    return;
}
	if(check(TK_RETURN))
{
	parse_return();
	return;
}
	if(check(TK_OUTL))
	{
		parse_outl();
		return;
	}

	if(check(TK_OUT))
	{
		parse_out();
		return;
	}

	if(check(TK_IF))
	{
		parse_if();
		return;
	}
	if(check(TK_WHILE))
	{
		parse_while();
		return;
	}
	if(check(TK_BREAK))
	{
		parse_break();
		return;
	}
	if(check(TK_CONTINUE))
	{
		parse_continue();
		return;
	}
	if(
	check(TK_IDENT) &&
	pos+1<tokens.size() &&
	tokens[pos+1].type==TK_LPAREN
)
	{
		parse_call_statement();
		return;
	}
	if(
	check(TK_IDENT) &&
	pos+1<tokens.size() &&
	tokens[pos+1].type==TK_LBRACKET
)
{
	parse_list_assignment();
	return;
}
	if(
    check(TK_IDENT) &&
    pos+1<tokens.size() &&
    tokens[pos+1].type==TK_DOT
)
{
    parse_struct_assignment();
    return;
}
	if(
    check(TK_IDENT) &&
    struct_defs.count(current().value)
)
{
    parse_struct_instance();
    return;
}
	
	if(
		check(TK_IDENT) &&
		pos+1<tokens.size() &&
		tokens[pos+1].type==TK_ASSIGN
	)
	{
		parse_assignment();
		return;
	}

	if(check(TK_NEWLINE))
	{
		next();
		return;
	}

	cout<<"Unexpected token: "
		<<current().value
		<<" at line "
		<<current().line
		<<'\n';
	exit(1);
}
void parse_return()
{
	expect(TK_RETURN);

	Value v;

	if(check(TK_NEWLINE))
	{
		v=make_bool(true);
	}
	else
	{
		v=parse_or();
	}

	if(check(TK_NEWLINE))
		next();

	throw ReturnException{v};
}
void parse_function()
{
	expect(TK_FUN);

	TokenType returnType = TK_BOOL; // موقت، بعداً از سینتکس بخوان

	string name = expect(TK_IDENT).value;

	expect(TK_LPAREN);

	vector<Parameter> params;

	if(!check(TK_RPAREN))
	{
		while(true)
		{
			TokenType type;

			if(match(TK_INT))
				type = TK_INT;
			else if(match(TK_FLOAT))
				type = TK_FLOAT;
			else if(match(TK_STR))
				type = TK_STR;
			else if(match(TK_BOOL))
				type = TK_BOOL;
			else
			{
				cout<<"Expected parameter type\n";
				exit(1);
			}

			string pname = expect(TK_IDENT).value;

			params.push_back({
				type,
				pname
			});

			if(check(TK_COMMA))
			{
				next();
				continue;
			}

			break;
		}
	}

	expect(TK_RPAREN);

	expect(TK_LBRACE);

	int depth = 1;

	vector<Token> body;

	while(depth)
	{
		if(check(TK_LBRACE))
			depth++;

		if(check(TK_RBRACE))
			depth--;

		if(depth)
			body.push_back(current());

		next();
	}

	functions[name]={
		returnType,
		params,
		body
	};
}
Value call_function()
{
	string name = expect(TK_IDENT).value;

	expect(TK_LPAREN);

	vector<Value> args;

	if(!check(TK_RPAREN))
	{
		while(true)
		{
			args.push_back(parse_or());

			if(check(TK_COMMA))
			{
				next();
				continue;
			}

			break;
		}
	}

	expect(TK_RPAREN);

	auto it = functions.find(name);

	if(it==functions.end())
	{
		cout<<"Undefined function\n";
		exit(1);
	}

	Function &f = it->second;

	if(args.size()!=f.params.size())
	{
		cout<<"Wrong number of arguments\n";
		exit(1);
	}

	push_scope();

	for(size_t i=0;i<f.params.size();i++)
	{
		if(args[i].type!=f.params[i].type)
		{
			cout<<"Argument type mismatch\n";
			exit(1);
		}

		define_var(
			f.params[i].name,
			f.params[i].type,
			args[i].value
		);
	}

	Parser p(f.body);

try
{
	p.parse();

	pop_scope();

	return make_bool(true); // اگر return نداشت
}
catch(ReturnException &r)
{
	pop_scope();

	return r.value;
}
}
	Value parse_factor()
	{
		Token t=current();
		if(match(TK_NOT))
		{
			Value v=parse_factor();

			if(v.type!=TK_BOOL)
			{
				cout<<"Type error\n";
				exit(1);
			}

			return make_bool(!as_bool(v));
		}
		if(match(TK_MIN))
		{
   		 Value v=parse_factor();

 		   if(v.type==TK_INT)
        		return make_int(-as_int(v));

  		  if(v.type==TK_FLOAT)
      		  return make_float(-as_float(v));

    		cout<<"Type error\n";
   		 exit(1);
		}
		if(match(TK_LPAREN))
		{
			Value v=parse_or();

			expect(TK_RPAREN);

			return v;
		}
		if(t.type==TK_NUMBER)
		{
			next();

			if(t.value.find('.')!=string::npos)
			{
				return make_float(
					stod(t.value)
				);
			}

			return make_int(
				stol(t.value)
			);
		}
		if(t.type==TK_TRUE)
		{
			next();
			return {TK_BOOL,"true"};
		}

		if(t.type==TK_FALSE)
		{
			next();
			return {TK_BOOL,"false"};
		}

		if(t.type==TK_STRING)
		{
			next();

			return make_str(
			t.value
			);
		}
		if(t.type==TK_LSTRING)
		{
			next();

			return make_str(
			t.value
			);
		}
		if(
	check(TK_IDENT) &&
	pos+1<tokens.size() &&
	tokens[pos+1].type==TK_LIBCALL
)
{
	return call_book_function();
}
		if(pos+1<tokens.size() &&
  		 tokens[pos+1].type==TK_LPAREN)
		{
  		  auto it=functions.find(t.value);

  		  if(it!=functions.end())
     		   return call_function();

 		   return parse_builtin();
		}
		if(t.type==TK_IDENT)
{
	Var* v=find_var(t.value);

	if(v==nullptr)
	{
		cout<<"Undefined variable\n";
		exit(1);
	}

	next();
	//for struct
	if(v->type==TK_STRUCT && check(TK_DOT))
{
    next();

    string field=expect(TK_IDENT).value;

    Value obj=
    {
        TK_STRUCT,
        v->value
    };

    return struct_get(obj,field);
}
	// list[index]
	if(v->type==TK_LIST && check(TK_LBRACKET))
	{
		if(v->type==TK_LIST)
{
    Value cur={TK_LIST,v->value};

    while(check(TK_LBRACKET))
    {
        next();

        Value index=parse_or();

        expect(TK_RBRACKET);
		if(cur.type==TK_STRUCT)
		{
   		 long i=as_int(index);
  		  cur=struct_get_index(cur,i);
   		 continue;
		}
        if(cur.type!=TK_LIST)
        {
            cout<<"Cannot index non-list\n";
            exit(1);
        }

        auto &list=as_list(cur);

        long i=as_int(index);

        if(i<0 || i>=list.items.size())
        {
            cout<<"Index out of range\n";
            exit(1);
        }

        cur=list.items[i];
    }

    return cur;
	}
	}

	// string[index]
	if(v->type==TK_STR && check(TK_LBRACKET))
	{
		next();

		Value index=parse_or();

		expect(TK_RBRACKET);

		if(index.type!=TK_INT)
		{
			cout<<"Index must be int\n";
			exit(1);
		}

		long i=as_int(index);

		if(i<0 || i>=v->value.size())
		{
			cout<<"Index out of range\n";
			exit(1);
		}

		return make_str(string(1,v->value[i]));
	}

	return
	{
		v->type,
		v->value
	};
}

		cout<<"Expected value\n";
		exit(1);
	}
	void parse_list_assignment()
{
	string name=expect(TK_IDENT).value;

	vector<long> indexes;

while(check(TK_LBRACKET))
{
    next();

    Value idx=parse_or();

    expect(TK_RBRACKET);

    if(idx.type!=TK_INT)
    {
        cout<<"Index must be int\n";
        exit(1);
    }

    indexes.push_back(as_int(idx));
}

	expect(TK_ASSIGN);

	Value value=parse_or();

	Var* var=find_var(name);

	if(var==nullptr)
	{
		cout<<"Undefined list\n";
		exit(1);
	}

	if(var->type!=TK_LIST)
	{
		cout<<"Variable is not list\n";
		exit(1);
	}

	Value cur={TK_LIST,var->value};

for(size_t k=0;k+1<indexes.size();k++)
{
    auto &list=as_list(cur);

    long i=indexes[k];

    if(i<0 || i>=list.items.size())
    {
        cout<<"Index out of range\n";
        exit(1);
    }

    cur=list.items[i];

    if(cur.type!=TK_LIST)
    {
        cout<<"Cannot index non-list\n";
        exit(1);
    }
}

auto &list=as_list(cur);

long last=indexes.back();

if(last<0 || last>=list.items.size())
{
    cout<<"Index out of range\n";
    exit(1);
}

list.items[last]=value;
	if(check(TK_NEWLINE))
  	  next();
}
	Value parse_power()
	{
		Value left=parse_factor();

		if(check(TK_POW))
		{
			next();

			Value right=parse_power();

			if(left.type==TK_FLOAT || right.type==TK_FLOAT)
			{
				double l = left.type==TK_FLOAT ? as_float(left) : as_int(left);
				double r = right.type==TK_FLOAT ? as_float(right) : as_int(right);

				return make_float(pow(l,r));
			}
			else
			{
				return make_int(pow(as_int(left),as_int(right)));
			}
		}

		return left;
	}
	Value parse_term()
	{
		Value left=parse_power();

		while(
			check(TK_MUL) ||
			check(TK_DIV)||
			check(TK_MOD)
		)
		{
			TokenType op=current().type;

			next();

			Value right=parse_power();

			if(left.type==TK_FLOAT || right.type==TK_FLOAT)
			{
				double l=
					left.type==TK_FLOAT ?
					as_float(left) :
					as_int(left);

				double r=
					right.type==TK_FLOAT ?
					as_float(right) :
					as_int(right);

				if(op==TK_MUL)
					left=make_float(l*r);
				else
					if (r==0)
					{
						
						cout<<"Division by zero\n";
						exit(1);
					}
					left=make_float(l/r);
			}
			else if(
				left.type==TK_INT &&
				right.type==TK_INT
			)
			{
				long l=as_int(left);
				long r=as_int(right);

				if(op==TK_MUL)
					left=make_int(l*r);
				else if (op==TK_DIV)
				{
					if(r==0)
					{
						cout<<"Division by zero\n";
						exit(1);
					}
					left=make_int(l/r);
				}
				else
					left=make_int(l%r);
			}
			else
			{
				cout<<"Type error\n";
				exit(1);
			}
		}

		return left;
	}
	Value parse_expr()
	{
	Value left=parse_term();

	while(
		check(TK_ADD) ||
		check(TK_MIN)
	)
	{
		TokenType op=current().type;

		next();

		Value right=parse_term();

		if(op==TK_ADD)
		{
			// string + string
			if(
				left.type==TK_STR &&
				right.type==TK_STR
			)
			{
				left=make_str(
					left.value+
					right.value
				);
			}
			// float/int + float/int
			else if(
				(left.type==TK_FLOAT || left.type==TK_INT) &&
				(right.type==TK_FLOAT || right.type==TK_INT)
			)
			{
				double l=
					left.type==TK_FLOAT ?
					as_float(left) :
					as_int(left);

				double r=
					right.type==TK_FLOAT ?
					as_float(right) :
					as_int(right);

				if(
					left.type==TK_FLOAT ||
					right.type==TK_FLOAT
				)
					left=make_float(l+r);
				else
					left=make_int((long)(l+r));
			}
			else
			{
				cout<<"Type error\n";
				exit(1);
			}
		}
		else
		{
			// float/int - float/int
			if(
				(left.type==TK_FLOAT || left.type==TK_INT) &&
				(right.type==TK_FLOAT || right.type==TK_INT)
			)
			{
				double l=
					left.type==TK_FLOAT ?
					as_float(left) :
					as_int(left);

				double r=
					right.type==TK_FLOAT ?
					as_float(right) :
					as_int(right);

				if(
					left.type==TK_FLOAT ||
					right.type==TK_FLOAT
				)
					left=make_float(l-r);
				else
					left=make_int((long)(l-r));
			}
			else
			{
				cout<<"Type error\n";
				exit(1);
			}
		}
	}

	return left;
	}
	
	Value parse_compare()
	{
		Value left=parse_expr();

		while(
			check(TK_LT)||
			check(TK_GT)||
			check(TK_LE)||
			check(TK_GE)
		)
		{
			TokenType op=current().type;

			next();

			Value right=parse_expr();

			bool ans=false;

			// عددی
			if(
				(left.type==TK_INT || left.type==TK_FLOAT) &&
				(right.type==TK_INT || right.type==TK_FLOAT)
			)
			{
				double l=
					left.type==TK_FLOAT ?
					as_float(left) :
					as_int(left);

				double r=
					right.type==TK_FLOAT ?
					as_float(right) :
					as_int(right);

				if(op==TK_LT) ans=l<r;
				if(op==TK_GT) ans=l>r;
				if(op==TK_LE) ans=l<=r;
				if(op==TK_GE) ans=l>=r;
			}
			// رشته
			else if(
				left.type==TK_STR &&
				right.type==TK_STR
			)
			{
				if(op==TK_LT) ans=left.value<right.value;
				if(op==TK_GT) ans=left.value>right.value;
				if(op==TK_LE) ans=left.value<=right.value;
				if(op==TK_GE) ans=left.value>=right.value;
			}
			// بولین
			else if(
				left.type==TK_BOOL &&
				right.type==TK_BOOL
			)
			{
				bool l=as_bool(left);
				bool r=as_bool(right);

				if(op==TK_LT) ans=l<r;
				if(op==TK_GT) ans=l>r;
				if(op==TK_LE) ans=l<=r;
				if(op==TK_GE) ans=l>=r;
			}
			else
			{
				cout<<"Type mismatch in comparison\n";
				exit(1);
			}

			left=make_bool(ans);
		}

		return left;
	}
	Value parse_equal()
	{
		Value left=parse_compare();

		while(
			check(TK_EQ)||
			check(TK_NE)
		)
		{
			TokenType op=current().type;

			next();

			Value right=parse_compare();

			bool ans=false;

			// int و float
			if(
				(left.type==TK_INT || left.type==TK_FLOAT) &&
				(right.type==TK_INT || right.type==TK_FLOAT)
			)
			{
				double l=
					left.type==TK_FLOAT ?
					as_float(left) :
					as_int(left);

				double r=
					right.type==TK_FLOAT ?
					as_float(right) :
					as_int(right);

				ans=(op==TK_EQ)?
					(l==r):
					(l!=r);
			}
			// string
			else if(
				left.type==TK_STR &&
				right.type==TK_STR
			)
			{
				ans=(op==TK_EQ)?
					(left.value==right.value):
					(left.value!=right.value);
			}
			// bool
			else if(
				left.type==TK_BOOL &&
				right.type==TK_BOOL
			)
			{
				bool l=as_bool(left);
				bool r=as_bool(right);

				ans=(op==TK_EQ)?
					(l==r):
					(l!=r);
			}
			else
			{
				cout<<"Type mismatch in equality\n";
				exit(1);
			}

			left=make_bool(ans);
		}

		return left;
	}
	Value parse_and()
	{
		Value left=parse_equal();

		while(check(TK_AND))
		{
			next();

			Value right=parse_equal();

			left=make_bool(
			as_bool(left)&&
			as_bool(right)
			);
		}

		return left;
	}
	Value parse_or()
	{
		Value left=parse_and();

		while(check(TK_OR))
		{
			next();

			Value right=parse_and();

			left=make_bool(
			as_bool(left)||
			as_bool(right)
			);
		}

		return left;
	}
	//func intenal
	Value parse_builtin()
	{
		Token name=expect(TK_IDENT);

		expect(TK_LPAREN);

		vector<Value> args;

		if(!check(TK_RPAREN))
		{
			while(true)
		{
				args.push_back(parse_or());

				if(check(TK_COMMA))
				{
					next();
					continue;
				}

				break;
			}
		}

		expect(TK_RPAREN);

		auto it=builtins.find(name.value);

		if(it==builtins.end())
		{
			cout<<"Unknown builtin: "<<name.value<<'\n';
			exit(1);
		}

		return (this->*(it->second))(args);
	}
	Value builtin_sqrt(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"sqrt expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT ?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(sqrt(x));
	}
	Value builtin_pow(const vector<Value>& args)
	{
		if(args.size()!=2)
		{
			cout<<"pow expects 2 arguments\n";
			exit(1);
		}

		double a=args[0].type==TK_FLOAT ?
		as_float(args[0]):
		as_int(args[0]);

		double b=args[1].type==TK_FLOAT ?
		as_float(args[1]):
		as_int(args[1]);

		return make_float(pow(a,b));
	}
	Value builtin_len(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"len expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR && args[0].type!=TK_LIST)
		{
			cout<<"len only supports {string and list}\n";
			exit(1);
		}
		if(args[0].type==TK_LIST)
		{
			return make_int((long)as_list(args[0]).items.size());
		}

		return make_int(args[0].value.size());
	}
	Value builtin_type(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"type expects 1 argument\n";
			exit(1);
		}

		switch(args[0].type)
		{
			case TK_INT:
				return make_str("int");

			case TK_FLOAT:
				return make_str("float");

			case TK_BOOL:
				return make_str("bool");

			case TK_STR:
				return make_str("str");
			case TK_LIST:
				return make_str("list");
			case TK_STRUCT:
 			   return make_str("struct");

			default:
				return make_str("unknown");
		}
	}
	Value builtin_in(const vector<Value>& args)
	{
		string s;

		getline(cin,s);

		return make_str(s);
	}
	Value builtin_abs(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"abs expects 1 argument\n";
			exit(1);
		}

		if(args[0].type==TK_INT)
			return make_int(labs(as_int(args[0])));

		return make_float(fabs(as_float(args[0])));
	}
	Value builtin_floor(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"floor expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(floor(x));
	}
	Value builtin_ceil(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"ceil expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(ceil(x));
	}
	Value builtin_round(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"round expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(round(x));
	}
	Value builtin_sin(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"sin expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(sin(x));
	}
	Value builtin_cos(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"cos expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(cos(x));
	}
	Value builtin_tan(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"tan expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(tan(x));
	}
	Value builtin_log(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"log expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		if(x<=0)
		{
			cout<<"log domain error\n";
			exit(1);
		}

		return make_float(log(x));
	}
	Value builtin_log10(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"log10 expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		if(x<=0)
		{
			cout<<"log10 domain error\n";
			exit(1);
		}

		return make_float(log10(x));
	}
	Value builtin_exp(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"exp expects 1 argument\n";
			exit(1);
		}

		double x=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		return make_float(exp(x));
	}
	Value builtin_min(const vector<Value>& args)
	{
		if(args.size()!=2)
		{
			cout<<"min expects 2 arguments\n";
			exit(1);
		}

		double a=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		double b=args[1].type==TK_FLOAT?
			as_float(args[1]):
			as_int(args[1]);

		return make_float(min(a,b));
	}
	Value builtin_max(const vector<Value>& args)
	{
		if(args.size()!=2)
		{
			cout<<"max expects 2 arguments\n";
			exit(1);
		}

		double a=args[0].type==TK_FLOAT?
		as_float(args[0]):
		as_int(args[0]);

		double b=args[1].type==TK_FLOAT?
		as_float(args[1]):
		as_int(args[1]);

		return make_float(max(a,b));
	}
	//string
	Value builtin_upper(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"upper expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR)
		{
			cout<<"upper only supports string\n";
			exit(1);
		}

		string s=args[0].value;

		for(char &c:s)
			c=toupper((unsigned char)c);

		return make_str(s);
	}
	Value builtin_lower(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"lower expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR)
		{
			cout<<"lower only supports string\n";
			exit(1);
		}

		string s=args[0].value;

		for(char &c:s)
			c=tolower((unsigned char)c);

		return make_str(s);
	}
	Value builtin_ltrim(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"ltrim expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR)
		{
			cout<<"ltrim only supports string\n";
			exit(1);
		}

		string s=args[0].value;

		size_t i=0;

		while(
			i<s.size() &&
			isspace((unsigned char)s[i])
		)
		{
			i++;
		}

		return make_str(s.substr(i));
	}
	Value builtin_rtrim(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"rtrim expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR)
		{
			cout<<"rtrim only supports string\n";
			exit(1);
		}

		string s=args[0].value;

		if(s.empty())
			return make_str("");

		int i=s.size()-1;

		while(
			i>=0 &&
			isspace((unsigned char)s[i])
		)
		{
			i--;
		}

		return make_str(s.substr(0,i+1));
	}
	Value builtin_trim(const vector<Value>& args)
	{
		if(args.size()!=1)
		{
			cout<<"trim expects 1 argument\n";
			exit(1);
		}

		if(args[0].type!=TK_STR)
		{
			cout<<"trim only supports string\n";
			exit(1);
		}

		string s=args[0].value;

		size_t start=0;

		while(
			start<s.size() &&
			isspace((unsigned char)s[start])
		)
		{
			start++;
		}

		if(start==s.size())
			return make_str("");

		int end=s.size()-1;

		while(
			end>=0 &&
			isspace((unsigned char)s[end])
		)
		{
			end--;
		}

		return make_str(
			s.substr(start,end-start+1)
		);
	}
	Value builtin_left(const vector<Value>& args)
	{
		if(args.size()!=2)
		{
			cout<<"left expects 2 arguments\n";
			exit(1);
		}

		if(args[0].type!=TK_STR || args[1].type!=TK_INT)
		{
			cout<<"left(string,int)\n";
			exit(1);
		}

		long n=as_int(args[1]);

		if(n<0)
			n=0;

		if(n>(long)args[0].value.size())
			n=args[0].value.size();

		return make_str(args[0].value.substr(0,n));
	}
	Value builtin_right(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"right expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR || args[1].type!=TK_INT)
	{
		cout<<"right(string,int)\n";
		exit(1);
	}

	long n=as_int(args[1]);

	if(n<0)
		n=0;

	if(n>(long)args[0].value.size())
		n=args[0].value.size();

	return make_str(
		args[0].value.substr(
			args[0].value.size()-n
		)
	);
}
Value builtin_mid(const vector<Value>& args)
{
	if(args.size()!=3)
	{
		cout<<"mid expects 3 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_INT ||
		args[2].type!=TK_INT)
	{
		cout<<"mid(string,int,int)\n";
		exit(1);
	}

	long start=as_int(args[1]);
	long len=as_int(args[2]);

	if(start<0)
		start=0;

	if(len<0)
		len=0;

	return make_str(
		args[0].value.substr(start,len)
	);
}
Value builtin_reverse(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"reverse expects 1 argument\n";
		exit(1);
	}

	if(args[0].type!=TK_STR)
	{
		cout<<"reverse only supports string\n";
		exit(1);
	}

	string s=args[0].value;

	reverse(s.begin(),s.end());

	return make_str(s);
}
Value builtin_repeat(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"repeat expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_INT)
	{
		cout<<"repeat(string,int)\n";
		exit(1);
	}

	long n=as_int(args[1]);

	if(n<0)
		n=0;

	string out;

	for(long i=0;i<n;i++)
		out+=args[0].value;

	return make_str(out);
}
Value builtin_replace(const vector<Value>& args)
{
	if(args.size()!=3)
	{
		cout<<"replace expects 3 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR ||
		args[2].type!=TK_STR)
	{
		cout<<"replace(string,string,string)\n";
		exit(1);
	}

	string s=args[0].value;
	string from=args[1].value;
	string to=args[2].value;

	size_t pos=0;

	while((pos=s.find(from,pos))!=string::npos)
	{
		s.replace(pos,from.size(),to);
		pos+=to.size();
	}

	return make_str(s);
}
Value builtin_contains(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"contains expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR)
	{
		cout<<"contains(string,string)\n";
		exit(1);
	}

	return make_bool(
		args[0].value.find(args[1].value)!=string::npos
	);
}
Value builtin_startswith(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"startswith expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR)
	{
		cout<<"startswith(string,string)\n";
		exit(1);
	}

	string s=args[0].value;
	string p=args[1].value;

	if(p.size()>s.size())
		return make_bool(false);

	return make_bool(
		s.compare(0,p.size(),p)==0
	);
}
Value builtin_endswith(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"endswith expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR)
	{
		cout<<"endswith(string,string)\n";
		exit(1);
	}

	string s=args[0].value;
	string p=args[1].value;

	if(p.size()>s.size())
		return make_bool(false);

	return make_bool(
		s.compare(
			s.size()-p.size(),
			p.size(),
			p
		)==0
	);
}
Value builtin_find(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"find expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR)
	{
		cout<<"find(string,string)\n";
		exit(1);
	}

	size_t pos=args[0].value.find(args[1].value);

	if(pos==string::npos)
		return make_int(-1);

	return make_int((long)pos);
}
Value builtin_count(const vector<Value>& args)
{
	if(args.size()!=2)
	{
		cout<<"count expects 2 arguments\n";
		exit(1);
	}

	if(args[0].type!=TK_STR ||
		args[1].type!=TK_STR)
	{
		cout<<"count(string,string)\n";
		exit(1);
	}

	string s=args[0].value;
	string sub=args[1].value;

	if(sub.empty())
		return make_int(0);

	long cnt=0;
	size_t pos=0;

	while((pos=s.find(sub,pos))!=string::npos)
	{
		cnt++;
		pos+=sub.size();
	}

	return make_int(cnt);
}
Value builtin_capitalize(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"capitalize expects 1 argument\n";
		exit(1);
	}

	if(args[0].type!=TK_STR)
	{
		cout<<"capitalize only supports string\n";
		exit(1);
	}

	string s=args[0].value;

	if(!s.empty())
	{
		s[0]=toupper((unsigned char)s[0]);

		for(size_t i=1;i<s.size();i++)
			s[i]=tolower((unsigned char)s[i]);
	}

	return make_str(s);
}
	Value builtin_int(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"int expects 1 argument\n";
		exit(1);
	}

	switch(args[0].type)
	{
		case TK_INT:
			return args[0];

		case TK_FLOAT:
			return make_int((long)as_float(args[0]));

		case TK_BOOL:
			return make_int(as_bool(args[0])?1:0);

		case TK_STR:
		{
			try
			{
				size_t p;

				long x=stol(args[0].value,&p);

				if(p!=args[0].value.size())
					throw 1;

				return make_int(x);
			}
			catch(...)
			{
				cout<<"Cannot convert string to int\n";
				exit(1);
			}
		}

		default:
			cout<<"Cannot convert to int\n";
			exit(1);
	}
}
	Value builtin_float(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"float expects 1 argument\n";
		exit(1);
	}

	switch(args[0].type)
	{
		case TK_FLOAT:
			return args[0];

		case TK_INT:
			return make_float(as_int(args[0]));

		case TK_BOOL:
			return make_float(as_bool(args[0])?1.0:0.0);

		case TK_STR:
		{
			try
			{
				size_t p;

				double x=stod(args[0].value,&p);

				if(p!=args[0].value.size())
					throw 1;

				return make_float(x);
			}
			catch(...)
			{
				cout<<"Cannot convert string to float\n";
				exit(1);
			}
		}

		default:
			cout<<"Cannot convert to float\n";
			exit(1);
	}
}
	Value builtin_bool(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"bool expects 1 argument\n";
		exit(1);
	}

	switch(args[0].type)
	{
		case TK_BOOL:
			return args[0];

		case TK_INT:
			return make_bool(as_int(args[0])!=0);

		case TK_FLOAT:
			return make_bool(as_float(args[0])!=0);

		case TK_STR:
		{
			string s=args[0].value;

			if(s.empty())
				return make_bool(false);

			if(s=="false")
				return make_bool(false);

			if(s=="0")
				return make_bool(false);

			return make_bool(true);
		}

		default:
			cout<<"Cannot convert to bool\n";
			exit(1);
	}
}
	Value builtin_str(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"str expects 1 argument\n";
		exit(1);
	}

	switch(args[0].type)
	{
		case TK_STR:
			return args[0];

		case TK_INT:
			return make_str(args[0].value);

		case TK_FLOAT:
			return make_str(args[0].value);

		case TK_BOOL:
			return make_str(as_bool(args[0])?"true":"false");

		default:
			cout<<"Cannot convert to string\n";
			exit(1);
	}
}

	Value make_int(long x)
	{
		return {
		TK_INT,
		to_string(x)
		};
	}

	Value make_str(const string& s)
	{
		return {
		TK_STR,
		s
		};
	}

	long as_int(const Value& v)
	{
		return stol(v.value);
	}
	Value make_float(double x)
	{
		return {
		TK_FLOAT,
		to_string(x)
		};
	}

	double as_float(const Value& v)
	{
		return stod(v.value);
	}
	Value make_bool(bool x)
	{
		return {TK_BOOL,x ? "true" : "false"};
	}

	bool as_bool(const Value& v)
	{
		return v.value=="true";
	}
	Value builtin_beep(const vector<Value>& args)
{
	long freq = 1000;
	long duration = 200;

	if(args.size() >= 1)
	{
		if(args[0].type != TK_INT)
		{
			cout<<"beep(freq,duration): freq must be int\n";
			exit(1);
		}

		freq = as_int(args[0]);
	}

	if(args.size() >= 2)
	{
		if(args[1].type != TK_INT)
		{
			cout<<"beep(freq,duration): duration must be int\n";
			exit(1);
		}

		duration = as_int(args[1]);
	}

	PlatformBeep(freq,duration);

	return make_bool(true);
}
	Value builtin_slice(const vector<Value>& args)
{
	if(args.size()!=3)
	{
		cout<<"slice(string,start,end)\n";
		exit(1);
	}

	if(
		args[0].type!=TK_STR ||
		args[1].type!=TK_INT ||
		args[2].type!=TK_INT
	)
	{
		cout<<"slice(string,int,int)\n";
		exit(1);
	}

	string s=args[0].value;

	long start=as_int(args[1]);
	long end=as_int(args[2]);

	long len=s.size();

	if(start<0)
		start=len+start;

	if(end<0)
		end=len+end;

	if(start<0)
		start=0;

	if(end>len)
		end=len;

	if(start>end)
		return make_str("");

	string out;

	for(long i=start;i<end;i++)
		out+=s[i];

	return make_str(out);
}
void PlatformBeep(int frequency,int duration)
{
#ifdef _WIN32
    Beep(frequency,duration);

#elif defined(__ANDROID__)
	cout<<"\a";
    cout.flush();
    //cout<<"[BEEP "<<frequency<<"Hz "<<duration<<"ms]\n";

#elif defined(__linux__)
    cout<<"\a";
    cout.flush();

#else
    cout<<"\a";
    cout.flush();
#endif
}
	Value builtin_color(const vector<Value>& args)
{
	if(args.size()!=1)
	{
		cout<<"color expects 1 argument\n";
		exit(1);
	}

	if(args[0].type!=TK_STR)
	{
		cout<<"color(string)\n";
		exit(1);
	}

	string c=args[0].value;

	// Normal colors
	if(c=="black")          cout<<"\033[30m";
	else if(c=="red")       cout<<"\033[31m";
	else if(c=="green")     cout<<"\033[32m";
	else if(c=="yellow")    cout<<"\033[33m";
	else if(c=="blue")      cout<<"\033[34m";
	else if(c=="magenta")   cout<<"\033[35m";
	else if(c=="cyan")      cout<<"\033[36m";
	else if(c=="white")     cout<<"\033[37m";

	// Bright colors
	else if(c=="gray")          cout<<"\033[90m"; // Bright Black
	else if(c=="lightred")      cout<<"\033[91m";
	else if(c=="lightgreen")    cout<<"\033[92m";
	else if(c=="lightyellow")   cout<<"\033[93m";
	else if(c=="lightblue")     cout<<"\033[94m";
	else if(c=="lightmagenta")  cout<<"\033[95m";
	else if(c=="lightcyan")     cout<<"\033[96m";
	else if(c=="brightwhite")   cout<<"\033[97m";

	// Reset
	else if(c=="reset")         cout<<"\033[0m";

	else
	{
		cout<<"Unknown color: "<<c<<'\n';
		exit(1);
	}

	cout.flush();

	return make_bool(true);
}
	Value builtin_cls(const vector<Value>& args)
{
	cout<<"\033[2J";
	cout<<"\033[H";
	cout.flush();

	return make_bool(true);
}
	void parse_call_statement()
{
	if(current().type != TK_IDENT)
		return;

	if(pos + 1 >= tokens.size())
		return;

	if(tokens[pos+1].type != TK_LPAREN)
		return;

	auto it = functions.find(current().value);

	if(it != functions.end())
	{
		call_function();
	}
	else
	{
		parse_builtin();
	}

	if(check(TK_NEWLINE))
		next();
}
	Value make_struct()
{
    int id=next_struct_id++;

    struct_heap[id]=StructObject();

    return
    {
        TK_STRUCT,
        to_string(id)
    };
}

StructObject& as_struct(const Value& v)
{
    return struct_heap[stoi(v.value)];
}

void struct_set(
    Value obj,
    const string& name,
    const Value& value
)
{
    as_struct(obj).fields[name]=value;
}

Value struct_get(
    Value obj,
    const string& name
)
{
    auto &m=as_struct(obj).fields;

    auto it=m.find(name);

    if(it==m.end())
    {
        cout<<"Unknown field "<<name<<'\n';
        exit(1);
    }

    return it->second;
}
	Value make_list(const vector<Value>& values)
{
	int id=next_list_id++;

	list_heap[id].items=values;

	return
	{
		TK_LIST,
		to_string(id)
	};
}
	List& as_list(const Value& v)
{
	return list_heap[stoi(v.value)];
}
	vector<Value> parse_list()
{
    vector<Value> values;

    expect(TK_LBRACKET);

    if(!check(TK_RBRACKET))
    {
        while(true)
        {
            if(check(TK_LBRACKET))
            {
                vector<Value> child=parse_list();
                values.push_back(make_list(child));
            }
            else
            {
                values.push_back(parse_or());
            }

            if(check(TK_COMMA))
            {
                next();
                continue;
            }

            break;
        }
    }

    expect(TK_RBRACKET);

    return values;
}
	void print_list(const Value &v)
{
    auto &list=as_list(v);

    cout<<"[";

    for(size_t i=0;i<list.items.size();i++)
    {
        if(list.items[i].type==TK_LIST)
  		  print_list(list.items[i]);
		else if(list.items[i].type==TK_STRUCT)
    		print_struct(list.items[i]);
		else
   		 cout<<list.items[i].value;

        if(i+1<list.items.size())
            cout<<",";
    }

    cout<<"]";
}
	void print_struct(const Value& v)
{
    auto &s=as_struct(v);

    cout<<"{";

    bool first=true;

    for(auto &it:s.fields)
    {
        if(!first)
            cout<<",";

        first=false;

        cout<<it.first<<":";

        if(it.second.type==TK_LIST)
            print_list(it.second);
        else if(it.second.type==TK_STRUCT)
            print_struct(it.second);
        else
            cout<<it.second.value;
    }

    cout<<"}";
}
	Value builtin_push(const vector<Value>& args)
{
    if(args.size()!=2 || args[0].type!=TK_LIST)
    {
        cout<<"push(list,value)\n";
        exit(1);
    }

    as_list(args[0]).items.push_back(args[1]);

    return make_bool(true);
}
	Value builtin_pop(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_LIST)
    {
        cout<<"pop(list)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    if(l.items.empty())
    {
        cout<<"Empty list\n";
        exit(1);
    }

    Value v=l.items.back();

    l.items.pop_back();

    return v;
}
	Value builtin_insert(const vector<Value>& args)
{
    if(args.size()!=3 || args[0].type!=TK_LIST || args[1].type!=TK_INT)
    {
        cout<<"insert(list,index,value)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    long i=as_int(args[1]);

    if(i<0 || i>l.items.size())
    {
        cout<<"Index out of range\n";
        exit(1);
    }

    l.items.insert(l.items.begin()+i,args[2]);

    return make_bool(true);
}
	Value builtin_remove(const vector<Value>& args)
{
    if(args.size()!=2 || args[0].type!=TK_LIST || args[1].type!=TK_INT)
    {
        cout<<"remove(list,index)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    long i=as_int(args[1]);

    if(i<0 || i>=l.items.size())
    {
        cout<<"Index out of range\n";
        exit(1);
    }

    l.items.erase(l.items.begin()+i);

    return make_bool(true);
}
	Value builtin_clear(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_LIST)
    {
        cout<<"clear(list)\n";
        exit(1);
    }

    as_list(args[0]).items.clear();

    return make_bool(true);
}
	Value builtin_size(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_LIST)
    {
        cout<<"size(list)\n";
        exit(1);
    }

    return make_int(as_list(args[0]).items.size());
}
	Value builtin_index(const vector<Value>& args)
{
    if(args.size()!=2 || args[0].type!=TK_LIST)
    {
        cout<<"index(list,value)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    for(size_t i=0;i<l.items.size();i++)
    {
        if(l.items[i].type==args[1].type &&
           l.items[i].value==args[1].value)
            return make_int(i);
    }

    return make_int(-1);
}
	Value builtin_count_list(const vector<Value>& args)
{
    if(args.size()!=2 || args[0].type!=TK_LIST)
    {
        cout<<"count(list,value)\n";
        exit(1);
    }

    long c=0;

    auto &l=as_list(args[0]);

    for(auto &v:l.items)
    {
        if(v.type==args[1].type &&
           v.value==args[1].value)
            c++;
    }

    return make_int(c);
}
	Value builtin_reverse_list(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_LIST)
    {
        cout<<"reverse_list(list)\n";
        exit(1);
    }

    auto &l=as_list(args[0]);

    reverse(l.items.begin(),l.items.end());

    return make_bool(true);
}
	Value builtin_copy(const vector<Value>& args)
{
    if(args.size()!=1 || args[0].type!=TK_LIST)
    {
        cout<<"copy(list)\n";
        exit(1);
    }

    vector<Value> v=as_list(args[0]).items;

    return make_list(v);
}
};
//section parsers

int main()
{
string source = R"(
str txt="abcdefg"

while true{
	int i=0
	while i<len(txt){
		out txt[i]
		i=i+1
	}
	i=len(txt)-1
	while i>=0{
		out txt[i]
		i=i-1
	}
}
)";
	EnableANSI();
	vector<Token> tokens = lexer(source);

	for(const Token& t : tokens)
	{
	cout<< token_name(t.type)<< " -> "<< t.value<< " (line "<< t.line<< ")\n";
	}
	push_scope();

	Parser parser(tokens);

	parser.parse();
	
}
//builtins["in"]=&Parser::builtin_in;
//		builtins["len"]=&Parser::builtin_len;
//		builtins["sqrt"]=&Parser::builtin_sqrt;
//		builtins["pow"]=&Parser::builtin_pow;
//		builtins["type"]=&Parser::builtin_type;
//		builtins["abs"]=&Parser::builtin_abs;
//		builtins["floor"]=&Parser::builtin_floor;
//		builtins["ceil"]=&Parser::builtin_ceil;
//		builtins["round"]=&Parser::builtin_round;
//		builtins["sin"]=&Parser::builtin_sin;
//		builtins["cos"]=&Parser::builtin_cos;
//		builtins["tan"]=&Parser::builtin_tan;
//		builtins["log"]=&Parser::builtin_log;
//		builtins["log10"]=&Parser::builtin_log10;
//		builtins["exp"]=&Parser::builtin_exp;
//		builtins["min"]=&Parser::builtin_min;
//		builtins["max"]=&Parser::builtin_max;
//		//str
//		builtins["upper"]=&Parser::builtin_upper;
//		builtins["lower"]=&Parser::builtin_lower;
//		builtins["trim"]=&Parser::builtin_trim;
//		builtins["ltrim"]=&Parser::builtin_ltrim;
//		builtins["rtrim"]=&Parser::builtin_rtrim;
//		builtins["left"]=&Parser::builtin_left;
//		builtins["right"]=&Parser::builtin_right;
//		builtins["mid"]=&Parser::builtin_mid;
//		builtins["reverse"]=&Parser::builtin_reverse;
//		builtins["repeat"]=&Parser::builtin_repeat;
//		builtins["replace"]=&Parser::builtin_replace;
//		builtins["startswith"]=&Parser::builtin_startswith;
//		builtins["endswith"]=&Parser::builtin_endswith;
//		builtins["contains"]=&Parser::builtin_contains;
//		builtins["find"]=&Parser::builtin_find;
//		builtins["count"]=&Parser::builtin_count;
//		builtins["capitalize"]=&Parser::builtin_capitalize;
//		builtins["slice"]=&Parser::builtin_slice;
//			builtins["split"]=&Parser::builtin_split;
//			builtins["join"]=&Parser::builtin_join;
//		//parse to
//		builtins["Int"]=&Parser::builtin_int;
//		builtins["Float"]=&Parser::builtin_float;
//		builtins["Bool"]=&Parser::builtin_bool;
//		builtins["Str"]=&Parser::builtin_str;
//		//beep
//		builtins["beep"] = &Parser::builtin_beep;
//		//color clear
//		builtins["color"]=&Parser::builtin_color;
//		builtins["cls"]=&Parser::builtin_cls;
//		//range
//		builtins["range"]=&Parser::builtin_range;
//		//list
//		builtins["push"]=&Parser::builtin_push;
//		builtins["pop"]=&Parser::builtin_pop;
//		builtins["insert"]=&Parser::builtin_insert;
//		builtins["remove"]=&Parser::builtin_remove;
//		builtins["clear"]=&Parser::builtin_clear;
//		builtins["size"]=&Parser::builtin_size;
//		builtins["index"]=&Parser::builtin_index;
//		builtins["count"]=&Parser::builtin_count_list;
//		builtins["reverse_list"]=&Parser::builtin_reverse_list;
//		builtins["copy"]=&Parser::builtin_copy;
//		//file
//		builtins["read"]=&Parser::builtin_read;
//		builtins["write"]=&Parser::builtin_write;
//		builtins["append"]=&Parser::builtin_append;
//		builtins["exists"]=&Parser::builtin_exists;
//		builtins["remove_file"]=&Parser::builtin_remove_file;
//		builtins["rename_file"]=&Parser::builtin_rename_file;
//		//terminal
//		builtins["screen"]=&Parser::builtin_screen;
//		builtins["endscreen"]=&Parser::builtin_endscreen;
//		builtins["refresh"]=&Parser::builtin_refresh;
//		builtins["move"]=&Parser::builtin_move;
//		builtins["printxy"]=&Parser::builtin_printxy;
//		builtins["getkey"]=&Parser::builtin_getkey;
//		builtins["cursor"]=&Parser::builtin_cursor;
//		builtins["box"]=&Parser::builtin_box;
//		builtins["width"]=&Parser::builtin_width;
//		builtins["height"]=&Parser::builtin_height;
//		builtins["clear_screen"]=&Parser::builtin_clear_screen;
//		builtins["clear_line"]=&Parser::builtin_clear_line;
//		builtins["clear_bottom"]=&Parser::builtin_clear_bottom;
//		builtins["goto"]=&Parser::builtin_goto;
//		builtins["putch"]=&Parser::builtin_putch;
//		builtins["hline"]=&Parser::builtin_hline;
//		builtins["vline"]=&Parser::builtin_vline;
//		builtins["resize"]=&Parser::builtin_resize;
//		builtins["is_resize"]=&Parser::builtin_is_resize;
//		builtins["echo"]=&Parser::builtin_echo;
//		builtins["init_color"]=&Parser::builtin_init_color;
//		builtins["color_pair"]=&Parser::builtin_color_pair;
//		builtins["color_off"]=&Parser::builtin_color_off;
//		builtins["delay"]=&Parser::builtin_delay;
//		builtins["nodelay"]=&Parser::builtin_nodelay;
//		builtins["border"]=&Parser::builtin_border;
//		builtins["line"]=&Parser::builtin_line;
//		builtins["fill"]=&Parser::builtin_fill;
//		builtins["rand"]=&Parser::builtin_rand;
//		builtins["randint"]=&Parser::builtin_randint;
//		builtins["randfloat"]=&Parser::builtin_randfloat;
//		builtins["seed"]=&Parser::builtin_seed;
//		builtins["choice"]=&Parser::builtin_choice;
