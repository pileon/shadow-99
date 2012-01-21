/********************************************************************
* File: mudconfig.cc                              Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "server.h"

/*******************************************************************/

/* This structure contains all parsed configuration data. */
/* (See mudconfig.h for its definition) */
struct cfgdata mudconfig;

enum ctyp_t
{
	CFG_ILLEGAL,
	CFG_STRING,
	CFG_INT,
	CFG_FLOAT,
	CFG_BOOL,
};

/*
 * This table contains all variables in the configuration file.
 * If more are needed, add them here.
 */
static struct
{
	char  *name;
	ctyp_t type;
	void  *value;
} cfgtable[] = {
	{ "mudname"   , CFG_STRING,  mudconfig.mudname    },
	{ "masterport", CFG_INT   , &mudconfig.masterport },
	{ "libdir"    , CFG_STRING,  mudconfig.libdir     },
	{ "maxusers"  , CFG_INT   , &mudconfig.maxusers   },
	{ "adminmail" , CFG_STRING,  mudconfig.admail     },
	{ "adminname" , CFG_STRING,  mudconfig.admname    },
	{ "room_mstart",CFG_STRING, &mudconfig.room_mstart},
	{ "room_wstart",CFG_STRING, &mudconfig.room_wstart},
	{ "room_sanc" , CFG_STRING, &mudconfig.room_sanc  },
	{ "area_index", CFG_STRING,  mudconfig.areaindex  },

	/* end-of-table entry, do not remove or change */
	{ NULL, CFG_ILLEGAL, NULL }
};

/*******************************************************************/

static void cfg_printhelp(const char *name, const bool verbose);
static char *get_cfgfilename(int ac, char *av[]);
static void parse_line(const char *file, const char *input, const int line);
static void parse_command(const char *file, const int ln,
						  const char *cmd, const char *arg, const int idx);
static void cfg_string(const char *file, const int ln, const int idx,
					   const char *cmd, const char *arg, char *dest);
static void cfg_int(const char *file, const int ln, const int idx,
					const char *cmd, const char *arg, int *dest);
static void cfg_float(const char *file, const int ln, const int idx,
					  const char *cmd, const char *arg, float *dest);
static void cfg_bool(const char *file, const int ln, const int idx,
					 const char *cmd, const char *arg, int *dest);

/*******************************************************************/

bool cfg_setdef(void)
{
	memset(&mudconfig, 0, sizeof(mudconfig));

	strcpy(mudconfig.mudname  , DFLT_NAME);
	strcpy(mudconfig.libdir   , DFLT_LIB);
	strcpy(mudconfig.areaindex, DFLT_AINDEX);
	strcpy(mudconfig.room_mstart, DFLT_ROOM_MSTART);
	strcpy(mudconfig.room_wstart, DFLT_ROOM_WSTART);
	strcpy(mudconfig.room_sanc  , DFLT_ROOM_SANC);
	mudconfig.masterport  = DFLT_PORT;
	mudconfig.maxusers    = DFLT_MAX_USERS;

	mudconfig.wellcome = NULL;
	mudconfig.menu =
		"\r\n"
		"        Shadow-99       \r\n"
		"---------------------------\r\n"
		"0:  Quit from Shadow-99!\r\n"
		"1:  Enter Shadow-99!\r\n"
		"2:  Read background story\r\n"
		"\r\n"
		"Your choice: ";
	/* If you add to, or change, the menu above, remember to update
	 * the User::menu_pick() method in user.cc
	 */

	return true;
}

bool cfg_readcfg(int ac, char *av[])
{
	/* We need the program arguments here because the user might
	 * have entered the '-c' argument to use another config file
	 * than the default.  If no '-c' argument is given, use av[0]
	 * and append '.cfg' as configuration file.
	 */

	char *cfgfilename = NULL;
	FILE *fp;
	char  buf[256];
	int   line = 0;

	if ((cfgfilename = get_cfgfilename(ac, av)) == NULL)
	{
		fprintf(stderr, "Configuration not available\n");
		return false;
	}

	if ((fp = fopen(cfgfilename, "r")) == NULL)
	{
		perror(cfgfilename);
		delete [] cfgfilename;
		return false;
	}

	for (;;)
	{
		if (!fgets(buf, 256, fp))
		{
			if (feof(fp))
				break;

			perror(cfgfilename);
			fclose(fp);
			delete [] cfgfilename;
			return false;
		}

		parse_line(cfgfilename, buf, ++line);
	}
	fclose(fp);
	delete [] cfgfilename;

	return true;
}

void cfg_free(void)
{
}

bool cfg_parseargs(int ac, char *av[])
{
	int  opt, ill = 0;
	int  port = 0;
	char lib[MAX_CFG_STRLEN + 1] = { 0 };

	while ((opt = getopt(ac, av, "vhp:d:c:")) != EOF)
	{
		switch (opt)
		{
			case 'p':
				port = atoi(optarg);
				break;
			case 'd':
				strncpy(lib, optarg, MAX(MAX_CFG_STRLEN, strlen(optarg)));
				break;
			case 'c':
				break;
			case 'v':
				server.print_version();
				exit(0);
				break;
			case 'h':
				cfg_printhelp(av[0], true);
				exit(0);
				break;

			default:
				ill++;
				break;
		}
	}

	if (port && port > 1024)
		mudconfig.masterport = port;
	if (*lib)
		strcpy(mudconfig.libdir, lib);

	if (ill)
	{
		cfg_printhelp(av[0], false);
		return false;
	}

	return true;
}

/* This function checks a values in the mudconfig structure to see
 * if they are sane, and that all directories and files exists.
 */
bool cfg_checkenv(void)
{
	char path[256];

	if (mudconfig.masterport < 1024 || mudconfig.masterport > 9999)
	{
		log("Master port number must be between 1024 and 9999 (inclusive).\n");
		return false;
	}

	if (file_exists(mudconfig.libdir) >= 0)
	{
		log("Directory '%s' either does not exist, or is not a directory.\n",
			mudconfig.libdir);
		return false;
	}

	snprintf(path, sizeof(path), "%s/%s",
			 mudconfig.libdir, mudconfig.areaindex);
	if (file_exists(path) <= 0)
	{
		log("Could not fine area index file: %s\n", mudconfig.areaindex);
		return false;
	}

	return true;
}

/*******************************************************************/

static void cfg_printhelp(const char *name, const bool verbose)
{
	printf("Usage:  %s [-c <file>] [-p <port>] [-d <dir>] [-v] [-h]\n", name);

	if (verbose)
	{
		printf("Where:\n");
		printf("    -c <file> : Specify a configuration file\n");
		printf("    -p <port> : Select master port number\n");
		printf("    -d <dir>  : Select library directory\n");
		printf("    -v        : Print version number and exit\n");
		printf("    -h        : Print this help and exit\n");
		printf("\n");
		printf("Defaults:\n");
		printf("    Config: %s.cfg\n", name);
		printf("    Port  : %d\n"    , mudconfig.masterport);
		printf("    Libdir: %s\n"    , mudconfig.libdir);
	}
}

static char *get_cfgfilename(int ac, char *av[])
{
	char *cfgfilename = NULL;
	int   p;

	for (p = 1; p < ac; p++)
	{
		if (av[p][0] == '-' && av[p][1] == 'c')
		{
			if (av[p][2] != 0)
				cfgfilename = STRDUP(&av[p][2]);
			else
				cfgfilename = STRDUP(av[p + 1]);
			break;
		}
	}

	if (cfgfilename == NULL)
	{
		/* +1 for NULL char, +4 for ".cfg" */
		cfgfilename = new char [strlen(av[0]) + 1 + 4];
		if (cfgfilename)
		{
			strcpy(cfgfilename, av[0]);
			strcat(cfgfilename, ".cfg");
		}
	}

	return cfgfilename;
}

static void parse_line(const char *file, const char *input, const int ln)
{
	int c;
	char *line = NULL;
	char *cmd  = NULL;
	char *arg  = NULL;
	char *p;

	if (!input)
		return;

	cmd = line = STRDUP(input);
	strip_spaces(&cmd, false);
	if (*cmd == '#' || !*cmd)
	{
		delete [] line;
		return;  /* a comment or an empty line */
	}

	/* 1: split the line into command and argument parts */
	if ((p = strchr(cmd, '=')) == NULL)
	{
		fprintf(stderr, "%s:%2d: illegal configuration line\n", file, ln);
		delete [] line;
		return;
	}
	*p  = 0;       /* terminate the command */
	arg = (p + 1); /* set argument start */

	/* 2: strip ending comments */
	if ((p = strchr(arg, '#')) != NULL)
		*p = 0;

	/* 3: strip leading and ending spaces from the command and argument */
	strip_spaces(&cmd, true);
	strip_spaces(&arg, true);

	/* */
	for (c = 0; cfgtable[c].name != NULL; c++)
	{
		if (*cfgtable[c].name == *cmd)
		{
			if (strcasecmp(cfgtable[c].name, cmd) == 0)
				parse_command(file, ln, cmd, arg, c);
		}
	}

	delete [] line;
}

static void parse_command(const char *file, const int ln,
						  const char *cmd, const char *arg, const int idx)
{
	switch (cfgtable[idx].type)
	{
		case CFG_ILLEGAL:
			break;
		case CFG_STRING:
			cfg_string(file, ln, idx, cmd, arg, (char *) cfgtable[idx].value);
			break;
		case CFG_INT:
			cfg_int(file, ln, idx, cmd, arg, (int *) cfgtable[idx].value);
			break;
		case CFG_FLOAT:
			cfg_float(file, ln, idx, cmd, arg, (float *) cfgtable[idx].value);
			break;
		case CFG_BOOL:
			cfg_bool(file, ln, idx, cmd, arg, (int *) cfgtable[idx].value);
			break;

		default:
			break;
	}
}

/*******************************************************************/

static void cfg_string(const char *file, const int ln, const int idx,
					   const char *cmd, const char *arg, char *dest)
{
	strncpy(dest, arg, MAX_CFG_STRLEN);
}

static void cfg_int(const char *file, const int ln, const int idx,
					const char *cmd, const char *arg, int *dest)
{
	*dest = atoi(arg);
}

static void cfg_float(const char *file, const int ln, const int idx,
					  const char *cmd, const char *arg, float *dest)
{
	*dest = atof(arg);
}

static void cfg_bool(const char *file, const int ln, const int idx,
					 const char *cmd, const char *arg, int *dest)
{
	if (strcasecmp(arg, "TRUE") == 0 ||
		strcasecmp(arg, "YES")  == 0 ||
		strcasecmp(arg, "1")    == 0)
	{
		*dest = 1;
	}
	else if (strcasecmp(arg, "FALSE") == 0 ||
			 strcasecmp(arg, "NO")    == 0 ||
			 strcasecmp(arg, "0")     == 0)
	{
		*dest = 0;
	}
	else
		fprintf(stderr, "%s:%2d: illegal boolean value\n", file, ln);
}
