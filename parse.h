#ifndef PARSE_H_
#define PARSE_H_

#include "lexical.h"

/* --- Parse Syntax Module Main Function --- */

/* Macro Syntax Result in Parse Module */
#define NORMAL 0
#define ERROR 1
#define INTEGER 2
#define BOOLEAN 3
#define CHAR 4

/* Macro Error Pattern in Parse Module */
#define ERR_PAT_TOKEN 0
#define ERR_PAT_SYNTAX 1

/* Run Function in Parse */
extern int parse ( void );

/* Read Next Token and Pretty Print */
extern void next_token ( void );

/* Out Error Message */
extern int error_syntax ( char * mes , int pattern );

#endif /* PARSE_H_ */


