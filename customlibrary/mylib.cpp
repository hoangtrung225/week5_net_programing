#include "stdafx.h"
#include <string.h>
#include "mylib.h"

#define PASSLENGHT 32
#define USERLENGHT 32
#define BUFFSIZE 512

struct client_struct {
	char buffer[BUFFSIZE];
	char user_name[USERLENGHT];
	int state;
};

char* CodeReference(int code) {
	char* return_name;
	switch (code)
	{
	case 230:
		return_name = "Login! Service ready\n";
		break;
	case 231:
		return_name = "User logout! Service terminate\n";
		break;
	case 331:
		return_name = "User name is ok! need password\n";
		break;
	case 332:
		return_name = "Need account for login!\n";
		break;
	case 430:
		return_name = "Invalid username or password\n";
		break;
	case 530:
		return_name = "Not login";
		break;
	default:
		return_name = "Error is not reconigzed";
		break;
	}
	return return_name;
}

int GetClientAction(char* action_name) {
	if (strcmp(action_name, "USER") == 0)
		return 1;
	else if (strcmp(action_name, "PASS") == 0)
		return 2;
	else if (strcmp(action_name, "BYE!") == 0)
		return 0;
	else if (strcmp(action_name, "LOUT") == 0)
		return 9;
	else
		return 2205;
}