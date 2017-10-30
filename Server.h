#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <cstring>
#include <string>
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
        explicit Server_t() {
            flag = true;
            error_file = NULL;
            Memory_Id = 0;
            num = 1;
            error = "";
            message_size.mtype = 0;
            message_size.size = 0;
            message_string.mtype = 0;
            message_string.message[1] = 0;
            Id_Mas.push_back(system_id);
        }
        ~Server_t();
        bool Start();

    private:
        FILE* error_file;
        bool flag;
        string error;
        size_t Memory_Id;
        size_t num;
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
        bool Send_Messages(long id, string message);
        bool Send_Messages_S(long id, string message);
        bool Send_Messages_L(long id, string message);
        long Find_New_id();
        void Print_Users();
        void print_er(string error);
        bool Find_Message();

		bool System_New_User(string name);
		bool System_Exit_User(string name);
		void Admin_Id_User(string name);
		bool Admin_Del_User(string name);
        void Admin_Stop_Server();
};

Server_t::~Server_t() {
    Print_Users();
    fclose(error_file);
}

void Server_t::print_er(string error) {
	printf("%s", error.c_str());
}

void Server_t::Print_Users() {
    auto runner = Data.begin();
    auto end = Data.end();
    printf("Map:\n");
    for ( ; runner != end; ++runner) {
        printf("%ld : ", runner->first);
        printf("%s\n", runner->second.c_str());
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
        Memory_Id = Messenger.Create_Message_Thread(error_log, SYSTEM);
        if (Memory_Id == ERROR) {
            error = "Error - Memory Id\n";
            throw error;
        }
        printf("Memory Id:%zu\n", Memory_Id);
        if (!Get_Messages()) {
            error = "Error - Get Messages\n";
            throw error;
        }
    }
    catch(string error) {
		print_er(error);
        return false;
    }
    return true;
}

bool Server_t::Find_Message() {
    while(flag) {
        size_t length = Id_Mas.size();
        message_size.size = 0;
        for (size_t i = 0; i < length; i++) {
            size_t id = Id_Mas[i];
            if (! Messenger.Get_Message_Size(Memory_Id, &message_size, id, IPC_NOWAIT)) {
                error = "Error - Get size of message Server\n";
                throw error;
            }
            if(message_size.size != 0) {
                return true;
            }
        }
    }
}

bool Server_t::Get_Messages() {
    while (flag) {
        try {
            printf("num of message:%zu\n", num);
            if (!Find_Message()) {
                error = "Error - Find Message\n";
                throw error;
            }
            else {
                num++;
                printf("Client Id:%ld, size:%zu\n", message_size.mtype, message_size.size);
                if (! Messenger.Get_Message_String(Memory_Id, message_get, message_size.mtype, 0,   message_size.size)) {
                    error = "Error - Get string of message Server\n";
                    throw error;
                }
                printf("Client Id:%ld, message:%s\n", message_size.mtype, message_get.c_str());
                if (!Processing()) {
                    error = "Error - Process Server Mail: " + std::to_string(num) + "\n";
                    throw error;
                }
            }
        }
        catch(string error) {
            print_er(error);
            return false;
        }
    }
    return true;
}

bool Server_t::Processing() {
    try {
        long id = message_size.mtype;
        if (id == system_id) {
            if (!System_Call(message_get)) {
                error = "Error - System Call\n";
                throw error;
            }
        }
        else if (!Send_Messages(id, message_get)) {
            error = "Error - Send Messages\n";
            throw error;
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
        auto begin = Data.begin();
        auto end = Data.end();
        for (auto it = begin; it != end; ++it) {
            long Id_send = it->first;
            if (message_size.mtype != Id_send) {
                if(!Messenger.Send_Message_Size(Memory_Id, &message_size, Id_send + 1, message.size(), 0)) {
                    error = "Error - Send message size\n";
                    throw error;
                }
                if(!Messenger.Send_Message_String(Memory_Id, &message_string, Id_send + 1, message, 0)) {
                    error = "Error - Send message \n";
                    throw error;
                }
                printf("Send to %ld, message: %s\n", Id_send, message.c_str());
            }
        }
        return true;
    }
    catch(string error) {
        print_er(error);
        return false;
    }
}

bool Server_t::Send_Messages(long id, string message) {
    try {
        auto it_id = Data.find(id);
        if (it_id == Data.end()) {
            error = "Error - no client of send\n";
            throw error;
        }
        string name_user = it_id->second;
        string new_message = name_user + " : " + message; 
        auto begin = Data.begin();
        auto end = Data.end();
        for (auto it = begin; it != end; ++it) {
            long Id_send = it->first;
            if (Id_send != message_size.mtype) {
                if(!Messenger.Send_Message_Size(Memory_Id, &message_size, Id_send + 1, new_message.size(), 0)) {
                    error = "Error - Send message size\n";
                    throw error;
                }
                if(!Messenger.Send_Message_String(Memory_Id, &message_string, Id_send + 1, new_message, 0)) {
                    error = "Error - Send message \n";
                    throw error;
                }
                printf("Send to %ld, message: %s\n",Id_send, new_message.c_str());
            }
        }
        return true;
    }
    catch(string error) {
        print_er(error);
        return false;
    }
}

bool Server_t::Send_Messages_S(long id, string message) {
    try{
        message += std::to_string(id);
        size_t size = message.size();
        if(!Messenger.Send_Message_Size(Memory_Id, &message_size, system_id + 1, size, 0)) {
            error = "Error - Send message size\n";
            throw error;
        }
        if(!Messenger.Send_Message_String(Memory_Id, &message_string, system_id + 1, message, 0)) {
            error = "Error - Send message \n";
            throw error;
        }
        printf("Send to %ld, message: %s\n", message_size.mtype - 1, message.c_str());
        return true;
    }
    catch(string error) {
        print_er(error);
        return false;
    }
}

bool Server_t::Send_Messages_L(long id, string message) {
    try{
        size_t size = message.size();
        if(!Messenger.Send_Message_Size(Memory_Id, &message_size, id + 1, size, 0)) {
            error = "Error - Send message size\n";
            throw error;
        }
        if(!Messenger.Send_Message_String(Memory_Id, &message_string, id + 1, message, 0)) {
            error = "Error - Send message \n";
            throw error;
        }
        printf("Send to %ld, message: %s\n", id + 1, message.c_str());
        return true;
    }
    catch(string error) {
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
		if (message.compare(0, 19, new_client_sys) == 0) {
            string name = message.substr(19);
			if (!System_New_User(name)){
				error = "Error - new user\n";
				throw error;
			}
        }

		else if(message.compare(0, 12, exit_sys) == 0) {
            string id = message.substr(12);
			System_Exit_User(id);
        }

		else if(message == stop_server_sys) {
			Admin_Stop_Server();
		}

		else if(message.compare(0, 20, delete_user_sys) == 0) {
			string name_user = message.substr(20);
			if (!Admin_Del_User(name_user)) {
				error = "Error - del user";
				throw error;
			}
		}

		else if (message.compare(0, 16, find_id_sys) == 0) {
			string name_user = message.substr(16);
			Admin_Id_User(name_user);
		}

        return true;
    }
    catch(string error) {
		print_er(error);
        return false;
    }   
}

bool Server_t::System_New_User(string name) {
	try {
		if (name == "Admin") {
			auto it = Data.find(ADMIN);
			if (it == Data.end()) {
				Send_Messages_S(ADMIN, "Admin");
			}
			else {
                Data.insert(pair<long, string>(ADMIN, "Admin"));
                Id_Mas.push_back(ADMIN);
				Send_Messages_S(ADMIN, "Admin");
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
			if (!Send_Messages_S(client_id, name)) {
				error = "Error - new user\n";
				throw error;
            }
            name = "New user";
            Send_Messages(client_id, name);
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
        Send_Messages_L(client_id, end_sys);
        Data.erase(client_id);
        auto run = Id_Mas.begin();
        auto end = Id_Mas.end();
        for ( ; run != end; ++run) {
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

void Server_t::Admin_Id_User(string name) {
	auto runner = Data.begin();
	auto end = Data.end();
	for ( ; runner != end; ++runner){
		if (runner->second == name) {
			Send_Messages(ADMIN, std::to_string(runner->first));
			return;
		}
	}
	Send_Messages(ADMIN, "error find user");
	return;
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
					Send_Messages(ADMIN, "error - user");
				}
				else {
					Send_Messages(client_id, "#system:end");
                    Data.erase(client_id);
                    auto run = Id_Mas.begin();
                    auto end = Id_Mas.end();
                    for ( ; run != end; ++run) {
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
	if (message_string.mtype == ADMIN) {
		string message = end_sys;
		Send_Messages(message);
		flag = false;
	}
}

#endif // SERVER_HEADER_H