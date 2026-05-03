#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "browser.h"

#define INPUT_ROW 1
#define RESPONSE_ROW 2

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

static int terminal_rows(void)
{
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)==-1 || ws.ws_row<RESPONSE_ROW)
        return 24;

    return ws.ws_row;
}

static void setup_screen(void)
{
    int rows = terminal_rows();

    printf("\033[2J");
    printf("\033[%d;%dr", RESPONSE_ROW, rows);
    printf("\033[%d;1H", INPUT_ROW);
    printf("\033[2K");
    fflush(stdout);
}

static void reset_screen(void)
{
    printf("\033[r");
    printf("\033[%d;1H", terminal_rows());
    printf("\n");
    fflush(stdout);
}

static void draw_input_bar(void)
{
    printf("\033[%d;1H", INPUT_ROW);
    printf("\033[2K");
    printf("URL> ");
    fflush(stdout);
}

static void clear_response_area(void)
{
    printf("\033[%d;1H", RESPONSE_ROW);
    printf("\033[J");
    fflush(stdout);
}

static int should_exit(const char *url)
{
    return url[0]=='\0' || strcmp(url, "quit")==0 || strcmp(url, "exit")==0;
}

int main(void){

    char url[MAX_URL_LENGTH];

    setup_screen();

    while(1){
        char *response = NULL;
        size_t response_len = 0;
        Request_info r_info;
        BRWS_STATUS ret_code;

        memset(&r_info, 0, sizeof(r_info));
        memset(url, 0, sizeof(url));

        draw_input_bar();
        if(fgets(url, sizeof(url), stdin)==NULL)
            break;

        crop_new_line(url);
        if(should_exit(url))
            break;

        clear_response_area();
        loadrequest(&r_info, url);
        ret_code = getpage(&r_info, &response, &response_len);

        printf("\033[%d;1H", RESPONSE_ROW);
        if(ret_code==OK && response_len>0){
            fwrite(response, 1, response_len, stdout);
        }
        else{
            parse_error(ret_code);
        }
        fflush(stdout);
        free(response);
    }

    reset_screen();
    return 0;
}
