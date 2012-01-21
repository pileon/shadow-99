#ifndef __PARSER_H__
#define __PARSER_H__
/********************************************************************
* File: parser.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

/*******************************************************************/

#define TK_INTC   300
#define TK_STRC   301
#define TK_IDENT  302
#define TK_TRUE   303
#define TK_FALSE  304

/*******************************************************************/

typedef union
{
	long  i;
	char *s;
} YYSTYPE;

extern YYSTYPE yylval;

/*******************************************************************/

#ifdef __GNUC__
void yyerror(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#else
void yyerror(const char *fmt, ...);
#endif
void yyerror(void);

int yylex(void);

const char *lex_getfn(void);
const int   lex_getln(void);

/*******************************************************************/
#endif /* __PARSER_H__ */
