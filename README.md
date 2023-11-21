# SSS PJ2 ReadMe  
### SUMMARY  
이 프로그램은 야찌 게임을 수행하는 게임 서버와 클라이언트가 존재합니다.  
야찌 주사위 게임 룰은 https://namu.wiki/w/야찌 를 참고해 주세요  
빌드는 아래와 같습니다.  
#### client  
gcc -o client clinet.c  
#### server  
gcc -o server server.c  

### 개발환경  
Ubuntu 22.04 LTS  

### 실행조건  
서버는 실행 시 포트로 지정할 인자가 필요합니다.  
클라이언트는 실행 시 출력할 배너 파일이 필요합니다.  

### COMMAND  
```
[@] ./client start!
id : cocoa
playerID : cocoa
```   
클라이언트를 실행하면 닉네임을 입력한 뒤 서버에서 정보를 받아옵니다.  
```
cocoa$ join
=============
[!] join complate..
=============@
```
join 명령어를 실행하면 방에 입장 할 수 있습니다.  
```
=============
[!] your 1p your turn!
=============@
cocoa$ 
=============
first dice :5 1 4 1 4
=============
cocoa$ 0 1 1 0 1
=============
reroll dice :5 1 1 1 3
=============@
cocoa$ 1 0 0 0 1
=============
reroll dice :4 1 1 1 1
=============@
cocoa$ 
```
상대방이 매칭된다면 엔터를 입력하여 주사위를 던질 수 있습니다.  
만약 내가 선이 아니라면 턴이 아니라고 메세지가 옵니다.  
던져진 주사위의 index에 0을 입력하면 해당하는 index에 존재하는 주사위는 고정한 뒤 새로 던집니다.  
턴에 end를 입력한다면 주사위 던지기 단계를 종료하고 점수 기입을 할 수 있습니다.  

```
cocoa$ set 1
=============
[!] turn end
=============@
cocoa$ show
=============
player 		num	|	cocoa	|	happy	
one		1	|	4	|	-1	
two		2	|	-1	|	-1	
three		3	|	-1	|	-1	
four		4	|	-1	|	-1	
five		5	|	-1	|	-1	
six		6	|	-1	|	-1	
three_of_a_kind	7	|	-1	|	-1	
four_of_a_kind	8	|	-1	|	-1	
full_house	9	|	-1	|	-1	
small_straight	10	|	-1	|	-1	
large_straight	11	|	-1	|	-1	
chance		12	|	-1	|	-1	
yahtzee!	13	|	-1	|	-1	
bonus			|	0	|	0	
sum			|	4	|	0	
=============@
```
set 1을 입력한다면 one에 해당하는 족보에 점수를 기입합니다.  
이전 단계에서 1이 4개였기 때문에 4점이 기입됩니다.  
show 명령어를 입력하면 현재 게임의 점수를 확인 가능합니다.  
```
first dice :1 4 5 1 3
=============@
happy$ 0 1 1 0 1
=============
reroll dice :1 1 5 1 3
=============@
happy$ end
=============
dice :1 1 5 1 3
=============@
happy$ set 1
=============
[!] turn end
=============@
```
end를 입력한다면 점수 기입단계로 진입합니다.  
게임이 끝난다면 결과에 따라 전적을 기입합니다.  
join을 새로해 새로운 게임을 진행할 수 있습니다.  
```
happy$ userinfo
=============
happy 0 0 3
=============@
```
userinfo 명령어를 입력한다면 나의 전적 및 게임중이라면 매칭된 상대의 전적을 확인 할 수 있습니다.
