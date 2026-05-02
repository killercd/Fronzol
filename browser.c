#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include "browser.h"


/** Split parameters of url to obtain protocol
 *  host and parameters ex: [http] - [www.google.com] - [?params1=a&params2=b]
 * 
 * **/
void loadrequest(Request_info *request_info, char *raw_url){
    char *str_protosep = strstr(raw_url, "://");
    char *str_paramsep = strstr(str_protosep+4, "/");
    int proto_len  = (int)(str_protosep-raw_url);

    strncpy(request_info->proto, raw_url, proto_len);
    if(str_paramsep!=0){
        strncpy(request_info->host, raw_url+proto_len+3, (str_paramsep) - (raw_url+proto_len+3));
        strncpy(request_info->url_params, str_paramsep+2, MAX_URL_PARAMS);
    }
    else{
        strncpy(request_info->host, raw_url+proto_len+3, sizeof(request_info->host)-1);
    }

    printf("PROTO: %s\n", request_info->proto);
    printf("URL: %s\n", request_info->host);
    printf("PARAMS: %s\n", request_info->url_params);


}


BRWS_STATUS getpage(Request_info *request_info, char *retdata, int maxlen){
    printf("INIT GETPAGE\n");
    char data_send[MAX_URL_LENGTH+1000];  
    char ret_buff[1024];
    struct sockaddr_in server_addr;
    int sck_connect, s_status, b_recv;
  
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);

    
    if (inet_pton(AF_INET, request_info->host, &server_addr.sin_addr) <= 0)
        return INVALID_ADDRESS;
    sck_connect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(sck_connect==-1)
       return SOCKET_ERROR;
    
    if (connect(sck_connect, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sck_connect);
        return CONNECTION_ERROR;
    }
    
    snprintf(data_send, 
            sizeof(data_send), 
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            request_info->url_params,
            request_info->host);

    s_status = send(sck_connect, data_send, strlen(data_send),0);
    if(s_status<0)
        return SEND_ERROR;

    memset(ret_buff, 0, sizeof(ret_buff));
    b_recv = recv(sck_connect,ret_buff, sizeof(ret_buff)-1, 0);
    int data_counter = 0;
    char *data_ptr = retdata;
    while(b_recv>0){
        printf("%s", ret_buff);
        b_recv = recv(sck_connect,ret_buff, sizeof(ret_buff)-1, 0);
        data_counter = data_counter+b_recv;
        //max size of string reached
        if(data_counter>=maxlen)
            break;
        for(int i=0; i<b_recv; i++){
            *data_ptr = ret_buff[i];
            data_ptr++;

        
        }
        
        printf("DATA RECEIVED %d\n", data_counter);
            
        

    }
    close(sck_connect);
    return OK;
}

void parse_error(BRWS_STATUS error){
    switch(error){
        case INVALID_ADDRESS:
            printf("Invalid Address\n");
            break;
        case SOCKET_ERROR:
            printf("Socket Error\n");
            break;
        case CONNECTION_ERROR:
            printf("Connection Error\n");
            break;
        case SEND_ERROR:
            printf("Send Error\n");
            break;
        case DATA_RECV:
            printf("Data Recv\n");
            break;
        case OK:
            printf("OK\n");
            break;
        default:
            printf("????\n");
            break;

    }
}