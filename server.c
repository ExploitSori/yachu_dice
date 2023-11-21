#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/ip.h>
#include<time.h>
#include<netinet/in.h>
#include<pthread.h>
#include<unistd.h>
#include "types.h"
#include "list_head.h"
#include <stdlib.h>
#define PORT 7777
int server_socket=-1;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int clients[7];
struct socketInfo* ready = NULL;
//int clientSockets[7];
int clientCnt = 0;
int clientSocketCnt =-1;
int room[1][2] = {{-1,-1}};
char* scoresName[20] = {"one\t\t1\t","two\t\t2\t","three\t\t3\t","four\t\t4\t","five\t\t5\t","six\t\t6\t","three_of_a_kind\t7\t","four_of_a_kind\t8\t","full_house\t9\t","small_straight\t10\t","large_straight\t11\t","chance\t\t12\t","yahtzee!\t13\t","bonus\t\t\t","sum\t\t\t"};
pthread_mutex_t mutex;//es[1];
pthread_mutex_t fileMutex;
//pthread_mutex_t turn; 
struct turn play;
struct list_head head;
int parse_command(char *command, int *nr_tokens, char *tokens[]);
void showScore(struct socketInfo* user);
void calcWin(struct socketInfo* c);
void showUserInfo(struct socketInfo* user);
void updateUserInfo(struct socketInfo* user);
LIST_HEAD(head);
void ujoin(struct socketInfo* c,char* userid){
	pthread_mutex_lock(&fileMutex);
	printf("uuuu\n");
	FILE* fp = fopen("users.txt","a+");
	printf("ujoin\n");
	fprintf(fp,"%s 0 0 0\n",userid);
	fclose(fp);
	strcpy(c->infos.name,userid);
	c->infos.win = 0;
	c->infos.draw = 0;
	c->infos.lose = 0;	
	printf("joinend\n");
	pthread_mutex_unlock(&fileMutex);
}

void login(struct socketInfo* c , char* userid){
	char line[600]="";
	char* data[100];
	int nr =0;
	FILE* fp = fopen("users.txt","r+");
	int flag = 1;
	printf("file open\n");
	if(fp != NULL){
		while (fgets(line, sizeof(line), fp) != NULL ) {
			parse_command(line,&nr,data);
			if(strcmp(data[0],userid) == 0){
				strcpy(c->infos.name,data[0]);
				c->infos.win = atoi(data[1]);
				c->infos.draw = atoi(data[2]);
				c->infos.lose = atoi(data[3]);
				flag = 0;
			}
		}
		printf("login end\n");
		fclose(fp);
		if(flag){
			ujoin(c,userid);
		}
	}
	else{
		printf("null file\n");
		printf("fclose\n");
		ujoin(c,userid);
		printf("ujoin in\n");
	}
}
int makeSocket(){
	int socket_fd;
	socket_fd= socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1){
		fprintf(stderr, "socket fd err\n");
		exit(1);
	}
	return socket_fd;
}
void printClient(){
	struct socketInfo* p = NULL;
	printf("====================\n[");
	list_for_each_entry(p,&head,list){
		if(p->fd != 3)
			printf(" | %d | ",p->fd);
	}
	printf("]\n===================\n");
}
int rollDice(int w){
	int dice = -1;
	int ran = rand()+w;
	dice = ran%6+1;
	return dice;
}
int exitCheck(char* buf){
	char* exitstr="exit";
	if(strcmp(buf,exitstr) == 0 ){
		struct socketInfo* pos;
		list_for_each_entry_reverse(pos, &head,list){
			strcpy(buf,"byebye\n");
	                write(pos->fd, buf, strlen(buf));
                	close(pos->fd);
		}

		return 1;
	}
	return 0;
}
void clientSend(struct socketInfo* c,char* str){
    char buf[1025];
	strcpy(buf,str);
	buf[1024] = '\0';
	write(c->fd, buf, strlen(buf));
}
void clientRecv(struct socketInfo* c){
	char buf[1025];
	read(c->fd,buf,1024);
	buf[1024] = '\0';
	printf("id[%d] : %s\n",c->id, buf);
}
void first(struct socketInfo* c, char *buf){
	char msg[100] = "";
	c->dices[0] = rollDice(3);
	c->dices[1] = rollDice(3);
	c->dices[2] = rollDice(3);
	c->dices[3] = rollDice(3);
	c->dices[4] = rollDice(3);
	sprintf(msg,"first dice :%d %d %d %d %d",c->dices[0], c->dices[1], c->dices[2], c->dices[3],c->dices[4]);
	printf("%s\n",msg);
	clientSend(c,msg);
}
int reroll(struct socketInfo* c, char *buf){
	printf("reroll \n");
	char* flag[5] = {"",};
	int flagIdx = 0;
	char msg[100] = "";
	char nullStr[2] = "1";
	if(strcmp(buf,"end\n") == 0){
		printf("return\n");
		sprintf(msg,"dice :%d %d %d %d %d\n",c->dices[0], c->dices[1], c->dices[2], c->dices[3],c->dices[4]);
		clientSend(c,msg);
		return 1;
	}
	else{
	parse_command(buf, &flagIdx, flag);
	printf("buf : %s\n",buf);
	printf("%d %s %s %s %s %s\n",flagIdx,flag[0],flag[1],flag[2],flag[3],flag[4]);
	printf("rerollllll\n");
	for(int step = 0;step<5;step++){
		printf("[@] catch %s @\n",flag[step]);
		if(flag[step] == NULL){
			printf("NULL check\n");
			//strcpy(flag[step],"0");
			flag[step] = nullStr;
		}
		if(strcmp(flag[step],"0") != 0){
			printf("change change change\n");
			c->dices[step] = rollDice(3);
		}
		else{
			printf("%s %s %d\n",flag[step],"0",strcmp(flag[step],"0") != 0);
			printf("heheheheheh\n");
		}
	}
	sprintf(msg,"reroll dice :%d %d %d %d %d\n",c->dices[0], c->dices[1], c->dices[2], c->dices[3],c->dices[4]);
	printf("%s\n",msg);
	clientSend(c,msg);
	}
	return 0;
}
int calcScore(struct socketInfo* c, int num){
	int s = 0;
	if(num == 1){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 1){
				s += 1;
			}
		}
	}
	if(num == 2){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 2){
				s += 2;
			}
		}
	}
	if(num == 3){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 3){
				s += 3;
			}
		}
	}
	if(num == 4){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 4){
				s += 4;
			}
		}
	}
	if(num == 5){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 5){
				s += 5;
			}
		}
	}
	if(num == 6){
		for(int i=0;i<5;i++){
			if(c->dices[i] == 6){
				s += 6;
			}
		}
	}
	if(num == 7){//three of a kind
		int cnt[6] = {0};
		int flag = 0;
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		for(int i=0;i<6;i++){
			if(cnt[i] == 3){
				flag = 1;
			}
		}
		if(flag){
			for(int i=0;i<5;i++){
				s += c->dices[i];
			}
		}
		else{
			s = 0;
		}
	}
	if(num == 8){//four of a kind
		int cnt[6] = {0};
		int flag = 0;
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		for(int i=0;i<6;i++){
			if(cnt[i] == 4){
				flag = 1;
			}
		}
		if(flag){
			for(int i=0;i<5;i++){
				s += c->dices[i];
			}
		}
		else{
			s = 0;
		}
	}
	if(num == 9){//full house
		int cnt[5] = {6};
		int flag1 = 0;
		int flag2 = 0;
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		for(int i=0;i<5;i++){
			if(cnt[i] == 3){
				flag1 = 1;
			}
			if(cnt[i] == 2){
				flag2 = 1;
			}
		}
		if(flag1 == 1 && flag2 == 1){
			s = 25;
		}
		else{
			s = 0;
		}
	}
	if(num == 10){//small straight
		int cnt[6] = {0};
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		if(cnt[0]==1 && cnt[1]==1 &&cnt[2]==1 &&cnt[3]==1){//1234
			s = 30;
		}
		if(cnt[1]==1 && cnt[2]==1 &&cnt[3]==1 &&cnt[4]==1){//2345
			s = 30;
		}
		if(cnt[2]==1 && cnt[3]==1 &&cnt[4]==1 &&cnt[5]==1){//,3456
			s = 30;
		}
	
	
	}
	if(num == 11){//large straight
		int cnt[6] = {0};
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		if(cnt[0]==1 && cnt[1]==1 &&cnt[2]==1 &&cnt[3]==1 && cnt[4] ==1){//12345
			s = 40;
		}
		if(cnt[1]==1 && cnt[2]==1 &&cnt[3]==1 &&cnt[4]==1 && cnt[5] ==1){//23456
			s = 40;
		}
	
	}
	if(num == 12){//chance
		for(int i=0;i<5;i++){
			s += c->dices[i];
		}
	}
	if(num == 13){//yahtzee
		int cnt[5] = {0};
		int flag = 0;
		for(int i=0;i<5;i++){
			for(int j=1;j<=6;j++){
				if(c->dices[i] == j){
					cnt[j-1] += 1;
				}
			}
		}
		for(int i=0;i<5;i++){
			if(cnt[i] == 5){
				flag = 1;
			}
		}
		if(flag){
			s = 50;
		}
		else{
			s = 0;
		}
	}
	return s;
}

int selectScore(struct socketInfo* c, char *buf){
	char* s[10];
	int cnt = 0;
	int ret = 0;
	int tmp = 0;
	printf("select score\n");
	parse_command(buf,&cnt,s);
	if(cnt == 2){
		printf("set score : %d\n",c->scores[0]);
		printf("s[0] = %s\n",s[0]);
		if(strcmp(s[0], "set") == 0){
			int num = atoi(s[1]);
			printf("num : %d\n",num);
			if(num <= 13 && num>= 1){
				if(c->scores[num-1] == -1){
					c->scores[num-1] = calcScore(c,num);
					c->scores[14] += calcScore(c,num);
					for(int i=0;i<6;i++){//bonus calc
						if(c->scores[i] == -1){
							tmp += 0;
						}
						else{
							tmp += c->scores[i];
						}
					}
					if(c->scores[13] == -1 && tmp >= 63){
						c->scores[13] = 35;
						c->scores[14] += 35;
					}
					ret = 1;
				}
				else{
					ret = 0;
				}
			}
			else{
				ret = 0;
			}
		}
		else{
			ret = 0;
		}
	}
	else{
		ret = 0;
	}
	return ret;
}
void cmd(struct socketInfo* c, char *buf){
	int endflag=1;
	if(c->vs == NULL){
			printf("[%d] no match.. \n",c->id);
			clientSend(c,"[%d] no match.. \n");
	}
	else if(play.id == c->id && (c->vs != NULL)){
		pthread_mutex_lock(&play.turn);
		if(c->step  == 0){
			first(c,buf);
			c->step++;
		}
		else{
			if(c->step == 1){
				int res = reroll(c,buf);
				printf("res : %d\n",res);
				if(res){
					c->step = 3;
				}
				else{
					c->step++;
				}
			}
			else if(c->step == 2){
				reroll(c,buf);
				c->step++;
			}
			else if(c->step >= 3){
				int res = selectScore(c,buf);
				if(res){				
					c->step = 0;
					play.id = c->vs->id;
					clientSend(c, "[!] turn end\n");
					clientSend(c->vs, "[!] your turn\n");
					c->dices[0] = -1;
					for(int u=0;u<13;u++){//step 13
						if(c->scores[u] == -1 || c->vs->scores[u] == -1){
							endflag = 0;
						}
					}
					if(endflag){
						printf("@@@@calc win\n");
						calcWin(c);
					}
				}
				else{
					clientSend(c, "[!] re try!\n");
					printf("re select\n");
				}
			
			}
			else{
				printf("c->step : %d\n",c->step);
				printf("err\n");
			}
		}
		printf("id [%d]: %s\n",c->id,buf);
		pthread_mutex_unlock(&play.turn);
	}
	else{
			printf("[%d] no your turn \n",c->id);
			clientSend(c,"[%d] no your turn \n");
	}
}
void sockSet(int port){
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}
void join(struct socketInfo* c){
	pthread_mutex_lock(&mutex);
	if(ready == NULL){
		if(play.id != -1){
			clientSend(c, "played game\n");
		}
		else{
			ready = c;
			clientSend(c, "[!] join complate..\n");
		}
	}
	else{
		if(c->id != ready->id){
			ready->vs = c;
			c->vs = ready;
			clientSend(ready, "[!] your 1p your turn!\n");
			clientSend(c, "[!] your 2p\n");
			play.id = ready->id;
			ready = NULL;
			for(int i=0;i<13;i++){
					c->scores[i] = -1;
					c->vs->scores[i] = -1;
			}//sori
		}
		else{
			clientSend(c, "[!] waitting plz.\n");
		}
	}
	pthread_mutex_unlock(&mutex);
}
void showScore(struct socketInfo* user){
	char msg[1024];
	printf("player \t\tnum\t|\t%s\t|\t%s\t\n",user->infos.name,user->vs->infos.name);
	sprintf(msg,"player \t\tnum\t|\t%s\t|\t%s\t\n",user->infos.name,user->vs->infos.name);
	for(int i=0;i<15;i++){
		int view1 = user->scores[i];
		int view2 = user->vs->scores[i];
		if(view1 == -1){
//			view1 = 0;
		}
		if(view2 == -1){
//			view2 = 0;
		}
		printf("%s|\t%d\t|\t%d\t\n",scoresName[i],view1,view2);
		sprintf(msg,"%s%s|\t%d\t|\t%d\t\n",msg,scoresName[i],view1,view2);
	}
	clientSend(user, msg);

}
void showUserInfo(struct socketInfo* user){
	char msg[1000];
	sprintf(msg, "%s %d %d %d\n",user->infos.name, user->infos.win, user->infos.draw, user->infos.lose);
	if(user->vs != NULL){
		sprintf(msg, "%s%s %d %d %d\n",msg,user->vs->infos.name, user->vs->infos.win, user->vs->infos.draw, user->vs->infos.lose);
	}
	clientSend(user,msg);
}
void updateUserInfo(struct socketInfo* user){
	pthread_mutex_lock(&fileMutex);
	FILE* fp = fopen("users.txt","r+");
	char update[2000] = "";
	char *tokens[30];
	int nr=0;
	char line[100];
	if(fp != NULL){
		while (fgets(line, sizeof(line), fp) != NULL ) {
			parse_command(line,&nr,tokens);
			if(strcmp(tokens[0],user->infos.name) == 0){
				sprintf(update, "%s%s %d %d %d\n",update,user->infos.name,user->infos.win,user->infos.draw,user->infos.lose);
			}
			else if(strcmp(tokens[0],user->vs->infos.name) == 0){
				sprintf(update, "%s%s %d %d %d\n",update,user->vs->infos.name,user->vs->infos.win,user->vs->infos.draw,user->vs->infos.lose);
			}
				
			else{
				sprintf(update, "%s%s %s %s %s\n",update,tokens[0],tokens[1],tokens[2],tokens[3]);
			}
        }
		fclose(fp);
	}
	printf("update : %s",update);
	fp = fopen("users.txt", "w+");
	fputs(update, fp);
	fclose(fp);
	pthread_mutex_unlock(&fileMutex);
}
void *printWorker(void* p){
	struct socketInfo* clientEntry = (struct socketInfo*)p;
	struct socketInfo* vs = NULL;
	printf("print worker %d\n",clientEntry->fd);
	char msg[100] = "";
	char userid[16]="";
	read(clientEntry->fd,userid,sizeof(userid));
	printf("uid : %s\n",userid);
	login(clientEntry,userid);
	sprintf(msg,"%s %d %d %d\n",clientEntry->infos.name,clientEntry->infos.win,clientEntry->infos.draw,clientEntry->infos.lose);
	write(clientEntry->fd, msg, strlen(msg));

	while(1){
		char buf[1025] = {0};
		read(clientEntry->fd,buf,1024);
		buf[1024] = '\0';
		if(buf[0] == 'e' && buf[1] == 'x' && buf[2]=='i' && buf[3]=='t' && buf[4]=='\n'){
			printf("[!] %d disconnected\n",clientEntry->fd);
			strcpy(buf,"byebye\n");
			write(clientEntry->fd, buf, strlen(buf));
			close(clientEntry->fd);
			list_del(&clientEntry->list);
			clientCnt--;
			free(clientEntry);
			if(clientEntry->vs == NULL){
				ready = NULL;
			}
			printClient();
			break;
		}
		if(buf[0] !='\0'){
			if(strcmp(buf,"show\n") == 0 && play.id != -1){
				showScore(clientEntry);
			}
			else if(strcmp(buf,"join\n") == 0){
				join(clientEntry);
			}
			else if(strcmp(buf,"userinfo\n") == 0){
				showUserInfo(clientEntry);
			}
			else {
				cmd(clientEntry,buf);
			}

		}
		else{
			printf("%c",buf[0]);
		}
	}
}
void *run_go(void* p){
	int* port = (int*)(p);
	printf("server on %d\n",*port);
	play.id = -1;
	server_socket = makeSocket();
	sockSet(*port);
	if(-1 == bind(server_socket,(struct sockaddr*)&server_addr, sizeof(server_addr))){
		fprintf(stderr,"bind error\n");
		exit(1);
	}
	if(-1 == listen(server_socket, 5)){
		fprintf(stderr,"listen err\n");
		exit(1);
	}
	int client_addr_size;
	client_addr_size = sizeof(client_addr);
	fd_set reads, cpy_reads;
	int maxi,maxfd,fd_num;
	struct timeval timeout; 
	int client;
	maxfd = server_socket;
	// socket linked list
	struct socketInfo* serverEntry = (struct socketInfo*) malloc(sizeof(struct socketInfo));
	INIT_LIST_HEAD(&serverEntry->list);
	serverEntry->fd = server_socket;
	list_add_tail(&serverEntry->list,&head);
	// socket linked list end
	FD_ZERO(&reads);
	FD_SET(server_socket, &reads);
	pthread_t threads[7];
	int threadIdx = 0;
	while(1){
		cpy_reads = reads;
		timeout.tv_sec=5;
		timeout.tv_usec=5000;
		struct socketInfo* last = list_last_entry(&head,struct socketInfo,list);
		fd_num = select(maxfd+1, &cpy_reads, 0, 0, &timeout);
		if(fd_num == -1){
			fprintf(stderr,"fd_errrrr\n");
			maxfd = server_socket;
			continue;
		}
		else if(fd_num == 0){
			continue;
		}
		int i=0;
		for(;i<maxfd+1;i++){
			if(FD_ISSET(i,&cpy_reads)){
					printf("maxfd : %d\n",maxfd);
					client = accept(server_socket, (struct sockaddr*)&client_addr,&client_addr_size);
					if (client == -1){
						fprintf(stderr,"client conn err\n");
						break;
        			}
					else{
						FD_SET(client,&reads);
						if(maxfd < client)
							maxfd=client;
						printf("client Connect : %d\n",client);
						struct socketInfo* new = (struct socketInfo*) malloc(sizeof(struct socketInfo));
						new->fd = client;
						new->id = clientCnt;
						new->vs = NULL;
						new->dices[0] = -1;
						for(int i=0;i<13;i++){
							new->scores[i] = -1;
						}
						INIT_LIST_HEAD(&new->list);
						list_add_tail(&new->list,&head);
						printf("client %d\n",client);
						printf("clints[?] = %d\n",new->fd);
						pthread_create(&threads[threadIdx], NULL, printWorker, (void*)new);
						clientCnt++;
						threadIdx++;
					}
			}
		}
	}
}


int main(int argc, char **argv){
	if(argc < 2){
		printf("%s PORT\n",argv[0]);
		return 0;
	}
	printf("initialized...\n");
	srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&fileMutex, NULL);
	pthread_t run_t;
	int p = atoi(argv[1]);
	pthread_create(&run_t, NULL, run_go,(void*)(&p));
	sleep(1);
	while(1){
		char buf[100];
		printf("$ ");
		scanf("%s",buf);
		if(exitCheck(buf))
			break;
		
	}
	printf("end\n");
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
void calcWin(struct socketInfo* c){
	printf("calcWin func\n");
	showScore(c);
	showScore(c->vs);
	if(c->scores[14] > c->vs->scores[14]){
		clientSend(c, "[!] you win!\n");
		clientSend(c->vs, "[!] you lose\n");
		c->infos.win += 1;
		c->vs->infos.lose +=1;
	}
	if(c->scores[14] < c->vs->scores[14]){
		clientSend(c->vs, "[!] you win!\n");
        clientSend(c, "[!] you lose\n");
		c->infos.lose += 1;
		c->vs->infos.win += 1;
	}
	if(c->scores[14] == c->vs->scores[14]){
		clientSend(c->vs, "[!] Draw!\n");
		clientSend(c, "[!] Draw\n");
		c->infos.draw += 1;
		c->vs->infos.draw += 1;
	}
	printf("calcwin end\n");
	printf("c : %s %d %d %d\n",c->infos.name, c->infos.win, c->infos.draw, c->infos.lose);
	printf("v : %s %d %d %d\n",c->vs->infos.name, c->vs->infos.win, c->vs->infos.draw, c->vs->infos.lose);
	updateUserInfo(c);
	c->vs->vs = NULL;
	c->vs = NULL;
	play.id = -1;
}
