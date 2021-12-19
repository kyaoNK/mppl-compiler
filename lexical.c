#include "compiler.h"	/* Read lexical header file */

/* ---------- Global Variable --------------------------------- */
/* ---------- Struct Scan Module --------------- */
struct Key key[NUM_OF_KEYWORD] = {	/* Struct Keyword List from Scan Module */
	{"and", 	TAND	},
	{"array",	TARRAY	},
	{"begin",	TBEGIN	},
	{"boolean",	TBOOLEAN},
	{"break",	TBREAK  },
	{"call",	TCALL	},
	{"char",	TCHAR	},
	{"div",		TDIV	},
	{"do",		TDO	},
	{"else",	TELSE	},
	{"end",		TEND	},
	{"false",	TFALSE	},
	{"if",		TIF	},
	{"integer",	TINTEGER},
	{"not",		TNOT	},
	{"of",		TOF	},
	{"or",		TOR	},
	{"procedure", TPROCEDURE},
	{"program",	TPROGRAM},
	{"read",	TREAD	},
	{"readln",	TREADLN },
	{"return", 	TRETURN },
	{"then",	TTHEN	},
	{"true",	TTRUE	},
	{"var",		TVAR	},
	{"while",	TWHILE	},
	{"write",	TWRITE  },
	{"writeln",	TWRITELN}
};
struct Symbol symbol[NUM_OF_SYMBOL] = {	/* Struct Symbol List from Scan Module */
	{"+", 	TPLUS},
	{"-",	TMINUS},
	{"*", 	TSTAR},
	{"=", 	TEQUAL},
	{"<>", 	TNOTEQ},
	{"<", 	TLE},
	{"<=", 	TLEEQ},
	{">", 	TGR},
	{">=", 	TGREQ},
	{"(", 	TLPAREN},
	{")", 	TRPAREN},
	{"[", 	TLSQPAREN},
	{"]", 	TRSQPAREN},
	{":=", 	TASSIGN},
	{".", 	TDOT},
	{",", 	TCOMMA},
	{":", 	TCOLON},
	{";", 	TSEMI}
};

/* ---------- Token List Module --------------- */
struct TokenTable token_table[NUM_OF_TOKEN+1] = {	/* Struct Token Count Table from Token List Module */
	{"", 0},
	{"NAME", 0},
	{"program", 0}, 
	{"var", 0},
	{"array", 0}, 
	{"of", 0}, 
	{"begin", 0},
	{"end", 0}, 
	{"if", 0}, 
	{"then", 0},
	{"else", 0}, 
	{"procedure", 0}, 
	{"return", 0}, 
	{"call", 0}, 
	{"while", 0}, 
	{"do", 0}, 
	{"not", 0}, 
	{"or", 0}, 
	{"div", 0}, 
	{"and", 0},
	{"char", 0}, 
	{"integer", 0}, 
	{"boolean", 0}, 
	{"readln", 0}, 
	{"writeln", 0}, 
	{"true", 0},
	{"false", 0}, 
	{"NUMBER", 0}, 
	{"STRING", 0}, 
	{"+", 0}, 
	{"-", 0}, 
	{"*", 0}, 
	{"=", 0}, 
	{"<>", 0}, 
	{"<", 0}, 
	{"<=", 0}, 
	{">", 0}, 
	{">=", 0}, 
	{"(", 0}, 
	{")", 0}, 
	{"[", 0}, 
	{"]", 0}, 
	{":=", 0}, 
	{".", 0}, 
	{",", 0}, 
	{":", 0}, 
	{";", 0}, 
	{"read", 0},
	{"write", 0}, 
	{"break", 0}
};

/* ---------- Name Module --------------- */
struct NameTable name_table[MAX_NUM_NAME];	/* Name Count Table from Name Module (Extend) */
int name_end;	/* Number of Name Table End from Name Module (Extend) */

/* ---------- Scan Module --------------- */
int num_attr;	/* Variable Number Attitude */
char string_attr[MAX_STR_SIZE];	/* Variable String Attitude */

/* ---------- Function Prototype Declaration ------------------ */
/* ---------- Name Module ---------- */
int cntup_name ( char * name );		/* Exist Name Count Up */
void add_newname ( char * name );	/* New Name add Name Table */

/* ---------- Print Module ---------- */
void debug_token ( int token ) ;	/* Print Debug Token */
void print_token_table ( void ) ;	/* Print Token Table */
void print_name_table ( void ) ;	/* Print Name Table */

/* ---------- Function Definition ----------------------------- */
/* ---------- Main Token List -------------------- */
void token_list ( void ) {
	/* Declaration variable */
	int i;	/* Index init table */
	int token;	/* Token */

	name_end = 0;	/* Number of name table end (Extend) */

    /* Initialize token count table */
	for (i = 0; i < NUM_OF_TOKEN; i++) {
		token_table[i].cnt = 0;
	}

	/* Initialize name count talbe (Extend) */
	for (i = 0; i < MAX_NUM_NAME; i++) {
		memset(&name_table[i].str, '\0', sizeof(name_table[i].str));
		name_table[i].cnt = 0;
	}

	/* ----- Scan token from file ---------- */
    while((token = scan()) >= 0) {

		/* Count up Token */
		token_table[token].cnt++;

		/* ===== Debug ========== */
		/* Print token info */
		debug_token(token);
		/* ===== Debug ========== */

		/* ----- Extend ---------- */
		if ( token == TNAME ) {
			/* Exist name is count up */
			if ( cntup_name (string_attr) == -1 )	/* if name not exist, add name as new */
			{
				add_newname(string_attr);
			}
		}
    }

	/* ----- Print table ---------- */
	print_token_table();	/* Print Token count table */

	print_name_table();	/* Print Name count table (Extend) */

    end_scan();	/* Close file */
    return ;
}


/* ---------- Name Module -------------------- */
/* ---------- Exist Name Count Up ---------- */
int cntup_name ( char * name ) {
	int i;
	for ( i = 0; i < name_end; i++) {
		if (strcmp(name_table[i].str, name) == 0) {
            name_table[i].cnt++;
			return 0;
        }
	}
	return -1;
}
/* ---------- Add New Name to Name Table ---------- */
void add_newname ( char * name ) {
	strcpy(name_table[name_end].str, name);
	name_table[name_end++].cnt++;

	/* ERROR */
	if (name_end >= MAX_NUM_NAME) printf("\n ERROR NAME : Over name list %d\n", name_end);
}

/* ---------- Name Module -------------------- */

/* ---------- Print Module -------------------- */
/* ---------- Print Debug Token ---------- */
void debug_token( int token ) {
    switch ( token ) {
    case TNUMBER :
        printf("line: %d , token: %d , NUMBER: %d\n", get_linenum(), token, num_attr);
        break;
    case TSTRING :
        printf("line: %d , token: %d , STRING: '%s'\n", get_linenum(), token, string_attr);
        break;
    case TNAME :
        printf("line: %d , token: %d , NAME: '%s'\n", get_linenum(), token, string_attr);
        break;
    default:
        printf("line: %d , token: %d , OTHER : '%s'\n", get_linenum(), token, string_attr);
        break;
    }
}

/* ---------- Print Token Table ---------- */
void print_token_table ( void ) {
    int i;
    printf("Write Token Count Table\n");
    // Print Token Table
    printf("-----------+--------------+-------\n");
    printf(" TOKEN NUM |  TOKEN NAME  | COUNT \n");
    printf("-----------+--------------+-------\n");
    for (i = 1; i <= NUM_OF_TOKEN; i++) {
        if (token_table[i].cnt > 0) {
            printf("       %3d |%14s| %3d \n", i, token_table[i].str, token_table[i].cnt);
        }
    }
    printf("-----------+--------------+-------\n");
}

/* ---------- Print Name Token ---------- */
void print_name_table ( void ) {
    int i;
    /* name not exist , finish */
    if (name_end == 0) return ;
    printf("Write Name table in terminal.\n");
    // Print Token Table
    printf("-------+------------------------------------- \n");
    printf(" COUNT | NAME STRING  \n");
    printf("-------+------------------------------------- \n");
    for (i = 0; i < name_end; i++) {
        printf("   %3d | %s \n", name_table[i].cnt, name_table[i].str);
    }
    printf("-------+------------------------------------- \n");
}

/* ---------- Print Module -------------------- */
