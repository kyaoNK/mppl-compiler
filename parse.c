#include "parse.h"

/* ---------- Global Variable --------------------------------- */
int token;      /* current token */
char string_attr[MAX_STR_SIZE]; /* string or name */
int cnt_iter;   /* iteration statement nest counter */
int cnt_tab;    /* tab counter */

/* ---------- Function Prototype Declaration ------------------ */
/* ----- Macro Syntax in Parse ----------------- */
static int program ( void );      /* Program */
static int block ( void );              /* Block */
static int var_decl ( void );           /* Variable Declaration */
static int var_names ( void );          /* Variable Names */
static int var_name ( void );           /* Variable Name */
static int type ( void );               /* Type */
static int standard_type ( void );      /* Standard Type */
static int array_type ( void );         /* Array Type */
static int subprogram_decl ( void );    /* Subprogram Declaration */
static int procedure_name ( void );     /* Procedure Name */
static int formal_params ( void );      /* Formal Parameters */
static int comp_state ( void );         /* Compound Statement */
static int statement ( void );          /* Statement */
static int cond_state ( void );         /* Condition Statement */
static int iter_state ( void );         /* Iteration Statement */
static int exit_state ( void );         /* Exit Statement */
static int call_state ( void );         /* Call Statement */
static int expressions ( void );        /* Expressions */
static int return_state ( void );       /* Return Statement */
static int assign_state ( void );       /* Assign Statement */
static int left_part ( void );          /* Left Part */
static int variable ( void );           /* Variable */
static int expression ( void );         /* Expression */
static int simple_expression ( void );  /* Simple Expression */
static int term ( void );               /* Term */
static int factor ( void );             /* Factor */
static int constant ( void );           /* Constant */
static int multiply_ope ( void );       /* Multiplicative Operator */
static int add_ope ( void );            /* Additive Operator */
static int relat_ope ( void );          /* Relation Operator */
static int input_state ( void );        /* Input Statement */
static int output_state ( void );       /* Output Statement */
static int output_format ( void );      /* Output Format */
static int empty_state ( void );        /* Empty Statement */

/* ----- Other in Parse ----------------- */
void next_token ( void ) ;  /* Read Next Token and Pretty Print */
void print_tab ( void ) ;   /* Print Tab */
static int error ( char * mes , int pattern);  /* Out Error Message */

/* Main parse */
int parse ( void ) {

    /* initialize counter */
    cnt_iter = 0;
    cnt_tab = 0;

    next_token();   /* Read next token */
    
    int res = program();    /* Program */

    return res;
}

/* Read Next Token */
void next_token ( void ) {
    /* Pretty print */
    // pretty_print();

    /* Read next token */
    // token_pre = token;
    token = scan();
}

void print_tab ( void ) {
    int i;
    for (i = 0; i < cnt_tab; i++) {
        printf("\t");
    }
}

/* Error Message */
int error_syntax ( char * mes , int pattern ) {
    if (pattern == ERR_PAT_TOKEN) {
        printf("\nERROR PARSE: line=%d | TOKEN=%d, STR: %s , MES: %s\n", get_linenum(), token, string_attr, mes);
        end_scan();
    }
    else {
        printf("| %s", mes);
    }
    return ERROR;
}

/* ---------- Syntax Function Definition ----------------------------- */

/* program ::= "program" "NAME" ";" BLOCK "." */
static int program ( void ) {
    /* "program" */
    if ( token != TPROGRAM ) return (error_syntax("Keyword 'program' is not found at the first of program", ERR_PAT_TOKEN));
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* "NAME" */
    if ( token != TNAME ) return (error_syntax("Program 'NAME' is not found after keyword 'program'", ERR_PAT_TOKEN));   /* check "NAME" */
    printf("%s", string_attr);
    next_token();   /* Read next token */
    
    /* ";" */
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found after program name", ERR_PAT_TOKEN));      /* check ";" */
    printf("%s\n", string_attr);
    next_token();   /* Read next token */
    
    /* BLOCK */
    if ( block() == ERROR ) return (error_syntax("in block", ERR_PAT_SYNTAX));
    
    /* "." */
    if ( token != TDOT ) return (error_syntax("Period '.' is not found at the end of program", ERR_PAT_TOKEN));    /* check "." */
    printf("%s\n", string_attr);
    next_token();   /* Read next token */
    
    return NORMAL;
}

/* BLOCK ::= { VARIABLE_DECLARATION | SUBPROGRAM_DECLARATION } COMPOUND_STATEMENT */
static int block ( void ) {
    /* { VARIABLE_DECLARATION | SUBPROGRAM_DECLARATION } */
    while ( token == TVAR || token == TPROCEDURE ) {
        /* VARIABLE_DECLARATION | SUBPROGRAM_DECLARATION */
        if ( token == TVAR ) {
            /* VARIABLE_DECLARATION */
            if (var_decl() == ERROR) return (error_syntax("in variable declaration", ERR_PAT_SYNTAX));
        }
        else if ( token == TPROCEDURE ) {
            /* SUBPROGRAM_DECLARATION */
            if (subprogram_decl() == ERROR ) return (error_syntax("in subprogram declaration", ERR_PAT_SYNTAX));
        }
    }

    /* COMPOUND_STATEMENT */
    cnt_tab = 0;
    if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));

    return NORMAL;
}

/* VARIABLE_DECLARATION ::= "var" VARIABLE_NAMES ":" TYPE ";" {  VARIABLE_NAMES ":" TYPE ";" } */
static int var_decl ( void ) {
    /* "var" */
    if ( token != TVAR ) return (error_syntax("Keyword 'var' is not found at the first of variable declaration", ERR_PAT_TOKEN));
    cnt_tab++;
    print_tab();
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* VARIABLE_NAMES */
    if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));

    /* ":" */
    if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
    printf(" %s ", string_attr);
    next_token();   /* Read next token */

    /* TYPE */
    if ( type() == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
    
    /* ";" */
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of variable declaration", ERR_PAT_TOKEN));
    printf("%s\n", string_attr);
    next_token();   /* Read next token */
    
    /* { VARIABLE_NAMES ":" TYPE ";" } */
    while ( token == TNAME ) {
        print_tab();
        printf("\t");
        /* VARIABLE_NAMES */
        if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));
        
        /* ":" */
        if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
        printf(" %s ", string_attr);
        next_token();   /* Read next token */
        
        /* TYPE */
        if ( type() == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
        
        /* ";" */
        if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of variable declaration", ERR_PAT_TOKEN));
        printf("%s\n", string_attr);
        next_token();   /* Read next token */
    }
    cnt_tab--;

    return NORMAL;
}

/* VARIABLE_NAMES ::= VARIABLE_NAME { "," VARIABLE_NAME } */
static int var_names ( void ) {
    /* VARIABLE_NAME */
    if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));

    /* { "," VARIABLE_NAME } */
    while ( token == TCOMMA ) {
        /* "," Already Checked */
        printf("%s ", string_attr);
        next_token();   /* Read next token */

        /* VARIABLE_NAME */
        if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));
    }

    return NORMAL;
}

/* VARIABLE_NAME ::= "NAME" */
static int var_name ( void ) {
    /* "NAME" */
    if ( token != TNAME ) return (error_syntax("Variable 'NAME' is not found at the variable name", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    return NORMAL;
}

/* TYPE ::= STANDARD_TYPE | ARRAY_TYPE */
static int type ( void ) {
    /* STANDARD_TYPE | ARRAY_TYPE */
    if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR ) {
        /* STANDARD_TYPE */
        if ( standard_type() == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));
    }
    else if ( token == TARRAY ) {
        /* ARRAY_TYPE */
        if ( array_type() == ERROR ) return (error_syntax("in array type", ERR_PAT_SYNTAX));
    }
    else {
        return (error_syntax("standard type , array type is not found in type", ERR_PAT_TOKEN));
    }

    return NORMAL;
}

/* STANDARD_TYPE ::= "integer" | "boolean" | "char" */
static int standard_type( void ) {
    /* "integer" | "boolean" | "char" */
    if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Keyword neither 'integer' , 'boolean' , 'char' is not found", ERR_PAT_TOKEN));
    }

    return NORMAL;
}

/* ARRAY_TYPE ::= "array" "[" "UNSIGNED_INTEGER" "]" "of" STANDARD_TYPE */
static int array_type ( void ) {
    /* "array" */
    if ( token != TARRAY ) return (error_syntax("Keyword 'array' is not found at the first of array type", ERR_PAT_TOKEN));
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* "[" */
    if ( token != TLSQPAREN ) return (error_syntax("Left square paren '[' is not found after keyword 'array'", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    /* "UNSIGNED_INTEGER" */
    if ( token != TNUMBER ) return (error_syntax("Unsigned integer is not found after left square paren '['", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    /* "]" */
    if ( token != TRSQPAREN ) return (error_syntax("Right square paren ']' is not found after unsigned integer", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    /* "of" */
    if ( token != TOF ) return (error_syntax("Keyword 'of' is not found after right square paren ']'", ERR_PAT_TOKEN));
    printf(" %s", string_attr);
    next_token();   /* Read next token */

    /* STANDARD_TYPE */
    if ( standard_type() == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));
    
    return NORMAL;
}

/* SUBPROGRAM_DECLARATION ::= "procedure" PROCEDURE_NAME [ FORMAL_PARAMETERS ] ";" [ VARIABLE_DECLARATION ] COMPOUND_STATEMENT ";" */
static int subprogram_decl ( void ) {
    /* "procedure" */
    if ( token != TPROCEDURE ) return (error_syntax("Keyword 'procedure' is not found at the first of subprogram declaration", ERR_PAT_TOKEN));
    cnt_tab++;
    print_tab();
    printf("%s ", string_attr);
    next_token();   /* Read next token */
    
    /* PROCEDURE_NAME */
    if ( procedure_name() == ERROR ) return (error_syntax("in procedure name", ERR_PAT_SYNTAX));
    
    /* [ FORMAL_PARAMETERS ] */
    if ( token == TLPAREN ) {
        /* FORMAL_PARAMETERS */
        if ( formal_params() == ERROR ) return (error_syntax("in formal parameters", ERR_PAT_SYNTAX));
    } 

    /* ";" */
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found after procedure name or formal paramters", ERR_PAT_TOKEN));
    printf("%s\n", string_attr);
    next_token();   /* Read next token */

    /* [ VARIABLE_DECLARATION ] */
    if ( token == TVAR ) { 
        /* VARIABLE_DECLARATION */
        if ( var_decl() == ERROR ) return (error_syntax("in variable declaration", ERR_PAT_SYNTAX));
    }

    /* COMPOUND_STATEMENT */
    if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));

    /* ";" */
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of subprogram declaration", ERR_PAT_TOKEN));
    printf("%s\n", string_attr);
    cnt_tab--;
    next_token();   /* Read next token */

    return NORMAL;
}

/* PROCEDURE_NAME ::= "NAME" */
static int procedure_name ( void ) {
    /* "NAME" */
    if ( token != TNAME ) return (error_syntax("Procedure 'NAME' is not found at the procedure name", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    return NORMAL;
}

/* FORMAL_PARAMS ::= "(" VARIABLE_NAMES ":" TYPE { ";" VARIABLE_NAMES ":" TYPE } ")"  */
static int formal_params ( void ) {
    /* "(" */
    if ( token != TLPAREN ) return (error_syntax("Left paren '(' is not found at the first formal parameters", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */

    /* VARIABLE_NAMES */
    if ( var_names() == ERROR) return (error_syntax("in variable names", ERR_PAT_SYNTAX));
    
    /* ":" */
    if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */
    
    /* TYPE */
    if ( type() == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
    
    /* { ";" VARIABLE_NAMES ":" TYPE } */
    while ( token == TSEMI ) {
        /* ";" already checked */
        printf("%s ", string_attr);
        next_token();   /* Read next token */

        /* VARIABLE_NAMES */
        if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));
        
        /* ":" */
        if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
        
        /* TYPE */
        if ( type() == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
    }
    
    /* ")" */
    if ( token != TRPAREN ) return (error_syntax("Right Paren ')' is not found at the end of fromal parameters", ERR_PAT_TOKEN));
    printf("%s", string_attr);
    next_token();   /* Read next token */
    
    return NORMAL;
}

/* COMPOUND_STATEMENT ::= "begin" STATEMENT { ";" STATEMENT } "end" */
static int comp_state ( void ) {
    /* "begin" */
    if ( token != TBEGIN ) return (error_syntax("Keyword 'begin' is not found at the first of compound statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s\n", string_attr);
    cnt_tab++;
    next_token();   /* Read next token */

    /* STATEMENT */
    if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    
    /* { ";" STATEMENT } */
    while ( token == TSEMI ) {
        /* ";" alread checked */
        printf("%s\n", string_attr);
        next_token();   /* Read next token */
    
        /* STATEMENT */
        if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    }

    /* "end" */
    if ( token != TEND ) return (error_syntax("Keyword 'end' is not found at the end of compound statement", ERR_PAT_TOKEN));
    printf("\n");
    cnt_tab--;
    print_tab();
    printf("%s", string_attr);
    next_token();   /* Read next token */
    
    return NORMAL;
}

/* STATEMENT ::= ASSIGNMENT | CONDITION | ITERATION | BREAK | CALL | RETURN | INPUT | OUTPUT | COMPOUND | EMPTY _STATEMENT */
static int statement ( void ) {
    if ( token == TNAME ) {
        /* ASSIGNMENT_STATEMENT */
        if ( assign_state() == ERROR ) return (error_syntax("in assign statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TIF ) {
        /* CONDITION_STATEMENT */
        if ( cond_state() == ERROR ) return (error_syntax("in condition statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TWHILE ) {
        /* ITERATION_STATEMENT */
        if ( iter_state() == ERROR ) return (error_syntax("in iteration statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TBREAK ) {
        /* EXIT_STATEMENT */
        if ( exit_state() == ERROR ) return (error_syntax("in exit statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TCALL ) {
        /* CALL_STATEMENT */
        if ( call_state() == ERROR ) return (error_syntax("in call statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TRETURN ) {
        /* RETURN STATEMENT */
        if ( return_state() == ERROR ) return (error_syntax("in return statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TREAD || token == TREADLN ) {
        /* INPUT_STATEMENT */
        if ( input_state() == ERROR ) return (error_syntax("in input statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TWRITE || token == TWRITELN ) { 
        /* OUTPUT_STATEMENT */
        if ( output_state() == ERROR ) return (error_syntax("in output statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TBEGIN ) {
        /* COMPOUND_STATEMENT */
        if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));
    }
    else if ( token == 0 ) {
        /* EMPTY_STATEMENT */
        if ( empty_state() == ERROR ) return (error_syntax("in empty statement", ERR_PAT_SYNTAX));
    }
    return NORMAL;
}

/* CONDITION_STATEMENT ::= "if" EXPRESSION "then" STATEMENT [ "else" STATEMENT ] */
static int cond_state ( void ) {
    /* "if" */
    if ( token != TIF ) return (error_syntax("Keyword  'if' is not found at the first of condition statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* EXPRESSION */
    if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
    
    /* "then" */
    if ( token != TTHEN ) return (error_syntax("Keyword 'then' is not found after expression", ERR_PAT_TOKEN));
    printf(" %s\n", string_attr);
    cnt_tab++;
    next_token();   /* Read next token */

    /* STATEMENT */
    if ( statement()== ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));

    /* [ "else" STATEMENT ] */
    if ( token == TELSE ) {
        /* "else" already checked */
        printf("\n");
        cnt_tab--;
        print_tab();
        printf("%s\n", string_attr);
        next_token();   /* Read next token */
        cnt_tab++;

        /* STATEMENT */
        if ( statement()== ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    }
    cnt_tab--;
    return NORMAL;
}

/* ITERATION_STATEMENT ::= "while" EXPRESSION "do" STATEMENT */
static int iter_state ( void ) {
    /* "while" */
    if ( token != TWHILE ) return (error_syntax("Keyword 'while' is not found at the first of iteration statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* EXPRESSION */
    if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

    /* "do" */
    if ( token != TDO ) return (error_syntax("Keyword 'do' is not found at the expression", ERR_PAT_TOKEN));
    printf(" %s\n", string_attr);
    cnt_tab++;
    next_token();   /* Read next token */

    /* Iteration counter increment */
    cnt_iter++;

    /* STATEMENT */
    if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));

    /* Iteration counter decrement */
    cnt_iter--;

    cnt_tab--;

    return NORMAL;
}

/* EXIT_STATE ::= "break" */
static int exit_state ( void ) {
    /* "break" */
    if ( token != TBREAK ) return (error_syntax("Keyword 'break' is not found at the exit statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s", string_attr);
    next_token();   /* Read next token */

    if ( cnt_iter < 1) return (error_syntax("'break' not exit in iteration", ERR_PAT_TOKEN));

    return NORMAL;
}

/* CALL_STATEMENT ::= "call" PROCEDURE_NAME [ "(" EXPRESSIONS ")" ] */
static int call_state ( void ) {
    /* "call" */
    if ( token != TCALL ) return (error_syntax("Keyword 'call' is not found at the first of call statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s ", string_attr);
    next_token();   /* Read next token */

    /* PROCEDURE_NAME */
    if ( procedure_name() == ERROR ) return (error_syntax("in procedure name", ERR_PAT_SYNTAX));

    /* [ "(" EXPRESSIONS ")" ] */
    if ( token == TLPAREN ) {
        /* "(" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* EXPRESSIONS */
        if ( expressions() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

        /* ")" */
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found after expression", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    return NORMAL;
}

/* EXPRESSIONS ::= EXPRESSION { "," EXPRESSION } */
static int expressions ( void ) {
    /* EXPRESSION */
    if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

    /* { "," EXPRESSION } */
    while ( token == TCOMMA ) {
        /* "," already checked */
        printf("%s ", string_attr);
        next_token();   /* Read next token */

        /* EXPRESSION */
        if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
    }
    return NORMAL;
}

/* RETURN_STATEMENT ::= "return" */
static int return_state ( void ) {
    /* "return" */
    if ( token != TRETURN ) return (error_syntax("Keyword 'return' is not found at return statement", ERR_PAT_TOKEN));
    print_tab();
    printf("%s", string_attr);
    next_token();   /* Read next token */

    return NORMAL;
}

/* ASSIGNMENT ::= LEFT_PART ":=" EXPRESSION */
static int assign_state ( void ) {
    print_tab();
    /* LEFT_PART */
    if ( left_part() == ERROR ) return (error_syntax("in left part", ERR_PAT_SYNTAX));

    /* ":=" */
    if ( token != TASSIGN ) return (error_syntax("Assign ':=' is not found after left part", ERR_PAT_TOKEN));
    printf(" %s ", string_attr);
    next_token();   /* Read next token */

    /* EXPRESSION */
    if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
    
    return NORMAL;
}

/* LEFT_PART ::= VARIABLE */
static int left_part ( void ) {
    /* VARIABLE */
    if ( variable() == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));

    return NORMAL;
}

/* VARIABLE ::= VARIABLE_NAME [ "[" EXPRESSION "]" ] */
static int variable ( void ) {
    /* VARIABLE_NAME */
    if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));

    /* [ "[" EXPRESSION "]" ] */
    if ( token == TLSQPAREN ) {
        /* "[" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* EXPRESSION */
        if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

        /* "]" */
        if ( token != TRSQPAREN ) return (error_syntax("Right square paren ']' is not found after expression", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    return NORMAL;
}

/* EXPRESSION ::= SIMPLE_EXPRESSION { RELATION_OPERATOR SIMPLE_EXPRESSION } */
static int expression ( void ) {
    /* SIMPLE_EXPRESSION */
    if ( simple_expression() == ERROR ) return (error_syntax("in simple expression", ERR_PAT_SYNTAX));

    /* { RELATION_OPERATOR SIMPLE_EXPRESSION } */
    while ( token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ ) {
        /* RELATION_OPERATOR */
        if ( relat_ope() == ERROR ) return (error_syntax("in relation operator", ERR_PAT_SYNTAX));

        /* SIMPLE_EXPRESSION */
        if ( simple_expression() == ERROR ) return (error_syntax("in simple expression", ERR_PAT_SYNTAX));
    }
    return NORMAL;
}

/* SIMPLE_EXPRESSION ::= [ "+" | "-" ] TERM { ADDITIVE_OPERATOR TERM }*/
static int simple_expression ( void ) {
    /* [ "+" | "-" ] */
    if ( token == TPLUS || token == TMINUS ) {
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }

    /* TERM */
    if ( term() == ERROR ) return (error_syntax("in term", ERR_PAT_SYNTAX));
    
    /* { ADDITIVE_OPERATOR TERM } */
    while ( token == TPLUS || token == TMINUS || token == TOR ) {
        /* ADDITIVE_OPERATOR */
        if ( add_ope() == ERROR ) return (error_syntax("in additive operator", ERR_PAT_SYNTAX));

        /* TERM */
        if ( term() == ERROR ) return (error_syntax("in term", ERR_PAT_SYNTAX));
    }

    return NORMAL;
}

/* TERM ::= FACTOR { MULTIPLICATIVE_OPERATOR FACTOR } */
static int term ( void ) {
    /* FACTOR */
    if ( factor() == ERROR ) return (error_syntax("in factor", ERR_PAT_SYNTAX));

    /* { MULTIPLICATIVE_OPERATOR FACTOR } */
    while ( token == TSTAR || token == TDIV || token == TAND ) {
        /* MULTIPLICATIVE_OPERATOR */
        if ( multiply_ope() == ERROR ) return (error_syntax("in multiply operator", ERR_PAT_SYNTAX));

        /* FACTOR */
        if ( factor() == ERROR ) return (error_syntax("in factor", ERR_PAT_SYNTAX));
    }
    return NORMAL;
}

/* FACTOR ::= VARIABLE | CONSTANT | "(" EXPRESSION ")" | "not" FACTOR | STANDARD_TYPE "(" EXPRESSION ")" */
static int factor ( void ) {
    if ( token == TNAME ) {
        /* VARIABLE */
        if ( variable() == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));
    }
    else if ( token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING ) {
        /* CONSTANT */
        if ( constant() == ERROR ) return (error_syntax("in constant", ERR_PAT_SYNTAX));
    }
    else if ( token == TLPAREN ) {
        /* "(" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */
        
        /* EXPRESSION */
        if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
        
        /* ")" */
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of factor", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    else if ( token == TNOT ) {
        /* "not" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* FACTOR */
        if ( factor() == ERROR ) return (error_syntax("in factor", ERR_PAT_SYNTAX));
    }
    else if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR ) {
        /* STANDARD_TYPE */
        if ( standard_type() == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));

        /* "(" */
        if ( token != TLPAREN ) return (error_syntax("Left paren '(' is not found at the first of factor", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* EXPRESSION */
        if ( expression() == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

        /* ")" */
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of factor", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }

    return NORMAL;
}

/* CONSTANT ::= "UNSIGNED_INTEGER" | "false" | "true" | "STRING" */
static int constant ( void ) {
    /* "UNSIGNED_INTEGER" | "false" | "true" | "STRING" */
    if ( token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING ) {
        if (token == TSTRING) {
            printf("'%s'", string_attr);
        }
        else {
            printf("%s", string_attr);
        }
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Unsigned integer , 'false' , 'true' , string token is not found at the constant", ERR_PAT_TOKEN));
    }

    return NORMAL;
}

/* MULTIPLICATIVE_OPERATOR ::= "*" | "div" | "and" */
static int multiply_ope ( void ) {
    /* "*" | "div" | "and" */
    if ( token == TSTAR || token == TDIV || token == TAND ) {
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Star '*' , 'div' , 'and' is not found at the multiplicative operator", ERR_PAT_TOKEN));
    }
    return NORMAL;
}

/* ADDITIVE_OPERATOR ::= "+" | "-" | "or" */
static int add_ope ( void ) {
    /* "+" | "-" | "or" */
    if ( token == TPLUS || token == TMINUS || token == TOR ) {
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Plus '+' , minus '-' , 'or' is not found at the additive operator", ERR_PAT_TOKEN));
    }

    return NORMAL;
}

/* RELATIONAL_OPERATOR ::= "=" | "<>" | "<" | "<=" | ">" | ">=" */
static int relat_ope ( void ) {
    /* "=" | "<>" | "<" | "<=" | ">" | ">=" */
    if ( token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ ) {
        printf(" %s ", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Equal '=', noteq <>, le <, leeq <=, gr >, greq >= is not found at the relation operator", ERR_PAT_TOKEN));
    }
    return NORMAL;
}

/* INPUT_STATEMENT ::= ( "read" | "readln" ) [ "(" VARIABLE { "," VARIABLE } ")" ] */
static int input_state ( void ) {
    /* ( "read" | "readlen" ) */
    if ( token == TREAD || token == TREADLN ) {
        print_tab();
        printf("%s ", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Keyword 'read' , 'readln' is not found at the first of input statement", ERR_PAT_TOKEN));
    }

    /* [ "(" VARIABLE { "," VARIABLE } ")" ] */
    if ( token == TLPAREN ) {
        /* "(" already check */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* VARIABLE */
        if ( variable() == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));

        /* { "," VARIABLE } */
        while ( token == TCOMMA ) {
            /* "," already checked */
            printf("%s ", string_attr);
            next_token();   /* Read next token */

            /* VARIABLE */
            if ( variable() == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));
        }

        /* ")" */
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of input statement", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }
    return NORMAL;
}

/* OUTPUT_STATEMENT ::= ( "write" | "writeln" ) [ "(" OUTPUT_FORMAT { "," OUTPUT_FORMAT } ")" ] */
static int output_state ( void ) {
    /* ( "write" | "writelen" ) */
    if ( token == TWRITE || token == TWRITELN ) {
        print_tab();
        printf("%s ", string_attr);
        next_token();   /* Read next token */
    }
    else {
        return (error_syntax("Keyword 'write' , 'writeln' is not found at the first of output statement", ERR_PAT_TOKEN));
    }

    /* [ "(" OUTPUT_FORMAT { "," OUTPUT_FORMAT } ")" ] */
    if ( token == TLPAREN ) {
        /* "(" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* OUTPUT_FORMAT */
        if ( output_format() == ERROR ) return (error_syntax("in output format", ERR_PAT_SYNTAX));

        /* { "," OUTPUT_FORMAT } */
        while ( token == TCOMMA ) {
            /* "," already checked */
            printf("%s ", string_attr);
            next_token();   /* Read next token */
            
            /* OUTPUT_FORMAT */
            if ( output_format() == ERROR ) return (error_syntax("in output format", ERR_PAT_SYNTAX));
        }

        /* ")" */
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of output statement", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    }

    return NORMAL;
}

/* OUTPUT_FORMAT ::= EXPRESSION [ ":" "UNSIGNED_INTEGER" ] | "STRING" */
static int output_format ( void ) {
    /* "STRING" */
    if ( token == TSTRING ) {
        /* "STRING" already checked */
        printf("'%s'", string_attr);
        next_token();   /* Read next token */
        return NORMAL;
    }
    /* EXPRESSION [ ":" "UNSIGNED_INTEGER" ] */
    else if ( expression() == ERROR ) {
        return (error_syntax("in expression", ERR_PAT_SYNTAX));
    }
    /* [ ":" UNSIGNGED_INTEGER ] */
    else if ( token == TCOLON ) {
        /* ":" already checked */
        printf("%s", string_attr);
        next_token();   /* Read next token */

        /* "UNSIGNED_INTEGER" */
        if ( token != TNUMBER ) return (error_syntax("Unsigned integer is not found at the end of output format", ERR_PAT_TOKEN));
        printf("%s", string_attr);
        next_token();   /* Read next token */
    } 

    return NORMAL;
}

/* EMPTY_STATEMENT ::=  */
static int empty_state ( void ) {
    return NORMAL;
}
