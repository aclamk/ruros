%{
#include "gen.h"
#include <string>
#define TOK_NAME 301
#define TOK_BICOLON 304         
         #define YYSTYPE std::string
         //#include "gen.tab.h"
        extern "C" int yywrap(){return 1;}

gen::YYpos yypos=
		{
			std::string("--stdin--"),1,1
		};

//        std::string yypos.file("--stdin--");
//        int yypos.line=1;
//        int yypos_col=1;
        int yypos_col=1;
        
%}
     
NAME       [_a-zA-Z][_a-zA-Z0-9]*
SPEC_CHAR	[@{}();,]
%x SC_OPT
%%


{NAME}      { 
				//yylval=yytext;
				yypos.col=yypos_col; yypos_col+=strlen(yytext);
				return TOK_NAME;
			}
::			{ yypos.col=yypos_col; yypos_col+=2; return TOK_BICOLON; }
     
     
"{"[\^{}}\n]*"}"     /* eat up one-line comments */
     
[ \t]+      /* eat up whitespace */ { yypos.col=yypos_col; yypos_col+=strlen(yytext);}
\n\r		{ yypos.line++; yypos_col=1; }
\r\n		{ yypos.line++; yypos_col=1; }
\n			{ yypos.line++; yypos_col=1; }

{SPEC_CHAR} { yypos.col=yypos_col; yypos_col++; return yytext[0];}
     
.           printf( "Unrecognized character: %s %2.2x\n", yytext,yytext[0] );
     
%%
#include <stdarg.h>
//#include "gen.h"

gen::Service curr_service;
//gen::Function curr_function;
void begin_sc_opt()
{
	BEGIN(SC_OPT);
}  
void begin_initial()
{
	BEGIN(INITIAL);
}

std::string service_name;

void fatal(const char* fmt, ...)
{
	std::string log;
	va_list argList;
	va_start(argList, fmt);
	char* p=NULL;
	int rr,rr1;
	rr=vasprintf(&p, fmt, argList);
	va_end(argList);
	char errorloc[50];
	sprintf(errorloc,"At %s:%d:%d ",yypos.file.c_str(),yypos.line,yypos.col);
	log=std::string(errorloc)+p;
	if(rr>0){free(p);}
	printf("%s\n",log.c_str());
	exit(1);
}
//#include "gen.cpp"
int tok;
bool peeked=false;

int yytok()
{
	if(peeked)
	{
		peeked=false;
		return tok;
	}
	tok=yylex();
	return tok;
}
int yypeek()
{
	tok=yylex();
	peeked=true;
	return tok;
}

std::string gettoken()
{
	if(yytok()==0) return "";
	return yytext;
}

void expect(const std::string& exp)
{
	std::string t=gettoken();
	if(t!=exp)
		fatal("'%s' expected but '%s' found",exp.c_str(),yytext);
}

std::string qualified_name()
{
	std::string s;
	yytok();
	if(tok==TOK_BICOLON)
	{
		s+="::";
		yytok();
		if(tok!=TOK_NAME)
			fatal("identifier expected but '%s' found",yytext);
		s+=yytext;
	}
	else if(tok==TOK_NAME)
		s+=yytext;
	else 
		fatal("identifier expected but '%s' found",yytext);
	while(TOK_BICOLON==yypeek())
	{
		yytok();
		s+="::";
		yytok();
		if(tok!=TOK_NAME)
			fatal("identifier expected but '%s' found",yytext);
		s+=yytext;
	}
	return s;
}

void service_func()
{
	gen::Function function;
	function.name=yytext;
	expect("(");
	yytok();
	if(tok==')') goto end;
	while(true)
	{
		gen::Arg arg;
		arg.dir=gen::d_in;
		if(tok!=TOK_NAME)
			fatal("identifier expected but '%s' found",yytext);
		std::string a;
		a=yytext;
		bool has_inout=true;
		if(a=="in")
			arg.dir=gen::d_in;
		else if(a=="out")
			arg.dir=gen::d_out;
		else if(a=="inout")
			arg.dir=gen::d_inout;
		else
			has_inout=false;
		
		if(has_inout)
		{
			yytok();
			if(tok!=TOK_NAME)
				fatal("identifier expected but '%s' found",yytext);
			arg.type=yytext;
		
			yytok();
			if(tok!=TOK_NAME)
				fatal("identifier expected but '%s' found",yytext);
			arg.name=yytext;	
		}
		else
		{
			arg.type=a;
			yytok();
			if(tok!=TOK_NAME)
				fatal("identifier expected but '%s' found",yytext);
			arg.name=yytext;
		}
		function.args.push_back(arg);
		yytok();
		if(tok==',') {yytok(); continue; }
		if(tok==')') goto end;
		fatal("')' expected but '%s' found",yytext);
	}
	end:
		curr_service.functions.push_back(function);
	expect(";");
}

void service_option()
{
	int grammar=0;
	std::string cs;
	std::string option;
	yytok();
	if(tok!=TOK_NAME)
		fatal("'client' or 'server' expected but '%s' found",yytext);
	cs=yytext;
	if(cs!="client" && cs!="server")
		fatal("'client' or 'server' expected but '%s' found",yytext);
	
	yytok();
	if(tok!=TOK_NAME)
		fatal("option expected but '%s' found",yytext);
	option=yytext;	
	std::string name=cs+" "+option;
	if(	name=="client name"||
		name=="client disconnect"||
		name=="server name"||
		name=="server disconnect"||
		name=="server cleanup"||
		name=="server accept"
		)		
	{
		grammar=1;
	}
	else
	{
		fatal("'%s' does not define proper options",name.c_str());
	}
	gen::YYpos option_pos=yypos;
	int i;
	for(i=0;i<curr_service.options.size();i++)
	{
		if(curr_service.options[i].name==name)
		{
			fatal("option '%s' already defined at %s:%d:%d",
					name.c_str(),
					curr_service.options[i].pos.file.c_str(),
					curr_service.options[i].pos.line,
					curr_service.options[i].pos.col
					);
		}
	}
	std::string value;
	switch(grammar)
	{
	case 1:
		value=qualified_name();
		break;
	default:
		fatal("internal error, unknown grammar for '%s'",name.c_str());
	}
	
	gen::Option opt;
	opt.name=name;
	opt.value=value;
	opt.pos=option_pos;
	curr_service.options.push_back(opt);
	
	expect(";");
}


void service()
{
	expect("service");
	yytok();
	if(tok!=TOK_NAME)
		fatal("expected identified, but '%s' found",yytext);
	curr_service.name=yytext;
	expect("{");
	
	//now, either @ or identifier
	
	while(true)
	{
		yytok();
		if(tok=='@')
			service_option();
		else if(tok==TOK_NAME)
			service_func();
		else if(tok=='}')
			break;
		else
			fatal("unexpected token found '%s'",yytext);
	}
	return;
}



std::string generate_dispatch(const gen::Service& service);
std::string generate_client_stubs(const gen::Service& service);
std::string generate_service_server_side(const gen::Service& service);
std::string generate_service_client_side(const gen::Service& service);

void usage()
{
	exit(1);
}

int main( int argc, char **argv )
{
	FILE* fserver=NULL;
	FILE* fclient=NULL;
	FILE* fidl=NULL;
    curr_service.debug=false;
	++argv, --argc;  /* skip over program name */
	while(argc > 0)
	{
		if(strcmp(*argv,"-server")==0)
		{
			argc--;argv++;
			if(argc<1) 
			{
				fprintf(stderr,"No file given for -server param\n");
				usage();
			}
			fserver=fopen(*argv, "w+");
			if(fserver==NULL)
			{
				fprintf(stderr,"Cannot open '%s'\n",*argv);
				exit(1);
			}
			argc--;argv++;
		} 
		else if(strcmp(*argv,"-client")==0)
		{
			argc--;argv++;
			if(argc<1) 
			{
				fprintf(stderr,"No file given for -client param\n");
				usage();
			}
			fclient=fopen(*argv, "w+");
			if(fclient==NULL)
			{
				fprintf(stderr,"Cannot open '%s'\n",*argv);
				exit(1);
			}
			argc--;argv++;
		}
		else if(strcmp(*argv,"-idl")==0)
		{
			argc--;argv++;
			if(argc<1) 
			{
				fprintf(stderr,"No file given for -idl param\n");
				usage();
			}
			fidl=fopen(*argv, "r");
			if(fidl==NULL)
			{
				fprintf(stderr,"Cannot open '%s'\n",*argv);
				exit(1);
			}
			argc--;argv++;
		}
        else if(strcmp(*argv,"-debug")==0)
		{
			argc--;argv++;
            curr_service.debug=true;
		}
		else
		{
			fprintf(stderr,"Unknown option '%s'\n",*argv);
			usage();
		}		
	}
	if(fidl==NULL)
	{
		fprintf(stderr,"Option -idl not given\n");
		usage();
	}
	yyin=fidl;

	int d;
	service();
	std::string s;
	if(fclient!=NULL)
	{
		s=generate_service_client_side(curr_service);
		fprintf(fclient,"%s\n",s.c_str());
		s=generate_client_stubs(curr_service);
		fprintf(fclient,"%s\n",s.c_str());
	}
	if(fserver!=NULL)
	{
		s=generate_dispatch(curr_service);
		fprintf(fserver,"%s\n",s.c_str());
		s=generate_service_server_side(curr_service);
		fprintf(fserver,"%s\n",s.c_str());
	}
}
