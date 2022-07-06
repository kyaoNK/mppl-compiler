#include "compiler.h"

#define MAX_LABEL_SIZE 128

Label *label_root;
Label *while_label_root;

FILE *fp_out;
int label_cnt;

int error_assy (char * mes) {
    printf("ERROR ASSY: line=%d | MES: %s\n", get_linenum(), mes);
    return ERROR;
}

void print_assy_func( char * func_name ) {
#if FLAG_ASSY_FUNC_DEBUG
    printf("\t\t%s\n", func_name);
#endif
}

void init_label_list ( void ) {
    label_root = NULL;
    while_label_root = NULL;
}

int add_label (Label **root, char *label_str, char *value) {
    Label *new_label;

    if ( (new_label = (Label *)malloc(sizeof(Label))) == NULL ) {
        return error_assy("can not malloc for Label in add label");
    }
    
    new_label->label = label_str;
    new_label->value = value;
    new_label->next_label = *root;
    *root = new_label;

    return 0;
}

void pop_while_label_list ( void ) {
    Label *p_label;
    if ( while_label_root == NULL ) {
        return;
    } 
    p_label = while_label_root->next_label;
    free(while_label_root);
    while_label_root = p_label;
}

void release_label ( Label **root ) {
    Label *p_label = *root;
    while ( p_label != NULL ) {
        Label *next_label = p_label->next_label;
        free(p_label);
        p_label = next_label;
    }
    *root = NULL;
}

void release_label_lists ( void ) {
    release_label(&label_root);
    release_label(&while_label_root);
}

int create_newlabel ( char **label_str ) {
    char *newlabel;

    if ( (newlabel = (char *)malloc(sizeof(char) * (MAX_LABEL_SIZE + 1))) == NULL ) {
        return error_assy("can not malloc in create new label\n");
    }

    label_cnt++;

    sprintf(newlabel, "L%04d", label_cnt);
    *label_str = newlabel;

    return 0;
}

int init_assy ( char *filename_mppl ) {
    char *filename = strtok(filename_mppl, ".");
    char filename_csl[128];

    sprintf(filename_csl, "%s.csl", filename);

    if ( (fp_out = fopen(filename_csl, "w")) == NULL ) {
        error_assy("can not open filename csl with fopen in function init assy");
        return ERROR;
    }

    label_cnt = 0;
    
    return NORMAL;
}

int end_assy ( void ) {
    if ( fclose(fp_out) == EOF ) {
        error_assy("can not close output file with fclose in function end assy");
        return ERROR;
    }
    return NORMAL;
}

void assy_labels ( void ) {
    Label *p_label = label_root;
    while ( p_label != NULL ) {
        fprintf(fp_out, "%s \tDC %s\n", p_label->label, p_label->value);
        p_label = p_label->next_label;
    }
}

void assy_start ( char *program_name ) {
    fprintf(fp_out, "$$%s \tSTART\n", program_name);
    fprintf(fp_out, "\tLAD \tgr0, 0\n");
    fprintf(fp_out, "\tCALL \tL0001\n");
    fprintf(fp_out, "\tCALL \tFLUSH\n");
    fprintf(fp_out, "\tSVC \t0\n");
}

void assy_block_end ( void ) {
    fprintf(fp_out, "\tRET\n");
}

void assy_procedure_declaration ( void ) {
#if FLAG_CMT
    fprintf(fp_out, ";procedure declaration\n");
#endif
    fprintf(fp_out, "$%s\n", current_procedure_name_str);
}

int assy_procedure_begin ( void ) {
    ID *p_id;
    ID *p_idlist = NULL;
    
    p_id = local_id_root->next_id;

    int flag_exist_local = FALSE;

    while ( p_id != NULL) {
        if ( strcmp(p_id->procedure_name, current_procedure_name_str) == 0 ) {
            flag_exist_local = TRUE;
            break;
        }
        p_id = p_id->next_id;
    }

    if ( !flag_exist_local ) {
        return (NORMAL);
    }
    
    fprintf(fp_out, "\tPOP\t gr2\n");
    
    while ( p_id != NULL ) {
        if ( strcmp(p_id->procedure_name, current_procedure_name_str) == 0 && p_id->is_formal_param == TRUE ) {
            fprintf(fp_out, "\tPOP gr1\n");
            fprintf(fp_out, "\tST \tgr1, $%s%%%s\n", p_id->name, current_procedure_name_str);
        }
        p_id = p_id->next_id;
    }
    fprintf(fp_out, "\tPUSH \t0, gr2\n");

    return NORMAL;
}

void assy_procedure_end ( void ) {
    fprintf(fp_out, "\tRET\n");
}

void assy_variable_declaration ( char *variable_name, char *procedure_name, Type **type) {
#if FLAG_CMT
    fprintf(fp_out, ";variable declaration\n");
#endif
    
    fprintf(fp_out, "$%s", variable_name);
    
    if ( procedure_name != NULL ) {
        fprintf(fp_out, "%%%s", procedure_name);
    }
    
    if (is_array_type((*type)->type)) {
        fprintf(fp_out, " \tDS \t%d\n", (*type)->size_array);
    }
    else {
        fprintf(fp_out, " \tDC \t0\n");
    }
}

void assy_ref_variable_left ( ID *ref_variable ) {
    print_assy_func("\tvariable left\n");

    if ( ref_variable->is_formal_param ) {
        fprintf(fp_out, "\tLD \tgr1, $%s%%%s\n", ref_variable->name, ref_variable->procedure_name);
    }
    else if ( ref_variable->procedure_name != NULL ) {
        fprintf(fp_out, "\tLAD \tgr1, $%s%%%s\n", ref_variable->name, ref_variable->procedure_name);
    }
    else {
        fprintf(fp_out, "\tLAD \tgr1, $%s\n", ref_variable->name);
    }

    if ( is_array_type(ref_variable->id_type->type) ) {
        fprintf(fp_out, "\tPOP \tgr2\n");
        fprintf(fp_out, "\tLAD \tgr3, %d\n", ref_variable->id_type->size_array - 1);
        fprintf(fp_out, "\tCPA \tgr2, gr3\n");
        fprintf(fp_out, "\tJPL \tEROV\n");
        fprintf(fp_out, "\tADDA \tgr1, gr2\n");
        fprintf(fp_out, "\tJOV \tEOVF\n");
    }
}

void assy_ref_variable_right ( ID *ref_variable ) {
    print_assy_func("\tvariable right\n");
    if ( is_array_type(ref_variable->id_type->type) ) {
        assy_ref_variable_left(ref_variable);
        fprintf(fp_out, "\tPOP \tgr1\n");
        fprintf(fp_out, "\tLD \tgr1, 0, gr1\n");
    }
    else {
        if ( ref_variable->is_formal_param ) {
            fprintf(fp_out, "\tLD \tgr1, $%s%%%s\n", ref_variable->name, ref_variable->procedure_name);
            fprintf(fp_out, "\tLD \tgr1, 0, gr1\n");
        }
        else if ( ref_variable->procedure_name != NULL ) {
            fprintf(fp_out, "\tLD \tgr1, $%s%%%s\n", ref_variable->name, ref_variable->procedure_name);
        }
        // // else if ( flag_in_call_statement ) {
        // //    fprintf(fp_out, "\tLAD \tgr1, $%s\n", ref_variable->name);
        // // }
        else {
            // // fprintf(fp_out, "here!!!!!!!!!\n");
            fprintf(fp_out, "\tLD \tgr1, $%s\n", ref_variable->name);
        }
    }
    // // fprintf(fp_out, "\tPUSH \t0, gr1\n");
}

void assy_real_param_2_label ( void ) {
    char *label = NULL;
    create_newlabel(&label);
    add_label(&label_root, label, "0");
    fprintf(fp_out, "\tLAD \tgr2, %s\n", label);
    // // fprintf(fp_out, "\tPOP \tgr1\n");
    fprintf(fp_out, "\tST \tgr1, 0, gr2\n");
    fprintf(fp_out, "\tPUSH \t0, gr2\n");

}

void assy_assign ( void ) {
    fprintf(fp_out, "\tPOP \tgr2\n");
    fprintf(fp_out, "\tST \tgr1, 0, gr2\n");
}

void assy_if_condition ( char * else_label_str ) {
#if FLAG_CMT
    fprintf(fp_out, "; if condition\n");
#endif
    fprintf(fp_out, "\tPOP \tgr1\n");
    fprintf(fp_out, "\tCPA \tgr1 \tgr0\n");
    fprintf(fp_out, "\tJZE \t%s\n", else_label_str);
}

void assy_else ( char * if_end_label_str , char * else_label_str ) {
#if FLAG_CMT
    fprintf(fp_out, "; else\n");
#endif
    fprintf(fp_out, "\tJUMP \t%s\n", if_end_label_str);
    fprintf(fp_out, "%s\n", else_label_str);
}

void assy_iter_condition ( char * bottom_label_str ) {
    // // fprintf(fp_out, "\tPOP \tgr1\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0\n");
    fprintf(fp_out, "\tJZE \t%s\n", bottom_label_str);
}

void assy_break ( void ) {
    fprintf(fp_out, "\tJUMP \t%s\n", while_label_root->label);
}

void assy_return ( void ) {
    if (flag_in_subprogram) 
        fprintf(fp_out, "\tRET\n");
    else
        fprintf(fp_out, "\tSVC \t0\n");
}

void assy_call ( ID * id_procedure ) {
    fprintf(fp_out, "\tCALL \t$%s\n", id_procedure->name);
}

void assy_expression ( int rel_ope_token ) {
    char *label_t_jmp = NULL;
    char *label_f_jmp = NULL;

    create_newlabel(&label_t_jmp);
    create_newlabel(&label_f_jmp);

    fprintf(fp_out, "\tPOP \tgr2\n");
    // // fprintf(fp_out, "\tPOP \tgr1\n");
    // // fprintf(fp_out, "\tCPA \tgr1, gr2\n");
    fprintf(fp_out, "\tCPA \tgr2, gr1\n");

    switch ( rel_ope_token ) {
        case TEQUAL:
            fprintf(fp_out, "\tJZE \t%s\n", label_t_jmp);
            break;
        
        case TNOTEQ:
            fprintf(fp_out, "\tJNZ \t%s\n", label_t_jmp);
            break;

        case TLE:
            fprintf(fp_out, "\tJMI \t%s\n", label_t_jmp);
            break;

        case TLEEQ:
            fprintf(fp_out, "\tJMI \t%s\n", label_t_jmp);
            fprintf(fp_out, "\tJZE \t%s\n", label_t_jmp);
            break;

        case TGR:
            fprintf(fp_out, "\tJPL \t%s\n", label_t_jmp);
            break;

        case TGREQ:
            fprintf(fp_out, "\tJPL \t%s\n", label_t_jmp);
            fprintf(fp_out, "\tJZE \t%s\n", label_t_jmp);
            break;
    }

    fprintf(fp_out, "\tLD \tgr1, gr0\n");
    // // fprintf(fp_out, "\tPUSH \t0, gr1\n");
    fprintf(fp_out, "\tJUMP \t%s\n", label_f_jmp);

    fprintf(fp_out, "%s\n", label_t_jmp);
    fprintf(fp_out, "\tLAD \tgr1, \t1\n");
    // // fprintf(fp_out, "\tPUSH \t0, gr1\n");
    fprintf(fp_out, "%s\n", label_f_jmp);
}

void assy_minus_sign ( void ) {
    fprintf(fp_out, "\tLAD \tgr1, -1\n");
    fprintf(fp_out, "\tPUSH \t0, gr1\n");
    assy_operator(TSTAR);
}

int assy_constant ( int value ) {
    fprintf(fp_out, "\tLAD \tgr1, %d\n", value);
    // // fprintf(fp_out, "\tPUSH \t0, gr1\n");
    return 0;
}

void assy_not_factor ( void ) {
    char * label_zero_jmp = NULL;
    char * label_not_end_jmp = NULL;
    
    create_newlabel(&label_zero_jmp);
    create_newlabel(&label_not_end_jmp);

    fprintf(fp_out, "\tPOP \tgr1\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0\n");
    fprintf(fp_out, "\tJNZ \t%s\n", label_zero_jmp);
    fprintf(fp_out, "\tLAD \tgr1, 1\n");
    fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
    fprintf(fp_out, "\tJUMP \t%s\n", label_not_end_jmp);

    fprintf(fp_out, "%s\n", label_zero_jmp);
    fprintf(fp_out, "\tLD \tgr1, \tgr0\n");
    fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
    fprintf(fp_out, "%s\n", label_not_end_jmp);
}

void assy_cast ( int to_type , int from_type ) {
    char *label_true_jmp = NULL;
    char *label_cast_end_jmp = NULL;
    create_newlabel(&label_true_jmp);
    create_newlabel(&label_cast_end_jmp);

    if (from_type == TYPE_INT) {
        if (to_type == TYPE_INT) {
            /* No Operation */
        } else if (to_type == TYPE_BOOL) {
            fprintf(fp_out, "\tPOP \tgr1\n");
            fprintf(fp_out, "\tCPA \tgr1, \tgr0\n");
            fprintf(fp_out, "\tJNZ \t%s\n", label_true_jmp);
            fprintf(fp_out, "\tLD \tgr1, \tgr0\n");
            fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
            fprintf(fp_out, "\tJUMP \t%s\n", label_cast_end_jmp);

            fprintf(fp_out, "%s\n", label_true_jmp);
            fprintf(fp_out, "\tLAD \tgr1, \t1\n");
            fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
            fprintf(fp_out, "%s\n", label_cast_end_jmp);
        } else if (to_type == TYPE_CHAR) {
            fprintf(fp_out, "\tPOP \tgr1\n");
            fprintf(fp_out, "\tLAD \tgr2, \t#007F\n");
            fprintf(fp_out, "\tAND \tgr1, \tgr2\n");
            fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
        }
    } else if (from_type == TYPE_BOOL) {
        /* No Operation */
    } else if (from_type == TYPE_CHAR) {
        if (to_type == TYPE_BOOL) {
            fprintf(fp_out, "\tPOP \tgr1\n"); 
            fprintf(fp_out, "\tCPA \tgr1, \tgr0\n");
            fprintf(fp_out, "\tJNZ \t%s\n", label_true_jmp);
            fprintf(fp_out, "\tLD \tgr1, \tgr0\n");
            fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
            fprintf(fp_out, "\tJUMP \t%s\n", label_cast_end_jmp);

            fprintf(fp_out, "%s\n", label_true_jmp);
            fprintf(fp_out, "\tLAD \tgr1, \t1\n");
            fprintf(fp_out, "\tPUSH \t0, \tgr1\n");
            fprintf(fp_out, "%s\n", label_cast_end_jmp);
        } else if (to_type == TYPE_INT || to_type == TYPE_CHAR) {
            /* No Operation */
        }
    }
}

void assy_operator ( int token ) {
    fprintf(fp_out, "\tPOP \tgr2\n");
    // // fprintf(fp_out, "\tPOP \tgr1\n");
    switch (token) {
        case TPLUS:
            fprintf(fp_out, "\tADDA \tgr1, gr2\n");
            break;
        
        case TMINUS:
            fprintf(fp_out, "\tSUBA \tgr2, gr1\n");
            break;

        case TOR:
            fprintf(fp_out, "\tOR \tgr1, gr2\n");
            break;

        case TSTAR:
            fprintf(fp_out, "\tMULA \tgr1, gr2\n");
            break;

        case TDIV:
            fprintf(fp_out, "\tDIVA \tgr1, gr2\n");
            break;

        case TAND:
            fprintf(fp_out, "\tAND \tgr1, g2\n");
            break;
    }

    if ( token == TPLUS || token == TMINUS || token == TSTAR ) {
        fprintf(fp_out, "\tJOV \tEOVF\n");
        if ( token == TMINUS ) {
            fprintf(fp_out, "\tLD \tgr1, gr2\n");
        }
    }
    else if ( token == TDIV ) {
        fprintf(fp_out, "\tJOV \tE0DIV\n");
    }

    // // fprintf(fp_out, "\tPUSH \t0, gr1\n");
}

int assy_output_format_string ( char * strings ) {
    char * label;
    char * all_strings;

    if ( ( all_strings = (char *)malloc(sizeof(char) * (strlen(strings) + 3)) ) == NULL ) {
        return (error_assy("can not malloc for char in function assy output format string"));
    }

    create_newlabel(&label);

    sprintf(all_strings, "'%s'", strings);

    add_label(&label_root, label, all_strings);
    
    fprintf(fp_out, "\tLAD \tgr1, %s\n", label);
    fprintf(fp_out, "\tLD \tgr2, gr0\n");
    fprintf(fp_out, "\tCALL \tWRITESTR\n");

    return 0;
}

void assy_output_format_std_type ( int type , int num ) {
    if ( num > 0 ) {
        fprintf(fp_out, "\tPOP \tgr1\n");
        fprintf(fp_out, "\tLAD \tgr2, \t%d\n", num);
    }

    fprintf(fp_out, "\tLD \tgr2, gr0\n");

    if ( type == TYPE_INT ) 
        fprintf(fp_out, "\tCALL \tWRITEINT\n");
    else if ( type == TYPE_CHAR ) 
        fprintf(fp_out, "\tCALL \tWRITECHAR\n");
    else if ( type == TYPE_BOOL ) 
        fprintf(fp_out, "\tCALL \tWRITEBOOL\n");
}

void assy_output_line ( void ) {
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
}

void assy_read ( int type ) {
    // // fprintf(fp_out, "\tPOP \tgr1\n");
    if ( type == TYPE_INT ) 
        fprintf(fp_out, "\tCALL \tREADINT\n");
    else if ( type == TYPE_ARRAY_CHAR ) 
        fprintf(fp_out, "\tCALL \tREADCHAR\n");
}

void assy_read_line ( void ) {
    fprintf(fp_out, "\tCALL \tREADLINE\n");
}

void assy_library ( void ) {
#if FLAG_CMT
    fprintf(fp_out, ";-- Library --\n");
#endif
    fprintf(fp_out, "EOVF\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tLAD \tgr1, EOVF1\n");
    fprintf(fp_out, "\tLD \tgr2, gr0\n");
    fprintf(fp_out, "\tCALL \tWRITESTR\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tSVC \t1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\toverflow error stop");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "EOVF1 \tDC \t'***** Run-Time Error : Overflow *****'\n");
    fprintf(fp_out, "E0DIV\n");
    fprintf(fp_out, "\tJNZ \tEOVF\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tLAD \tgr1, E0DIV1\n");
    fprintf(fp_out, "\tLD \tgr2, gr0\n");
    fprintf(fp_out, "\tCALL \tWRITESTR\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tSVC \t2");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t0-divide error stop");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "E0DIV1 \tDC \t'***** Run-Time Error : Zero-Divide *****'\n");
    fprintf(fp_out, "EROV\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tLAD \tgr1, EROV1\n");
    fprintf(fp_out, "\tLD \tgr2, gr0\n");
    fprintf(fp_out, "\tCALL \tWRITESTR\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tSVC \t3");
#if FLAG_CMT
    fprintf(fp_out, "\t;\trange-over error stop");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "EROV1 \tDC \t'***** Run-Time Error : Range-Over in Array Index *****'\n");
    fprintf(fp_out, "WRITECHAR\n");
#if FLAG_CMT
    fprintf(fp_out, "; Outputs the value (characters) of gr1 with the number of digits of gr2.\n");
    fprintf(fp_out, "; If gr2 is 0, output the minimum number of digits necessary.\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr6, SPACE\n");
    fprintf(fp_out, "\tLD \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "WC1\n");
    fprintf(fp_out, "\tSUBA \tgr2, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\twhile(--c > 0) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tWC2\n");
    fprintf(fp_out, "\tJMI \tWC2\n");
    fprintf(fp_out, "\tST \tgr6, OBUF, gr7");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t*p++ = ' ';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tBOVFCHECK\n");
    fprintf(fp_out, "\tJUMP \tWC1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t}");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WC2\n");
    fprintf(fp_out, "\tST \tgr1, OBUF, gr7");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t*p++ = gr1;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tBOVFCHECK\n");
    fprintf(fp_out, "\tST \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "WRITESTR\n");
#if FLAG_CMT
    fprintf(fp_out, "; Output the string pointed to by gr1 with gr2 as the number of digits.\n");
    fprintf(fp_out, "; If gr2 is 0, output the minimum number of digits necessary.\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr6, gr1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\tp = gr1;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WS1\n");
    fprintf(fp_out, "\tLD \tgr4, 0, gr6\n");
    fprintf(fp_out, "\tJZE \tWS2\n");
    fprintf(fp_out, "\tADDA \tgr6, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\tp++;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tSUBA \tgr2, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\tc--;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJUMP \tWS1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t}");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WS2\n");
    fprintf(fp_out, "\tLD \tgr7, OBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\tq = OBUFSIZE;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr5, SPACE\n");
    fprintf(fp_out, "WS3\n");
    fprintf(fp_out, "\tSUBA  \tgr2, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t while(--c >= 0) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJMI \tWS4\n");
    fprintf(fp_out, "\tST \tgr5, OBUF, gr7");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *q++ = ' ';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tBOVFCHECK\n");
    fprintf(fp_out, "\tJUMP \tWS3");
#if FLAG_CMT
    fprintf(fp_out, "; }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WS4\n");
    fprintf(fp_out, "\tLD \tgr4, 0, gr1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t while ( *gr1 != '\\0') {");
#endif 
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tWS5\n");
    fprintf(fp_out, "\tST \tgr4, OBUF, gr7");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *q++ = *gr1++ ;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tADDA \tgr1, ONE\n");
    fprintf(fp_out, "\tCALL \tBOVFCHECK\n");
    fprintf(fp_out, "\tJUMP \tWS4");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WS5\n");
    fprintf(fp_out, "\tST \tgr7, OBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t OBUFSIZE = q;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "BOVFCHECK\n");
    fprintf(fp_out, "\tADDA \tgr7, ONE\n");
    fprintf(fp_out, "\tCPA \tgr7, BOVFLEVEL\n");
    fprintf(fp_out, "\tJMI \tBOVF1\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "\tLD \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "BOVF1\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "BOVFLEVEL \tDC 256\n");
    fprintf(fp_out, "WRITEINT\n");
#if FLAG_CMT
    fprintf(fp_out, "; Outputs the value of gr1 (integer) as the number of digits in gr2.\n");
    fprintf(fp_out, "; If gr2 is 0, output the minimum number of digits required.\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr7, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t flag = 0;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( gr1 >= 0 ) goto WI1;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJPL \tWI1\n");
    fprintf(fp_out, "\tJZE \tWI1\n");
    fprintf(fp_out, "\tLD \tgr4, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr1= - gr1;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tSUBA \tgr4, gr1\n");
    fprintf(fp_out, "\tCPA \tgr4, gr1\n");
    fprintf(fp_out, "\tJZE \tWI6\n");
    fprintf(fp_out, "\tLD \tgr1, gr4\n");
    fprintf(fp_out, "\tLD \tgr7, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t flag = 1;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WI1\n");
    fprintf(fp_out, "\tLD \tgr6, SIX");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t p = INTBUF + 6;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr0, INTBUF, gr6");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *p = '\\0';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tSUBA \tgr6, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t p--;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( gr1 == 0 )");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJNZ \tWI2\n");
    fprintf(fp_out, "\tLD \tgr4, ZERO");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *p = '0';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr4, INTBUF, gr6\n");
    fprintf(fp_out, "\tJUMP \tWI5");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WI2");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t else {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t while(gr1 != 0) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tWI3\n");
    fprintf(fp_out, "\tLD \tgr5, gr1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr5 = gr1 - (gr1 / 10) * 10;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tDIVA \tgr1, TEN");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr1 /= 10;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr4, gr1\n");
    fprintf(fp_out, "\tMULA \tgr4, TEN\n");
    fprintf(fp_out, "\tSUBA \tgr5, gr4\n");
    fprintf(fp_out, "\tADDA \tgr5, ZERO");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr5 += '0';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr5, INTBUF, gr6");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *p = gr5;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tSUBA \tgr6, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t p--;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJUMP \tWI2");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WI3\n");
    fprintf(fp_out, "\tCPA \tgr7, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( flag != 0 ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tWI4\n");
    fprintf(fp_out, "\tLD \tgr4, MINUS");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *p = '-';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr4, INTBUF, gr6\n");
    fprintf(fp_out, "\tJUMP \tWI5");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WI4\n");
    fprintf(fp_out, "\tADDA \tgr6, ONE");
#if FLAG_CMT    
    fprintf(fp_out, "\t;\t else p++;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\t");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WI5\n");
    fprintf(fp_out, "\tLAD \tgr1, INTBUF, gr6");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr1 = p;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tWRITESTR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t WRITESTR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "WI6\n");
    fprintf(fp_out, "\tLAD \tgr1, MMINT\n");
    fprintf(fp_out, "\tCALL \tWRITESTR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t WRITESTR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "MMINT \tDC \t'-32768'\n");
    fprintf(fp_out, "WRITEBOOL\n");
#if FLAG_CMT
    fprintf(fp_out, "; Output 'FALSE' if the value (truth value) of gr1 is 0, and 'TRUE' if it is not 0, as the digit number of gr2.\n");
    fprintf(fp_out, "; If gr2 is 0, output the minimum number of digits necessary.\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tCPA \tgr1, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( gr1 != 0 )");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tWB1\n");
    fprintf(fp_out, "\tLAD \tgr1, WBTRUE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr1 = \" TRUE \";");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJUMP \tWB2\n");
    fprintf(fp_out, "WB1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t else");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLAD \tgr1, WBFALSE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t gr1 = \" FALSE \";");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "WB2\n");
    fprintf(fp_out, "\tCALL \tWRITESTR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t WRITESTR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "WBTRUE \tDC \t'TRUE'\n");
    fprintf(fp_out, "WBFALSE \tDC \t'FALSE'\n");
    fprintf(fp_out, "WRITELINE\n");
#if FLAG_CMT
    fprintf(fp_out, "; Output a newline\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "\tLD \tgr6, NEWLINE\n");
    fprintf(fp_out, "\tST \tgr6, OBUF, gr7\n");
    fprintf(fp_out, "\tADDA \tgr7, ONE\n");
    fprintf(fp_out, "\tST \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "\tOUT \tOBUF, OBUFSIZE\n");
    fprintf(fp_out, "\tST \tgr0, OBUFSIZE\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "FLUSH\n");
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr7, OBUFSIZE\n");
    fprintf(fp_out, "\tJZE \tFL1\n");
    fprintf(fp_out, "\tCALL \tWRITELINE\n");
    fprintf(fp_out, "FL1\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "READCHAR\n");
#if FLAG_CMT
    fprintf(fp_out, "; Read a single character at the address pointed to by gr1\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "\tLD \tgr5, RPBBUF");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( RPBBUF != '\\0' ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tRC0\n");
    fprintf(fp_out, "\tST \tgr5, 0, gr1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *gr1 = RPBBUF;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr0, RPBBUF");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t RPBBUF = '\\0'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJUMP \tRC3");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t return; }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RC0\n");
    fprintf(fp_out, "\tLD \tgr7, INP");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t inp = INP;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr6, IBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( IBUFSIZE == 0 ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJNZ \tRC1\n");
    fprintf(fp_out, "\tIN \tIBUF, IBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t IN();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr7, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t inp = 0;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\t");
#if FLAG_CMT
    fprintf(fp_out, "; }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RC1\n");
    fprintf(fp_out, "\tCPA \tgr7, IBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( inp == IBUFSIZE ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJNZ \tRC2\n");
    fprintf(fp_out, "\tLD \tgr5, NEWLINE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *gr1 = '\\n';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr5, 0, gr1\n");
    fprintf(fp_out, "\tST \tgr0, IBUFSIZE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t IBUFSIZE = INP = 0;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr0, INP\n");
    fprintf(fp_out, "\tJUMP \tRC3");
#if FLAG_CMT
    fprintf(fp_out, "; }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RC2");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t else {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr5, IBUF, gr7");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *gr1 = *inp++;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tADDA \tgr7, ONE\n");
    fprintf(fp_out, "\tST \tgr5, 0, gr1\n");
    fprintf(fp_out, "\tST \tgr7, INP");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t INP = inp;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RC3");
#if FLAG_CMT
    fprintf(fp_out, "\t; }\n");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "READINT\n");
#if FLAG_CMT
    fprintf(fp_out, "; Reads a single integer value at the address pointed to by gr1\n");
#endif
    fprintf(fp_out, "\tRPUSH\n");
    fprintf(fp_out, "RI1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t do {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tREADCHAR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t ch = READCHAR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr7, 0,gr1\n");
    fprintf(fp_out, "\tCPA \tgr7, SPACE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t } while(ch == ' ' || ch == '\\t' || ch == '\\n');");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJZE \tRI1\n");
    fprintf(fp_out, "\tCPA \tgr7, TAB\n");
    fprintf(fp_out, "\tJZE \tRI1\n");
    fprintf(fp_out, "\tCPA \tgr7, NEWLINE\n");
    fprintf(fp_out, "\tJZE \tRI1\n");
    fprintf(fp_out, "\tLD \tgr5, ONE");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t flag = 1");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCPA \tgr7, MINUS");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if(ch == '-') {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJNZ \tRI4\n");
    fprintf(fp_out, "\tLD \tgr5, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t flag = 0;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCALL \tREADCHAR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t ch = READCHAR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr7, 0, gr1\n");
    fprintf(fp_out, "RI4");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr6, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t v = 0;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RI2\n");
    fprintf(fp_out, "\tCPA \tgr7, ZERO");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t while ( '0' <= ch && ch <= '9' ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJMI \tRI3\n");
    fprintf(fp_out, "\tCPA \tgr7, NINE\n");
    fprintf(fp_out, "\tJPL \tRI3\n");
    fprintf(fp_out, "\tMULA \tgr6, TEN");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t v = v*10+ch-'0';");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tADDA \tgr6, gr7\n");
    fprintf(fp_out, "\tSUBA \tgr6, ZERO\n");
    fprintf(fp_out, "\tCALL \tREADCHAR");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t ch = READSCHAR();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tLD \tgr7, 0, gr1\n");
    fprintf(fp_out, "\tJUMP \tRI2");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "RI3\n");
    fprintf(fp_out, "\tST \tgr7, RPBBUF");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t ReadPushBack();");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr6, 0, gr1");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *gr1 = v;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tCPA \tgr5, gr0");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t if ( flag == 0 ) {");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tJNZ \tRI5\n");
    fprintf(fp_out, "\tSUBA \tgr5, gr6");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t *gr1 = -v;");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tST \tgr5, 0, gr1\n");
    fprintf(fp_out, "RI5");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t }");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "\tRPOP\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "READLINE\n");
#if FLAG_CMT
    fprintf(fp_out, "; Skip reading input up to (and including) the newline code\n");
#endif
    fprintf(fp_out, "\tST \tgr0, IBUFSIZE\n");
    fprintf(fp_out, "\tST \tgr0, INP\n");
    fprintf(fp_out, "\tST \tgr0, RPBBUF\n");
    fprintf(fp_out, "\tRET\n");
    fprintf(fp_out, "ONE \tDC \t1\n");
    fprintf(fp_out, "SIX \tDC \t6\n");
    fprintf(fp_out, "TEN \tDC \t10\n");
    fprintf(fp_out, "SPACE \tDC \t#0020");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t ' '");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "MINUS \tDC \t#002D");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t '-'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "TAB \tDC \t#0009");
#if FLAG_CMT
    fprintf(fp_out, "  ; '\\t'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "ZERO \tDC \t#0030");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t '0'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "NINE \tDC \t#0039");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t '9'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "NEWLINE \tDC \t#000A");
#if FLAG_CMT
    fprintf(fp_out, "\t;\t '\\n'");
#endif
    fprintf(fp_out, "\n");
    fprintf(fp_out, "INTBUF \tDS \t8\n");
    fprintf(fp_out, "OBUFSIZE \tDC \t0\n");
    fprintf(fp_out, "IBUFSIZE \tDC \t0\n");
    fprintf(fp_out, "INP \tDC \t0\n");
    fprintf(fp_out, "OBUF \tDS \t257\n");
    fprintf(fp_out, "IBUF \tDS \t257\n");
    fprintf(fp_out, "RPBBUF \tDC \t0\n");
    fprintf(fp_out, "\tEND\n");
}
