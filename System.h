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
    SUCCESS_SYS = 42,
    system_id = 128,
};

int do_Nothing() {
    return SUCCESS_SYS;
}

void print_er(string error) {
    printf("%s", error.c_str());
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