#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100
#define MAX_CLNT 5
#define MAX_IP 30
 
void * handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);
char* serverState(int count);
void menu(char port[]);
 
 
 
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
 
int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;
 
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
 
    if (argc != 2)
    {
        printf(" Usage : %s <port>\n", argv[0]);
        exit(1);
    }
 
    menu(argv[1]);
 
    pthread_mutex_init(&mutx, NULL);
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
 
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");
    if (listen(serv_sock, 5)==-1)
        error_handling("listen() error");
 
    while(1)
    {
        if (clnt_cnt >= MAX_CLNT) {
            printf("더 이상 접속할 수 없습니다. 최대 허용 클라이언트 수를 초과했습니다.\n");
            continue;
        }

        t=localtime(&timer);
        clnt_adr_sz=sizeof(clnt_adr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
 
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutx);
 
        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf(" 접속한 클라이언트 IP(시간) : %s ", inet_ntoa(clnt_adr.sin_addr));
        printf("(%d-%d-%d %d:%d)\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday,
        t->tm_hour, t->tm_min);
        printf(" 접속자 수 : (%d/5)\n", clnt_cnt);
        


    }
    close(serv_sock);
    return 0;
}
 
void *handle_clnt(void *arg) 
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i;
    char msg[BUF_SIZE];
 
    while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
        send_msg(msg, str_len);
 
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
    {
        if (clnt_sock==clnt_socks[i])
        {
            while(i++<clnt_cnt-1)
                clnt_socks[i]=clnt_socks[i+1];
            break;
        }
    }

    printf("클라이언트 %d가 연결을 해제했습니다.\n", clnt_sock);

    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
 
void send_msg(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}
 
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
 
char* serverState(int count)
{
    char* stateMsg = malloc(sizeof(char) * 20);
    strcpy(stateMsg ,"None");
    
    if (count < 5)
        strcpy(stateMsg, "연결 안정");
    else
        strcpy(stateMsg, "연결 불안정");
    
    return stateMsg;
}        
 
void menu(char port[])
{
    system("clear");
    printf(" ========= 채팅 서버 상태 =========\n");
    printf(" 서버 포트      : %s\n", port);
    printf(" 서버 상태      : %s\n", serverState(clnt_cnt));
    printf(" 최대 접속자 수 : %d\n", MAX_CLNT);
    printf(" ==========   접속 로그  ==========\n\n");
}

