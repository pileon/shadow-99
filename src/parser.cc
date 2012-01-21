/********************************************************************
* File: parser.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "parser.h"
#include "loader.h"

#include <setjmp.h>

/*******************************************************************/

YYSTYPE yylval;

static jmp_buf errjmp;
static int     yytok;
static Loader *loader = NULL;

#define IS_EOF(tok)  ((tok) == EOF || (tok) == 0)

/*******************************************************************/

/*
 * '('
 * '['
 * '{'
 * '}'
 * ']'
 * ')'
 * ';'
 * '='
 * ','
 * '@'
 * TK_INTC
 * TK_STRC
 * TK_IDENT
 */

/********************************************************************
 *
 * Grammar for the parser
 * ----------------------
 *
 * file
 *   : <empty>
 *   | object_list
 *   ;
 *
 * object_list
 *   : object
 *   | object object_list
 *   ;
 *
 * object
 *   : IDENT IDENT '{' pos_prop_list '}'
 *   ;
 *
 * pos_prop_list
 *   : <empty>
 *   | prop_list
 *   ;
 *
 * prop_list
 *   : prop
 *   | prop prop_list
 *   ;
 *
 * prop
 *   : IDENT '=' thing  ';'
 *   | IDENT '=' tupple ';'
 *   ;
 *
 * thing
 *   : ref
 *   | STRC
 *   | INTC
 *   | TRUE
 *   | FALSE
 *   ;
 *
 * ref
 *   : IDENT
 *   | IDENT '@' IDENT
 *   | IDENT '@' IDENT '@' IDENT
 *   ;
 *
 * tupple
 *   : '{' thing ',' thing '}'
 *   ;
 *
 *
 * The parser follows this grammar very closely.
 *
 *******************************************************************/

static void yy_file(void);
static void yy_object_list(void);
static void yy_object(void);
static void yy_pos_prop_list(void);
static void yy_prop_list(void);
static void yy_prop(void);
static void yy_thing(const char *pn);
static void yy_ref(const char *pn);
static void yy_tupple(const char *pn);

int yyparse(Loader *l)
{
	if ((loader = l) == NULL)
		return 0;

	if (setjmp(errjmp))
		return 1;

	yytok = yylex();
	yy_file();

	return 0;
}

/*******************************************************************/

static void yymatch(const int tok)
{
	if (yytok == tok)
		yytok = yylex();
	else
	{
		yyerror();
		longjmp(errjmp, 1);
	}
}

/*******************************************************************/

static void yy_file(void)
{
	if (IS_EOF(yytok))
	{
		log("   %s: file is empty\n", lex_getfn());
		return;
	}
	yy_object_list();
}

static void yy_object_list(void)
{
	yy_object();
	if (!IS_EOF(yytok))
		yy_object_list();
}

static void yy_object(void)
{
	char *t, *n;

	/* object type */
	if (yytok == TK_IDENT)
		t = STRDUP(yylval.s);
	yymatch(TK_IDENT);

	/* object name */
	if (yytok == TK_IDENT)
		n = STRDUP(yylval.s);
	yymatch(TK_IDENT);

	if (!loader->add_object(t, n))
		longjmp(errjmp, 1);
	delete [] t;
	delete [] n;

	yymatch('{');
	yy_pos_prop_list();
	yymatch('}');
}

static void yy_pos_prop_list(void)
{
	if (yytok == TK_IDENT)
		yy_prop_list();
}

static void yy_prop_list(void)
{
	yy_prop();
	if (yytok == TK_IDENT)
		yy_prop_list();
}

static void yy_prop(void)
{
	char *pn;

	if (yytok == TK_IDENT)
		pn = STRDUP(yylval.s);
	yymatch(TK_IDENT);
	yymatch('=');

	if (yytok == '{')
		yy_tupple(pn);
	else
		yy_thing(pn);

	yymatch(';');

	delete [] pn;
}

static void yy_thing(const char *pn)
{
	if (yytok == TK_IDENT)
		yy_ref(pn);
	else if (yytok == TK_INTC)
	{
		if (!loader->set_property(pn, yylval.i))
			longjmp(errjmp, 1);
		yymatch(TK_INTC);
	}
	else if (yytok == TK_STRC)
	{
		if (!loader->set_property(pn, yylval.s))
			longjmp(errjmp, 1);
		yymatch(TK_STRC);
	}
	else if (yytok == TK_TRUE)
	{
		if (!loader->set_propbool(pn, true))
			longjmp(errjmp, 1);
		yymatch(TK_TRUE);
	}
	else if (yytok == TK_FALSE)
	{
		if (!loader->set_propbool(pn, false))
			longjmp(errjmp, 1);
		yymatch(TK_FALSE);
	}
	else
	{
		yyerror();
		longjmp(errjmp, 1);
	}
}

static void yy_ref(const char *pn)
{
	char ref[256];

	strcpy(ref, yylval.s);
	yymatch(TK_IDENT);

	if (yytok == '@')
	{
		yymatch('@');

		if (yytok == TK_IDENT)
			strcat(ref, yylval.s);
		yymatch(TK_IDENT);

		if (yytok == '@')
		{
			yymatch('@');

			if (yytok == TK_IDENT)
				strcat(ref, yylval.s);
			yymatch(TK_IDENT);
		}
	}

	if (!loader->set_propref(pn, ref))
		longjmp(errjmp, 1);
}

static void yy_tupple_thing(const char *, value_t *v)
{
	if (yytok == TK_IDENT)
	{
		v->type    = value_t::T_REF;
		v->value.s = STRDUP(yylval.s);
		yymatch(yytok);
	}
	else if (yytok == TK_INTC)
	{
		v->type    = value_t::T_INT;
		v->value.i = yylval.i;
		yymatch(yytok);
	}
	else if (yytok == TK_STRC)
	{
		v->type    = value_t::T_STR;
		v->value.s = STRDUP(yylval.s);
		yymatch(yytok);
	}
	else if (yytok == TK_TRUE)
	{
		v->type    = value_t::T_BOOL;
		v->value.i = 1;
		yymatch(yytok);
	}
	else if (yytok == TK_FALSE)
	{
		v->type    = value_t::T_BOOL;
		v->value.i = 0;
		yymatch(yytok);
	}
	else
	{
		yyerror();
		longjmp(errjmp, 1);
	}
}

static void yy_tupple(const char *pn)
{
	tripple_t t;
	bool istri = false;
	bool rc    = false;

	yymatch('{');
	yy_tupple_thing(pn, &t.v1);
	yymatch(',');
	yy_tupple_thing(pn, &t.v2);
	if (yytok == ',')
	{
		yymatch(yytok);
		yy_tupple_thing(pn, &t.v3);
		istri = true;
	}
	yymatch('}');

	if (istri)
		rc = loader->set_property(pn, &t);
	else
		rc = loader->set_property(pn, (tupple_t *) &t);
	if (rc == false)
		longjmp(errjmp, 1);
}

/*******************************************************************/
