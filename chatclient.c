
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100
#define NORMAL_SIZE 20
 
void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
 
void menu();
void changeName();
void menuOptions(); 
 
char name[NORMAL_SIZE]="[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];        // server port number
char clnt_ip[NORMAL_SIZE];            // client ip address
 
 
int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
 
    if (argc!=4)
    {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
        exit(1);
    }
 
    /** local time **/
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
    sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
    t->tm_min);
 
    sprintf(name, "[%s]", argv[3]);
    sprintf(clnt_ip, "%s", argv[1]);
    sprintf(serv_port, "%s", argv[2]);
    sock=socket(PF_INET, SOCK_STREAM, 0);
 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
 
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling(" conncet() error");
 
    /** call menu **/
    menu();
 
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}
 
void* send_msg(void* arg)
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];
 
    /** send join messge **/
    printf(" >> 채팅에 입장하였습니다. \n");
    sprintf(myInfo, "%s 님이 채팅에 참여하엿습니다.(IP : %s)\n",name , clnt_ip);
    write(sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // menu_mode command -> !menu
        if (!strcmp(msg, "!menu\n"))
        {
            menuOptions();
            continue;
        }
 
        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            close(sock);
            exit(0);
        }
 
        // send message
        sprintf(name_msg, "%s %s", name,msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}
 
void* recv_msg(void* arg)
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    int str_len;
 
    while(1)
    {
        str_len=read(sock, name_msg, NORMAL_SIZE+BUF_SIZE-1);
        if (str_len==-1)
            return (void*)-1;
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
 
 
void menuOptions() 
{
    int select;
    // print menu
    printf("\n\t*============ 메뉴 모드 ============\n");
    printf("\t1. 대화명 바꾸기\n");
    printf("\t2. 새로고침 \n\n");
    printf("\t다른 키를 누르면 메뉴 모드를 취소합니다.");
    printf("\n\t=====================================");
    printf("\n\t>> ");
    scanf("%d", &select);
    getchar();
    switch(select)
    {
        // change user name
        case 1:
        changeName();
        break;
 
        // console update(time, clear chatting log)
        case 2:
        menu();
        break;
 
        // menu error
        default:
        printf("\tcancel.");
        break;
    }
}
 
 
/** change user name **/
void changeName()
{
    char nameTemp[100];
    printf("\n\t새로운 대화명을 입력하세요. -> ");
    scanf("%s", nameTemp);
    sprintf(name, "[%s]", nameTemp);
    printf("\n\t새로고침 완료.\n\n");
}
 
void menu()
{
    system("clear");
    printf(" ========= 클라이언트 정보 =========\n");
    printf(" 서버 포트     : %s \n", serv_port);
    printf(" 클라이언트 IP : %s \n", clnt_ip);
    printf(" 대화명        : %s \n", name);
    printf(" 서버 시간     : %s \n", serv_time);
    printf(" ============== 메뉴 ==============\n");
    printf(" !menu 입력 시 메뉴 화면으로 이동 \n");
    printf(" 1. 대화명 바꾸기 \n");
    printf(" 2. 화면 새로고침 \n");
    printf(" ==================================\n");
    printf(" 종료하려면 q & Q 를 입력하세요\n\n");
}    
 
void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

