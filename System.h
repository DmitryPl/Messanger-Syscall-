#ifndef SYSTEM_HEADER_H
#define SYSTEM_HEADER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using std::string;

enum System_Num {
    ERROR = -1,
    SYSTEM,
    SUCCESS,
	ADMIN,
    SUCCESS_SYS = 42,
    system_id = 128,
};

const char* new_client_sys = "#system:new_client:";
const char* exit_sys = "#system:exit";
const char* stop_server_sys = "#system:stop_server";
const char* delete_user_sys = "#system:delete_user:";
const char* find_id_sys = "#system:find_id:";
const char* end_sys = "#system:end";
const char* error_log = "/home/dmitry/WorkSpace/Message/error.log";

int do_Nothing() {
    return SUCCESS_SYS;
}

bool IsItNumber(string word){
	size_t i = 0;
	if (word[0] == '-'){
		i++;
	}
	while (word[i] != '\0'){
		if (!isdigit(word[i++]))
			return false;
	}
	return true;
}

typedef struct _msgbuf {
    long mtype;
    char message[1];
} msgbuf_t;

typedef struct _msgsize {
    long mtype;
    size_t size;
} msgsize_t;

#endif // SYSTEM_HEADER