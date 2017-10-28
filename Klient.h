#ifndef KLIENT_HEADER
#define KLIENT_HEADER

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/msg.h>
#include <cstring>
#include <string>
#include <iostream>
#include <map>

#include "Message.h"
#include "System.h"

using std::string;
using std::map;
using std::pair;

class Klient_t
{
public:
	explicit Klient_t() {
		flag = true;
		error_file = NULL;
		Memory_Id = 0;
		size_t num = 0;
		error = "";
	}
	~Klient_t();
	bool Start();

private:
	FILE* error_file;
	bool flag;
	string error;
	size_t Memory_Id;
	string user_name;
	long user_id;
	msgbuf_t message_string;
	msgsize_t message_size;
	Message_t Messenger;

	void print_er(string error);
	bool Get_Id();
	bool Get_Message();
	bool Send_Message();
};

Klient_t::~Klient_t() {
	fclose(error_file);
}

void Klient_t::print_er(string error) {
	printf("%s", error.c_str());
}

bool Klient_t::Start() {
	try {
		error_file = fopen(error_log, "w");
		if (error_file == NULL) {
			error = "Error - open error file\n";
			throw error;
		}
		Memory_Id = Messenger.Create_Message_Thread(error_log, SYSTEM);
		if (Memory_Id == ERROR) {
			error = "Error - Memory Id\n";
			throw error;
		}
		if (!Get_Id()) {
			error = "Error - Get_Id\n";
			throw error;
		}
		pid_t status = fork();
		switch (status) {
		case -1: //error
			error = "Error - fork \n";
			throw error;
		case 0:  //son
			if (!Get_Message()) {
				error = "Error - Get Message \n";
				throw error;
			}
			break;
		default: //father
			if (!Send_Message()) {
				error = "Error - Send Message \n";
				throw error;
			}
			break;
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Klient_t::Send_Message() {
	try {
		while (flag) {
			string message = "";
			std::cin >> message;
			printf("%s\n", message.c_str());
			if (message == exit) {
				message += ":";
				message += user_name;
				flag = false;
			}
			if (!Messenger.Send_Message_Size(Memory_Id, &message_size, user_id, message.size(), 0)) {
				error = "Error - Send size of Message Normal\n";
				throw error;
			}
			if (!Messenger.Send_Message_String(Memory_Id, &message_string, user_id, message, 0)) {
				error = "Error - Send Message Normal\n";
				throw error;
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Klient_t::Get_Message() {
	try {
		while (flag) {
			string message = "";
			if (!Messenger.Get_Message_Size(Memory_Id, &message_size, user_id, 0)) {
				error = "Error - get size message normal\n";
				throw error;
			}
			if (!Messenger.Get_Message_String(Memory_Id, &message_string, user_id, 0, message_size.size)) {
				error = "Error - get message normal\n";
				throw error;
			}
			message = message_string.message;
			if (message == exit) {
				printf("Server is disabled\n");
				flag = false;
			}
			else {
				printf("%s\n", message.c_str());
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Klient_t::Get_Id() {
	try {
		printf("Hello, What is your name?\n");
		std::cin >> user_name;
		user_name = new_client + user_name;
		if (!Messenger.Send_Message_Size(Memory_Id, &message_size, system_id, user_name.size(), 0)) {
			error = "Error - Send Size of Message\n";
			throw error;
		}
		if (!Messenger.Send_Message_String(Memory_Id, &message_string, system_id, user_name, 0)) {
			error = "Error - Send Message\n";
			throw error;
		}
		if (!Messenger.Get_Message_Size(Memory_Id, &message_size, system_id, 0)) {
			error = "Error - Get Size\n";
			throw error;
		}
		if (!Messenger.Get_Message_String(Memory_Id, &message_string, system_id, 0, message_size.size)) {
			error = "Error - Get Message\n";
			throw error;
		} 
		size_t pas = 0;
		user_id = stoul(message_string.message.substr(user_name.size()), &pas, 10);
		if (pas != (message_string.message.size() - user_name.size())) {
			error = "Error - Get Id\n";
			throw error;
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

#endif KLIENT_HEADER