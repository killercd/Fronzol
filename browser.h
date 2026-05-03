#ifndef _FRZ_BROWSER_
#define _FRZ_BROWSER_


#define MAX_HOST_LENGTH 1024
#define MAX_URL_PARAMS 1024

#define MAX_URL_LENGTH 2048

#include <netdb.h>

typedef enum {
    INVALID_ADDRESS = 0,
    SOCKET_ERROR = 1,
    CONNECTION_ERROR = 2,
    SEND_ERROR = 3,
    DATA_RECV = 4,
    OK = 5

} BRWS_STATUS;


typedef struct Request_info {
    char proto[10];
    char host[MAX_HOST_LENGTH];
    char url_params[MAX_URL_PARAMS];
} Request_info;



void loadrequest(Request_info *request_info, char *raw_url);
BRWS_STATUS getpage(Request_info *request_info, char *retdat, int maxlen);
void parse_error(BRWS_STATUS error);


#endif
