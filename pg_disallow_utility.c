#include "postgres.h"

#include "tcop/utility.h"

PG_MODULE_MAGIC;

/* Saved hook value in case of unload */
static ProcessUtility_hook_type prev_ProcessUtility = NULL;

/* GUC variables */
static bool	disallow_alter_system = false;
static bool	disallow_truncate = false;

/* Function declarations */
void		_PG_init(void);
void		_PG_fini(void);

static void pgdu_ProcessUtility(Node *parsetree, const char *queryString,
					ProcessUtilityContext context, ParamListInfo params,
					DestReceiver *dest, char *completionTag);
static void PreventCommandIfDisallowed(const char *cmdname, bool disallowed);

/*
 * Module load callback
 */
void
_PG_init(void)
{
	/* Define (or redefine) custom GUC variables */
	DefineCustomBoolVariable("pg_disallow_utility.alter_system",
							 "Disallows ALTER SYSTEM commands to be executed.",
							 NULL,
							 &disallow_alter_system,
							 false,
							 PGC_SIGHUP,
							 0,
							 NULL,
							 NULL,
							 NULL);

	DefineCustomBoolVariable("pg_disallow_utility.truncate",
							 "Disallows TRUNCATE commands to be executed.",
							 NULL,
							 &disallow_truncate,
							 false,
							 PGC_SIGHUP,
							 0,
							 NULL,
							 NULL,
							 NULL);

	EmitWarningsOnPlaceholders("pg_disallow_utility");

	/* Install hook */
	prev_ProcessUtility = ProcessUtility_hook;
	ProcessUtility_hook = pgdu_ProcessUtility;
}

/*
 * Module unload callback
 */
void
_PG_fini(void)
{
	/* Uninstall hook */
	ProcessUtility_hook = prev_ProcessUtility;
}

/*
 * ProcessUtility hook
 */
static void
pgdu_ProcessUtility(Node *parsetree, const char *queryString,
					ProcessUtilityContext context, ParamListInfo params,
					DestReceiver *dest, char *completionTag)
{
	switch (nodeTag(parsetree))
	{
		case T_AlterSystemStmt:
			PreventCommandIfDisallowed("ALTER SYSTEM", disallow_alter_system);
			break;
		case T_TruncateStmt:
			PreventCommandIfDisallowed("TRUNCATE", disallow_truncate);
			break;
		default:
			/* do nothing */
			break;
	}

	if (prev_ProcessUtility)
		prev_ProcessUtility(parsetree, queryString,
							context, params,
							dest, completionTag);
	else
		standard_ProcessUtility(parsetree, queryString,
								context, params,
								dest, completionTag);
}

static void
PreventCommandIfDisallowed(const char *cmdname, bool disallowed)
{
	if (disallowed)
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("%s is not allowed by pg_disallow_utility", cmdname)));
}
