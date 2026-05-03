#include <stdio.h>
#include <string.h>
#include "browser.h"


void crop_new_line(char *str){
    while(*str!='\x00'){
        if(*str=='\x0D' || *str=='\x0A')
        {
            *str='\x00';
            return;
        }
        str++;

    }
}

int main(int argc, char *argv[]){

    char url[MAX_URL_LENGTH];
    char response[1024];
    
    
    Request_info r_info;
    
    memset(&r_info, 0, sizeof(r_info));

    
    //printf("URL: ");
    //fgets(url, MAX_URL_LENGTH-1, stdin);
    if(argc<2){
        printf("Params error\n");
        return -1;
    }
    strncpy(url, argv[1], MAX_URL_LENGTH-1);
    crop_new_line(url);
    loadrequest(&r_info, url);
    BRWS_STATUS ret_code = getpage(&r_info, response, 1020);
    parse_error(ret_code);
    return 0;
}