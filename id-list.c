#include "compiler.h"

/* id root */
ID *global_id_root;     /* variable in program */
ID *local_id_root;      /* variable in subprogram */
ID *undefined_type_id_root;  /* name only variable */

/* type info */
int size_array;
char *procedure_name_str;
int  *is_formal_parameters;

/* flag */
int flag_in_subprogram;
int flag_is_formal_parameters;
int flag_in_variable_declaration;
int flag_procedure_name_in_subprogram_declaration;

/* id */
extern ID *id_procedure_name;

/* ID List Function */
/* create struct */
ID* create_id_root ( void ) ;
ID* create_id ( char *name , char *procedure_name, Type *ttype, int is_formal_param , int def_linenum , Line *ref_line ) ;
Type* create_type ( int itype ) ;
Line* create_line ( void ) ; 

void copy_id_table ( ID *id_root_src, ID *id_root_dest ) ;

ID* search_id( char * name );
ID* search_id_intable ( char *name , char *procedure_name ,  ID *id_root ) ;

void release_id_root ( ID *id_root ) ;

ID* get_tail_id_root ( ID *id_root ) ;

void add_formal_parameter ( char *procedure_name , Type *ttype ) ;

int check_duplicate_id_intable ( char *name , char *procedure_name , ID *id_root ) ;

/* debug */
void print_func_name ( char *func_name ) ;
void print_id ( ID *id ) ;
void print_id_table ( ID *id_root ) ;
void print_type ( Type *ttype ) ;

int error_id ( char *mes );

/* Cross Reference Function */
ID* sort ( ID *id_root ) ;
void print_id_name ( char *name , char *procedure_name ) ;
void print_id_type ( Type *ttype ) ;
void print_id_linenum ( int def , Line *ref ) ;
void print_id_format ( ID *id ) ;

/* --------------------------------------- */
/* ---------- internal function ---------- */
/* --------------------------------------- */
/* create struct id */
ID* create_id ( char *name , char *procedure_name, Type *ttype, int is_formal_param , int def_linenum , Line *ref_line ) {
    print_func_name("create id");
    ID *id;
    if ( ( id = (ID *)malloc(sizeof(ID)) ) == NULL ) {  /* struct id memory allocation */
        error_id("cannot malloc for struct id in create id");
        return NULL;
    }

    if ( ( id->name = (char *)malloc(strlen(name) + 1) ) == NULL ) {     /* name memory allocation */
        error_id("cannot malloc for char name of struct id in create id");
        return NULL;
    }
    strcpy(id->name, name); /* name set into id */

    if ( flag_in_subprogram ) { /* procedure name memory allocation */
        if ( flag_procedure_name_in_subprogram_declaration ) {
            id->procedure_name = NULL;
        }
        else {
            int size_procedure_name = strlen(procedure_name_str) + 1;
            if ( ( id->procedure_name = (char *)malloc(size_procedure_name) ) == NULL ) {
                error_id("cannot malloc for char procedure name of struct id in create id");
                return NULL;
            }
            strcpy(id->procedure_name, procedure_name_str);
        }
    }

    id->id_type = ttype;    /* set type */
    id->is_formal_param = is_formal_param; /* is formal parameters */
    id->def_linenum = def_linenum;  /* define linenum */
    id->ref_linenum = ref_line;     /* refer linenum */    
    id->next_id = NULL;     /* next id */
    return (id);
}

/* create id root */
ID* create_id_root ( void ) {
    print_func_name("create id root");
    ID *id_root;
    if ( ( id_root = (ID *)malloc(sizeof(ID)) ) == NULL ) {
        error_id("cannot malloc for struct id in create id root");
        return NULL;
    }
    id_root->next_id = NULL;
    return id_root;
}

/* create struct type */
Type* create_type ( int itype ) {
    print_func_name("create type");
    Type *ttype;
    Type *elem_type;
    if ( ( ttype = (Type *)malloc(sizeof(Type)) ) == NULL ) {
        error_id("cannot malloc for struct type in create type");
        return NULL;
    }
    ttype->type = itype;
    ttype->size_array = size_array;
    ttype->next_param_type = NULL;

    if ( is_array_type(itype) ) {
        if ( ( elem_type = (Type *)malloc(sizeof(Type)) ) == NULL ) {
            error_id("cannot malloc for struct array elem type in create type");
            return NULL;
        }
        
        if ( itype == TYPE_ARRAY_INT ) {
            elem_type->type = TYPE_INT;
        }
        else if ( itype == TYPE_ARRAY_CHAR ) {
            elem_type->type = TYPE_CHAR;
        }
        else if ( itype == TYPE_ARRAY_BOOL ) {
            elem_type->type = TYPE_BOOL;
        }

        elem_type->size_array = 0;
        elem_type->array_elem_type = NULL;
        elem_type->next_param_type = NULL;

        ttype->array_elem_type = elem_type;
    }
    else {
        ttype->array_elem_type = NULL;
    }
    return (ttype);
}

/* create struct line */
Line* create_line() {
    print_func_name("create line");
    Line *line;
    if ( ( line = (Line *)malloc(sizeof(Line)) ) == NULL ) {
        error_id("cannot malloc for struct line in create line");
        return NULL;
    }
    return line;
}

/* Copy of id table */
void copy_id_table ( ID *id_root_src, ID *id_root_dest ) {
    print_func_name("copy id table");
    ID *tail = get_tail_id_root(id_root_dest);
    
    ID *p_src  = id_root_src->next_id;
    int i = 1;
    while ( p_src != NULL ) {
        tail->next_id = create_id(p_src->name, p_src->procedure_name, p_src->id_type, p_src->is_formal_param, p_src->def_linenum, p_src->ref_linenum);
        
        tail = tail->next_id;
        p_src  = p_src->next_id;
        i++;
    }
    return;
}

/* Search for id in the order of local and global */
ID* search_id ( char *name ) {
    print_func_name("search id");
    ID *p;
#if FLAG_DEBUG
    printf("\tname:%s, procname:%s\n", name, procedure_name_str);
#endif
    /* local */
    if ( ( p = search_id_intable(name, procedure_name_str, local_id_root) ) == NULL ) {
        /* global */
        if ( ( p = search_id_intable(name, NULL, global_id_root) ) == NULL ) {
            error_id("This vairable is undefined");
            return NULL;
        }
        else {
            #if FLAG_DEBUG
            printf("\tname:%s, procname:%s, type:%s, def:%d in search id\n", p->name, p->procedure_name, get_type_str(p->id_type->type), p->def_linenum);
            #endif
            return p;
        }
    }
    #if FLAG_DEBUG
    printf("\tname:%s, procname:%s, type:%s, def:%d in local\n", p->name, p->procedure_name, get_type_str(p->id_type->type), p->def_linenum);
    #endif
    return p;
}

/* search id by name and procedure name in id table */
ID* search_id_intable ( char *name , char *procedure_name , ID *id_root ) {
    print_func_name("search id table");
    ID *p;
    for ( p = id_root->next_id ; p != NULL ; p = p->next_id ) {
        if ( strcmp(p->name, name) == 0 ) {
            if ( procedure_name == NULL && p->procedure_name == NULL ) return p;
            else if ( procedure_name != NULL && p->procedure_name != NULL && strcmp(p->procedure_name, procedure_name) == 0 ) return p;
        }
    }
    return NULL;
}

/* release id root */
void release_id_root ( ID *id_root ) {
    print_func_name("release id root");
    id_root->next_id = NULL;
}

/* get tail of id root next (null) */
ID* get_tail_id_root ( ID *id_root ) {
    print_func_name("get tail id root");
    ID *p = id_root->next_id;

    if ( p == NULL ) return id_root;
    while ( p->next_id != NULL ) {
        p = p->next_id;
    }
    return p;
}

/* add formal parameters type to procedure name id */
void add_formal_parameter ( char *procedure_name , Type *ttype ) {
    print_func_name("add formal parameter");
    ID *id;
    ID *p_id;
    Type *p_type;
    /* search procedure name id */
    if ( ( id = search_id_intable(procedure_name, NULL, global_id_root )) == NULL ) {
        error_id("Procedure name is not found");
        return ;
    }
#if FLAG_DEBUG
    printf("type:%s, procname:%s\n", get_type_str(id->id_type->type), id->name);
#endif
    /* procedure name type */
    p_type = id->id_type;

#if FLAG_DEBUG
    print_id_table(undefined_type_id_root);
#endif

    /* search is formal param true in undefined root */
    for ( p_id = undefined_type_id_root->next_id ; p_id != NULL ; p_id = p_id->next_id ) {
        if ( ( p_type->next_param_type = (Type *)malloc(sizeof(Type)) ) == NULL ) {
            error_id("cannot malloc for struct type in formal parameter");
            return;
        }
        /* type in param */
        p_type->next_param_type->type = p_id->id_type->type;
        /* size in param */
        p_type->next_param_type->size_array = p_id->id_type->size_array;
        /* next in param */
        p_type->next_param_type->next_param_type = NULL;
        /*  */
        if( is_array_type(p_id->id_type->type) ){
            Type *p_elem_type;
            if( ( p_elem_type = (Type *)malloc(sizeof(Type)) ) == NULL ){
                error_id("cannot malloc for struct elem type in add formal parameter");
                return;
            }
            p_elem_type->type = p_id->id_type->type;
            p_elem_type->size_array = p_id->id_type->size_array;
            p_elem_type->array_elem_type = NULL;
            p_elem_type->next_param_type = NULL;

            p_type->next_param_type->array_elem_type = p_elem_type;
        }else{
            /* Parameter's type is standard type */
            p_type->next_param_type->array_elem_type = NULL;
        }
        p_type = p_type->next_param_type;
    }

#if FLAG_DEBUG
    Type *p;
    int i = 1;
    printf("-------------------------- formal parametes ---------------------------\n");
    printf("procedure name:%s\n", id->name);
    for ( p = id->id_type ; p != NULL ; p = p->next_param_type ) {
        printf("[%d] type:%s\n", i, get_type_str(p->type));
        i++;
    }
    printf("-------------------------- formal parametes ---------------------------\n");
#endif
    return ;
}

/* check duplicate defined in table (global or local) */
int check_duplicate_id_intable ( char *name , char *procedure_name , ID *id_root ) {
    print_func_name("check duplicate");
    ID *p;
    if ( ( p = search_id_intable(name, procedure_name, id_root) ) == NULL ) {
        return (NORMAL);
    }   
    else {
        error_id("Duplicate defined");
        return (ERROR);
    }
} 

/* error id */
int error_id ( char *mes ) {
    printf("------------------------------------------------------------\n");
    printf("ERROR ID LIST: line=%d | MES: %s\n", get_linenum(), mes);
    printf("------------------------------------------------------------\n");
    return (ERROR);
}

/* ----- Debug Function ----- */
void print_func_name ( char *func_name ) {
    #if FLAG_DEBUG
    printf("\t%s\n", func_name);
    #endif
}

void print_id ( ID *id ) {
    // print_func_name("print id");
#if FLAG_DEBUG
    Line *p;
    if ( id == NULL ) {
        error_id("refering id is null");
        return;
    }
    printf("id|name:%s, ", id->name);
    if ( id->procedure_name != NULL )
        printf("procedure_name:%s,", id->procedure_name);
    if ( id->id_type != NULL )
    printf("type:%s, ", get_type_str(id->id_type->type));
    printf("is_forpara:%s, " , (id->is_formal_param) ? "true" : "false");
    printf("def_line:%d", id->def_linenum);
    if ( id->ref_linenum != NULL ) {
        printf(", ref_line:");
        for ( p = id->ref_linenum ; p != NULL ; p = p->p_next_line)
            printf("%d->", p->refer_linenum);
    }
    printf("\n");
#endif
    return;
}

void print_type ( Type *ttype ) {
#if FLAG_DEBUG
    print_func_name("print type");
    printf("------------------------------------------\n");
    printf("type:%s, ", get_type_str(ttype->type));
    if ( ttype->size_array >= 1 ) printf("size array:%d", ttype->size_array);
    printf("\n");
    printf("------------------------------------------\n");
#endif
    return;
}

void print_id_table ( ID *id_root ) {
    print_func_name("print id table");
#if FLAG_DEBUG
    ID *p = id_root;
    printf("----------------------------------------------------------------------------------------------------\n");
    while ( p != NULL ) {
        print_id(p);
        p = p->next_id;
    }
    printf("----------------------------------------------------------------------------------------------------\n");
#endif
    return;
}

/* type string */
char* get_type_str ( int ttype ) {
    switch (ttype) {
        case TYPE_NONE :
            return "none";
        case TYPE_INT :
            return "integer";
        case TYPE_BOOL :
            return "boolean";
        case TYPE_CHAR :
            return "char";
        case TYPE_ARRAY :
            return "array";
        case TYPE_ARRAY_INT :
            return "array-int";
        case TYPE_ARRAY_BOOL :
            return "array-bool";
        case TYPE_ARRAY_CHAR :
            return "array-char";
        case TYPE_PROCEDURE :
            return "procedure";
        default :
            return "default";
    }
}

/* ----- Function in Cross Reference ----- */
/* sort by lexicographic order */
ID* sort ( ID *id_root ) {
    ID *head_sorted;
    ID *head_unsorted;
    ID *max;
    ID *prev_max;
    ID *prev_comp;
    
    head_unsorted = id_root;
    head_sorted = NULL;

    while ( head_unsorted != NULL ) {
        max = head_unsorted;
        prev_max = NULL;
        prev_comp = head_unsorted;
        while ( prev_comp->next_id != NULL ) {
            if (  strcmp(prev_comp->next_id->name, max->name) > 0 ) {
                max = prev_comp->next_id;
                prev_max = prev_comp;
            } 
            else if ( strcmp(prev_comp->next_id->name, max->name) == 0 ) {
                if ( max->procedure_name == NULL || prev_comp->procedure_name == NULL ) {
                    max = prev_comp->next_id;
                    prev_max = prev_comp;
                }
                else if ( strcmp(prev_comp->next_id->procedure_name, max->procedure_name) > 0 ) {
                    max = prev_comp->next_id;
                    prev_max = prev_comp;
                }
            }
            prev_comp = prev_comp->next_id;
        }
        if ( prev_max == NULL ) {
            head_unsorted = max->next_id;
        }
        else {
            prev_max->next_id = max->next_id;
        }
        
        if ( head_sorted == NULL ) {
            head_sorted = max;
            max->next_id = NULL;
        }
        else {
            max->next_id = head_sorted;
            head_sorted = max;
        }

    }
	
	return(head_sorted);
}

/* max name plus procedurename string size 30 */
void print_id_name ( char *name, char *procedure_name) {
    char name_str[MAX_BUF_SIZE];
    if ( procedure_name != NULL ) {
        sprintf(name_str, "%s:%s", name, procedure_name);
    }
    else {
        sprintf(name_str, "%s", name);
    }
    printf("%-30s\t", name_str);
}

/* max type string size 40 */
void print_id_type ( Type *ttype ) {
    Type *p_type;
    char type_str[MAX_BUF_SIZE];
    if ( is_array_type(ttype->type) ) { /* array */
        sprintf(type_str, "array [%d] of %s",  ttype->size_array, get_type_str(ttype->array_elem_type->type));
    }
    else if ( ttype->type == TYPE_PROCEDURE ) {
        strcpy(type_str, "procedure");
        if ( ttype->next_param_type != NULL ) strcat(type_str, "(");
        for( p_type = ttype->next_param_type ; p_type != NULL ; p_type = p_type->next_param_type ) {
            strcat(type_str, get_type_str(p_type->type));
            if ( p_type->next_param_type != NULL ) strcat(type_str, ",");
            else strcat(type_str, ")");
        }
    }
    else {
        sprintf(type_str, "%s", get_type_str(ttype->type));
    }
    printf("%-40s\t", type_str);
} 

/* max define and refer linenum string size 30 */
void print_id_linenum (int def, Line *ref) {
    char line_str[MAX_BUF_SIZE];
    char num_str[MAX_BUF_SIZE];
    sprintf(line_str, "%3d | ", def);
    Line *p_line;
    for ( p_line = ref ; p_line->refer_linenum != 0 ; p_line = p_line->p_next_line ) {;
        sprintf(num_str, "%d", p_line->refer_linenum);
        strcat(line_str, num_str);
        if (p_line->p_next_line->refer_linenum != 0 ) strcat(line_str, ", ");
    }
    printf("%-30s\t", line_str);
}

/* id print format */
void print_id_format ( ID *id ) {
    print_id_name(id->name, id->procedure_name);
    print_id_type(id->id_type);
    print_id_linenum(id->def_linenum, id->ref_linenum);
    printf("\n");
}

/* ----------------------------------------------------------- */
/* -------------------- external function -------------------- */
/* ----------------------------------------------------------- */

/* ----- Function in ID List ----- */
/* initialize id root */
void init_id_root ( void ) {
    print_func_name("init id root");
    global_id_root      = create_id_root();
    local_id_root       = create_id_root();
    undefined_type_id_root = create_id_root();
}

/* add undefined type id into undefined type id table */
int add_undefined_type_id ( char *name ) {
    print_func_name("add undefined type id");
    char *procedure_name = procedure_name_str;

    if ( check_duplicate_id_intable(name, procedure_name, undefined_type_id_root) == ERROR ) {
        return (ERROR);
    }

    ID *id = create_id(name, procedure_name, NULL, *is_formal_parameters, get_linenum(), create_line());
    ID *tail_id = get_tail_id_root(undefined_type_id_root);
    tail_id->next_id = id;
    id->next_id = NULL;

#if FLAG_DEBUG
    printf("undefined");
    print_id_table(undefined_type_id_root);
#endif

    return (NORMAL);
}

/* assign type to undefined type id in undefined type id table */
int assign_type ( int itype ) {
    print_func_name("assign type");
    Type *ttype = create_type(itype);
    ID *p;
    for ( p = undefined_type_id_root ; p != NULL ; p = p->next_id ) {
        p->id_type = ttype;
    }
#if FLAG_DEBUG
    printf("undefined");
    print_id_table(undefined_type_id_root);
#endif

    if ( flag_in_subprogram && !flag_procedure_name_in_subprogram_declaration) {    /* local */
        copy_id_table(undefined_type_id_root, local_id_root);
#if FLAG_DEBUG
        printf("local");
        print_id_table(local_id_root);
#endif
    } 
    else {  /* global */
        copy_id_table(undefined_type_id_root, global_id_root);
#if FLAG_DEBUG
        printf("global");
        print_id_table(global_id_root);
#endif
    }

    if ( flag_in_formal_parameters ) {
        add_formal_parameter(procedure_name_str, ttype);
    }

    release_id_root(undefined_type_id_root);

    return (NORMAL);
}

/* add refer linenum to global or local id table */
int add_ref_linenum( char *name ) {
    print_func_name("add ref linenum");
    ID *id;
    if ( ( id = search_id(name) ) == NULL ) return (ERROR);

    /* procedure */
    if ( flag_in_subprogram ) {
        if ( strcmp(id->name, procedure_name_str) == 0 && id->id_type->type == TYPE_PROCEDURE ) {
            error_id("This procedure name cannot be called recursively");
            return (ERROR);
        } 
    }
    Line *p = id->ref_linenum;
    while ( p->refer_linenum != 0 ) {
        p = p->p_next_line;
    }
    p->refer_linenum = get_linenum();
    p->p_next_line = create_line();
    return (id->id_type->type);
}

int is_array_type ( int itype ) {
    print_func_name("is array type");
    if ( TYPE_ARRAY <= itype && itype <= TYPE_ARRAY_BOOL ) return (TRUE);
    else return (FALSE);
}

/* set procedure name into procedure_name_str */
int set_procedure_name ( char *procedure_name ) {
    print_func_name("set procedure name");
    int size = strlen(procedure_name) + 1;
    if ( ( procedure_name_str = (char *)malloc(size) ) == NULL ) return (ERROR);
    strcpy(procedure_name_str, procedure_name);
    return (NORMAL);
}

/* set null into procedure_name_str */
void free_procedure_name ( void ) {
    print_func_name("free procedure name");
    procedure_name_str = NULL;
}

/* search procedure name id in global id table */
ID* search_procedure_id ( char *procedure_name ) {
    ID *p;
    p = search_id_intable(procedure_name, NULL, global_id_root);
#if FLAG_DEBUG
    printf("name:%s, type:%s\n", p->name, get_type_str(p->id_type->type));
#endif
    return p;
}

/* ----- Function in Cross Reference ----- */
void print_crossreference ( void ) {
#if FLAG_DEBUG
    printf("global");
    print_id_table(global_id_root);
    printf("local");
    print_id_table(local_id_root);
#endif
    ID *tail = get_tail_id_root(global_id_root);
    print_id(tail);
    tail->next_id = local_id_root->next_id;

#if FLAG_DEBUG
    printf("all");
    print_id_table(global_id_root);
#endif

    ID *all_sorted = sort(global_id_root->next_id);
    ID *p;

    // printf("\n");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("-------------------------------------------------- Cross Reference --------------------------------------------------\n");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("%-30s\t%-40s\t%-30s\n", "Name", "Type", "Def | Ref");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    for ( p = all_sorted ; p != NULL ; p = p->next_id ) {
        print_id_format(p);
    }
    printf("---------------------------------------------------------------------------------------------------------------------\n");
}

