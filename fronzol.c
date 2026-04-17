#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>  

const int MAX_URL_LENGTH = 2048;

int getPage(char *url);

int getPage(char *url){
    
    char dataSend[MAX_URL_LENGTH+1000];  
             
    int sckConnect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sckConnect==-1){
        printf("Socket error\n");
        return -1;
    }
    //connect();
    snprintf(dataSend, 
            sizeof(dataSend), 
            "GET /%s HTTP/1.1\r\n",
            "Host: test\r\n",
            "Connection: close\r\n"
            "\r\n",
            url);

    int sStatus = send(sckConnect, dataSend, sizeof(dataSend),0);
    close(sckConnect);
    return 0;
}

int main(int argc, char* argv[]){

    char url[MAX_URL_LENGTH];
    
    printf("Url: ");
    fgets(url, MAX_URL_LENGTH-1, stdin);
    

    return 0;
}