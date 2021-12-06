#include "lexical.h" /* Read token list header file */

/* ---------- Global Variable --------------------------------- */
/* ----- Structure ----- */
FILE *fp;   /* File */

/* ----- Variable ----- */
int cbuf;   /* Character */
int cbuf_pre;   /* Previous Character */
int linenum;    /* Line Number */
int num_attr;   /* Number Attitude */
char string_attr[MAX_STR_SIZE]; /* String Attitude */

/* ---------- Function Prototype Declaration ------------------ */
int get_linenum (void); /* Get Line Number */
void shift_cbuf (void); /* Read Next Character */

/* ----- Search Struct ---------- */
int search_key ( char * str );    /* Search Key with String */
int search_symbol ( char * str ); /* Search Symbol with String */

/* ----- cbuf is cbuf ---------- */
int is_EOF (void);
int is_newline (void) ;
int is_separater (void) ;
int is_alphabet (void) ;		
int is_digit (void) ;
int is_symbol (void) ;
int is_string (void) ;
int is_left_cuparen_comment (void) ;
int is_right_cuparen_comment (void) ;
int is_start_ss_comment (void) ;
int is_end_ss_comment (void) ;

/* ----- Error Check ---------- */
int is_over_str_range (void);   /* String Out of range */
int is_over_num_range (void);   /* Number Out of range */

/* ----- Print Scan Error Message ---------- */
int error_scan (char * mes);

/* ---------- Function Definition ----------------------------- */
/* ---------- Initialize Scanner --------------- */
int init_scan(char * filepath) {
    /* Initialize line number with 0 */
    linenum = 0;

    /* Read file */
    fp = fopen(filepath, "r");  

    /* CHECK: not read file */
    if (fp == NULL) return -1;

    /* CHECK: cbuf equal EOF */
    if ( ( cbuf = fgetc(fp) ) == EOF )  return -1;

    /* Initialize previous character */
    cbuf_pre = 0;

    return 0;
}

/* ---------- Scan Token --------------- */
int scan(void) {
    int token;

    while ( 1 ) {

        /* Initialize string attr */
        memset(string_attr, '\0', sizeof(string_attr));

        /* ---------- First new line ------------------------------ */
        if (get_linenum() == 0) 
        {
            /* Line number input 1 (first line) */
            linenum = 1;
        }
        /* ---------- First new line ------------------------------ */
        /* ---------- EOF ----------------------------------------- */
        if (is_EOF()) 
        {
            /* Success finish with EOF */
            return -1;
        }
        /* ---------- EOF ----------------------------------------- */
        /* ---------- New line ------------------------------------ */
        else if (is_newline()) 
        {
            /* Line number count up */
            linenum++;

            /* Read next character */
            shift_cbuf();
            
            /* If second new-line's character, read next character */
            if (is_newline()) shift_cbuf(); 
            
            continue;
        }
        /* ---------- New line ------------------------------------ */
        /* ---------- Separater (space or tab) -------------------- */
        else if (is_separater()) 
        {
            /* Read next character */
            shift_cbuf();

            continue;
        }
        /* ---------- Separater (space or tab) -------------------- */
        /* ---------- Alphabet ------------------------------------ */
        else if (is_alphabet()) 
        {
            int i = 0;
            /* For Alphabet or Digit */
            do {
                /* Save character into string_attr */
                string_attr[i++] = cbuf;
                
                /* Read next character */
                shift_cbuf();
                
                /* CHECK: String attr out of range */
                if ( is_over_str_range() ) {
                    return error_scan("String attr is out of range in Alphabet");
                }

            } while ((is_alphabet() || is_digit()));

            /* Search Name */
            token = search_key(string_attr);

            /* If token not keyword's token, name's token */
            if ( token == -1 ) token = TNAME;  

            return token;
        }
        /* ---------- Alphabet ------------------------------------ */
        /* ---------- Digit --------------------------------------- */
        else if (is_digit()) 
        { 
            int i = 0;

            /* For Digit */
            do {
                /* Save character into string_attr */
                string_attr[i++] = cbuf;

                /* Read next character */
                shift_cbuf();

                /* CHECK: Num attr range */
                if ( is_over_num_range() ) {
                    return error_scan("Num attr is out of range in Digit");
                }

                /* CHECK: String attr range */
                if ( is_over_str_range() ) {
                    return error_scan("String attr is out of range in Digit");
                }

            } while (is_digit());

            /* Convert from string to int */
            num_attr = atoi(string_attr);

            return TNUMBER;
        }
        /* ---------- Digit --------------------------------------- */
        /* ---------- String (single quatation) ------------------- */
        else if (is_string()) 
        {
            int i = 0;
            

            while (1) {
                
                /* CHECK: New Line and EOF character not use in String */
                if (is_newline() || is_EOF()) {
                    return error_scan("New Line and EOF character not use in String.");
                }
                
                /* Read next character */
                shift_cbuf();
                
                /* Save character into string_attr */
                string_attr[i++] = cbuf;
                
                /* Combine into one string if there are consecutive single quotes */
                /* First single quotes */
                if (is_string()) {

                    /* Read next character */
                    shift_cbuf();
                    
                    /* Second single quote */
                    if (is_string()) {
                        string_attr[i++] = cbuf;
                    }
                    /* Finish if second single quote not found */
                    else {
                        string_attr[--i] = '\0';
                        break;
                    }
                }

                /* CHECK: Check String attr range*/
                if ( is_over_str_range() ) {
                    return error_scan("String attr is out of range in String");
                }
            }

            return TSTRING;
        }
        /* ---------- String (single quatation) ------------------- */
        /* ---------- Symbol -------------------------------------- */
        else if (is_symbol()) 
        {
            /* One Symbol */
            char str[] = {cbuf, '\0'};
            /* Search symbol's token */
            token = search_symbol(str);

            /* Save symbol if symbol's token found */
            if (token != -1) string_attr[0] = cbuf;
            
            /* Read next character */
            shift_cbuf();
            
            /* If symbol is any of < , > and : */
            if (token == TLE || token == TGR || token == TCOLON ) {
                
                /* Combine previous character and now character */
                char str2[] = {cbuf_pre, cbuf, '\0'};

                /* Search symbol's token */
                int tmp_token =  search_symbol(str2);
                
                /* If symbol's token found */
                if (tmp_token != -1) {
                    /* Update token */
                    token = tmp_token;
                    
                    /* Save character */
                    string_attr[1] = cbuf;

                    /* Read next character */
                    shift_cbuf();
                }
            } 

            /* CHECK: Not found Symbol token */
            if (token == -1) {
                return error_scan("Other symbol find in Symbol");
            }

            return token;
        }
        /* ---------- Symbol -------------------------------------- */
        /* ---------- Comment ------------------------------------- */
        /* ----- Curly Paren ----- */
        else if (is_left_cuparen_comment()) {
            int i = 0;

            /* Read next character */
            shift_cbuf();

            /* For not close comment character  */
            do {

                /* Save character */
                string_attr[i++] = cbuf;

                /* Read next character */
                shift_cbuf();

                /* Count up line num if new-line's character */
                if ( is_newline() ) {
                    /* Count up */
                    linenum++;
                    
                    /* Read next character */
                    shift_cbuf();
                    
                    /* If second new-line's character, read next character */
                    if (is_newline()) {

                        /* Save character */
                        string_attr[i++] = cbuf;

                        /* Read next character */
                        shift_cbuf();
                    }
                }

                /* CHECK: string range is over */
                if ( is_over_str_range() ) {
                    return error_scan("String attr is out of range in Curly Paren Comment");
                }

            } while ( !(is_right_cuparen_comment()) );

            /* Read next character */
            shift_cbuf();

            /* ===== Debug ========== */
            /* debug comment */
            // printf("Comment: %s\n", string_attr);
            /* ===== Debug ========== */

            continue;
        }
        /* ----- Star Slash ----- */
        else if (is_start_ss_comment()) {
            int i = 0;

            /* Read next character */
            shift_cbuf();

            /* For not close comment characters */
            do {

                /* Save character */
                string_attr[i++] = cbuf;
                
                /* Read next character */
                shift_cbuf();
                
                /* Count up line num if new-line's character */
                if ( is_newline() ) {
                    linenum++;

                    /* If second new-line's character, read next character */
                    if (is_newline()) {

                        /* Save character */
                        string_attr[i++] = cbuf;

                        /* Read next character */
                        shift_cbuf();
                    }
                }

                /* CHECK: string range is over */
                if ( is_over_str_range() ) {
                    return error_scan("String attr is out of range in Star Slash Comment");
                }

            } while ( !(is_end_ss_comment()) );

            /* String attr end character remove */
            int len = strlen(string_attr);

            /* Put null character at the end of string */
            string_attr[len-1] = '\0';

            /* Read next character */
            shift_cbuf();

            /* ===== Debug ========== */
            /* debug comment */
            // printf("Comment: %s\n", string_attr);
            /* ===== Debug ========== */

            continue;
        }
        /* ----- Star Slash ----- */
        /* ---------- Comment ------------------------------------- */
        /* ---------- OTHER --------------------------------------- */
        return error_scan("Can not found Token from Character");
        /* ---------- OTHER --------------------------------------- */
    }
}

/* ---------- Get Line Number --------------- */
int get_linenum(void) {
    return linenum;
}

/* ---------- End Scanner --------------- */
void end_scan(void) {   /* close file */
    fclose(fp);
}

/* ---------- Read Next Character --------------- */
void shift_cbuf(void) {
    cbuf_pre = cbuf;
    cbuf = fgetc(fp);
}

/* ---------- Search Key with String --------------- */
int search_key(char * str) {     /* Search key Token in keyword structure */
    int i;
    for (i = 0; i < NUM_OF_KEYWORD; i++) {
        if ( strcmp(key[i].keyword, str) == 0 ) {
            return key[i].keytoken;
        }
    }
    return -1;
}

/* ---------- Search Symbol with String --------------- */
int search_symbol(char * str) {  /* Search Symbol Token in keyword structure */
    int i;
    for (i = 0; i < NUM_OF_SYMBOL; i++) {
        if (strcmp(symbol[i].symbolword, str) == 0) {
            return symbol[i].symboltoken;
        }
    }
    return -1;
}

/* ----- cbuf is End Of File ---------- */
int is_EOF (void) {
    if (cbuf == EOF ) return 1;
    return 0;
}

/* ----- cbuf is New Line ---------- */
int is_newline (void) {
    if ( cbuf == '\n' || cbuf == '\r' ) return 1;
    return 0;
}

/* ----- cbuf is Space or Tab or New Line ---------- */
int is_separater (void) {
    if ( cbuf == '\r' || cbuf == '\n' || cbuf == ' ' || cbuf == '\t' ) return 1;
    return 0;
}

/* ----- cbuf is Alphabet ---------- */
int is_alphabet (void) {
    if( ( cbuf >= 'a' && cbuf <= 'z' ) || ( cbuf >= 'A' && cbuf <= 'Z' )) return 1;
    return 0;
}

/* ----- cbuf is Digit ---------- */
int is_digit (void) {
    if( cbuf >= '0' && cbuf <= '9' ) return 1;
    return 0;
}

/* ----- cbuf is Symbol ---------- */
int is_symbol (void) {
    if ( ('(' <= cbuf && cbuf <= '.') || (':' <= cbuf && cbuf <= '>') || cbuf == '[' || cbuf == ']') return 1;
    return 0;
}

/* ----- cbuf is String ---------- */
int is_string (void) {
    if ( cbuf == '\'' ) return 1;
    return 0;
}

/* ----- cbuf is Left Curly Paren Comment ---------- */
int is_left_cuparen_comment (void) {
    if ( cbuf == '{' ) return 1;
    return 0;
}

/* ----- cbuf is Right Curly Paren Comment ---------- */
int is_right_cuparen_comment (void) {
    if ( cbuf == '}' ) return 1;
    return 0;
}

/* ----- cbuf is Start Slash Star Comment ----------- */
int is_start_ss_comment (void) {
    if ( cbuf == '/' ) {
        shift_cbuf();
        if (cbuf == '*') return 1;
    }
    return 0;
}

/* ----- cbuf is End Slash Star Comment ---------- */
int is_end_ss_comment (void) {
    /* Star and Slash */
    if ( cbuf_pre == '*' && cbuf == '/' ) return 1;
    return 0;
}

/* ---------- Error Check --------------- */
/* ----- String Out of range ----------- */
int is_over_str_range(void) {
    if (strlen(string_attr) < MAX_STR_SIZE) return 0;
    return 1;
}
/* ----- Number Out of range ----------- */
int is_over_num_range(void) {
    if ( 0 <= num_attr && num_attr <= MAX_NUM_SIZE ) return 0;
    return 1;
}

/* ---------- Print Error Message in Scan Module ---------- */
int error_scan (char * mes) {
    printf("\n ERROR SCAN : line=%d , %s\n\n", get_linenum(), mes);
	end_scan();
    return -1;
}
