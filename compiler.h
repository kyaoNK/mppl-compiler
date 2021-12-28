#ifndef COMPILER_H_
#define COMPILER_H_

/* Standard Library */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ---------- Macro in Lexcial and Scan ---------- */
/* Token Macro */
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

/* Num Macro */
#define NUM_OF_TOKEN    49 /* number of token */
#define NUM_OF_KEYWORD	28	/* Number of keyword */
#define NUM_OF_SYMBOL  	18	/* Number of symbol */

/* Max Size Macro */
#define MAX_NUM_NAME  1000	/* Max name list size in Name*/
#define MAX_STR_SIZE  1024	/* String attitude max size in Scan */
#define MAX_NUM_SIZE 32767	/* Number attitude max size in Scan */
#define MAX_BUF_SIZE  1024  /* Max buffer size */

/* ---------- Macro in Parse and ID List ---------- */
/* FLAG Macro */
#define FLAG_PP (0)     /* flag of pretty print */
#define FLAG_CR (1)     /* flag of cross reference */
#define FLAG_DEBUG (0)  /* flag of debug in coding */

/* Macro Boolean Result */
#define FALSE 0
#define TRUE  1

/* Macro Parse Result in Parse */
#define NORMAL 0
#define ERROR -1

/* Macro Error Pattern in Parse */
#define ERR_PAT_TOKEN 0
#define ERR_PAT_SYNTAX 1

/* Macro Type in Parse and ID List */
#define TYPE_NONE       10
#define TYPE_INT        11
#define TYPE_CHAR       12
#define TYPE_BOOL       13
#define TYPE_ARRAY      14
#define TYPE_ARRAY_INT  15
#define TYPE_ARRAY_CHAR 16
#define TYPE_ARRAY_BOOL 17
#define TYPE_PROCEDURE  18

/* ---------- Struct in Lexcial and Scan ---------- */
extern struct Key {		/* Keyword Structure */
	char * keyword;
	int keytoken;
} key[NUM_OF_KEYWORD];

extern struct Symbol {	/* Symbol Structure */
	char * symbolword;
	int symboltoken;
} symbol[NUM_OF_SYMBOL];

extern struct TokenTable {	/* Token Count Table Structure */
	char * str;
	int cnt;
} token_table[NUM_OF_TOKEN+1];

extern struct NameTable {	/* Name Count Table Structure */
	char str[MAX_STR_SIZE];
	int cnt;
} name_table[MAX_NUM_NAME] ;

/* ---------- Struct in ID List ---------- */
typedef struct _Type Type;
typedef struct _Line Line;
typedef struct _ID ID;

struct _Type {
    int type;   /* Macro Type */
    int size_array; /* size of array if TYPE_ARRAY */   /* else : 0 */
    Type *array_elem_type;  /* pointer to element type if TYPE_ARRAY */
    Type *next_param_type; /* pointer to parameters type list if type is TYPE_PROCEDURE */
};

struct _Line {
    int refer_linenum;  /* refer in line num */
    Line *p_next_line;  /* pointer to next line */
};

struct _ID {
    char *name;             /* variable name */
    char *procedure_name;   /* procedure name if variable name define in procedure , else null */
    Type *id_type;          /* variable type */
    int   is_formal_param;  /* 1: formal parameter, 0: else */
    int   def_linenum;      /* variable's define linenum */    
    Line *ref_linenum;      /* variable's refer  linenum */
    ID   *next_id;          /* next id pointer */
} *global_id_root, *local_id_root, *undefined_type_id_root;

/* ---------- Variable in Lexcial and Scan ---------- */
extern int cbuf;	/* Input character from file */
extern int linenum;	/* Reading file line num */

extern int num_attr;	/* Number from file with scan */
extern char string_attr[MAX_STR_SIZE]; /* String from file with scan */

extern int name_end;	/* Number of Name Table End */

/* ---------- Variable in ID List ---------- */
/* id info */
extern char *procedure_name_str;
extern int  *is_formal_parameters;

/* type info */
extern int size_array;

/* flag */
extern int flag_in_subprogram;
extern int flag_in_formal_parameters;
extern int flag_in_variable_declaration;
extern int flag_procedure_name_in_subprogram_declaration;

/* id */
extern ID *id_procedure_name;

/* ---------- Function in Scan ---------- */
extern int init_scan ( char * filepath ) ;	/* Initialize Scanner */
extern int scan ( void ) ;					/* Scan (run) Return Token */
extern int get_linenum ( void ) ;
extern void end_scan ( void ) ;

/* ---------- Function in ID List ---------- */
extern void init_id_root ( void ) ;
extern int add_undefined_type_id( char *name ) ;
extern int assign_type ( int itype ) ;
extern int add_ref_linenum ( char *name ) ;
/* procedure name */
extern int set_procedure_name ( char *procedure_name ) ;
extern void free_procedure_name ( void ) ;
extern ID* search_procedure_id ( char *procedure_name ) ;

extern int is_array_type ( int itype );

/* debug */
extern void print_id_table ( ID *id_root ) ;
extern char* get_type_str ( int itype ) ;

/* ---------- Function in Cross Reference ---------- */
extern void print_crossreference ( void ) ;

/* ---------- Main ---------- */
extern void token_list ( void ) ;   /* Main Lexical */
extern int parse ( void );          /* Main Parse */

#endif /* COMPILER_H_ */
