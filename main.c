#include "compiler.h"

int main (int nc, char * np[]) {
    /* input mppl file */
    if(nc < 2) {
        printf("File name id not given.\n");
        return 0;
    }

    if(init_scan(np[1]) < 0) {
        printf("File %s can not open.\n", np[1]);
        return 0;
    }
    printf("File %s can open.\n", np[1]);
    int res = parse();
    end_scan();
    printf("parse program result: %s\n", (res == ERROR) ? "ERROR" : "NORMAL");
    
    return 0;
}