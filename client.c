#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <unistd.h>
#include<string.h>
#include<netinet/ip.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
char playerID[16] = "";
char userid[16]="";
int rate[3] = {0,};
int connEnd = 0;
int parse_command(char *command, int *nr_tokens, char *tokens[]);
int exitCheck(char* buf,int socket){
	if(buf[0] == 'e' && buf[1] == 'x' && buf[2]=='i' && buf[3]=='t' && buf[4]=='\n'){
		printf("exit\n");
		return 1;
	}
	else
		return 0;
}
void showBanner(){
	char banner[600]="";
	FILE* fp = fopen("banner.txt","r");
	if(fp != NULL){
		fread(banner, 1, 600, fp);
		printf("%s\n",banner);
		fclose(fp);
	}
	else{
		printf("cannot banner file..\n");
	}
}
void showHelp(){
	printf("help!\n");	
}
void showMenu(){
	printf("\n==================\n");
	printf("$ join\t -> join the game..\n");
	printf("$ help\t -> show help\n");
	printf("$ menu\t -> show menu\n");
	printf("$ set [number]\t->write scores..\n");
	printf("$ [dicenum] [dicenum] [dicenum] [dicenum] [dicenum] -> re roll dice\n 0 != reroll\n");
}
void *recv_server(void* p){
	int* c = (int*)p;
	int client = *c;
	char bye[8] = "byebye\n";
	while(1){
		char recv[1025]="";
		read(client,recv,1024);
		recv[1024] = '\0';
		
		if(strcmp(recv,bye) == 0){
			printf("socket end@@ \n");
			close(client);
			break;
		}
		if(strcmp(recv,"[!] your turn\n") == 0){
			printf("my turnturnturnturn\n");
		}
		printf("\n=============\n");
		printf("\n%s\n",recv);
		printf("=============@\n\n%s$ ",playerID);
		
	}
	printf("thread end\n");
	connEnd = 1;
}
int main(int argc,char **argv){
	showBanner();
	struct sockaddr_in server;
	memset(&server,0,sizeof(server));
	printf("[@] %s start!\n",argv[0]);
	int client = socket(PF_INET, SOCK_STREAM, 0);
	if(client == -1){
		fprintf(stderr,"client socket err\n");
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(7777);
	if(connect(client,(struct sockaddr*)&server, sizeof(server))== -1){
		fprintf(stderr, "conn err\n");
	}
	else{
		char buf[1025];
		char* userinfos[16];
		printf("id : ");
		scanf("%s",userid);
		write(client,userid,strlen(userid));
		read(client,buf,1024);
		int nr =0;
		parse_command(buf,&nr,userinfos);
		strcpy(playerID,userinfos[0]);
		rate[0] = atoi(userinfos[1]);
		rate[1] = atoi(userinfos[2]);
		rate[2] = atoi(userinfos[3]);
		//playerID = atoi(idtok);
		printf("playerID : %s\n",playerID);
		sleep(1);
		pthread_t recv_t;
		pthread_create(&recv_t, NULL, recv_server, (void*)&client);
		while(1){
			char input[1000];
			sleep(1);
			fgets(input,999,stdin);
			fflush(stdin);
			fflush(stdout);
			input[999] = '\0';
			if(connEnd){
				break;
			}
			if(strcmp(input,"help\n") == 0){
				showHelp();
			}
			else if(strcmp(input,"menu\n") == 0){
				showMenu();
			}
			else if(!exitCheck(input, client)){
				write(client,input,strlen(input));
			}
			else{
				break;
			}
		}
	}
	return 0;
}
int parse_command(char *command, int *nr_tokens, char *tokens[]){
    const char *delimiters = " \t\r\f\r\n\v";
    char *curr;
    *nr_tokens = 0;
    while ((curr = strtok(command, delimiters))) {
        *tokens++ = strdup(curr);
        (*nr_tokens)++;
        command = NULL;
    }
    *tokens = NULL;
    return (*nr_tokens > 0);
}
