#include "compiler.h"

/* ---------- Global Variable --------------------------------- */
int token;      /* current token */
char string_attr[MAX_STR_SIZE]; /* string or name */
int num_attr;
int cnt_iter;   /* iteration statement nest counter */

#if FLAG_PP
int cnt_tab;    /* tab counter */
#endif

#if FLAG_CR

/* id info */
char *var_name_str;
char *procedure_name_str;
int  *is_formal_parameters;
/* type info */
int size_array;

/* flag */
int flag_in_subprogram;
int flag_in_formal_parameters;
int flag_in_variable_declaration;
int flag_procedure_name_in_subprogram_declaration;
int flag_in_call_statement;

/* id */
ID *id_procedure_name;

#endif

/* ---------- Function Prototype Declaration ------------------ */
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
static int error_syntax ( char * mes , int pattern) ;   /* Out Error Message */
static int error_semantic ( char * mes ) ;

#if FLAG_PP
void print_tab ( void ) ;   /* Print Tab */
#endif

/* Main parse */
int parse ( void ) {
#if FLAG_DEBUG
    printf("parse\n");
#endif
    /* initialize */
    cnt_iter = 0;
    #if FLAG_PP    
    cnt_tab = 0;
    #endif

#if FLAG_CR
    flag_in_subprogram = FALSE;
    flag_in_formal_parameters = FALSE;
    is_formal_parameters = &flag_in_formal_parameters;
    flag_in_variable_declaration = FALSE;
    flag_procedure_name_in_subprogram_declaration = FALSE;

    size_array = 0;
    id_procedure_name = NULL;
    init_id_root();
#endif

    next_token();
    int res = program();

#if FLAG_CR
    if ( res == NORMAL ) print_crossreference();
#endif

    return res;
}

/* Read Next Token */
void next_token ( void ) {
    token = scan();
}

#if FLAG_PP
/* Print Tab */
void print_tab ( void ) {
    int i;
    for (i = 0; i < cnt_tab; i++) {
        printf("\t");
    }
}
#endif

/* Error Message */
int error_syntax ( char * mes , int pattern ) {
    if (pattern == ERR_PAT_TOKEN) {
        printf("\n----- ERROR SYNTAX: line=%d -----\n| MES: %s\n", get_linenum(), mes);
        end_scan();
    }
    else {
        printf("| %s\n", mes);
    }
    return (ERROR);
}

int error_semantic ( char * mes ) {
    printf("\n----- ERROR SEMANTIC: line=%d -----\n| MES: %s\n", get_linenum(), mes);
    return (ERROR);
}

/* -------------------------------------------------------------------------------------- */
/* ----------------------------- Syntax Function Definition ----------------------------- */
/* -------------------------------------------------------------------------------------- */

/* program ::= "program" "NAME" ";" BLOCK "." */
static int program ( void ) {
#if FLAG_DEBUG
    printf("program\n");
#endif
    if ( token != TPROGRAM ) return (error_syntax("Keyword 'program' is not found at the first of program", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s ", string_attr);
    #endif
    next_token();

    if ( token != TNAME ) return (error_syntax("Program 'NAME' is not found after keyword 'program'", ERR_PAT_TOKEN));   /* check "NAME" */
    #if FLAG_PP    
    printf("%s", string_attr);
    #endif
    next_token();
    
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found after program name", ERR_PAT_TOKEN));      /* check ";" */
    #if FLAG_PP    
    printf("%s\n", string_attr);
    #endif
    next_token();
    
    if ( block() == ERROR ) return (error_syntax("in block", ERR_PAT_SYNTAX));
    
    if ( token != TDOT ) return (error_syntax("Period '.' is not found at the end of program", ERR_PAT_TOKEN));    /* check "." */
    #if FLAG_PP    
    printf("%s\n", string_attr);
    #endif
    next_token();
    
    return (NORMAL);
}

/* BLOCK ::= { VARIABLE_DECLARATION | SUBPROGRAM_DECLARATION } COMPOUND_STATEMENT */
static int block ( void ) {
#if FLAG_DEBUG
    printf("block\n");
#endif
    while ( token == TVAR || token == TPROCEDURE ) {
        if ( token == TVAR ) {
            if (var_decl() == ERROR) return (error_syntax("in variable declaration", ERR_PAT_SYNTAX));
        }
        else if ( token == TPROCEDURE ) {
            if (subprogram_decl() == ERROR ) return (error_syntax("in subprogram declaration", ERR_PAT_SYNTAX));
        }
    }

    #if FLAG_PP    
    cnt_tab = 0;
    #endif

#if FLAG_DEBUG
    printf("global");
    print_id_table(global_id_root);
#endif
    if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));

    return (NORMAL);
}

static int var_decl ( void ) {
#if FLAG_CR
#if FLAG_DEBUG
    printf("variable declaration\n");
#endif
    flag_in_variable_declaration = TRUE;
#endif
    int res_type;
    if ( token != TVAR ) return (error_syntax("Keyword 'var' is not found at the first of variable declaration", ERR_PAT_TOKEN));
    #if FLAG_PP    
    cnt_tab++;
    print_tab();
    printf("%s ", string_attr);
    #endif
    next_token();

    if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));

    if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
    #if FLAG_PP    
    printf(" %s ", string_attr);
    #endif
    next_token();

    if ( ( res_type = type() ) == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
#if FLAG_CR
    assign_type(res_type);
#endif
    
    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of variable declaration", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s\n", string_attr);
    #endif
    next_token();
    
    while ( token == TNAME ) {
        #if FLAG_PP
        print_tab();
        printf("\t");
        #endif
        if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));

        if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf(" %s ", string_attr);
        #endif
        next_token();

        res_type = type();
        if ( res_type == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
#if FLAG_CR
        assign_type(res_type);
#endif

        if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of variable declaration", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s\n", string_attr);
        #endif
        next_token();
    }
    #if FLAG_PP
    cnt_tab--;
    #endif
#if FLAG_CR
    flag_in_variable_declaration = FALSE;
#endif

    return (NORMAL);
}

/* VARIABLE_NAMES ::= VARIABLE_NAME { "," VARIABLE_NAME } */
static int var_names ( void ) {
#if FLAG_DEBUG
    printf("variable names\n");
#endif
    if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));
#if FLAG_CR
    if ( flag_in_variable_declaration || flag_in_formal_parameters ) {  /* define */
        add_undefined_type_id(string_attr);
    }
    else {  /* refer */
        if ( add_ref_linenum(string_attr) == ERROR ) return (error_semantic("This variable name does not be defined"));
    }
#endif

    next_token();

    while ( token == TCOMMA ) {
        #if FLAG_PP
        printf("%s ", string_attr);
        #endif
        next_token();
        if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( flag_in_variable_declaration || flag_in_formal_parameters ) {  /* define */
            add_undefined_type_id(string_attr);
        }
        else {  /* refer */
            if ( add_ref_linenum(string_attr) == ERROR ) return (error_semantic("This variable name does not be defined"));
        }
#endif
        next_token();
    }

    return (NORMAL);
}

/* VARIABLE_NAME ::= "NAME" */
static int var_name ( void ) {
#if FLAG_DEBUG
    printf("variable name\n");
#endif
    if ( token != TNAME ) return (error_syntax("Variable 'NAME' is not found at the variable name", ERR_PAT_TOKEN));

    #if FLAG_PP
    printf("%s", string_attr);
    #endif

    return (NORMAL);
}

/* TYPE ::= STANDARD_TYPE | ARRAY_TYPE */
static int type ( void ) {
#if FLAG_DEBUG
    printf("type\n");
#endif
    int res_type;
    if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR ) {
        res_type = standard_type();
        if ( res_type == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));
    }
    else if ( token == TARRAY ) {
        res_type = array_type();
        if ( res_type == ERROR ) return (error_syntax("in array type", ERR_PAT_SYNTAX));
    }
    else {
        return (error_syntax("standard type , array type is not found in type", ERR_PAT_TOKEN));
    }

    return res_type;
}

/* STANDARD_TYPE ::= "integer" | "boolean" | "char" */
static int standard_type ( void ) {
#if FLAG_DEBUG
    printf("standard type\n");
#endif
    int res_type;
    if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        switch ( token )
        {
        case TINTEGER :
            res_type = TYPE_INT ;    break;
        case TBOOLEAN : 
            res_type = TYPE_BOOL;    break;
        case TCHAR :
            res_type = TYPE_CHAR;    break;
        default:
            res_type = TYPE_NONE;    break;
        }
        next_token();
        return res_type;
    }
    else {
        return (error_syntax("Keyword neither 'integer' , 'boolean' , 'char' is not found", ERR_PAT_TOKEN));
    }
}

/* ARRAY_TYPE ::= "array" "[" "UNSIGNED_INTEGER" "]" "of" STANDARD_TYPE */
static int array_type ( void ) {
#if FLAG_DEBUG
    printf("array type\n");
#endif
    if ( token != TARRAY ) return (error_syntax("Keyword 'array' is not found at the first of array type", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s ", string_attr);
    #endif
    next_token();

    if ( token != TLSQPAREN ) return (error_syntax("Left square paren '[' is not found after keyword 'array'", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif
    next_token();

    if ( token != TNUMBER ) return (error_syntax("Unsigned integer is not found after left square paren '['", ERR_PAT_TOKEN));

    #if FLAG_PP
    printf("%s", string_attr);
    #endif

#if FLAG_CR
    if ( flag_in_variable_declaration ) {
        size_array = num_attr;
        if ( size_array < 1) return (error_semantic("Array size must be defined 1 or more"));
    }
    else {
        if ( num_attr < 0 ) return (error_semantic("Array index must be 0 or more"));
    }
#endif

    next_token();

    if ( token != TRSQPAREN ) return (error_syntax("Right square paren ']' is not found after unsigned integer", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif
    next_token();

    if ( token != TOF ) return (error_syntax("Keyword 'of' is not found after right square paren ']'", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf(" %s", string_attr);
    #endif
    next_token();

    int res_type = standard_type();
    if ( res_type == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));
    
    return ( res_type== TINTEGER ) ? TYPE_ARRAY_INT : ( res_type == TBOOLEAN ) ? TYPE_ARRAY_BOOL : TYPE_ARRAY_CHAR ;
}

/* SUBPROGRAM_DECLARATION ::= "procedure" PROCEDURE_NAME [ FORMAL_PARAMETERS ] ";" [ VARIABLE_DECLARATION ] COMPOUND_STATEMENT ";" */
static int subprogram_decl ( void ) {
#if FLAG_DEBUG
    printf("subprogram declaration\n");
#endif
    flag_in_subprogram = TRUE;
    if ( token != TPROCEDURE ) return (error_syntax("Keyword 'procedure' is not found at the first of subprogram declaration", ERR_PAT_TOKEN));

    #if FLAG_PP
    cnt_tab++;
    print_tab();
    printf("%s ", string_attr);
    #endif

    next_token();

    flag_procedure_name_in_subprogram_declaration = TRUE;
    if ( procedure_name() == ERROR ) return (error_syntax("in procedure name", ERR_PAT_SYNTAX));
    flag_procedure_name_in_subprogram_declaration = FALSE;

    if ( token == TLPAREN ) {
        flag_in_formal_parameters = TRUE;
        if ( formal_params() == ERROR ) return (error_syntax("in formal parameters", ERR_PAT_SYNTAX));
        flag_in_formal_parameters = FALSE;
    } 

    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found after procedure name or formal paramters", ERR_PAT_TOKEN));
    
    #if FLAG_PP
    printf("%s\n", string_attr);
    #endif
    
    next_token(); 

    if ( token == TVAR ) { 
        if ( var_decl() == ERROR ) return (error_syntax("in variable declaration", ERR_PAT_SYNTAX));
    }

    if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));

    if ( token != TSEMI ) return (error_syntax("Semicolon ';' is not found at the end of subprogram declaration", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s\n", string_attr);
    cnt_tab--;
    #endif
    next_token();

    flag_in_subprogram = FALSE;

    free_procedure_name();
#if FLAG_DEBUG
    printf("procname:%s in free\n", procedure_name_str);
#endif

    return (NORMAL);
}

/* PROCEDURE_NAME ::= "NAME" */
static int procedure_name ( void ) {
#if FLAG_DEBUG
    printf("procedure name\n");
#endif

    if ( token != TNAME ) return (error_syntax("Procedure 'NAME' is not found at the procedure name", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif

#if FLAG_CR
    if ( flag_procedure_name_in_subprogram_declaration ) {
        add_undefined_type_id(string_attr);
        assign_type(TYPE_PROCEDURE);
        set_procedure_name(string_attr);
    }
    else {
        if ( add_ref_linenum(string_attr) == ERROR ) return (error_semantic("This procedure name does not be defined"));
        id_procedure_name = search_procedure_id(string_attr);
    }
#endif

    next_token();

    return (NORMAL);
}

/* FORMAL_PARAMS ::= "(" VARIABLE_NAMES ":" TYPE { ";" VARIABLE_NAMES ":" TYPE } ")"  */
static int formal_params ( void ) {
#if FLAG_DEBUG
    printf("formal parameters\n");
#endif

    if ( token != TLPAREN ) return (error_syntax("Left paren '(' is not found at the first formal parameters", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif
    next_token();

    if ( var_names() == ERROR) return (error_syntax("in variable names", ERR_PAT_SYNTAX));
    
    if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif
    next_token();
    
    int type_in_formal_param = TYPE_NONE;
    if ( ( type_in_formal_param = type() ) == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
#if FLAG_CR
    if ( is_array_type(type_in_formal_param) ) return (error_semantic("formal parametes types must not be array when formal parameters"));
    assign_type(type_in_formal_param);
#endif

    while ( token == TSEMI ) {
        #if FLAG_PP
        printf("%s ", string_attr);
        #endif
        next_token();

        if ( var_names() == ERROR ) return (error_syntax("in variable names", ERR_PAT_SYNTAX));
        
        if ( token != TCOLON ) return (error_syntax("Colon ':' is not found after variable names", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
        
        if ( ( type_in_formal_param = type() ) == ERROR ) return (error_syntax("in type", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( is_array_type(type_in_formal_param) ) return (error_semantic("formal parametes types must not be array when formal parameters"));
        assign_type(type_in_formal_param);
#endif
    
    }
    
    if ( token != TRPAREN ) return (error_syntax("Right Paren ')' is not found at the end of fromal parameters", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("%s", string_attr);
    #endif
    next_token();

    return (NORMAL);
}

/* COMPOUND_STATEMENT ::= "begin" STATEMENT { ";" STATEMENT } "end" */
static int comp_state ( void ) {
#if FLAG_DEBUG
    printf("compound statement\n");
#endif
    if ( token != TBEGIN ) return (error_syntax("Keyword 'begin' is not found at the first of compound statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s\n", string_attr);
    cnt_tab++;
    #endif
    next_token();

    if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    
    while ( token == TSEMI ) {
        #if FLAG_PP
        printf("%s\n", string_attr);
        #endif
        next_token();

        if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    }

    if ( token != TEND ) return (error_syntax("Keyword 'end' is not found at the end of compound statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf("\n");
    cnt_tab--;
    print_tab();
    printf("%s", string_attr);
    #endif
    next_token();
    
    return (NORMAL);
}

/* STATEMENT ::= ASSIGNMENT | CONDITION | ITERATION | BREAK | CALL | RETURN | INPUT | OUTPUT | COMPOUND | EMPTY _STATEMENT */
static int statement ( void ) {
#if FLAG_DEBUG
    printf("statement\n");
#endif
    if ( token == TNAME ) {
        if ( assign_state() == ERROR ) return (error_syntax("in assign statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TIF ) {
        if ( cond_state() == ERROR ) return (error_syntax("in condition statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TWHILE ) {
        if ( iter_state() == ERROR ) return (error_syntax("in iteration statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TBREAK ) {
        if ( exit_state() == ERROR ) return (error_syntax("in exit statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TCALL ) {
        if ( call_state() == ERROR ) return (error_syntax("in call statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TRETURN ) {
        if ( return_state() == ERROR ) return (error_syntax("in return statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TREAD || token == TREADLN ) {
        if ( input_state() == ERROR ) return (error_syntax("in input statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TWRITE || token == TWRITELN ) { 
        if ( output_state() == ERROR ) return (error_syntax("in output statement", ERR_PAT_SYNTAX));
    }
    else if ( token == TBEGIN ) {
        if ( comp_state() == ERROR ) return (error_syntax("in compound statement", ERR_PAT_SYNTAX));
    }
    else if ( token == 0 ) {
        if ( empty_state() == ERROR ) return (error_syntax("in empty statement", ERR_PAT_SYNTAX));
    }
    return (NORMAL);
}

/* CONDITION_STATEMENT ::= "if" EXPRESSION "then" STATEMENT [ "else" STATEMENT ] */
static int cond_state ( void ) {
#if FLAG_DEBUG
    printf("condition statement\n");
#endif
    if ( token != TIF ) return (error_syntax("Keyword 'if' is not found at the first of condition statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s ", string_attr);
    #endif
    next_token();

    int expression_type = TYPE_NONE;
    if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

#if FLAG_CR
    if ( expression_type != TYPE_BOOL ) return (error_semantic("Expression type in condition statement must be bool"));
#endif    

    if ( token != TTHEN ) return (error_syntax("Keyword 'then' is not found after expression", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf(" %s\n", string_attr);
    cnt_tab++;
    #endif
    next_token();

    if ( statement()== ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));

    if ( token == TELSE ) {
        #if FLAG_PP
        printf("\n");
        cnt_tab--;
        print_tab();
        printf("%s\n", string_attr);
        #endif
        next_token();
        
        #if FLAG_PP
        cnt_tab++;
        #endif

        if ( statement()== ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));
    }
    #if FLAG_PP
    cnt_tab--;
    #endif
    return (NORMAL);
}

/* ITERATION_STATEMENT ::= "while" EXPRESSION "do" STATEMENT */
static int iter_state ( void ) {
#if FLAG_DEBUG
    printf("iteration statement\n");
#endif
    if ( token != TWHILE ) return (error_syntax("Keyword 'while' is not found at the first of iteration statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s ", string_attr);
    #endif
    next_token();

    int expression_type = TYPE_NONE;
    if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
#if FLAG_CR
    if ( expression_type != TYPE_BOOL ) return (error_semantic("Expression type in iteration statement must be bool"));
#endif

    if ( token != TDO ) return (error_syntax("Keyword 'do' is not found at the expression", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf(" %s\n", string_attr);
    cnt_tab++;
    #endif
    next_token();

    cnt_iter++; /* Iteration counter increment */

    if ( statement() == ERROR ) return (error_syntax("in statement", ERR_PAT_SYNTAX));

    cnt_iter--; /* Iteration counter decrement */

    #if FLAG_PP
    cnt_tab--;
    #endif

    return (NORMAL);
}

/* EXIT_STATE ::= "break" */
static int exit_state ( void ) {
#if FLAG_DEBUG
    printf("eixt statement\n");
#endif
    if ( token != TBREAK ) return (error_syntax("Keyword 'break' is not found at the exit statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s", string_attr);
    #endif
    next_token();

    if ( cnt_iter < 1) return (error_syntax("'break' not exit in iteration", ERR_PAT_TOKEN));

    return (NORMAL);
}

/* CALL_STATEMENT ::= "call" PROCEDURE_NAME [ "(" EXPRESSIONS ")" ] */
static int call_state ( void ) {
#if FLAG_DEBUG
    printf("call statement\n");
#endif
#if FLAG_CR
    flag_in_call_statement = TRUE;
#endif

    if ( token != TCALL ) return (error_syntax("Keyword 'call' is not found at the first of call statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s ", string_attr);
    #endif
    next_token();

    if ( procedure_name() == ERROR ) return (error_syntax("in procedure name", ERR_PAT_SYNTAX));

    if ( token == TLPAREN ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( expressions() == ERROR ) return (error_syntax("in expressions", ERR_PAT_SYNTAX));

        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found after expression", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }
    else {
        if( id_procedure_name->id_type->next_param_type != NULL ) return(error_semantic("Incorrect number of arguments"));
    }
    flag_in_call_statement = FALSE;

    return (NORMAL);
}

/* EXPRESSIONS ::= EXPRESSION { "," EXPRESSION } */
static int expressions ( void ) {
#if FLAG_DEBUG
    printf("expressions\n");
#endif
#if FLAG_CR
    int expression_type = TYPE_NONE;
    int num_expressions = 0;
    Type *param_type = id_procedure_name->id_type->next_param_type;

    // printf("param type:%d\n", param_type->type);
#endif

    if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

#if FLAG_CR
    num_expressions++;
    if ( flag_in_call_statement ) {
        if ( param_type == NULL ) return (error_semantic("This procedure takes no argments"));
        if ( param_type->type != expression_type) return(error_semantic("Type mismatch."));
    }
    
#endif

    while ( token == TCOMMA ) {
        #if FLAG_PP
        printf("%s ", string_attr);
        #endif
        next_token();

        if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
#if FLAG_CR
        num_expressions++;
        if ( flag_in_call_statement ) {
            param_type = param_type->next_param_type;
            if ( param_type == NULL ) return (error_semantic("This procedure takes no argments"));
            if ( param_type->type != expression_type) return(error_semantic("Type mismatch."));
        } 
#endif
    }

#if FLAG_CR
    if( flag_in_call_statement ) {
        // printf("param type:%d\n", param_type->next_param_type->type);
        if( param_type->next_param_type != NULL ) return (error_semantic("Insufficient arguments when calling the procedure name"));
    }
#endif
    return (NORMAL);
}

/* RETURN_STATEMENT ::= "return" */
static int return_state ( void ) {
#if FLAG_DEBUG
    printf("return statement\n");
#endif
    if ( token != TRETURN ) return (error_syntax("Keyword 'return' is not found at return statement", ERR_PAT_TOKEN));
    #if FLAG_PP
    print_tab();
    printf("%s", string_attr);
    #endif
    next_token();

    return (NORMAL);
}

/* ASSIGNMENT ::= LEFT_PART ":=" EXPRESSION */
static int assign_state ( void ) {
#if FLAG_DEBUG
    printf("assignment\n");
#endif
    #if FLAG_PP
    print_tab();
    #endif
    int left_part_type = TYPE_NONE;
    int expression_type = TYPE_NONE;

    if ( ( left_part_type = left_part() ) == ERROR ) return (error_syntax("in left part", ERR_PAT_SYNTAX));

    if ( token != TASSIGN ) return (error_syntax("Assign ':=' is not found after left part", ERR_PAT_TOKEN));
    #if FLAG_PP
    printf(" %s ", string_attr);
    #endif
    next_token();

    if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

#if FLAG_CR
#if FLAG_DEBUG
    printf("left:%s, exp:%s\n", get_type_str(left_part_type), get_type_str(expression_type));
#endif

    if ( left_part_type != expression_type ) return (error_semantic("leftpart type and expression type must be same type"));
    if ( !(TYPE_INT <= left_part_type && left_part_type <= TYPE_BOOL) ) return (error_semantic("assignment type must be standard type"));
#endif

    return (NORMAL);
}

/* LEFT_PART ::= VARIABLE */
static int left_part ( void ) {
#if FLAG_DEBUG
    printf("left part\n");
#endif
    int left_part_type = TYPE_NONE;
    if ( ( left_part_type = variable() ) == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));

    return (left_part_type);
}

/* VARIABLE ::= VARIABLE_NAME [ "[" EXPRESSION "]" ] */
static int variable ( void ) {
#if FLAG_DEBUG
    printf("variable\n");
#endif
    int variable_type = TYPE_NONE;
    if ( var_name() == ERROR ) return (error_syntax("in variable name", ERR_PAT_SYNTAX));
#if FLAG_CR
    if ( ( variable_type = add_ref_linenum(string_attr) ) == ERROR ) {
        return (error_semantic("This variable is not defined"));
    }
#endif

    next_token();

    if ( token == TLSQPAREN ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( !is_array_type(variable_type) ) return (error_semantic("Variable type must be array when there is a square paren"));

        int expression_type = TYPE_NONE;
        if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
        if ( expression_type != TYPE_INT ) return (error_semantic("Expression type must be int in square parene when variable type is array"));

        if ( token != TRSQPAREN ) return (error_syntax("Right square paren ']' is not found after expression", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( variable_type == TYPE_ARRAY_INT ) variable_type = TYPE_INT;
        else if ( variable_type == TYPE_ARRAY_BOOL ) variable_type = TYPE_BOOL;
        else if ( variable_type == TYPE_ARRAY_CHAR ) variable_type = TYPE_CHAR;
    }

    return (variable_type);
}

/* EXPRESSION ::= SIMPLE_EXPRESSION { RELATION_OPERATOR SIMPLE_EXPRESSION } */
static int expression ( void ) {
#if FLAG_DEBUG
    printf("expression\n");
#endif
    int simple1_type = TYPE_NONE;
    int simple2_type = TYPE_NONE;

    if ( ( simple1_type = simple_expression() ) == ERROR ) return (error_syntax("in simple expression", ERR_PAT_SYNTAX));

    while ( token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ ) {
        if ( relat_ope() == ERROR ) return (error_syntax("in relation operator", ERR_PAT_SYNTAX));

        if ( ( simple2_type = simple_expression() ) == ERROR ) return (error_syntax("in simple expression", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( simple1_type != simple2_type ) return (error_semantic("Simple expression type must be same standard type when related operator"));
        simple1_type = TYPE_BOOL;
#endif
    }
    return (simple1_type);
}

/* SIMPLE_EXPRESSION ::= [ "+" | "-" ] TERM { ADDITIVE_OPERATOR TERM }*/
static int simple_expression ( void ) {
#if FLAG_DEBUG
    printf("simple expression\n");
#endif
    int term1_type = TYPE_NONE;
    int term2_type = TYPE_NONE;

#if FLAG_CR
    int flag_left_ope = FALSE;
#endif

    if ( token == TPLUS || token == TMINUS ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif

#if FLAG_CR
        flag_left_ope = TRUE;
#endif
        next_token();
    }

    if ( ( term1_type = term() ) == ERROR ) return (error_syntax("in term", ERR_PAT_SYNTAX));

#if FLAG_CR
    if ( flag_left_ope && term1_type != TYPE_INT ) return (error_semantic("The left term type must be integer when there is a '+' or '-' on first in simple expression"));
#endif

    while ( token == TPLUS || token == TMINUS || token == TOR ) {
        int add_ope_token = token;
        if ( add_ope() == ERROR ) return (error_syntax("in additive operator", ERR_PAT_SYNTAX));

#if FLAG_CR
        if ( ( add_ope_token == TPLUS || add_ope_token == TMINUS ) && term1_type != TYPE_INT ) return (error_semantic("Term type must be integer when operator is '+' or '-'"));
        if ( add_ope_token == TOR && term1_type != TYPE_BOOL ) return (error_semantic("Term type must be boolean when operator is 'or'"));
#endif
        if ( ( term2_type = term() ) == ERROR ) return (error_syntax("in term", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( term1_type == TYPE_INT && term2_type != TYPE_INT ) return (error_semantic("Term type must be integer when operator is '+' or '-'"));
        else if ( term1_type == TYPE_BOOL && term2_type != TYPE_BOOL ) return (error_semantic("Term type must be boolean when operator is 'or'"));
#endif
    }

    return (term1_type);
}

/* TERM ::= FACTOR { MULTIPLICATIVE_OPERATOR FACTOR } */
static int term ( void ) {
#if FLAG_DEBUG
    printf("term\n");
#endif
    int factor1_type = TYPE_NONE;
    int factor2_type = TYPE_NONE;

    if ( ( factor1_type = factor() ) == ERROR ) return (error_syntax("in factor", ERR_PAT_SYNTAX));

    while ( token == TSTAR || token == TDIV || token == TAND ) {
        int multiply_ope_token = token;
        if ( multiply_ope() == ERROR ) return (error_syntax("in multiply operator", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( ( multiply_ope_token == TSTAR || multiply_ope_token == TDIV ) && factor1_type != TYPE_INT ) return (error_semantic("Factor type must be integer when operator is '*' or 'div'"));
        if ( multiply_ope_token == TAND && factor1_type != TYPE_BOOL ) return (error_semantic("Factor type must be boolean when operator is 'and'"));
#endif
        if ( ( factor2_type = factor() ) == ERROR ) return (error_syntax("in factor", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( factor1_type == TYPE_INT && factor2_type != TYPE_INT ) return (error_semantic("Factor type must be integer when operator is '*' or 'div'"));
        else if ( factor1_type == TYPE_BOOL && factor2_type != TYPE_BOOL ) return (error_semantic("Factor type must be boolean when operator is 'and'"));  
#endif
    }
    return (factor1_type);
}

/* FACTOR ::= VARIABLE | CONSTANT | "(" EXPRESSION ")" | "not" FACTOR | STANDARD_TYPE "(" EXPRESSION ")" */
static int factor ( void ) {
#if FLAG_DEBUG
    printf("factor\n");
#endif
    int factor_type = TYPE_NONE;
    if ( token == TNAME ) {
        if ( ( factor_type = variable() ) == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));
    }
    else if ( token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING ) {
        if ( ( factor_type = constant() ) == ERROR ) return (error_syntax("in constant", ERR_PAT_SYNTAX));
    }
    else if ( token == TLPAREN ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
        
        if ( ( factor_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));
        
        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of factor", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }
    else if ( token == TNOT ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( ( factor_type = factor() ) == ERROR )  return (error_syntax("in factor", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( factor_type != TYPE_BOOL ) return (error_semantic("Type operator of 'not' must be boolean"));
#endif
    }
    else if ( token == TINTEGER || token == TBOOLEAN || token == TCHAR ) {
        if ( ( factor_type = standard_type() ) == ERROR ) return (error_syntax("in standard type", ERR_PAT_SYNTAX));

        if ( token != TLPAREN ) return (error_syntax("Left paren '(' is not found at the first of factor", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        int expression_type = TYPE_NONE;
        if ( ( expression_type = expression() ) == ERROR ) return (error_syntax("in expression", ERR_PAT_SYNTAX));

#if FLAG_CR
        if ( expression_type != TYPE_INT && expression_type != TYPE_BOOL && expression_type != TYPE_CHAR ) return (error_semantic("Expression type must be standard type"));
#endif

        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of factor", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }

    return (factor_type);
}

/* CONSTANT ::= "UNSIGNED_INTEGER" | "false" | "true" | "STRING" */
static int constant ( void ) {
#if FLAG_DEBUG
    printf("constant\n");
#endif
    int constant_type = TYPE_NONE;
    if ( token == TNUMBER ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
#if FLAG_CR
        constant_type = TYPE_INT;
#endif
    }
    else if ( token == TFALSE || token == TTRUE ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
#if FLAG_CR
        constant_type = TYPE_BOOL;
#endif
    }
    else if ( token == TSTRING ) {
        #if FLAG_PP
        printf("'%s'", string_attr);
        #endif
#if FLAG_CR
        constant_type = TYPE_CHAR;
#endif
        if ( strlen(string_attr) != 1 ) return (error_semantic("Constant string length must be one"));
    }
    else {
        return (error_syntax("Unsigned integer , 'false' , 'true' , string token is not found at the constant", ERR_PAT_TOKEN));
    }

    next_token();
    return (constant_type);
}

/* MULTIPLICATIVE_OPERATOR ::= "*" | "div" | "and" */
static int multiply_ope ( void ) {
#if FLAG_DEBUG
    printf("multiply operator\n");
#endif
    if ( token == TSTAR || token == TDIV || token == TAND ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }
    else {
        return (error_syntax("Star '*' , 'div' , 'and' is not found at the multiplicative operator", ERR_PAT_TOKEN));
    }
    return (NORMAL);
}

/* ADDITIVE_OPERATOR ::= "+" | "-" | "or" */
static int add_ope ( void ) {
#if FLAG_DEBUG
    printf("additive operator\n");
#endif
    if ( token == TPLUS || token == TMINUS || token == TOR ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }
    else {
        return (error_syntax("Plus '+' , minus '-' , 'or' is not found at the additive operator", ERR_PAT_TOKEN));
    }

    return (NORMAL);
}

/* RELATIONAL_OPERATOR ::= "=" | "<>" | "<" | "<=" | ">" | ">=" */
static int relat_ope ( void ) {
#if FLAG_DEBUG
    printf("relational operator\n");
#endif
    if ( token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ ) {
        #if FLAG_PP
        printf(" %s ", string_attr);
        #endif
        next_token();
    }
    else {
        return (error_syntax("Equal '=', noteq <>, le <, leeq <=, gr >, greq >= is not found at the relation operator", ERR_PAT_TOKEN));
    }
    return (NORMAL);
}

/* INPUT_STATEMENT ::= ( "read" | "readln" ) [ "(" VARIABLE { "," VARIABLE } ")" ] */
static int input_state ( void ) {
#if FLAG_DEBUG
    printf("input statement\n");
#endif
    int variable_type = TYPE_NONE;

    if ( token == TREAD || token == TREADLN ) {
        #if FLAG_PP
        print_tab();
        printf("%s ", string_attr);
        #endif
        next_token();
    }
    else {
        return (error_syntax("Keyword 'read' , 'readln' is not found at the first of input statement", ERR_PAT_TOKEN));
    }

    if ( token == TLPAREN ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( ( variable_type = variable() ) == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));
#if FLAG_CR
        if ( variable_type != TYPE_INT && variable_type != TYPE_CHAR ) return (error_semantic("Variable type must be integer or char"));
#endif
        while ( token == TCOMMA ) {
            #if FLAG_PP
            printf("%s ", string_attr);
            #endif
            next_token();
            
            if ( ( variable_type = variable() ) == ERROR ) return (error_syntax("in variable", ERR_PAT_SYNTAX));
#if FLAG_CR
            if ( variable_type != TYPE_INT && variable_type != TYPE_CHAR ) return (error_semantic("Variable type must be integer or char"));
#endif
        }

        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of input statement", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }
    return (NORMAL);
}

/* OUTPUT_STATEMENT ::= ( "write" | "writeln" ) [ "(" OUTPUT_FORMAT { "," OUTPUT_FORMAT } ")" ] */
static int output_state ( void ) {
#if FLAG_DEBUG
    printf("output statement\n");
#endif
    if ( token == TWRITE || token == TWRITELN ) {
        #if FLAG_PP
        print_tab();
        printf("%s ", string_attr);
        #endif
        next_token();
    }
    else {
        return (error_syntax("Keyword 'write' , 'writeln' is not found at the first of output statement", ERR_PAT_TOKEN));
    }

    if ( token == TLPAREN ) {
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();

        if ( output_format() == ERROR ) return (error_syntax("in output format", ERR_PAT_SYNTAX));

        while ( token == TCOMMA ) {
            #if FLAG_PP
            printf("%s ", string_attr);
            #endif
            next_token();

            if ( output_format() == ERROR ) return (error_syntax("in output format", ERR_PAT_SYNTAX));
        }

        if ( token != TRPAREN ) return (error_syntax("Right paren ')' is not found at the end of output statement", ERR_PAT_TOKEN));
        #if FLAG_PP
        printf("%s", string_attr);
        #endif
        next_token();
    }

    return (NORMAL);
}

/* OUTPUT_FORMAT ::= EXPRESSION [ ":" "UNSIGNED_INTEGER" ] | "STRING" */
static int output_format ( void ) {
#if FLAG_DEBUG
    printf("output format\n");
#endif
    if ( token == TSTRING ) {
        #if FLAG_PP
        printf("'%s'", string_attr);
        #endif
#if FLAG_CR        
        if ( strlen(string_attr) < 1 ) return (error_semantic("String length must be 0 or 2 or more"));
#endif
        next_token();
        return (NORMAL);
    }

    int expression_type = expression();
    if ( expression_type == ERROR ) {
        return (error_syntax("in expression", ERR_PAT_SYNTAX));
    }
    else {
#if FLAG_CR
        if ( expression_type == TYPE_ARRAY ) return (error_semantic("Expression type must be standard type"));
#endif
        if ( token == TCOLON ) {
            #if FLAG_PP
            printf("%s", string_attr);
            #endif
            next_token();

            if ( token != TNUMBER ) return (error_syntax("Number is not found at the end of output format", ERR_PAT_TOKEN));
            #if FLAG_PP
            printf("%s", string_attr);
            #endif
            next_token();
        } 
    }

    return (NORMAL);
}

/* EMPTY_STATEMENT ::=  */
static int empty_state ( void ) {
#if FLAG_DEBUG
    printf("empty statement\n");
#endif
    return (NORMAL);
}
