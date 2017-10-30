#ifndef CLIENT_HEADER_H
#define CLIENT_HEADER_H

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

class Client_t {
	public:
	
		explicit Client_t() {
			flag = true;
			error_file = NULL;
			Memory_Id = 0;
			size_t num = 0;
			user_id = 0;
			error = "";
			message_size.mtype = 0;
            message_size.size = 0;
			message_string.mtype = 0;
			message_string.message[1] = 0;
		}
		~Client_t();
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

Client_t::~Client_t() {
	fclose(error_file);
}

void Client_t::print_er(string error) {
	printf("%s", error.c_str());
}

bool Client_t::Start() {
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
		printf("...\n");
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

bool Client_t::Send_Message() {
	try {
		while (flag) {
			string message = "";
			std::getline(std::cin, message);
			if (message == exit_sys) {
				message = message + std::to_string(user_id);
				Messenger.Send_Message_Size(Memory_Id, &message_size, system_id, message.size(), 0);
				Messenger.Send_Message_String(Memory_Id, &message_string, system_id, message, 0);
				flag = false;
			}
			else {
				if (!Messenger.Send_Message_Size(Memory_Id, &message_size, user_id, message.size(), 0)) {
					error = "Error - Send size of Message Normal\n";
					throw error;
				}
				if (!Messenger.Send_Message_String(Memory_Id, &message_string, user_id, message, 0)) {
					error = "Error - Send Message Normal\n";
					throw error;
				}
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Client_t::Get_Message() {
	try {
		while (flag) {
			string message = "";
			if (!Messenger.Get_Message_Size(Memory_Id, &message_size, user_id + 1, 0)) {
				error = "Error - get size message normal\n";
				throw error;
			}
			if (!Messenger.Get_Message_String(Memory_Id, message, user_id + 1, 0, message_size.size)) {
				error = "Error - get message normal\n";
				throw error;
			}
			if (message == end_sys) {
				printf("Server is disabled\n");
				flag = false;
			}
			else if (message == exit_sys) {
				printf("Exit\n");
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

bool Client_t::Get_Id() {
	try {
		printf("Hello, What is your name?\n");
		std::getline(std::cin, user_name);
		string user_name_mes = new_client_sys + user_name;
		if (!Messenger.Send_Message_Size(Memory_Id, &message_size, system_id, user_name_mes.size(), 0)) {
			error = "Error - Send Size of Message\n";
			throw error;
		}
		if (!Messenger.Send_Message_String(Memory_Id, &message_string, system_id, user_name_mes, 0)) {
			error = "Error - Send Message\n";
			throw error;
		}
		if (!Messenger.Get_Message_Size(Memory_Id, &message_size, system_id + 1, 0)) {
			error = "Error - Get Size\n";
			throw error;
		}
		if(!Messenger.Get_Message_String(Memory_Id, user_name_mes, system_id + 1, 0, message_size.size)) {
			error = "Error - Get Message\n";
			throw error;
		}
		size_t pas = 0;
		user_name_mes = user_name_mes.substr(user_name.size());
		if (!IsItNumber(user_name_mes)) {
			error = "Error - wrong new id\n";
			throw error;
		}
		user_id = stoul(user_name_mes, &pas, 10);
		if (pas != user_name_mes.size()) {
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

#endif // CLIENT_HEADER_H<<<<<<< HEAD:Client.h
#endif // CLIENT_HEADER_H
