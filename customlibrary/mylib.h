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
