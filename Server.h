#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <map>

#include "Message.h"
#include "System.h"

using std::vector;
using std::string;
using std::map;
using std::pair;

class Server_t {
public:
	explicit Server_t() :
		error_file(NULL), working_server_flag(true), last_id(0),
		error(" "), memory_id(0), num_of_message(1), message_get(" ")
	{
		message_string.message[1] = { 0 };	message_size.mtype = 0;
		message_size.size = 0;	message_string.mtype = 0;
		Id_Mas.push_back(SYSTEM_ID);
	}
	~Server_t() { Print_Users(); fclose(error_file); }
	bool Start();

private:
	FILE* error_file;
	bool working_server_flag;
	long last_id;
	string error;
	size_t memory_id;
	size_t num_of_message;
	string message_get;
	msgbuf_t message_string;
	msgsize_t message_size;
	Message_t Messenger;
	map<long, string> Data;
	vector<size_t> Id_Mas;

	bool Get_Messages();
	bool Processing();
	bool System_Call(string message);
	bool Send_Messages(string message);
	bool Send_Messages_New_User(long id, string message);
	bool Send_Messages_To_One(long id, string message);
	string Ret_Named_Message();
	long Find_New_id();
	void Print_Users();
	bool Find_Message();
	bool System_New_User(string name);
	bool System_Exit_User(string name);
	long Find_Id_User(string name);
	bool Admin_Del_User(string name);
	void Admin_Stop_Server();
	bool Get_System_Com();
};

void Server_t::Print_Users() {
	printf("Map:\n");
	for (auto it : Data) {
		printf("%ld : ", it.first);
		printf("%s\n", it.second.c_str());
	}
	printf("Mas:\n");
	size_t size = Id_Mas.size();
	for (size_t i = 0; i < size; i++) {
		printf("%ld ", Id_Mas[i]);
	}
	printf("\n");
}

bool Server_t::Start() {
	try {
		error_file = fopen(error_log, "w");
		if (error_file == NULL) {
			error = "Error - open error file\n";
			throw error;
		}
		memory_id = Messenger.Create_Message_Thread(error_log, SYSTEM);
		if (memory_id == ERROR) {
			error = "Error - Memory Id\n";
			throw error;
		}
		printf("Memory Id:%zu\n", memory_id);
		pid_t status = fork();
		switch (status) {
		case -1: //error
			error = "Error - fork \n";
			throw error;
		case 0:  //son
			if (!Get_Messages()) {
				error = "Error - Get Messages \n";
				throw error;
			}
			break;
		default: //father
			if (!Get_System_Com()) {
				error = "Error - Send Message \n";
				throw error;
			}
			break;
		}
	}
	catch (string error) {
		print_er(error);
		return false;
	}
	return true;
}

bool Server_t::Get_System_Com() {
	while (working_server_flag) {
		string command = "";
		std::getline(std::cin, command);
		if (command == "exit") {
			command = stop_server_sys;
			Messenger.Send_Message(memory_id, &message_string, &message_size, SYSTEM_ID, command, 0);
			working_server_flag = false;
			return true;
		}
	}
}

bool Server_t::Find_Message() {
	while (working_server_flag) {
		size_t length = Id_Mas.size();
		message_size.size = 0;
		for (size_t i = 0; i < length; i++) {
			size_t id = Id_Mas[i];
			if (!Messenger.Get_Message_Size(memory_id, &message_size, id, IPC_NOWAIT)) {
				error = "Error - Get size of message Server\n";
				throw error;
			}
			if (message_size.size != 0) {
				return true;
			}
		}
	}
}

bool Server_t::Get_Messages() {
	while (working_server_flag) {
		try {
			printf("\nnum of message:%zu\n", num_of_message);
			if (!Find_Message()) {
				error = "Error - Find Message\n";
				throw error;
			}
			else {
				num_of_message++;
				last_id = message_size.mtype;
				printf("Client Id:%ld, size:%zu\n", message_size.mtype, message_size.size);
				if (!Messenger.Get_Message_String(memory_id, message_get, message_size.mtype, 0, message_size.size)) {
					error = "Error - Get string of message Server\n";
					throw error;
				}
				printf("Client Id:%ld, message:%s\n", message_size.mtype, message_get.c_str());
				if (!Processing()) {
					error = "Error - Process Server Mail: " + std::to_string(num_of_message) + "\n";
					throw error;
				}
			}
		}
		catch (string error) {
			print_er(error);
			return false;
		}
	}
	return true;
}

bool Server_t::Processing() {
	try {
		if ((last_id == SYSTEM_ID) || (last_id == ADMIN)) {
			if (!System_Call(message_get)) {
				error = "Error - System Call\n";
				throw error;
			}
		}
		else {
			if (message_get[0] == '\"') {
				string name = "";
				auto it = message_get.find(" ");
				name = message_get.substr(1, it - 1);
				message_get = message_get.substr(it + 1, message_get.size());
				message_get = Ret_Named_Message();
				if (message_get == "false") {
					error = "Error - Ret names message\n";
					throw error;
				}
				long id = Find_Id_User(name);
				if (id != ERROR) {
					if (!Send_Messages_To_One(id, message_get)) {
						error = "Error - Send Messages\n";
						throw error;
					}
				}
				else {
					message_get = "Wrong name\n";
					if (!Send_Messages_To_One(last_id, message_get)) {
						error = "Error - Send Messages\n";
						throw error;
					}
				}
			}
			else {
				message_get = Ret_Named_Message();
				if (message_get == "false") {
					error = "Error - Ret names message\n";
					throw error;
				}
				if (!Send_Messages(message_get)) {
					error = "Error - Send Messages\n";
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

bool Server_t::Send_Messages(string message) {
	try {
		for (auto run : Data) {
			long id_send = run.first;
			if ((last_id != id_send) && (id_send != SYSTEM_ID)) {
				if (!Messenger.Send_Message(memory_id, &message_string, &message_size, id_send + 1, message, 0)) {
					error = "Error - Send messages string\n";
					throw error;
				}
				printf("Send to %ld, message: %s\n", id_send, message.c_str());
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

string Server_t::Ret_Named_Message() {
	try {
		auto it = Data.find(last_id);
		if (it == Data.end()) {
			error = "Error - no client of send\n";
			throw error;
		}
		string name_user = it->second;
		return name_user + " : " + message_get;
	}
	catch (string error) {
		print_er(error);
		return "false";
	}
}

bool Server_t::Send_Messages_New_User(long id, string message) {
	try {
		message += std::to_string(id);
		size_t size = message.size();
		if (!Messenger.Send_Message(memory_id, &message_string, &message_size, SYSTEM_ID + 1, message, 0)) {
			error = "Error - Send messages S\n";
			throw error;
		}
		printf("Send to %ld, message: %s\n", message_size.mtype - 1, message.c_str());
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Server_t::Send_Messages_To_One(long id, string message) {
	try {
		if (!Messenger.Send_Message(memory_id, &message_string, &message_size, id + 1, message, 0)) {
			error = "Error - Send message L\n";
			throw error;
		}
		printf("Send to %ld, message: %s\n", id + 1, message.c_str());
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

long Server_t::Find_New_id() {
	for (long id = 10; ; id += 2) {
		auto it = Data.find(id);
		if (it == Data.end()) {
			return id;
		}
	}
}

bool Server_t::System_Call(string message) {
	try {
		if (message.compare(0, strlen(new_client_sys), new_client_sys) == 0) {
			string name = message.substr(strlen(new_client_sys));
			if (!System_New_User(name)) {
				error = "Error - new user\n";
				throw error;
			}
		}
		else if (message.compare(0, strlen(exit_sys), exit_sys) == 0) {
			string id = message.substr(strlen(exit_sys));
			System_Exit_User(id);
		}
		else if (message == stop_server_sys) {
			Admin_Stop_Server();
		}
		else if (message.compare(0, strlen(delete_user_sys), delete_user_sys) == 0) {
			string name_user = message.substr(strlen(delete_user_sys));
			if (!Admin_Del_User(name_user)) {
				error = "Error - del user";
				throw error;
			}
		}
		else if (message.compare(0, strlen(find_id_sys), find_id_sys) == 0) {
			string name_user = message.substr(strlen(find_id_sys));
			long id = Find_Id_User(name_user);
			if (id) {
				Send_Messages_To_One(ADMIN, std::to_string(id));
			}
			else {
				Send_Messages_To_One(ADMIN, "error find user");
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Server_t::System_New_User(string name) {
	try {
		if (name == "Admin") {
			auto it = Data.find(ADMIN);
			if (it != Data.end()) {
				Send_Messages_New_User(ADMIN, "Admin");
				Print_Users();
			}
			else {
				Data.insert(pair<long, string>(ADMIN, "Admin"));
				Id_Mas.push_back(ADMIN);
				Send_Messages_New_User(ADMIN, "Admin");
				Print_Users();
			}
		}
		else {
			long client_id = Find_New_id();
			if (client_id == ERROR) {
				printf("%ld\n", client_id);
				error = "Error - find new id\n";
				throw error;
			}
			Data.insert(pair<long, string>(client_id, name));
			Id_Mas.push_back(client_id);
			Print_Users();
			if (!Send_Messages_New_User(client_id, name)) {
				error = "Error - new user\n";
				throw error;
			}
			name = "New user : " + name;
			last_id = client_id;
			Send_Messages(name);
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Server_t::System_Exit_User(string name) {
	try {
		size_t pos = 0;
		if (!IsItNumber(name)) {
			error = "Error - Wrong name\n";
			throw error;
		}
		printf("Exit...\n");
		long client_id = stoul(name, &pos, 10);
		if (pos != name.size()) {
			error = "Error - wrong user\n";
			throw error;
		}
		auto it = Data.find(client_id);
		if (it == Data.end()) {
			error = "Error - no client\n";
			throw error;
		}
		name = it->second;
		name = "User exit : " + name;
		Send_Messages(name);
		Send_Messages_To_One(client_id, exit_sys);
		Data.erase(client_id);
		auto run = Id_Mas.begin();
		auto end = Id_Mas.end();
		for (; run != end; ++run) {
			if (*run == client_id) {
				Id_Mas.erase(run);
			}
		}
		Print_Users();
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

long Server_t::Find_Id_User(string name) {
	for (auto it : Data) {
		printf("%s\n", it.second.c_str());
		string blat = it.second;
		if (blat == name) {
			return it.first;
		}
	}
	return -1;
}

bool Server_t::Admin_Del_User(string name) {
	try {
		if (message_string.mtype == ADMIN) {
			size_t pos = 0;
			if (!IsItNumber(name)) {
				error = "Error - Wrong name\n";
				throw error;
			}
			long client_id = stoul(name, &pos, 10);
			if (pos != (name.size())) {
				error = "Error - wrong user\n";
				throw error;
			}
			else {
				auto it = Data.find(client_id);
				if (it == Data.end()) {
					Send_Messages_To_One(ADMIN, "error - user");
				}
				else {
					Send_Messages_To_One(client_id, "#system:end");
					Data.erase(client_id);
					auto run = Id_Mas.begin();
					auto end = Id_Mas.end();
					for (; run != end; ++run) {
						if (*run == client_id) {
							Id_Mas.erase(run);
						}
					}
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

void Server_t::Admin_Stop_Server() {
	if ((last_id == ADMIN) || (last_id == SYSTEM_ID)) {
		string message = end_sys;
		Send_Messages(message);
		Send_Messages_To_One(last_id, message);
		Send_Messages_To_One(SYSTEM_ID, message);
		working_server_flag = false;
	}
}

#endif // SERVER_HEADER_H