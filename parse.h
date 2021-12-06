#ifndef PARSE_H_
#define PARSE_H_

#include "lexical.h"

/* --- Parse Syntax Module Main Function --- */

/* Macro Syntax Result in Parse Module */
#define NORMAL 0
#define ERROR 1

/* Macro Error Pattern in Parse Module */
#define ERR_PAT_TOKEN 0
#define ERR_PAT_SYNTAX 1

/* Run Function in Parse */
extern int parse ( void );


/* Read Next Token and Pretty Print */
extern void next_token ( void );

/* Out Error Message */
extern int error_syntax ( char * mes , int pattern );

/* --- Pretty-Print Module Main Function --- */

/* Init Pretty Print */
extern void init_pretty ( void );

/* Pretty Print */
extern void pretty_print ( void );

#endif /* PARSE_H_ */


