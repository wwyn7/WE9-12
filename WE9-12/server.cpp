//서버는 아이피를 가지고 시작을 합니다!
//아이피로 접속을 하는 것이죠?
//외부에서 접속을 할 때에는 퍼블릭IP가 필요하지만, 서버를 켤 때에는
//내부 공유기한테 개인IP로 열 거에요! 라고 이야기 할 필요가 있습니다.
//내부 IP를 여기에다가 입력해주시면 됩니다!
#define SEVER_PRIVATE_IP "172.31.47.25"

//컴퓨터에는 동시에 여러개의 프로그램이 작동하고 있습니다.
//엘든링을 하고 있었어요! 네트워크를 사용하고 있죠!
//마영전을 같이 켜뒀습니다! 아이피만 가지고 대화를 시도하면요! 들어온 메시지가 엘든링 것인지, 마영전 것인지 전혀 알 수가 없어요!
//"포트"라고 하는 것이 누구 메시지인지 구분할 수 있게 해줘요!
//몇 번 포트로 주면 이 프로그램에 줄게요^^ 라고 하는 느낌!
//49152 ~ 65535 까지 자유롭게 사용할 수 있는 "동적 포트"니까 이 사이에 있는 값으로 조정해줄게요!
#define SEVER_PORT 55123

//서버에서는 메시지를 보낼 겁니다!
//메시지 무한정 보낼 수는 없어요! 네트워크 계층에는 물리게층이 있는데 물리적인 한계가 존재할 수밖에 없죠!
//저희가 그렇게까지 큰 메시지를 많이 보내진 않을 거니까 가볍게 최대 버퍼사이즈를 조정해주도록 합시다!
#define MAX_BUFFER_SIZE 1024

//최대값을 정해야하는 다른 수가 있을 거에요!
#define MAX_USER_NUMBER 100

#include <iostream>

//지금 윈도우에선 안나옵니다! 리눅스에서 사용할 소켓을 헤더를 드리고 있는 거에요!
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

using namespace std;

//poll이라고 하는 것은 상대방이 반응을 해주었을 때! 인식해서 돌려주는 개념이에요!
//누군가 저한테 메시지를 전해줬을 때! 서버 내용이 돌아가도록!
//FD는 뭔가요? File Descripter 파일을 변경해주는 녀석이요?
//소켓을 형상화한 것인데! 리눅스는 소켓이라고 하는 것도 일종의 파일 형태로 관리를 합니다!
//파일 번호를 저장할 수 있는 공간이 될 거에요! 소켓에 해당되는 파일 정보!
struct pollfd pollFDArray[MAX_USER_NUMBER];


int StartServer(int currentFD);

int main()
{
	//듣기만 하는 fd가 필요해요!
	//사실 이상한 애가 와서 이야기를 하면 안들어야겠죠!
	//컴퓨터는 처음 보는 IP가 와서 이야기하면 "전화 잘못거셨어요!" 라고 하면서 끊어버려요!
	//ListenFD라고 하는 애는 잘 들어줄 겁니다!
	//아하! 이 분이 지금 접속을 하고 싶으신가 봅니다!
	//다른 FD한테 새로운 접속을 알려주는 역할로 둘 거에요! (입구 역할을 하는 거에요!)
	//0번째 유저를 리슨소켓으로 사용할 겁니다!
	struct pollfd& ListenFD = pollFDArray[0];

	                 //IPv4(4바이트짜리 IP)
	ListenFD.fd = socket(AF_INET, SOCK_STREAM, 0);
	ListenFD.events = POLLIN;
	ListenFD.revents = 0;

	//      0은 리슨소켓이니까!
	for (int i = 1; i < MAX_USER_NUMBER; i++)
	{
		//File Descripter가 등록되지 않은 pollFD는요! 연결이 안 된 것이죠!
		//해당되는 소켓이 없다는 뜻입니다!
		pollFDArray[i].fd = -1;
	};

	//여기서 FD는 준비가 되었고! 서버를 돌려봅시다!
	//리슨 소켓의 정보를 전달해주면서 서버를 시작할 거에요!
	StartServer(ListenFD.fd);

	while (true)
	{
		//poll에 대해서 말씀을 드릴 때! 누군가 저한테 메시지를 전달했을 때 실행!
		//0번까지도 폴에 넣어서 리슨소켓에 대답이 있을 때에도 들어갈 수 있게 위에서 설정해줬어요!
		int result = poll(pollFDArray, MAX_USER_NUMBER, -1);

		//누가 부르던데요? 0이면 아무도 대답안했다! 15라고 하면, 15명이 부른다!
		if (result > 0)
		{
			//리슨 소켓에 반응 확인!
			//누군가 접속을 시도하고 있습니다!
			if (ListenFD.revents == POLLIN)
			{
				cout << "누군가의 반응 식별!" << endl;
			};
		};
	};

	//리슨 소켓 닫고
	close(ListenFD.fd);

	for (int i = 0; i < MAX_USER_NUMBER; i++)
	{
		//엇.. 누가 있어? 닫아!
		if (pollFDArray[i].fd != -1) close(pollFDArray[i].fd);
	}
	return -4;
}

int StartServer(int currentFD)
{
	//서버를 시작하려고 하는데.. 아니!
	//리슨소켓이 막혀있어요!
	if (currentFD == -1)
	{
		//소켓때문에 에러가 났어요!
		perror("socket()");
		//혹시 모르니까 소켓 꺼주기!
		close(currentFD);
		return -1;
	};

	//소켓에다가 INET이라는 정보는 넣어줬는데요
	//그래.. 4바이트짜리 IP인 것은 알겠는데.. 그래서 IP가 뭐임?
	//그래서 저희는 서버의 IP를 리슨 소켓에다가 등록을 해주어야 합니다!
	sockaddr_in address;

	//혹시 모르니까 완전 초기화 해놓고 정보를 넣을게요!
	//address 내부를 전부 0으로 맞추기!
	memset(&address, 0, sizeof(address));

	//소켓이랑 아이피 종류를 맞춰주도록 할게요!
	address.sin_family = AF_INET;
	//실제 서버 IP를 여기에서 넣어주도록 합시다!
	address.sin_addr.s_addr = inet_addr(SEVER_PRIVATE_IP);
	//포트까지 적어주어야 했었죠?
	address.sin_port = htons(SEVER_PORT);

	//주소가 여기에요 로 끝난다면 서버가 실행이 안될 거에요! 저장만 한 것이죠!
	//사용해서 서버를 만들어야 할 거니까!
	//지금 설정한 주소를 소켓에다가 "묶어" 줄 거에요!                 실패다!
	if (bind(currentFD, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		perror("bind()");
		close(currentFD);
		return -1;
	};

	//실제로 리슨 소켓을 작동을 시켜봅니다!
	//그럼 진짜로 서버가 돌아가는 거겠죠?
	if (listen(currentFD, 8) == -1)
	{
		perror("listen()");
		close(currentFD);
		return -1;
	};

	//당신은 모든 시련을 훌륭하게 이겨내셨습니다
	return 1;
}