#ifndef MESSAGE_HEADER
#define MESSAGE_HEADER

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <cstring>
#include <string>

#include "System.h"

using std::string;

#define CHECK_ERROR(STATUS, STRING_ERROR)\
if(STATUS == ERROR){ error = STRING_ERROR; throw error; }

class Message_t {
    public:
        explicit Message_t() {
            _way = "Error";
            _code = 0;
            bool flag = true;
            _size = 0;
            error = "Error - ";
            error_file = NULL;
        }
        ~Message_t();

        int Create_Message_Thread(const char* way, size_t code);
        bool Send_Message_String(int msgid ,msgbuf_t* mes, long Id, string message, size_t type);
        bool Send_Message_Size(int msgid ,msgsize_t* mes, long Id, size_t size, size_t type);
        bool Get_Message_String(int msgid, msgbuf_t* mes, long msgtyp, int msgflg, size_t size);
        bool Get_Message_Size(int msgid, msgsize_t* mes, long msgtyp, int msgflg);

    private:
        const char* _way;
        size_t _code;
        size_t _size;
        bool flag;
        string error;
        FILE* error_file;
};

Message_t::~Message_t() {
    fclose(error_file);
}

int Message_t::Create_Message_Thread(const char* way, size_t code) {
	try {
        error_file = fopen(way, "w");
        if (error_file == NULL) {
            error = "Error - open error file\n";
            throw error;
        }
        _way = way;
        _code = code;
		key_t Key = ftok(way, code);
		CHECK_ERROR(Key,"Error - ftok\n")
		int Id = msgget( Key, 0666 | IPC_CREAT);
		if(!Id) {
			Id = msgget(Key, 0);
		}
		CHECK_ERROR(Id, "Error - Id\n")
		return Id;
	} 
	catch(string error) {
		print_er(error);
	    return ERROR;
	}
}

bool Message_t::Send_Message_Size(int msgid ,msgsize_t* mes, long Id, size_t size, size_t type) {
	try {
        if (_size != 0) {
            error = "Error - _size size of message send\n";
            throw error;
        }
		mes->mtype = Id;
        mes->size = size;
		int status = msgsnd(msgid, (void *) &mes, sizeof(size_t) , type);
        CHECK_ERROR(status, "msgsnd size of message send\n");
        _size = size;
		return true;
	} 
	catch (string error) {
		print_er(error);
        return false;
    }
}

bool Message_t::Send_Message_String(int msgid ,msgbuf_t* mes, long Id, string message, size_t type) {
    try {
        mes->mtype = Id;
        mes->message = message;
        size_t size = mes->message.size();
        if(_size != size) {
            error = "Error - _size message send\n";
            throw error;
        }
        int status = msgsnd(msgid, &mes, _size * sizeof(char) , type);
        CHECK_ERROR(status, "msgsnd message send\n");
        _size = 0;
        return true;
    }
    catch (string error) {
		print_er(error);
        return false;
    }
}

bool Message_t::Get_Message_Size(int msgid, msgsize_t* mes, long msgtyp, int msgflg) {
    while(flag) {
        try {
            if (_size != 0) {
                error = "Error - _size size of message get\n";
                throw error;
            }
            int status = msgrcv(msgid, &mes, sizeof(size_t), msgtyp , msgflg);
            CHECK_ERROR(status, "msgrcv get size of message\n")
            _size = mes->size;
            return true;
        }
        catch (string error) {
            print_er(error);
            return false;
        }
    }
}

bool Message_t::Get_Message_String(int msgid, msgbuf_t* mes, long msgtyp, int msgflg, size_t size) {
    while(flag) {
        try {
            if (_size != size) {
                error = "Error - _size message get\n";
                throw error;
            }
            int status = msgrcv(msgid, &mes, sizeof(char) * size, msgtyp , msgflg);
            CHECK_ERROR(status, "msgrcv get message\n")
            return true;
        }
        catch (string error) {
            print_er(error);
            return false;
        }
    }
}

#undef CHECK_ERROR
#endif MESSAGE_HEADER