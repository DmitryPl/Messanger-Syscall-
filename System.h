#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER

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

const char* new_client = "#system:new_client:";
const char* exit = "#system:exit:";
const char* stop_server = "#system:stop_server";
const char* delete_user = "#system:delete_user:";
const char* find_id = "#system:find_id:";
const char* end = "#system:end";

int do_Nothing() {
    return SUCCESS_SYS;
}

typedef struct _msgbuf {
    long mtype;
    string message;
} msgbuf_t;

typedef struct _msgsize {
    long mtype;
    size_t size;
} msgsize_t;

const char* error_log = "error.log";

#endif SYSTEM_HEADER