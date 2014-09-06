/*-< ANTIC.C >------------------------------------------------------*--------*/
/* Jlint                      Version 1.11.1     (c) 1998  GARRET   *     ?  */
/* (Java Lint)                                                      *   /\|  */
/*                                                                  *  /  \  */
/*                          Created:     28-Apr-1998  K.A. Knizhnik * / [] \ */
/*                          Last update: 24-Feb-2000  E.G. Parmelan * GARRET */
/*------------------------------------------------------------------*--------*/
/* AntiC - C/C++/Java syntax verifier                               *        */
/*------------------------------------------------------------------*--------*/
/* Edouard G. Parmelan e-mail address: egp@free.fr                           */
/*------------------------------------------------------------------*--------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define VERSION	"1.11.1"

char* file_name;
FILE* f;
int line;
int coln;

int force_java;
int token_line;
int token_coln;
int java;
int fall_thru_cmt;
int notreached_cmt;
int tab_size = 8;
int relax_else = 0;

/* Use only lowercase letters without spaces */
const char *fall_thru_cmt_text[] = {
    "fallthr",
    "nobreak"
};

/* Use only lowercase letters without spaces */
const char *notreached_cmt_text[] = {
    "notreach",
    "unreach",
};

#define items(a) (sizeof(a)/sizeof(*(a)))
#define nobreak

enum { 
    TKN_FOR = 257, 
    TKN_IF, 
    TKN_DO, 
    TKN_WHILE, 
    TKN_ELSE,
    TKN_BREAK,
    TKN_CASE,
    TKN_CATCH,
    TKN_FINALLY,
    TKN_TRY,
    TKN_SWITCH,
    TKN_STR,
    TKN_IDENT,
    TKN_CTX,
    TKN_ACCESS,
    TKN_AND_OP,
    TKN_OR_OP,
    TKN_BIN_OP,
    TKN_BIT_OP,
    TKN_SET_OP,
    TKN_CMP_OP,
    TKN_EQU_OP,
    TKN_INC_OP,
    TKN_SHIFT_OP,
    TKN_INCLUDE
};

typedef enum {
    ctx_statement,
    ctx_typedef, 
    ctx_body, 
    ctx_switch
} statement_ctx;



int get()
{
    int ch = getc(f);
    if (ch == '\n') { 
	line += 1;
	coln = 0;
    } else if (ch == '\t') { 
	coln += tab_size;
	coln -= coln % tab_size;
    } else { 
	coln += 1;
    }
    return ch;
}

void unget(int ch)
{
    ungetc(ch, f);
    if (ch == '\n') { 
	line -= 1;
    } else if (ch == '\t') { 
	coln -= tab_size;
    } else { 
	coln -= 1;
    }
}

int n_messages;
    
void message_at(int line, int coln, char* msg)
{
    printf("%s:%d:%d: %s\n", file_name, line, coln, msg);
    n_messages += 1;
}

void message(char* msg)
{
    message_at(token_line, token_coln, msg);
}

int check_special_character()
{
    int ch = get();
    switch (ch) { 
      case '\n':
	if (java) { 
	    message("Newline within string\n");
	}
	break;
      case 'n':
      case 't':
      case 'b':
      case 'r':
      case 'f':
      case '\\':
      case '"':
      case '\'':
	break;
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
	ch = get();
	if (ch >= '0' && ch <= '9') {
	    if (ch > '7') { 
		message("Octal digit expected");
	    }
	    ch = get();
	    if (ch >= '0' && ch <= '9') {
		if (ch > '7') { 
		    message("Octal digit expected");
		}
		ch = get();
		if (ch >= '0' && ch <= '9') {
		    message("May be more than three octal digits are specified");
		}
	    }
	}
	return ch;
      case 'x':
	if (java) {
	    message("Hexadecimal constant\n");
	}
	ch = get();
	if (!isxdigit(ch)) {
	    message("Hexadecimal digit expected\n");
	}
	else while (isxdigit(ch)) {
	    ch = get();
	}
	return ch;
      case 'u':
	if (java) {
	    do { 
		ch = get();
	    } while (ch == 'u');
	    if (!isxdigit(ch)) { 
		message("Invalid character constant");
	    }
	    ch = get();
	    if (!isxdigit(ch)) { 
		message("Invalid character constant");
	    }
	    ch = get();
	    if (!isxdigit(ch)) { 
		message("Invalid character constant");
	    }
	    ch = get();
	    if (!isxdigit(ch)) { 
		message("Invalid character constant");
	    }
	    ch = get();
	    if (isxdigit(ch)) { 
		message("May be more than four hex digits "
			"are specified for character constant");
	    }	
	    return ch;
	}	
	nobreak;
      default:
	message("May be incorrect escape sequence");
    }
    return get();
}

static void check_fall_thru(int ch)
{
    static unsigned index[items(fall_thru_cmt_text)];
    unsigned i;

    if (ch == ' ' || ch == '-') { 
	return;
    }
    if (ch == 0) {
	fall_thru_cmt = 0;
	for (i = 0; i < items(fall_thru_cmt_text); i++)
	    index[i] = 0;
    }
    else if (!fall_thru_cmt) {
	for (i = 0; i < items(index); i++) {
	    if (tolower(ch) == fall_thru_cmt_text[i][index[i]]) {
		if (fall_thru_cmt_text[i][++(index[i])] == '\0') {
		    fall_thru_cmt = 1;
		    return;
		}
	    }
	    else {
		index[i] = 0;
	    }
	}
    }
}

static void check_notreached(int ch)
{
    static unsigned index[items(notreached_cmt_text)];
    unsigned i;

    if (ch == ' ' || ch == '-') { 
	return;
    }
    if (ch == 0) {
	notreached_cmt = 0;
	for (i = 0; i < items(notreached_cmt_text); i++)
	    index[i] = 0;
    }
    else if (!notreached_cmt) {
	for (i = 0; i < items(index); i++) {
	    if (tolower(ch) == notreached_cmt_text[i][index[i]]) {
		if (notreached_cmt_text[i][++(index[i])] == '\0') {
		    notreached_cmt = 1;
		    return;
		}
	    }
	    else {
		index[i] = 0;
	    }
	}
    }
}

int scan_0(int cpp)
{
    int ch;
    unsigned i;
    char buf[256];
    
  skip_spaces:
    do {
	ch = get();
	if ((ch == EOF) || (ch == '\n') || (ch == '#'))
	    return ch;
    } while (isspace(ch));
    token_coln = coln;
    token_line = line;
    switch (ch) {
      case '/':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	} else if (ch == '*') { 
	    int nested = 0;
	    ch = get();
	    while (1) { 
		if (ch == EOF) { 
		    message("Unclosed comments");
		    return EOF;
		} else if (ch == '/') { 
		    ch = get();
		    if (ch == '*') { 
			if (!nested)
			    message("Nested comments");
			nested = 1;
		    }
		} else if (ch == '*') { 
		    ch = get();
		    if (ch == '/') { 
			goto skip_spaces;
		    }
		} else { 
		    check_fall_thru(ch);
		    check_notreached(ch);
		    ch = get();
		}
	    }		
	} else if (ch == '/') {
	    while ((ch = get()) != '\n' && ch != EOF) { 
		check_fall_thru(ch);
		check_notreached(ch);
	    }
	    return ch;
	}
	unget(ch);
	return TKN_BIN_OP;	
      case '"':
	ch = get();
	while (ch != '"') {
	    switch (ch) { 
	      case '\n':
		message("Newline within string constant");
		ch = get();
		continue;
	      case EOF: 
		return EOF;
	      case '\\':
		if (cpp == 2) {
		    ch = get();
		}   
		else {
		    ch = check_special_character();
		}
		continue;
	      case '?':
		if (!java) { 
		    ch = get();
		    if (ch == '?') { 
			ch = get();
			if (ch == '=' || ch == '/' || ch == '\'' ||
			    ch == '(' || ch == ')' || ch == '!' || 
			    ch == '<' || ch == '>') 
			{
			    message("Trigraph sequence inside string");
			}
		    }
		    continue;
		}
		nobreak;
	      default:
		ch = get();
	    }
	}
	return TKN_STR;
      case '\'':
	ch = get();
	if (ch == '\\') { 
	    ch = check_special_character();
	} else { 
	    ch = get();
	}
	if (ch != '\'') { 
	    message("Multibyte character constants are not portable");
	    do {
		ch = get();
		if (ch == '\\')
		    get();
	    } while ((ch != '\'') && (ch != EOF));
	}
	return TKN_STR;
      case ':':
	if (!java) { 
	    ch = get();
	    if (ch == ':') {
		return TKN_CTX;
	    } else { 
		unget(ch);
		return ':';
	    }
	}
	break;
      case '+':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	} else if (ch == '+') return TKN_INC_OP;
	unget(ch);
	return '+';	
      case '-':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	} else if (!java && ch == '>') { 
	    return TKN_ACCESS;
	} else if (ch == '-') return TKN_INC_OP;
	unget(ch);
	return '-';	
      case '*':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	}
	unget(ch);
	return '*';	
      case '%':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	}
	unget(ch);
	return TKN_BIN_OP;	
      case '&':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	} else if (ch == '&') { 
	    return TKN_AND_OP;
	}
	unget(ch);
	return '&';
      case '|':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	} else if (ch == '|') { 
	    return TKN_OR_OP;
	}
	unget(ch);
	return TKN_BIT_OP;
      case '^':
	ch = get();
	if (ch == '=') { 
	    return TKN_SET_OP;
	}
	unget(ch);
	return TKN_BIT_OP;
      case '>':
	ch = get();
	if (ch == '>') { 
	    ch = get();
	    if (java && ch == '>') { 
		ch = get();
		if (ch == '=') return TKN_SET_OP;
	    } else if (ch == '=') return TKN_SET_OP;
	    unget(ch); 
	    return TKN_SHIFT_OP;
	} else if (ch != '=') unget(ch); 
	return TKN_CMP_OP;
      case '<':
	ch = get();
	if (ch == '<') { 
	    ch = get();
	    if (ch == '=') return TKN_SET_OP;
	    unget(ch); 
	    return TKN_SHIFT_OP;
	} else if (ch != '=') unget(ch); 
	return TKN_CMP_OP;	
      case '=':
	ch = get();
	if (ch == '=') return TKN_EQU_OP;
	unget(ch); 
	return '=';	
      case '!':
	ch = get();
	if (ch == '=') return TKN_CMP_OP;
	unget(ch); 
	return '!';	
      default:
	if (isalnum(ch) || ch == '_' || ch == '$') { 
	    i = 0;
	    do { 
		if (i < sizeof(buf)) { 
		    buf[i++] = ch;
		}
		ch = get();
	    } while (isalnum(ch) || ch == '_' || ch == '$');
	    unget(ch);
	    if (i < sizeof(buf)) { 
		buf[i] = '\0';
		if (strcmp(buf, "if") == 0)       return TKN_IF; 
		if (strcmp(buf, "for") == 0)      return TKN_FOR; 
		if (strcmp(buf, "while") == 0)    return TKN_WHILE; 
		if (strcmp(buf, "do") == 0)       return TKN_DO; 
		if (strcmp(buf, "else") == 0)     return TKN_ELSE;
		if (strcmp(buf, "default") == 0)  return TKN_CASE;
		if (strcmp(buf, "case") == 0)     return TKN_CASE;
		if (strcmp(buf, "break") == 0)    return TKN_BREAK;
		if (strcmp(buf, "goto") == 0)     return TKN_BREAK;
		if (strcmp(buf, "throw") == 0)    return TKN_BREAK;
		if (strcmp(buf, "return") == 0)   return TKN_BREAK;
		if (strcmp(buf, "continue") == 0) return TKN_BREAK;
		if (strcmp(buf, "catch") == 0)    return TKN_CATCH;
		if (strcmp(buf, "try") == 0)      return TKN_TRY;
		if (strcmp(buf, "finally") == 0)  return TKN_FINALLY;
		if (strcmp(buf, "switch") == 0)   return TKN_SWITCH;
		if (!java && strcmp(buf, "nobreak") == 0)  return TKN_BREAK;
		if ((cpp == 1) && strcmp(buf, "include") == 0) return TKN_INCLUDE;
	    }
	    if (isdigit((unsigned char)buf[0]) && buf[i-1] == 'l') { 
		message("May be 'l' is used instead of '1' at the end of "
			"integer constant");
	    } 
	    return TKN_IDENT;
	}
    }
    return ch;
}


int scan()
{
    int ch;

    check_fall_thru(0);
    check_notreached(0);

    while (1) {
	ch = scan_0(0);
	if (ch == '#') {
	    int cpp = 1;
	    do {
		ch = scan_0(cpp);
		if (cpp == 1 && ch == TKN_INCLUDE)
		    cpp = 2;
		if (ch == '\\')
		    scan_0(cpp);
	    } while ((ch != '\n') && (ch != EOF));
	}
	else if (ch == '\\') {
	    message("Unexpected character '\\'");
	}
	else if (ch != '\n')
	    return ch;
    }
}


int parse_binary_operation(int* n_bin_ops);

int parse_expression()
{
    int lex, prev_op = 0;
    while (1) { 
	int n_bin_ops;
	lex = parse_binary_operation(&n_bin_ops);
	if (lex == '&') lex = TKN_BIT_OP;
	if (lex == TKN_CMP_OP || lex == TKN_BIT_OP || lex == TKN_AND_OP
	    || lex == TKN_OR_OP || lex == TKN_SET_OP || lex == '=' 
	    || lex == TKN_EQU_OP || lex == ',' || lex == '?' || lex == ':') 
        {
	    if (!java && lex == TKN_CMP_OP && prev_op == TKN_CMP_OP) { 
		prev_op = 0; /* may be template */
		continue;
	    }
	    if (((lex == TKN_CMP_OP || lex == TKN_EQU_OP) && prev_op == TKN_BIT_OP)
		|| (lex == TKN_BIT_OP 
		    && (n_bin_ops != 0 || prev_op == TKN_AND_OP 
			|| prev_op == TKN_OR_OP || prev_op == TKN_CMP_OP || prev_op == TKN_EQU_OP))
		|| ((lex == TKN_AND_OP || lex == TKN_OR_OP) 
		    && (prev_op == TKN_BIT_OP || prev_op == '=' ||
			prev_op == TKN_SET_OP))
		|| ((lex == '=' || lex == TKN_SET_OP) 
		    && (prev_op == TKN_AND_OP || prev_op == TKN_OR_OP)))
	    {
		message("May be wrong assumption about operators priorities");
	    }
	    else if (lex == TKN_AND_OP && prev_op == TKN_OR_OP) {  
		message("May be wrong assumption about logical "
			"operators precedence");	    
	    } 
	    prev_op = lex;
	} else return lex;
    }
}

void parse_block(int* pass_through, statement_ctx ctx);

int parse_term()
{
    int lex;
    
    while ((lex = scan()) == '&' || lex == '+' || lex == '-' || lex == '*' 
	   || lex == '!' || lex == TKN_INC_OP || lex == TKN_CTX || lex == '~');
    while (lex == '.' || lex == TKN_INC_OP || lex == TKN_IDENT 
	   || lex == TKN_ACCESS || lex == '(' || lex == '[' || lex == TKN_STR 
	   || lex == TKN_CTX)
    {
	if (lex == '(') { 
	    int lpr_line = token_line;
	    int lpr_coln = token_coln;
	    lex = parse_expression();
	    if (lex != ')') { 
		message_at(lpr_line, lpr_coln, "Unbalanced '('");
		return lex;
	    }
	} else if (lex == '[') { 
	    int lbr_line = token_line;
	    int lbr_coln = token_coln;
	    lex = parse_expression();
	    if (lex != ']') { 
		message_at(lbr_line, lbr_coln, "Unbalanced '['");
		return lex;
	    }
	} 
	lex = scan();
    }
    return lex;
}

int parse_switch_expression()
{
    int lex = scan();
    int lpr_line = token_line;
    int lpr_coln = token_coln;
    if (lex != '(') { 
	message_at(lpr_line, lpr_coln, "'(' expected");
	return lex;
    }
    lex = parse_expression();
    if (lex != ')') { 
	message_at(lpr_line, lpr_coln, "Unbalanced '('");
    }
    return scan();
}

int parse_binary_operation(int* n_bin_ops)
{
    int lex;
    int prev_op = 0;
    *n_bin_ops = 0;
    while (1) { 
	lex = parse_term();
	if (lex == '*' || lex == '-' || lex == '+') lex = TKN_BIN_OP;
	if (lex == TKN_SHIFT_OP) { 
	    if (prev_op == TKN_BIN_OP) { 
 		message("May be wrong assumption about shift operator priority");
	    } 
	} else if (lex == TKN_BIN_OP) { 
	    if (prev_op == TKN_SHIFT_OP) {
		message("May be wrong assumption about shift operator priority");
	    }
	} else if (java && lex == '{') { /* anonymous class */
	    int pass_through = 1;
	    parse_block(&pass_through, ctx_typedef);
	} else { 
	    return lex;
	}
	prev_op = lex;
	*n_bin_ops += 1;
    }
}


void parse_conditional_expression(int term_ch)
{
    int bit_op_line = 0;
    int bit_op_coln = 0;
    int n_bin_ops;
    int lex;
    int expr_line = 0;
    int expr_coln = 0;
    int prev_op = 0;

    if (term_ch == ')') { 
	if (scan() != '(') { 
	    message("'(' expected");
	    return;
	}
	expr_line = token_line;
	expr_coln = token_coln;
    }
    while ((lex = parse_binary_operation(&n_bin_ops)) != term_ch) {
	if (lex == '&') lex = TKN_BIT_OP;
	if (lex == EOF) { 
	    message_at(expr_line, expr_coln, term_ch == ')' 
		       ? "Unbalanced parentheses" 
		       : "No ')' after FOR condition");
	    return;
	} 	    
	if (lex == '=') { 
	    message("May be '=' used instead of '=='");
	} else if (lex == TKN_SET_OP) { 
	    message("May be skipped parentheses around assign operator");
	} else if (lex == TKN_BIT_OP) { 
	    if (n_bin_ops != 0 || prev_op == TKN_AND_OP 
		|| prev_op == TKN_OR_OP || prev_op == TKN_EQU_OP || prev_op == TKN_CMP_OP) 
            { 
		message("May be wrong assumption about bit operation priority");
	    } 
	    bit_op_line = token_line;
	    bit_op_coln = token_coln;
	    prev_op = TKN_BIT_OP;
	    continue;
	} else if (lex == TKN_AND_OP && prev_op == TKN_OR_OP) {  
	    message("May be wrong assumption about logical operators precedence");	    
	} else if (lex != '?' && lex != ':' && lex != ',' && lex != TKN_CMP_OP 
		 && lex != TKN_EQU_OP && lex != TKN_AND_OP && lex != TKN_OR_OP) 
	{
	    message("Syntax error");
	} 
	if (prev_op == TKN_BIT_OP) { 
	    message_at(bit_op_line, bit_op_coln, 
		       "May be wrong assumption about operators priorities");
	}
	prev_op = lex;
    }
}

int parse_statement(int lex, int* stmt_line, int* stmt_coln, 
		    int* pass_through, int* entry_point, statement_ctx ctx) 
{
    int next_stmt_line, next_stmt_coln;
    int if_coln;
    int stmt;
    int if_pass_through, else_pass_through;
    int statement_with_label = 0;
    int n_labels_before;
    int entries;
    static int n_labels;
    int miss_equal;

  classify_statement:
    miss_equal = 0;
    switch (lex) {
      case '{':
	parse_block(pass_through, ctx);
	nobreak;
      case ';':
	lex = scan();	
	*stmt_line = token_line;
	*stmt_coln = token_coln;
	return lex;
      case TKN_FOR:
	if ((lex = scan()) != '(') { 
	    message("No '(' after FOR");
	    return lex;
	}
	lex = parse_expression();
	if (lex != ';') { 
	    message("No ';' after FOR initialization part");
	    return lex;
	}
	parse_conditional_expression(';');
	lex = parse_expression();
	if (lex != ')') { 
	    message("No ')' after FOR increment part");
	    return lex;
	}
	goto loop_body;
      case TKN_WHILE:
	parse_conditional_expression(')');
      loop_body:
	stmt = scan();
	next_stmt_line = token_line;
	next_stmt_coln = token_coln;
	if_pass_through = 0;
	n_labels_before = n_labels;
	if (stmt != '{' && next_stmt_coln <= *stmt_coln) { 
	    message_at(next_stmt_line, next_stmt_coln, 
		       "May be wrong assumption about loop body (semicolon missed)");
	}
	lex = parse_statement(stmt, &next_stmt_line, &next_stmt_coln, 
			      &if_pass_through, &entries, ctx_statement);
	if (if_pass_through) { 
	    *pass_through = 1;
	}
	if (!java && n_labels != n_labels_before) { 
	    *entry_point = 1;
	}
	if (stmt != '{' && lex != '}' && next_stmt_coln > *stmt_coln) { 
	    message_at(next_stmt_line, next_stmt_coln, 
		       "May be wrong assumption about loop body");
	}
	break;
      case TKN_SWITCH:
	stmt = parse_switch_expression();
	next_stmt_line = token_line;
	next_stmt_coln = token_coln;
	if (stmt != '{') { 
	    message_at(next_stmt_line, next_stmt_coln, 
		       "Suspicios SWITCH without body");
	}
	if_pass_through = 0;
	lex = parse_statement(stmt, &next_stmt_line, &next_stmt_coln, 
			      &if_pass_through, &entries, ctx_switch);
	*pass_through = 1;
	break;
      case TKN_IF: 
	if_coln = *stmt_coln;
	parse_conditional_expression(')');
	stmt = scan();
	next_stmt_line = token_line;
	next_stmt_coln = token_coln;
	else_pass_through = if_pass_through = *pass_through;
	lex = parse_statement(stmt, &next_stmt_line, &next_stmt_coln, 
			      &if_pass_through, &entries, ctx_statement);
	if (if_pass_through) { 
	    *pass_through = 1;
	}
	if (lex == TKN_ELSE) { 
	    lex = scan();
	    if (next_stmt_coln < if_coln) {
		/* handle IF ... ELSE FOO where FOO is aligned with IF */
		if (!relax_else || (token_coln < if_coln)) {
		    message_at(next_stmt_line, next_stmt_coln, 
			    "May be wrong assumption about ELSE branch "
			    "association");
		}
	    }
	    next_stmt_coln = token_coln;
	    if (lex == TKN_IF && next_stmt_line == token_line) {  
		if (if_coln < token_coln) { 
		    next_stmt_coln = if_coln;
		}
	    }
	    next_stmt_line = token_line;
	    lex = parse_statement(lex, &next_stmt_line, &next_stmt_coln,
				  &else_pass_through, &entries, ctx_statement);
	    if (!if_pass_through && !else_pass_through) { 
		*pass_through = 0;
	    } else if (else_pass_through) { 
		*pass_through = 1;
	    }
	} 
	else if (stmt == ';' 
		 || (stmt != '{' && lex != '}' && next_stmt_coln > if_coln))
	{ 
	    message_at(next_stmt_line, next_stmt_coln, 
		       "May be wrong assumption about IF body");
	}
	break;
      case TKN_TRY: 
	lex = scan();
	next_stmt_line = token_line;
	next_stmt_coln = token_coln;
	if_pass_through = *pass_through;
	else_pass_through = 0;
	lex = parse_statement(lex, &next_stmt_line, &next_stmt_coln,
			      &if_pass_through, &entries, ctx_statement);
	else_pass_through |= if_pass_through;
	while ((lex == TKN_CATCH) || (lex == TKN_FINALLY)) { 
	    /* It is really not condition expression, but list of 
	     * catch paramters, but is possible to use this function to skip 
	     * catch paramter list.
	     */
	    if (lex == TKN_CATCH)
		parse_conditional_expression(')');
	    if_pass_through = *pass_through;
	    lex = parse_statement(scan(), &next_stmt_line, &next_stmt_coln, 
				  &if_pass_through, &entries, ctx_statement);
	    else_pass_through |= if_pass_through;
	} 
	*pass_through = else_pass_through;
	break;
      case TKN_DO:
	lex = scan();
	next_stmt_line = token_line;
	next_stmt_coln = token_coln;
	n_labels_before = n_labels;
	if_pass_through = 0;
	lex = parse_statement(lex, &next_stmt_line, &next_stmt_coln,
			      &if_pass_through, &entries, ctx_statement);
	if (if_pass_through) { 
	    *pass_through = 1;
	}
	if (!java && n_labels != n_labels_before) { 
	    *entry_point = 1;
	}
	if (lex != TKN_WHILE) { 	
	    message_at(next_stmt_line, next_stmt_coln, 
		       "No WHILE for DO");
	}
	parse_conditional_expression(')');
	lex = scan();
	if (lex != ';') { 
	    message_at(token_line, token_coln, "No ';' after DO WHILE");
	}
	lex = scan();	
	*stmt_line = token_line;
	*stmt_coln = token_coln;
	return lex;
      case TKN_CASE:
	if (ctx != ctx_switch) { 
	    message_at(*stmt_line, *stmt_coln, "Suspicious CASE/DEFAULT");
	} 
	else if (!statement_with_label && !fall_thru_cmt && *pass_through) { 
	    message_at(*stmt_line, *stmt_coln, 
		       "Possible miss of BREAK before CASE/DEFAULT");
	}	    
	while ((lex = scan()) != ':') { 
	    if (lex == EOF) { 
		message_at(*stmt_line, *stmt_coln, "No ':' after CASE");
		return EOF;
	    }
	}
	statement_with_label = 1;
	*entry_point = 1;
	*pass_through = 1;
	lex = scan();
	*stmt_line = token_line;
	*stmt_coln = token_coln;
	goto classify_statement;
      case TKN_BREAK:
	*pass_through = 0;
	goto parse_statement;
      case TKN_CATCH:
      case TKN_FINALLY:
	message_at(*stmt_line, *stmt_coln, "Suspicious CATCH/FINALLY");
	if (lex == TKN_CATCH)
	    parse_conditional_expression(')');
	*pass_through = 1;
	lex = scan();
	*stmt_line = token_line;
	*stmt_coln = token_coln;
	goto classify_statement;
      case TKN_IDENT:
	lex = scan();
	if (lex == ':') { 
	    n_labels += 1;
	    statement_with_label = 1;
	    *entry_point = 1;
	    *pass_through = 1;
	    lex = scan();
	    *stmt_line = token_line;
	    *stmt_coln = token_coln;
	    goto classify_statement;
	}
	miss_equal = 1;
	nobreak;
      parse_statement:
      default:
	/* ctx = ctx_typedef; */
	while (lex != ';') { 
	    switch (lex) {
	      case EOF:
		message_at(*stmt_line, *stmt_coln, 
			   "Statement not terminated by ';'");
		return EOF;
	      case '(':
		lex = parse_expression();
		if (lex != ')') { 
		    message("')' expected");
		    return lex;
		}
		ctx = ctx_body;
		miss_equal = 0;
		break;
	      case '[':
		lex = parse_expression();
		if (lex != ']') { 
		    message("']' expected");
		    return lex;
		}
		break;
	      case ')':
		message("Unbalanced ')' in statement");
		lex = scan();
		*stmt_line = token_line;
		*stmt_coln = token_coln;
		goto classify_statement;
              case TKN_EQU_OP:
                if (miss_equal && ctx != ctx_typedef)
                    message("Possible miss of '='");
                miss_equal = 0;
                break;
	      case TKN_SET_OP:
	      case '=':
		lex = parse_expression();
		miss_equal = 0;
		continue;
	      case '}':
		*stmt_line = token_line;
		*stmt_coln = token_coln;
		return lex;		
	      case '{':
		if_pass_through = 1;
		parse_block(&if_pass_through, ctx);
		lex = scan();
		*stmt_line = token_line;
		*stmt_coln = token_coln;
		return lex;
	      case TKN_FOR: 
	      case TKN_IF: 
	      case TKN_DO: 
	      case TKN_WHILE: 
	      case TKN_CASE:
	      case TKN_TRY:
	      case TKN_SWITCH:
		message_at(*stmt_line, *stmt_coln, "Possible miss of ';'");
		*stmt_line = token_line;
		*stmt_coln = token_coln;
		goto classify_statement;
	    }
	    lex = scan();	
	}
	lex = scan();	
	*stmt_line = token_line;
	*stmt_coln = token_coln;
	return lex;
    }
    *stmt_line = next_stmt_line;
    *stmt_coln = next_stmt_coln;
    return lex;
}


void parse_block(int* pass_through, statement_ctx ctx)
{
    int block_line = token_line;
    int block_coln = token_coln;
    int lex = scan();
    int stmt_line = token_line;
    int stmt_coln = token_coln;
    int reachable = 1;
    int unreachable = 0;
    int unreachable_line;
    int unreachable_coln;
    int entry_point;

    while (1) { 
	switch (lex) { 
	  case EOF:
	    message_at(block_line, block_coln, "Unbalanced brackets");
	    return;
	  case '}':	
	    if (notreached_cmt) {
		*pass_through = 1;
	    }
	    return;
	  case TKN_ELSE:
	    message_at(stmt_line, stmt_coln, "ELSE without IF");
	    lex = scan();
	    continue;
	  case TKN_FOR: 
	  case TKN_IF: 
	  case TKN_DO: 
	  case TKN_WHILE: 
	  case TKN_BREAK:
	  case TKN_CASE:
	  case TKN_CATCH:
	  case TKN_FINALLY:
	  case TKN_TRY:
	  case TKN_SWITCH:
	    if (ctx == ctx_typedef)
		ctx = ctx_statement;
	    nobreak;
	  default:
	    if (lex == TKN_CASE)
		reachable = 1;
	    else
		reachable = *pass_through;
	    unreachable_line = stmt_line;
	    unreachable_coln = stmt_coln;
	    entry_point = 0;
	    lex = parse_statement(lex, &stmt_line, &stmt_coln, 
				  pass_through, &entry_point, ctx);
	    if (ctx != ctx_typedef && unreachable && !entry_point) { 
		message_at(unreachable_line, unreachable_coln, 
			   "Unreachable statement");
	    } 		
	    if (reachable && !*pass_through && !notreached_cmt) { 
		unreachable = 1;
	    } else { 
		unreachable = 0;
	    }
	}
    }
}

void parse_file()
{
    int lex = scan();

    while (lex != EOF) { 
	if (lex == '{') { 
	    int pass_through = 1;
	    parse_block(&pass_through, ctx_typedef);
	} else if (lex == '=') { 
	    lex = parse_expression();	    
	    continue;
	} 
	lex = scan();
    }
}

int has_suffix(char* str, char* suffix) { 
    int suffix_len = strlen(suffix);
    int str_len = strlen(str);
    if (suffix_len > str_len) { 
	return 0;
    }
    str += str_len - suffix_len;
    while (--suffix_len >= 0) { 
	if (tolower(*(unsigned char*)str) != *suffix) { 
	    return 0;
	}
	suffix += 1;
	str += 1;
    }
    return 1;
}

void load_file(char* name, int recursive) 
{ 
#ifdef _WIN32
    HANDLE dir;
    char dir_path[MAX_PATH];
    WIN32_FIND_DATA file_data;
    if (recursive != 0) { 
	sprintf(dir_path, "%s\\*", name);
	if ((dir=FindFirstFile(dir_path, &file_data)) != INVALID_HANDLE_VALUE) 
	{
	    name = dir_path; 
	}
    } else {
	if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0) { 
	    load_file(name, 1);
	    return;
	}
	if ((dir = FindFirstFile(name, &file_data)) == INVALID_HANDLE_VALUE) { 
	    fprintf(stderr, "Failed to locate file '%s'\n", name);
	    return;
	}
    }
    if (dir != INVALID_HANDLE_VALUE) {
	do {
	    if (!recursive || *file_data.cFileName != '.') { 
		char file_path[MAX_PATH];
		char* file_dir = strrchr(name, '\\');
		char* file_name_with_path;
		if (file_dir != NULL) { 
		    int dir_len = file_dir - name + 1;
		    memcpy(file_path, name, dir_len);
		    strcpy(file_path+dir_len, file_data.cFileName);
		    file_name_with_path = file_path;
		} else { 
		    file_name_with_path = file_data.cFileName;
		}
		load_file(file_name_with_path, recursive+1);
	    }
	} while (FindNextFile(dir, &file_data));
	CloseHandle(dir);
	return;
    }
#else
    DIR* dir = opendir(name);
    if (dir != NULL) { 
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) { 
	    if (*entry->d_name != '.') { 
		char full_name[MAX_PATH];
		sprintf(full_name, "%s/%s", name, entry->d_name);
		load_file(full_name, 2);
	    }
	} 
	closedir(dir);
	return;
    }
#endif
    if (recursive < 2
	|| has_suffix(name, ".java") 
	|| has_suffix(name, ".c")
	|| has_suffix(name, ".cpp") 
	|| has_suffix(name, ".cxx")
	|| has_suffix(name, ".cc")
	|| has_suffix(name, ".h")
	|| has_suffix(name, ".hpp"))
    {
	if ((f = fopen(name, "r")) != NULL) { 
	    file_name = name;
	    line = 1;
	    coln = 0;
	    java = force_java || has_suffix(name, ".java");
	    parse_file();
	    fclose(f);
	} else { 
	    fprintf(stderr, "Failed to open file '%s'\n", name);
	}
    }
}

void usage()
{
    fprintf(stderr,
	    "Usage: antic [-version] [-java] [-tab <TAB SIZE>] [-relax-else] file|dir {file|dir...}\n");
}


int main(int argc, char* argv[])
{
    int i;
    if (argc == 1) { 
	usage();
	return 0;
    } 
    for (i = 1; i < argc; i++) { 
	if (*argv[i] == '-') { 
	    if (strcmp(argv[i], "-java") == 0) { 
		force_java = 1;
	    } else if (strcmp(argv[i], "-tab") == 0 && i+1 < argc) { 
		if (sscanf(argv[++i], "%d", &tab_size) != 1) { 
		    tab_size = 8;
		    fprintf(stderr, "Bad value '%s' for TAB size parameter\n",
			    argv[i]);
		}
	    } else if (strcmp(argv[i], "-relax-else") == 0) {
		relax_else = 1;
	    } else if (strcmp(argv[i], "-version") == 0) {
		fprintf(stderr,
			"AntiC - C/C++/Java syntax verifier"
			", version %s (" __DATE__ ")\n", VERSION);
		return 0;
	    } else { 
		fprintf(stderr, "Unrecognized option %s\n", argv[i]);
		usage();
	    } 
	} else { 
	    load_file(argv[i], 0);
	}
    }
    printf("Verification completed: %d reported messages\n", n_messages);
    return 0;
}	
