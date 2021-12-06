#ifndef TOKENCNT_H_
#define TOKENCNT_H_

/* ------ Standard Library ----- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ---------- Token Macro in Scan Module ---------------------- */

#define	TNAME		1	/* Name : Alphabet { Alphabet | Digit } */
#define	TPROGRAM	2	/* program : Keyword */
#define	TVAR		3	/* var : Keyword */
#define	TARRAY		4	/* array : Keyword */
#define	TOF			5	/* of : Keyword */
#define	TBEGIN		6	/* begin : Keyword */
#define	TEND		7  	/* end : Keyword */
#define	TIF			8  	/* if : Keyword */
#define	TTHEN		9	/* then : Keyword */
#define	TELSE		10	/* else : Keyword */
#define	TPROCEDURE	11	/* procedure : Keyword */
#define	TRETURN		12	/* return : Keyword */
#define	TCALL		13	/* call : Keyword */
#define	TWHILE		14	/* while : Keyword */
#define	TDO			15 	/* do : Keyword */
#define	TNOT		16	/* not : Keyword */
#define	TOR			17	/* or : Keyword */
#define	TDIV		18 	/* div : Keyword */
#define	TAND		19 	/* and : Keyword */
#define	TCHAR		20	/* char : Keyword */
#define	TINTEGER	21	/* integer : Keyword */
#define	TBOOLEAN	22 	/* boolean : Keyword */
#define	TREADLN		23	/* readln : Keyword */
#define	TWRITELN	24	/* writeln : Keyword */
#define	TTRUE		25	/* true : Keyword */
#define	TFALSE		26	/* false : Keyword */
#define	TNUMBER		27	/* unsigned integer */
#define	TSTRING		28	/* String */
#define	TPLUS		29	/* + : symbol */
#define	TMINUS		30 	/* - : symbol */
#define	TSTAR		31 	/* * : symbol */
#define	TEQUAL		32 	/* = : symbol */
#define	TNOTEQ		33 	/* <> : symbol */
#define	TLE			34 	/* < : symbol */
#define	TLEEQ		35 	/* <= : symbol */
#define	TGR			36	/* > : symbol */
#define	TGREQ		37	/* >= : symbol */
#define	TLPAREN		38 	/* ( : symbol */
#define	TRPAREN		39 	/* ) : symbol */
#define	TLSQPAREN	40	/* [ : symbol */
#define	TRSQPAREN	41 	/* ] : symbol */
#define	TASSIGN		42	/* := : symbol */
#define	TDOT		43 	/* . : symbol */
#define	TCOMMA		44	/* , : symbol */
#define	TCOLON		45	/* : : symbol */
#define	TSEMI		46	/* ; : symbol */
#define	TREAD		47	/* read : Keyword */
#define	TWRITE		48	/* write : Keyword */
#define	TBREAK		49	/* break : Keyword */

#define NUM_OF_TOKEN 49 /* number of token */

/* ---------- Macro in LEXICAL Module ---------------------- */
#define NUM_OF_KEYWORD	28	/* Number of keyword */
#define NUM_OF_SYMBOL  	18	/* Number of symbol */

/* ---------- Macro in Name Module ---------------------------- */
#define MAX_NUM_NAME  1000	/* Max name list size (Extend) */

/* ---------- Macro in Scan Module ---------------------------- */
#define MAX_STR_SIZE  1024	/* String attitude max size */
#define MAX_NUM_SIZE 32767	/* Number attitude max size */

/* ---------- Variable in Scan Module ------------------------- */
extern int cbuf;	/* Input character from file */
extern int linenum;	/* Reading file line num */

extern int num_attr;	/* Number from file with scan */
extern char string_attr[MAX_STR_SIZE]; /* String from file with scan */

/* ---------- Struct in Scan Module --------------------------- */
extern struct Key {		/* Keyword Structure */
	char * keyword;
	int keytoken;
} key[NUM_OF_KEYWORD];

extern struct Symbol {	/* Symbol Structure */
	char * symbolword;
	int symboltoken;
} symbol[NUM_OF_SYMBOL];

/* ---------- Token Table in LEXICAL Module ---------------- */
extern struct TokenTable {	/* Token Count Table Structure */
	char * str;
	int cnt;
} token_table[NUM_OF_TOKEN+1];

/* ---------- Name Table in Name Module ----------------------- */
extern struct NameTable {	/* Name Count Table Structure */
	char str[MAX_STR_SIZE];
	int cnt;
} name_table[MAX_NUM_NAME] ;

extern int name_end;	/* Number of Name Table End */

/* ---------- Scanner's Function in Scan Modul ---------------- */
extern int init_scan ( char * filepath ) ;	/* Initialize Scanner */
extern int scan ( void ) ;					/* Scan (run) Return Token */
extern int get_linenum ( void ) ;
extern void end_scan ( void ) ;
extern void shift_cbuf ( void ) ;			/* Shift Next Cbuf from File */
extern int error_scan ( char * mes ); 	/* Out Error Message in Scan Module */

/* ---------- Print Table Module ------------------------------ */
/* Debug print (line, Number or String Token, Num or Str) */
extern void debug_token( int token );

/* Output Token Table in text file or terminal */
extern void print_token_table ( void );

/* Output Name Table in text file or terminal (Extend) */
extern void print_name_table ( void );

/* ---------- Main Token Module Functions --------------------- */
extern void token_list ( void );	/* Run LEXICAL Module */


#endif /* TOKENCNT_H_ */