#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include "browser.h"

#define READ_BUFFER_SIZE 65536

static int connect_to_host(const char *host, const char *port)
{
    struct addrinfo hints, *res, *p;
    int sck_connect;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, port, &hints, &res);
    if(status!=0)
        return -1;

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
    return sck_connect;
}

static int send_all_socket(int fd, const char *data, size_t len)
{
    size_t sent = 0;

    while(sent<len){
        ssize_t ret = send(fd, data+sent, len-sent, 0);
        if(ret<=0)
            return -1;
        sent += (size_t)ret;
    }

    return 0;
}

static int send_all_ssl(SSL *ssl, const char *data, size_t len)
{
    size_t sent = 0;

    while(sent<len){
        int ret = SSL_write(ssl, data+sent, (int)(len-sent));
        if(ret<=0)
            return -1;
        sent += (size_t)ret;
    }

    return 0;
}

static char *find_body_start(char *data, size_t len)
{
    if(data==NULL || len<4)
        return NULL;

    for(size_t i=0; i<len-3; i++){
        if(data[i]=='\r' && data[i+1]=='\n' && data[i+2]=='\r' && data[i+3]=='\n')
            return data+i+4;
    }

    return NULL;
}

static int append_data(char **data, size_t *len, size_t *capacity, const char *chunk, size_t chunk_len)
{
    char *new_data;
    size_t needed;
    size_t new_capacity;

    if(chunk_len==0)
        return 0;

    needed = *len + chunk_len + 1;
    if(needed<=*capacity){
        memcpy(*data + *len, chunk, chunk_len);
        *len += chunk_len;
        (*data)[*len] = '\0';
        return 0;
    }

    new_capacity = *capacity==0 ? READ_BUFFER_SIZE : *capacity;
    while(new_capacity<needed)
        new_capacity *= 2;

    new_data = realloc(*data, new_capacity);
    if(new_data==NULL)
        return -1;

    *data = new_data;
    *capacity = new_capacity;
    memcpy(*data + *len, chunk, chunk_len);
    *len += chunk_len;
    (*data)[*len] = '\0';
    return 0;
}


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

}


BRWS_STATUS getpage(Request_info *request_info, char **retdata, size_t *retlen){
    char data_send[MAX_URL_LENGTH+1000];  
    char ret_buff[READ_BUFFER_SIZE];
    int sck_connect, b_recv;
    int use_ssl;
    const char *port;
    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;
    size_t capacity = 0;
    char *body_ptr;

    if(retdata==NULL || retlen==NULL)
        return DATA_RECV;

    *retdata = NULL;
    *retlen = 0;

    use_ssl = strcmp(request_info->proto, "https")==0;
    if(strcmp(request_info->proto, "http")!=0 && !use_ssl)
        return INVALID_ADDRESS;

    port = use_ssl ? "443" : "80";
    sck_connect = connect_to_host(request_info->host, port);
    if(sck_connect==-1)
        return CONNECTION_ERROR;

    if(use_ssl){
        OPENSSL_init_ssl(0, NULL);
        ssl_ctx = SSL_CTX_new(TLS_client_method());
        if(ssl_ctx==NULL){
            close(sck_connect);
            return CONNECTION_ERROR;
        }
        SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_3_VERSION);
        SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_3_VERSION);

        ssl = SSL_new(ssl_ctx);
        if(ssl==NULL){
            SSL_CTX_free(ssl_ctx);
            close(sck_connect);
            return CONNECTION_ERROR;
        }

        SSL_set_tlsext_host_name(ssl, request_info->host);
        SSL_set_fd(ssl, sck_connect);

        if(SSL_connect(ssl)<=0){
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
            close(sck_connect);
            return CONNECTION_ERROR;
        }
    }

    snprintf(data_send,
            sizeof(data_send),
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: Fronzol/1.0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Connection: close\r\n"
            "\r\n",
            request_info->url_params,
            request_info->host);

    if(use_ssl){
        if(send_all_ssl(ssl, data_send, strlen(data_send))<0){
            SSL_free(ssl);
            SSL_CTX_free(ssl_ctx);
            close(sck_connect);
            return SEND_ERROR;
        }
    }
    else if(send_all_socket(sck_connect, data_send, strlen(data_send))<0){
        close(sck_connect);
        return SEND_ERROR;
    }

    while((b_recv = use_ssl ? SSL_read(ssl, ret_buff, sizeof(ret_buff)-1) :
                              recv(sck_connect, ret_buff, sizeof(ret_buff)-1, 0))>0){
        if(append_data(retdata, retlen, &capacity, ret_buff, (size_t)b_recv)<0){
            if(ssl!=NULL){
                SSL_free(ssl);
            }
            if(ssl_ctx!=NULL)
                SSL_CTX_free(ssl_ctx);
            close(sck_connect);
            free(*retdata);
            *retdata = NULL;
            *retlen = 0;
            return DATA_RECV;
        }
    }

    if(ssl!=NULL){
        SSL_free(ssl);
    }
    if(ssl_ctx!=NULL)
        SSL_CTX_free(ssl_ctx);
    close(sck_connect);

    if(b_recv<0)
        return DATA_RECV;

    body_ptr = find_body_start(*retdata, *retlen);
    if(body_ptr!=NULL){
        size_t body_len = *retlen - (size_t)(body_ptr - *retdata);

        memmove(*retdata, body_ptr, body_len);
        *retlen = body_len;
        (*retdata)[*retlen] = '\0';
    }

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
            break;
        default:
            printf("????\n");
            break;

    }
}
