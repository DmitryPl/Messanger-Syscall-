#ifndef SERVER_HEADER
#define SERVER_HEADER

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <cstring>
#include <string>
#include <map>

#include "Message.h"
#include "System.h"

using std::string;
using std::map;
using std::pair;

class Server_t {
    public:    
        explicit Server_t() {
            flag = true;
            error_file = NULL;
            Memory_Id = 0;
            size_t num = 0;
            error = "";
        }
        ~Server_t();
        bool Start();

    private:
        FILE* error_file;
        bool flag;
        string error;
        size_t Memory_Id;
        size_t num;
        msgbuf_t message_string;
        msgsize_t message_size;
        Message_t Messenger;
        map<long, string> Data;

        bool Get_Messages();
        bool Processing();
        bool System_Call(string message);
        bool Send_Messages(long id, string message);
        bool Send_Messages_S(long id, string message);
        bool Send_Messages(string message);
        long Find_New_id();
        void Print_Users();
		void print_er(string error);

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
    for ( ; runner != end; ++runner) {
        printf("%ld : ", runner->first);
        printf("%s\n", runner->second.c_str());
    }
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

bool Server_t::Get_Messages() {
    while (flag) {
        try {
            if (! Messenger.Get_Message_Size(Memory_Id, &message_size, 0, 0)) {
                error = "Error - Get size of message Server\n";
                throw error;
            }
            if (! Messenger.Get_Message_String(Memory_Id, &message_string, message_size.mtype, 0, message_size.size)) {
                error = "Error - Get string of message Server\n";
                throw error;
            }
            if (!Processing()) {
                error = "Error - Process Server Mail: ";
				error += std::to_string(num);
                error += "\n";
                throw error;
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
        string message = message_string.message;
        num++;
        printf("%s\n", message.c_str());
        long id = message_string.mtype;
        if (id == system_id) {
            if (!System_Call(message)) {
                error = "Error - System Call\n";
                throw error;
            }
        }
        else if (!Send_Messages(id, message)) {
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
            if(!Messenger.Send_Message_Size(Memory_Id, &message_size, Id_send, message.size(), 0)) {
                error = "Error - Send message size\n";
                throw error;
            }
            if(!Messenger.Send_Message_String(Memory_Id, &message_string, Id_send, message, 0)) {
                error = "Error - Send message \n";
                throw error;
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
        message = name_user + " : " + message;
        auto begin = Data.begin();
        auto end = Data.end();
        for (auto it = begin; it != end; ++it) {
            long Id_send = it->first;
            if(!Messenger.Send_Message_Size(Memory_Id, &message_size, Id_send, message.size(), 0)) {
                error = "Error - Send message size\n";
                throw error;
            }
            if(!Messenger.Send_Message_String(Memory_Id, &message_string, Id_send, message, 0)) {
                error = "Error - Send message \n";
                throw error;
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
        if(!Messenger.Send_Message_Size(Memory_Id, &message_size, system_id, message.size(), 0)) {
            error = "Error - Send message size\n";
            throw error;
        }
        if(!Messenger.Send_Message_String(Memory_Id, &message_string, system_id, message, 0)) {
            error = "Error - Send message \n";
            throw error;
        }
        return true;
    }
    catch(string error) {
        print_er(error);
        return false;
    }
}

long Server_t::Find_New_id() {
    bool flag_stop = false;
    for (long id = 10; flag_stop == true; id++)
    {
        auto it = Data.find(id);
        if (it == Data.end()) {
            return id;
        }
    }
}

bool Server_t::System_Call(string message) {
    try {
		if (message.compare(0, 19, new_client) == 0) {
            string name = message.substr(20);
			if (!System_New_User(name)){
				error = "Error - new user\n";
				throw error;
			}
        }

		else if(message.compare(0, 13, exit) == 0) {
            string id = message.substr(14);
			if (!System_New_User(id)) {
				error = "Error -  user\n";
				throw error;
			}
        }

		else if(message == stop_server) {
			Admin_Stop_Server();
		}

		else if(message.compare(0, 20, delete_user) == 0) {
			string name_user = message.substr(21);
			if (!Admin_Del_User(name_user)) {
				error = "Error - del user";
				throw error;
			}
		}

		else if (message.compare(0, 16, find_id) == 0) {
			string name_user = message.substr(17);
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
				Send_Messages_S(ADMIN, "Admin");
			}
		}
		else {
			long client_id = Find_New_id();
			if (client_id == ERROR) {
				error = "Error - find new id\n";
				throw error;
			}
			Data.insert(pair<long, string>(client_id, name));
			if (!Send_Messages_S(client_id, name)) {
				error = "Error - new user\n";
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

bool Server_t::System_Exit_User(string name) {
	try {
		size_t pos = 0;
		long client_id = stoul(name, &pos, 10);
		if (pos != (name.size())) {
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
		Data.erase(client_id);
		Send_Messages(name);
		Send_Messages(client_id, end);
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
		string message = end;
		Send_Messages(message);
		flag = false;
	}
}

#endif SERVER_HEADER