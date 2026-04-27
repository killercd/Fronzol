#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>  
#include <arpa/inet.h>
#include <string.h>

const int MAX_URL_LENGTH = 2048;


void cropNewLine(char *str){
    while(*str!='\x00'){
        if(*str=='\x0D' || *str=='\x0A')
        {
            *str='\x00';
            return;
        }
        str++;

    }
}

int getPage(char *url){
    
    
    char dataSend[MAX_URL_LENGTH+1000];  
    struct sockaddr_in server_addr;
    int sckConnect;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);

    
    if (inet_pton(AF_INET, url, &server_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 0;
    }
    sckConnect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(sckConnect==-1){
        printf("Socket error\n");
        return 0;
    }
    if (connect(sckConnect, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection error\n");
        close(sckConnect);
        return 0;
    }
    char page[]="robots.txt";
    snprintf(dataSend, 
            sizeof(dataSend), 
            "GET /%s HTTP/1.1\r\n"
            "Host: test\r\n"
            "Connection: close\r\n"
            "\r\n",
            page);

    int sStatus = send(sckConnect, dataSend, strlen(dataSend),0);
    char retBuffer[1024];
    memset(retBuffer, 0, sizeof(retBuffer));
    int bRecv = recv(sckConnect,retBuffer, sizeof(retBuffer)-1, 0);
    while(bRecv>0){
        printf("%s", retBuffer);
        bRecv = recv(sckConnect,retBuffer, sizeof(retBuffer)-1, 0);
    }
    close(sckConnect);
    return 1;
}

int main(int argc, char* argv[]){

    char url[MAX_URL_LENGTH];
    
    printf("URL: ");
    fgets(url, MAX_URL_LENGTH-1, stdin);
    cropNewLine(url);
    getPage(url);

    return 0;
}