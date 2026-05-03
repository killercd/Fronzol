#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "browser.h"


/** Split parameters of url to obtain protocol
 *  host and parameters ex: [http] - [www.google.com] - [?params1=a&params2=b]
 * 
 * **/
void loadrequest(Request_info *request_info, char *raw_url){
    
    char *str_protosep = strstr(raw_url, "://");
    char *host_start;
    char *str_paramsep;
    int proto_len;

    memset(request_info, 0, sizeof(*request_info));

    if(str_protosep==NULL)
        return;

    host_start = str_protosep+3;
    str_paramsep = strstr(host_start, "/");
    proto_len  = (int)(str_protosep-raw_url);
    
    if(proto_len >= (int)sizeof(request_info->proto))
        proto_len = (int)sizeof(request_info->proto)-1;

    strncpy(request_info->proto, raw_url, proto_len);
    if(str_paramsep!=0){
        size_t host_len = (size_t)(str_paramsep - host_start);

        if(host_len >= sizeof(request_info->host))
            host_len = sizeof(request_info->host)-1;

        strncpy(request_info->host, host_start, host_len);
        strncpy(request_info->url_params, str_paramsep+1, sizeof(request_info->url_params)-1);
    }
    else{
        strncpy(request_info->host, host_start, sizeof(request_info->host)-1);
    }

    printf("PROTO: %s\n", request_info->proto);
    printf("URL: %s\n", request_info->host);
    printf("PARAMS: %s\n", request_info->url_params);
}


BRWS_STATUS getpage(Request_info *request_info, char *retdata, int maxlen){
    printf("INIT GETPAGE\n");
    char data_send[MAX_URL_LENGTH+1000];  
    char ret_buff[1024];
    struct addrinfo hints, *res, *p;
    int sck_connect, s_status, b_recv;
    int status;
    int data_counter = 0;
    char *data_ptr = retdata;

    memset(retdata, 0, (size_t)maxlen);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(request_info->host, "80", &hints, &res);
    if(status!=0)
        return INVALID_ADDRESS;

    sck_connect = -1;
    for(p=res; p!=NULL; p=p->ai_next){
        sck_connect = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sck_connect==-1)
            continue;

        if(connect(sck_connect, p->ai_addr, p->ai_addrlen)==0)
            break;

        close(sck_connect);
        sck_connect = -1;
    }

    freeaddrinfo(res);

    if(sck_connect==-1)
        return CONNECTION_ERROR;
    
    snprintf(data_send, 
            sizeof(data_send), 
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            request_info->url_params,
            request_info->host);

    s_status = send(sck_connect, data_send, strlen(data_send),0);
    if(s_status<0){
        close(sck_connect);
        return SEND_ERROR;
    }

    while((b_recv = recv(sck_connect, ret_buff, sizeof(ret_buff)-1, 0))>0){
        ret_buff[b_recv] = '\0';
        printf("%s", ret_buff);

        //max size of string reached
        if(data_counter>=maxlen)
            break;

        for(int i=0; i<b_recv && data_counter<maxlen-1; i++){
            *data_ptr = ret_buff[i];
            data_ptr++;
            data_counter++;
        }
    }

    *data_ptr = '\0';
    close(sck_connect);

    if(b_recv<0)
        return DATA_RECV;

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
