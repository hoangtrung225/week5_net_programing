#define PASSLENGHT 32
#define USERLENGHT 32
#define BUFFSIZE 512

//Server respond format [3char: respond code][string: message]
struct ServerRespondHeader {
	char code[3];
};

//Client send format [3char: message identify type][string: message]
struct ClientRequestHeader {
	char type[4];
};
char* CodeReference(int);
int GetClientAction(char*);