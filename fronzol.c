#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>  

const int MAX_URL_LENGTH = 2048;

void getPage(char *url);



void getPage(char *url){
    socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int main(int argc, char* argv[]){

    char url[MAX_URL_LENGTH];
    
    printf("Url: ");
    fgets(url, MAX_URL_LENGTH-1, stdin);
    printf("URL IS %s\n", url);

    return 0;
}