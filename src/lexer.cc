/********************************************************************
* File: lexer.cc                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
*********************************************************************
*                                                                   *
* This file contains the lexical analyser used by the world file    *
* parser.  I opted to handcode it instead of using programs like    *
* lex or flex, since this approach gives me greater flexibility.    *
*                                                                   *
* The functions in this file are fast, small, and effective.  This  *
* also means that some of them might be hard to read and understand *
* for the novice coder.  It also means, that if you don't understand*
* these functions, you shouldn't mess with them!                    *
*                                                                   *
* So how does this miracle of a lexer work?                         *
* Basicly I have a large chunk of memory, divided into two parts.   *
* At initialisation, the first half is filled bytes from the file,  *
* and then I'm getting one character at a time through the lex_getc *
* function.  When lex_getc finds a 0 byte, it means that we have hit*
* the end of the buffer, and the other buffer (not the one we are   *
* reading from) is filled and the buffer pointer is moved to the    *
* start of that buffer.  If we find an EOF while reading a char, it *
* means that we hit the end of the file.                            *
* Do note that this buffering scheme can have partial lines in a    *
* buffer, so we have to keep track of newlines when encountered.    *
*                                                                   *
* The following figure illustrates the buffer system:               *
* +-----------------------------+-+-----------------------------+-+ *
* | buffer 1                    |0| buffer 2                    |0| *
* +-----------------------------+-+-----------------------------+-+ *
* ^                                                                 *
* |                                                                 *
* buffer pointer                                                    *
*                                                                   *
* This figure illustrates a buffer where the second half recently   *
* been filled:
* +-----------------------------+-+-----------------------------+-+ *
* |Some text that needs to be pa|0|rsed<EOF>                    |0| *
* +-----------------------------+-+-----------------------------+-+ *
*                                 ^                                 *
*                                 |                                 *
*                           buffer pointer                          *
*                                                                   *
********************************************************************/

#include "shadow.h"
#include "parser.h"
#include <stdarg.h>
#include <sys/stat.h>

/*******************************************************************/

struct yyfile
{
	int   yyfd;  /* file descriptor */
	char *yyfn;  /* filename */
	int   yyln;  /* current line in file */
	char *yyp;   /* next character to get */
	char *yyb;   /* the buffer */
	int   yysz;  /* size of buffer */
	char *yydir; /* base directory of file */

	struct yyfile *prev;  /* previous node in stack */

#ifdef __cplusplus
	yyfile()
		{ yyfd = -1; yyfn = yyb = yyp = NULL; yyln = yysz = 0; prev = NULL; }
#endif
};

static struct yyfile *yyfstack = NULL;
static int  last_token;
static char last_file[PATH_MAX + 1];
static int  nerrors;

//static const char *toksym(const int tok);
bool lex_pushfile(const char *fn, const char *dn, const char *dir);

#define D 0x1  /* decimal digit */
#define H 0x2  /* hexadecimal digit */
#define I 0x4  /* identifier character */
#define S 0x8  /* whitespace */
#define O 0x10 /* octal digit */
static const char yychars[256] = {
        /*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F      */
        /*00*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 8, 8, 8, 0, 0, /*0F*/
        /*10*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*1F*/
        /*20*/ 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*2F*/
        /*30*/19,19,19,19,19,19,19,19, 3, 3, 0, 0, 0, 0, 0, 0, /*3F*/
        /*40*/ 0, 6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, /*4F*/
        /*50*/ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4, /*5F*/
        /*60*/ 0, 6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, /*6F*/
        /*70*/ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 8, /*7F*/
        /*80*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*8F*/
        /*90*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*9F*/
        /*A0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*AF*/
        /*B0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*BF*/
        /*C0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*CF*/
        /*D0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*DF*/
        /*E0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*EF*/
        /*F0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*FF*/
        /*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F      */
};
#define ISDIGIT(c)  (yychars[(int) (c)] & 0x01)
#define ISXDIGIT(c) (yychars[(int) (c)] & 0x02)
#define ISIDENT(c)  (yychars[(int) (c)] & 0x04)
#define ISSPACE(c)  (yychars[(int) (c)] & 0x08)
#define ISODIGIT(c) (yychars[(int) (c)] & 0x10)
#define ISUPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c) ((c) >= 'a' && (c) <= 'z')

#define TOLOWER(c) (ISUPPER(c) ? (c) - 'A' + 'a' : (c))
#define TOUPPER(c) (ISLOWER(c) ? (c) - 'a' + 'A' : (c))
#define TOXNUM(c)  (ISDIGIT(c) ? (c) - '0' : 10 + TOLOWER(c) -'a')
#define TONUM(c)   ((c) - '0')

/*******************************************************************/

/* file, buffer, and character functions */

/* note that I use my own buffering here, instead of using stdio
 * this is mainly because it allows me to switch between two buffers,
 * and secondly because I think it's faster in this case
 */

static void lex_freefile(struct yyfile *f)
{
	if (f)
	{
		if (f->yyfd > STDERR_FILENO && f->yyfn)
			close(f->yyfd);  /* only close it if we opened it */
		if (f->yyb)
			delete [] f->yyb;
		if (f->yyfn)
			delete [] f->yyfn;
		if (f->yydir)
			delete [] f->yydir;
		delete f;
	}
}

static void lex_popfile(void)
{
	struct yyfile *f = yyfstack;
	if (f)
	{
		strncpy(last_file, f->yyfn, PATH_MAX);
		last_file[PATH_MAX] = 0;
		yyfstack = f->prev;
		lex_freefile(f);
	}
}

static int lex_fillbuffer(void)
{
	register int n = read(yyfstack->yyfd, yyfstack->yyp, yyfstack->yysz);
	if (n < 0)
		perror(yyfstack->yyfn);
	return n;
}

/*
 * lex_getc: get the next available character
 * if it returns 0 or EOF, it  means we hit the end of the current file
 * (or an error)
 */
static int lex_getc(void)
{
	register int n = -1;
	register int c = 0;

	if (!yyfstack)
		return EOF;

	c = (yyfstack && yyfstack->yyp ? *yyfstack->yyp++ : 0);
	while (c == 0)
	{
		/* switch buffers */
		if (yyfstack->yyp < (yyfstack->yyb + yyfstack->yysz + 1))
			/* switch to the second buffer */
			yyfstack->yyp = yyfstack->yyb + yyfstack->yysz + 1;
		else
			/* switch to the first buffer */
			yyfstack->yyp = yyfstack->yyb;

		n = lex_fillbuffer();
		if (n <= 0)
			return EOF;
		*(yyfstack->yyp + n) = 0; /* terminate the buiffer */
		c = *yyfstack->yyp++;
	}

	return c;
}

static void lex_ungetc(register const int c)
{
	if (yyfstack && yyfstack->yyp)
	{
		if ((yyfstack->yyp - 1) == (yyfstack->yyb + yyfstack->yysz))
			yyfstack->yyp--;
		else if ((yyfstack->yyp - 1) < yyfstack->yyb)
			yyfstack->yyp = yyfstack->yyb + yyfstack->yysz * 2 + 1;

		*(--yyfstack->yyp) = c;
	}
}

static void lex_skip_eol(void)
{
	/* get characters until end of line or end of file */
	register int c;
	while (1)
	{
		c = lex_getc();
		if (c == 0 || c == EOF || c == '\n')
		{
			lex_ungetc(c);
			return;
		}
	}
}

/*******************************************************************/

/* the core parsing functions */

static void lex_getcomment(void)
{
	register int c;

	while (1)
	{
		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			yyerror();
			lex_ungetc(c);
			return;
		}

		if (c == '\n')
			yyfstack->yyln++;
		else if (c == '*')
		{
			c = lex_getc();
			if (c == '/')
				return;
			lex_ungetc(c);
		}
		else if (c == '/')
		{
			c = lex_getc();
			if (c == '*')
				lex_getcomment();  /* nested comment */
			else
				lex_ungetc(c);
		}
	}
}

/* get a hexadecimal number */
static void lex_gethexnumber(void)
{
	register int  c;
	register long i = 0;

	while (1)
	{
		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			yyerror();
			yylval.i = i;
			lex_ungetc(c);
			return;
		}
		if (!ISXDIGIT(c))
		{
			lex_ungetc(c);
			yylval.i = i;
			return;
		}

		i = (i << 4) + TOXNUM(c);
	}
}

/* get a decimal integer */
static void lex_getnumber(const bool neg)
{
	register int   c;
	register long  i = 0;

	while (1)
	{
		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			yyerror();
			lex_ungetc(c);
			yylval.i = 0;
			return;
		}

		if (!ISDIGIT(c))
		{
			lex_ungetc(c);
			yylval.i = neg ? -i : i;
			return;
		}

		i = (i * 10) + TONUM(c);
	}
}

/* get an escape sequence in a string or character constant */
static int lex_getescape(void)
{
	/* TODO: support for octal escape secuences? */

	register int c;
	register int i = 0;

	c = lex_getc();
	switch (c)
	{
	case '0':
		return 0;
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 't':
		return '\t';
	case 'n':
		return '\n';
	case 'v':
		return '\v';
	case 'f':
		return '\f';
	case 'r':
		return '\r';
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '"':
		return '"';

	case 'x':
	case 'X':
		c = lex_getc();
		if (ISXDIGIT(c))
		{
			i = TOXNUM(c);
			c = lex_getc();
			if (ISXDIGIT(c))
				i = (i << 4) + TOXNUM(c);
			else
				lex_ungetc(c);
			return i;
		}
		/*FALLTHROUGH*/
	default:
		//yywarn("unknown escape sequence `%c'", c);
		return ' ';
	}

	return ' ';
}

/* get a character constant */
static void lex_getchar(void)
{
	register int c = lex_getc();

	if (c == '\\')
		yylval.i = lex_getescape();
	else
		yylval.i = c;

	/* now search for the ending ' */
	c = lex_getc();
	if (c == '\'')
		return;
	else if (c == 0 || c == EOF)
	{
		yyerror();
		lex_ungetc(c);
		return;
	}
	else
	{
		yyerror();
		while (1)
		{
			if (c == '\'')
				return;
			else if (c == '\n')
			{
				lex_ungetc(c);
				return;
			}
			else if (c == EOF || c == 0)
			{
				lex_ungetc(c);
				return;
			}
		}
	}
}

/* get a string */
static void lex_getstring(void)
{
	register int   c;
	register char *p;
	static char string[1025];  /* limit strings to 1k */

	p = string;
	while (1)
	{
		if (p >= (string + 1024))
		{
			/* uh oh, overflow detected! */
			yyerror();

			/* search for the ending '"' */
			while (c != '"' && c != 0 && c != EOF)
				c = lex_getc();

			*p = 0;
			break;
		}

		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			*p = 0;
			yyerror();
			lex_ungetc(c);
			break;
		}
		else if (c == '"')
		{
			/* concatenate whitespace separated strings */
			do
			{
				c = lex_getc();
				if (c == '\n')
					yyfstack->yyln++;
			} while (ISSPACE(c) || c == '\n');

			if (c != '"')
			{
				/* next token is not a string */
				lex_ungetc(c);
				*p = 0;
				break;
			}
		}
		else if (c == '\\')
			*p++ = lex_getescape();
		else
			*p++ = c;
	}

	yylval.s = string;
}

/* get an identifier */
static int lex_getident(void)
{
	register int   c;
	register char *p;
	static char ident[513];  /* limit the identifiers to 512 chars */

	/* first get the identifier */
	p = ident;
	while (1)
	{
		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			yyerror();
			lex_ungetc(c);
			return 0;
		}
		if (!ISIDENT(c) && !ISDIGIT(c) && c != '@')
		{
			*p = 0;
			lex_ungetc(c);
			break;
		}
		*p++ = c;
	}

	if (TOLOWER(*ident) == 't' && strcasecmp(ident, "true") == 0)
		return TK_TRUE;
	else if (TOLOWER(*ident) == 'f' && strcasecmp(ident, "false") == 0)
		return TK_FALSE;
	else
	{
		yylval.s = ident;
		return TK_IDENT;
	}
}

/*******************************************************************/

static const char *lex_get_include(const char delimit)
{
	static char string[PATH_MAX];
	register char *p;
	register char  c;

	p = string;
	while (1)
	{
		if (p > ((string + PATH_MAX) - 1))
		{
			yyerror();
			return NULL;
		}
		c = lex_getc();
		if (c == 0 || c == EOF)
		{
			yyerror();
			lex_ungetc(c);
			return NULL;
		}
		else if (c == delimit)
			break;
		*p++ = c;
	}
	*p = 0;

	return string;
}

static void lex_pp_include(void)
{
	const char  c = lex_getc();
	const char *fn, *p;
	char path[PATH_MAX * 2], file[PATH_MAX + 1];
	char dir[PATH_MAX + 1], dir2[PATH_MAX + 1];

	if (c == '"')
		fn = lex_get_include('"');
	else if (c == '<')
		fn = lex_get_include('>');
	else
	{
		lex_ungetc(c);
		return;
	}

	lex_skip_eol();

	if (!fn)
		return;

	if (yyfstack->yydir && c == '"')
		snprintf(path, PATH_MAX * 2, "%s/%s", yyfstack->yydir, fn);
	else
		snprintf(path, PATH_MAX * 2, "world/%s", fn);

	if ((p = strrchr(fn, '/')) != NULL)
	{
		memcpy(dir2, fn, p - fn);
		*(dir2 + (p - fn)) = 0;  /* terminate string */
		snprintf(dir, PATH_MAX, "%s/%s", yyfstack->yydir, dir2);
	}
	else
	{
		strncpy(dir, yyfstack->yydir, PATH_MAX);
		dir[PATH_MAX] = 0;
	}

	if ((p = strchr(yyfstack->yydir, '/')) != NULL)
		snprintf(file, PATH_MAX, "%s/%s", p + 1, fn);
	else
	{
		strncpy(file, fn, PATH_MAX);
		file[PATH_MAX] = 0;
	}

	//printf("yyfstack->yydir = %s, fn = %s\npath = %s, dir = %s\nfile = %s\n",
	//	   yyfstack->yydir, fn, path, dir, file);
	lex_pushfile(path, file, dir);
}

static void lex_preprocess(void)
{
	register char c;

	if (lex_getident() != TK_IDENT)
	{
		lex_skip_eol();
		return;
	}

	while (ISSPACE(c = lex_getc()))
		;
	lex_ungetc(c);

	if (strcasecmp(yylval.s, "include") == 0)
		lex_pp_include();
	else
		yyerror();
}

/*******************************************************************/

/* the main function */
#define DO_RETURN(t) do { last_token = (t); return (t); } while (0)

int yylex(void)
{
	register int c;
	register int c2;

	yylval.i = 0;

	while (1)
	{
		c = lex_getc();

restart_lex:
		if (c == 0 || c == EOF)
		{
			lex_popfile();
			if (yyfstack == NULL)
				DO_RETURN(0);
			continue;
		}

		/* just skip whitespace */
		if (ISSPACE(c))
		{
			while (ISSPACE(c))
				c = lex_getc();
		}

		/* eat up all linefeeds */
		if (c == '\n')
		{
			while (c == '\n')
			{
				yyfstack->yyln++;
				c = lex_getc();
			}
			goto restart_lex;
		}

		/* check for comments */
		if (c == '/')
		{
			c2 = lex_getc();
			if (c2 == '*')
			{
				lex_getcomment();
				continue;
			}
			else if (c2 == '/')
			{
				lex_skip_eol();
				continue;
			}

			lex_ungetc(c2);
		}

		/* preprocessor directives */
		if (c == '#')
		{
			lex_preprocess();
			continue;
		}

		/* check for numbers */
		if (c == '0')
		{
			c2 = lex_getc();
			if (c2 == 'X' || c2 == 'x')
			{
				/*lex_ungetc(c2);*/
				lex_gethexnumber();
				DO_RETURN(TK_INTC);
			}
			else
				lex_ungetc(c2);
		}

		if (ISDIGIT(c))
		{
			lex_ungetc(c);
			lex_getnumber(false);
			DO_RETURN(TK_INTC);
		}

		if (c == '-')
		{
			c2 = lex_getc();
			lex_ungetc(c2);
			if (ISDIGIT(c2))
			{
				lex_getnumber(false);
				yylval.i = -yylval.i;
				DO_RETURN(TK_INTC);
			}
		}

		/* identifiers */
		if (ISIDENT(c))
		{
			lex_ungetc(c);
			c2 = lex_getident();
			if (c2 == 0)
				continue;
			else
				DO_RETURN(c2);
		}

		if (c == '$')
		{
			lex_gethexnumber();
			DO_RETURN(TK_INTC);
		}

		/* character and string constants */
		if (c == '\'')
		{
			lex_getchar();
			DO_RETURN(TK_INTC);
		}
		if (c == '"')
		{
			lex_getstring();
			DO_RETURN(TK_STRC);
		}

		/* operators and other characters */
		switch (c)
		{
		case '(': case '[': case '{':
		case '}': case ']': case ')':
		case ';': case '=': case ',':
		case '@':
			DO_RETURN(c);
		default:
			break;
		}

		/* if we have reached this point, c is not an acceptable
		 * chacacter in the language
		 */
		yyerror();
	}

	return 0;
}

/*******************************************************************/

/* public buffer/file interface */

const char *lex_getfn(void) { return (yyfstack ? yyfstack->yyfn : last_file); }
int   lex_getln(void) { return (yyfstack ? yyfstack->yyln :   -1); }
int   lex_getnerrs(void) { return nerrors; }
void lex_setnerrs(const int n) { nerrors = n; }

/* puch a new file onto the buffer stack */
bool lex_pushfile(const char *fn, const char *dn, const char *dir)
{
	struct stat st;
	struct yyfile *f;

	if (fn != NULL)
	{
		if (stat(fn, &st) < 0)
		{
			if (errno == ENOENT)
				log("   No such file: %s\n", dn);
			else
				perror("stat");
			return false;
		}
		if (!S_ISREG(st.st_mode))
		{
			log("   %s is not a regular file\n", dn);
			return false;
		}
	}

	if ((f = new yyfile) == NULL)
		out_of_memory();

	f->yyfd = -1;
	f->yysz = fn ? st.st_blksize : 4096;
	f->yyln = 1;
	f->yydir= dir ? STRDUP(dir) : NULL;

	if (fn == NULL)
		f->yyfd = STDIN_FILENO;
	else
	{
		if ((f->yyfd = open(fn, O_RDONLY)) < 0)
		{
			perror("open");
			lex_freefile(f);
			return false;
		}
		if ((f->yyfn = STRDUP(dn)) == NULL)
			out_of_memory();
	}

	if ((f->yyb = new char [f->yysz * 2 + 2]) == NULL)
		out_of_memory();

	f->yyp = NULL;

	f->prev  = yyfstack;
	yyfstack = f;

	return true;
}

void lex_popall(void)
{
	while (yyfstack)
		lex_popfile();
}

/*******************************************************************/

/* TODO:  Acumulate all errors in a list, and print them all out
 *        when done loading.
 */

void yyerror(void)
{
	if (yyfstack)
		log("   %s: Error in line %d\n", yyfstack->yyfn, yyfstack->yyln);
	else
		log("   %s: Error at end of file\n", last_file);
	nerrors++;
}

/*VARARGS1*/
void yyerror(const char *fmt, ...)
{
	va_list args;
	char    str[512];

	if (yyfstack)
		log("   %s: Error in line %d\n", yyfstack->yyfn, yyfstack->yyln);
	else
		log("   %s: Error at end of file\n", last_file);

	va_start(args, fmt);
	vsnprintf(str, 512, fmt, args);
	va_end(args);
	log("      %s\n", str);
	nerrors++;
}

/*******************************************************************/

/*
static const char *toksym(const int tok)
{
	switch (tok)
	{
	case TK_INTC:
		return "integer constant";
	case TK_STRC:
		return "string constant";
	case TK_IDENT:
		return "identifier";
	case '(':
		return "(";
	case '[':
		return "[";
	case '{':
		return "{";
	case '}':
		return "}";
	case ']':
		return "]";
	case ')':
		return ")";
	case ';':
		return ";";
	case '=':
		return "=";
	case ',':
		return ",";
	default:
		return "unknown";
	}
}
*/

/*******************************************************************/
