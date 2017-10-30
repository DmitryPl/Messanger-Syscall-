#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

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
        bool Get_Message_String(int msgid, string& message, long msgtyp, int msgflg, size_t size);
        bool Get_Message_Size(int msgid, msgsize_t* mes, long msgtyp, int msgflg);

    private:
        const char* _way;
        size_t _code;
        size_t _size;
        bool flag;
        string error;
        FILE* error_file;

		void print_er(string error);
};

Message_t::~Message_t() {
    fclose(error_file);
}

void Message_t::print_er(string error) {
	printf("%s", error.c_str());
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
        if (Id == 0) {
            error = "Error - ID = 0!!!\n";
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
        int status = msgsnd(msgid, (void *) mes, sizeof(size_t) , type);
        CHECK_ERROR(status ,"Error - msgsnd - send size\n");
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
        size_t size = message.size();
        memcpy(mes->message, message.c_str(), size);
        if(_size != size) {
            error = "Error - _size message send\n";
            throw error;
        }
        int status = msgsnd(msgid, (void *) mes, _size * sizeof(char) , type);
        CHECK_ERROR(status ,"Error - msgsnd - send string\n");
        _size = 0;
        return true;
    }
    catch (string error) {
		print_er(error);
        return false;
    }
}

bool Message_t::Get_Message_Size(int msgid, msgsize_t* mes, long msgtyp, int msgflg) {
    try {
        if (_size != 0) {
            error = "Error - _size size of message get\n";
            throw error;
        }
        int status = msgrcv(msgid, (void *) mes, sizeof(size_t), msgtyp, msgflg);
        if (msgflg != IPC_NOWAIT) {
            CHECK_ERROR(status ,"Error - msgrcv - get size \n");
        }
        if (status != -1) {
            _size = mes->size;
        }
        return true;
    }
    catch (string error) {
        print_er(error);
        return false;
    }
}

bool Message_t::Get_Message_String(int msgid, string& message, long msgtyp, int msgflg, size_t size) {
    try {
        if (_size != size) {
            error = "Error - _size message get\n";
            throw error;
        }
        msgbuf_t* buf = NULL;
        buf = (msgbuf_t*)realloc(buf, size + sizeof(long));
        int status = msgrcv(msgid, (void *) buf, sizeof(char) * size, msgtyp, msgflg);
        char* test = new char[size + sizeof(long)];
        memcpy(test, buf->message, size);
        message = test;
        CHECK_ERROR(status ,"Error - msgrcv - get mes\n");
        _size = 0;
        return true;
    }
    catch (string error) {
        print_er(error);
        return false;
    }
}

#undef CHECK_ERROR
#endif // MESSAGE_HEADER_H