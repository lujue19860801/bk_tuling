#include "asr_config.h"
#if (ASR_SERVICE_TYPE==ASR_SERVICE_TULING)
#include <stdio.h> /* printf, sprintf */
#include <dirent.h>
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <sys/time.h>
#include <ctype.h>
#include <time.h>
#include "aes.h"
#include "cJSON.h"
#include "aiwifi.h"
#include "asr_common.h"
#include "board.h"

#define USE_PCM_FILE

typedef struct{
	char token[64];
	char user_id[32];
    char aes_key[32];
    char api_key[64];
	char uid[64];
}aiwifi_config;

typedef struct{
	int socket_fd;
	aiwifi_config config;
}aiwifi_client;

static aiwifi_client aiwifi_clients[AIWIFI_END];
static int aiwifi_channel = AIWIFI_COMMON;
static int g_tone = 2; // 0/1/2

int pRobot_ai_func = 0;

static char upload_head[] = 
	"POST /speech/chat HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
	"Content-Length: %d\r\n"
    "Cache-Control: no-cache\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"
	"Content-Type: multipart/form-data; boundary=%s\r\n"
    "Accept: */*\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4,zh-TW;q=0.2,es;q=0.2\r\n"
    "\r\n";

static char upload_parameters[] = 
	"Content-Disposition: form-data; name=\"parameters\"\r\n\r\n%s";

static char upload_speech[] = 
    "Content-Disposition: form-data; name=\"speech\"; filename=\"speech.wav\"\r\n"
    "Content-Type: application/octet-stream\r\n\r\n";


void get_rand_str(char s[], int number)
{
    char *str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int i,lstr;
    char ss[2] = {0};
    lstr = strlen(str);//��????��?-��??|??2��???o|
    srand((unsigned int)time((time_t *)NULL));//????����?3?????��?������??��????��???�C��????o??��??��?��???��
    for(i = 1; i <= number; i++){//??��???????���?��?��?��???????o��????-��??|??2
       sprintf(ss,"%c",str[(rand()%lstr)]);//rand()%lstr ??����????o��?��???0-71?1?������?????��??��, str[0-71]??����????o??��??��?��???-????-��??|
       strcat(s,ss);//?��?��????o?��????????-��??|??2��????��??��????????��??????��?��
    }
}

aiwifi_client* get_aiwifi_client(int aiwifi_index)
{
	if(aiwifi_index < 0 ||aiwifi_index>=AIWIFI_END)
		return NULL;
	return &aiwifi_clients[aiwifi_index];
}

int get_aiwifi_channel()
{
	return aiwifi_channel;
}

void set_aiwifi_channel(int channel)
{
	if(channel>=AIWIFI_COMMON&&channel<AIWIFI_END){
		aiwifi_channel = channel;
	}
}

void set_aiwifi_tone(int tone)
{
	if(tone >= 0 && tone <= 2)
		g_tone = tone;
}

int get_socket_fd(const char *host)
{
    int portno = 80,ret;
    int sockfd;
    int sock_opt = 1;
    int finish = 0,running = 1;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    struct timeval timeout={3,0};//3s
    
    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) printf("ERROR opening socket\n");

    /* lookup the ip address */
    server = gethostbyname(host);
    
    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
    setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));
    setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        printf("ERROR connecting\n");
    }
    else{
		printf("Connect host %s successful!\n",host);
    }
    return sockfd;
    
}


struct resp_header{    
	int status_code;//HTTP/1.1 '200' OK    
    char content_type[128];//Content-Type: application/gzip    
	long content_length;//Content-Length: 11683079    
};

static void get_resp_header(const char *response,struct resp_header *resp){    
	/*获取响应头的信息*/    
	char *pos = strstr(response, "HTTP/");    
	if (pos)        
		sscanf(pos, "%*s %d", &resp->status_code);//返回状态码    
		pos = strstr(response, "Content-Type:");//返回内容类型    
		if (pos)        
			sscanf(pos, "%*s %s", resp->content_type);    
		pos = strstr(response, "Content-Length:");//内容的长�?字节)    
		if (pos)        
			sscanf(pos, "%*s %ld", &resp->content_length);    
}

int buildRequest(int aiwifi_index, char *file_data, int len, int asr_type, int realtime, int index, char *identify,const char *host)
{
    int err = 0;
    char *boundary_header = "------AiWiFiBoundary";
    char* end = "\r\n"; 			//\u7f01\u64b3\u71ac\u93b9\u3223\ue511
    char* twoHyphens = "--";		//\u6d93\u3084\u91dc\u6769\u70b2\u74e7\u7ed7?
    int pos = 0;
	
    aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
    if(!pClient||pClient->socket_fd<0)
		return -1;
	
    char *boundary = sdram_malloc(strlen(boundary_header)+strlen(identify) +1);
	if(!boundary){
		goto OUT;
	}
    memset(boundary, 0, strlen(boundary_header)+strlen(identify) +1);
    strcat(boundary, boundary_header);
    strcat(boundary, identify);

    char firstBoundary[128]={0};
    char secondBoundary[128]={0};
    char endBoundary[128]={0};

	memset(firstBoundary,0,128);
	strcat(firstBoundary,twoHyphens);
	strcat(firstBoundary,boundary);
	strcat(firstBoundary,end);


	memset(secondBoundary,0,128);
	strcat(secondBoundary,end);
	strcat(secondBoundary,twoHyphens);
	strcat(secondBoundary,boundary);
	strcat(secondBoundary,end);

	memset(endBoundary,0,128);
	strcat(endBoundary,end);
	strcat(endBoundary,twoHyphens);
	strcat(endBoundary,boundary);
	strcat(endBoundary,twoHyphens);
	strcat(endBoundary,end);
	
    cJSON *root = cJSON_CreateObject();
	if(!root){
		goto OUT1;
	}
	cJSON_AddStringToObject(root,"ak",pClient->config.api_key);
	
	cJSON_AddNumberToObject(root,"asr", asr_type);
	cJSON_AddNumberToObject(root,"tts", 3);
    cJSON_AddNumberToObject(root,"flag", 3);
	cJSON_AddNumberToObject(root,"tone", g_tone);
    cJSON_AddStringToObject(root,"identify",identify);
    if(realtime)
    {
        cJSON_AddNumberToObject(root,"realTime", 1);
    }
    if(index)
    {
        cJSON_AddNumberToObject(root,"index", index);
    }
    
	cJSON_AddStringToObject(root,"uid", pClient->config.uid);
	cJSON_AddStringToObject(root,"token", pClient->config.token);
	char* str_js = cJSON_Print(root);
    cJSON_Delete(root);
	
    char *parameter_data = sdram_malloc(strlen(str_js)+ strlen(upload_parameters) + strlen(boundary) + strlen(end)*2 + strlen(twoHyphens) +1);
	if(!parameter_data){
		goto OUT2;
	}
	sprintf(parameter_data, upload_parameters, str_js);
    strcat(parameter_data, secondBoundary);
    
    int content_length = len+ strlen(boundary)*2 + strlen(parameter_data) + strlen(upload_speech) + strlen(end)*3 + strlen(twoHyphens)*3;
    //printf("parameter:  %s \n", parameter_data);
    
    char* header_data  = (char*)sdram_calloc(1,1024);
	if(!header_data){
		goto OUT3;
	}
    int ret = sprintf(header_data, upload_head, host, content_length, boundary);
    //header_data,boundary,parameter_data,boundary,upload_speech,fileData,end,boundary,boundary_end
    send(pClient->socket_fd, header_data, ret,0);
    send(pClient->socket_fd, firstBoundary, strlen(firstBoundary),0);
    send(pClient->socket_fd, parameter_data, strlen(parameter_data),0);
    send(pClient->socket_fd, upload_speech, strlen(upload_speech),0);

    int w_size=0,all_Size=0;
	pos = 0;
	while(1){		
		pos =send(pClient->socket_fd,file_data+w_size,len-w_size,0);
		
		if(pos>0){
			w_size +=pos;
			all_Size +=len;
			if( w_size>= len){
				w_size=0;
				break;
			}
		}else{
			err = 1;
			break;
		}
	}
    if(!err){
    	send(pClient->socket_fd, endBoundary, strlen(endBoundary),0);
    }  
	sdram_free(header_data);
OUT3: 
    sdram_free(parameter_data);
OUT2:
    sdram_free(str_js);
OUT1:
	sdram_free(boundary);
OUT:
    return err;
}

void getTulingResponse(int aiwifi_index, char **text)
{
    /* receive the response */
    char* response = (char*)sdram_calloc(1,1024);
	if(!response){
		return;
	}
    int length = 0,mem_size=1024;
    aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
    if(!pClient){
		sdram_free(response);
		return;
    }
    struct resp_header resp;
    int ret=0;
    while (1)	{	
		ret = recv(pClient->socket_fd, response+length, 1,0);
		if(ret<=0)
			return;
		//\u93b5\u60e7\u57cc\u935d\u5d85\u7c32\u6fb6\u5bf8\u6b91\u6fb6\u64ae\u5134\u6dc7\u2103\u4f05, \u6d93\u3084\u91dc"\r\n"\u6d93\u54c4\u578e\u9353\u832c\u5063		  
		int flag = 0;	
		int i;			
		for (i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		if (flag == 4)			  
			break;		  
		length += ret;	
		if(length>=mem_size-1){
			break;
		}
	}
	get_resp_header(response,&resp);
	sdram_free(response);
	//printf("status code:%d,length:%d\n",resp.status_code,resp.content_length);
	if(resp.status_code!=200||resp.content_length==0){	
		if(pClient->socket_fd>0){
			close(pClient->socket_fd);
			pClient->socket_fd = -1;
		}		
		printf("resp status code:%d\n",resp.status_code);
		return;
	}
	char *code = (char *)sdram_calloc(1,resp.content_length+1);
	if(code==NULL){
		return;
	}
	ret=0;
	length=0;

	while(1){
		ret = recv(pClient->socket_fd, code+length, resp.content_length-length,0);
		if(ret<=0){
			sdram_free(code);
			return;
		}
		length+=ret;
		//printf("result = %s len=%d\n",code,len);		
		if(length==resp.content_length)
			break;
	}
    *text = code;
}



/*
 * 更新token
 */
void updateTokenValue(int aiwifi_index,const char *token){
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	if(pClient){
		memset(pClient->config.token,0,sizeof(pClient->config.token));
		snprintf(pClient->config.token,sizeof(pClient->config.token),"%s",token);
	}
}


static const char* get_errorMsg(int error_code)
{
	int error_base = 40000;
	const char* errorMsgs[]={
		"in progress",
		"illegal value",
		"value is null or missing",
		"asr failure",
		"tts failure",
		"nlp failure",
		"token invalid",
		"is expired",
		"",
		"active interaction corpus not exist",
		"greeting corpus not exist",
		"request is forbidden",
		"out of device count limit"
	};

	if(error_code-error_base<sizeof(errorMsgs)/sizeof(errorMsgs[0]))
		return errorMsgs[error_code-error_base];
	else
		return "unknown error";
}


char* makeRequestParameters(int aiwifi_index,int type,int asr_type, int realtime, int index, char *identify)
{   
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	if(!pClient){
		return "";
	}

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"ak",pClient->config.api_key);
	cJSON_AddNumberToObject(root,"asr", asr_type);//0->16k 16bit
	cJSON_AddNumberToObject(root,"tts", 3);//4->amr_nb
    	cJSON_AddNumberToObject(root,"flag", 3);//3->output tts and asr text infor
    	if(identify)
    	{
        	cJSON_AddStringToObject(root,"identify",identify);
    	}
    	if(realtime)
    	{
        	cJSON_AddNumberToObject(root,"realTime", 1);
    	}
    	if(index)
    	{
        	cJSON_AddNumberToObject(root,"index", index);
    	}
	cJSON_AddNumberToObject(root,"type", type);
	cJSON_AddNumberToObject(root,"tone", g_tone);
	cJSON_AddStringToObject(root,"uid", pClient->config.uid);
	cJSON_AddStringToObject(root,"token", pClient->config.token);
	char* jsStr = cJSON_Print(root);
    	cJSON_Delete(root);
	return jsStr;
}

char* makeTtsRequestParameters(int aiwifi_index,char* text,int language)
{	   
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	if(!pClient){
		return "";
	}

	cJSON *root = cJSON_CreateObject();
	cJSON *parameters = cJSON_CreateObject();
	cJSON_AddStringToObject(parameters,"ak",pClient->config.api_key);
	cJSON_AddStringToObject(parameters,"text", text);
	cJSON_AddStringToObject(parameters,"uid", pClient->config.uid);
	cJSON_AddStringToObject(parameters,"token", pClient->config.token);
	cJSON_AddNumberToObject(parameters,"tts", 3);//4->amr_nb
	cJSON_AddNumberToObject(parameters,"tts_lan", language);
	cJSON_AddNumberToObject(parameters,"volume", 9);
	cJSON_AddNumberToObject(parameters,"tone", g_tone);
	cJSON_AddItemToObject(root,"parameters",parameters);
	
	char* jsStr = cJSON_Print(root);
    	cJSON_Delete(root);
	return jsStr;
}

static size_t save_response_callback(void *buffer,size_t size,size_t count,void **response)  
{  
    char * ptr = NULL;  
    ptr =(char *) sdram_malloc(count*size + 1);	
    memset(ptr,0,count*size+1);
    memcpy(ptr,buffer,count*size);  
    *response = ptr;  
  
    return count;  
}  

void socketRequest(int aiwifi_index,char *file_data, int len, int asr_type, int realtime, int index, char *identify,const char *host,char** response)
{
	int err = 0;
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	if(!pClient)
		return;
	if(pClient->socket_fd<0){
		if(index==1){
			pClient->socket_fd = get_socket_fd(host);
		}else{
			return;
		}
	}
	err = buildRequest(aiwifi_index,file_data,len,asr_type,realtime,index,identify,host);
	if(err==0){
		getTulingResponse(aiwifi_index,response);
	}else{
		printf("Socket request error\n");
		if(pClient->socket_fd>0){
			close(pClient->socket_fd);
			pClient->socket_fd = -1;
		}
	}
	if(index<0 && pClient->socket_fd>0){
		close(pClient->socket_fd);
		pClient->socket_fd = -1;
	}
}

int aiwifi_init(int aiwifi_index,const char *userId,const char *aes_key,const char *api_key,const char *token){
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	uint8_t in[17];    
	uint8_t out[64] = {'0'};	
	uint8_t aes_key_1[17];	  
	uint8_t iv[17]={0}; 	
	int i,aseLen=0;
	cJSON_Hooks cJSON_hook;
	
	if(pClient){
    	cJSON_hook.malloc_fn = sdram_malloc;
    	cJSON_hook.free_fn = sdram_free;

    	cJSON_InitHooks(&cJSON_hook);
		memset(pClient,0,sizeof(aiwifi_client));
		memcpy(pClient->config.user_id,userId,strlen(userId));
    	memcpy(pClient->config.aes_key,aes_key,strlen(aes_key));
    	memcpy(pClient->config.api_key,api_key,strlen(api_key));
		memcpy(pClient->config.token,token,strlen(token));
		pClient->socket_fd = -1;

		memset(iv,0,17);
		for(i=0;i<64;i++){
			out[i] = '0';
		}
		memcpy(in, pClient->config.user_id, strlen(pClient->config.user_id));	 
		memcpy(aes_key_1, pClient->config.aes_key, strlen(pClient->config.aes_key));	
		memcpy(iv, pClient->config.api_key, 16);		
		AES128_CBC_encrypt_buffer(out, in, 16, aes_key_1, iv);
		
		for(i=0;i < 16;i++){
			aseLen+=snprintf(pClient->config.uid+aseLen,64,"%.2x",out[i]);	 
		}
	}
	return 0;
}

void aiwifi_deinit(int aiwifi_index){	
	aiwifi_client* pClient = get_aiwifi_client(aiwifi_index);
	if(pClient){
		if(pClient->socket_fd>0){
			close(pClient->socket_fd);
			pClient->socket_fd = -1;
		}
	}
}
#endif
