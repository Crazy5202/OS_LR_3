#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "string.h"
#include "errno.h"

int main(int argc, char* argv[]) {
    int file_rw = open(argv[1], O_RDWR); // opening file for reading arguments
    if (file_rw == -1) {
        perror("File opening in child failure");
        _exit(-1);
    }
    char* buffer = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, file_rw, 0);
    if (buffer == MAP_FAILED) {
        perror("Mapping in child failure");
        _exit(-2);
    }
    char* var = strtok(buffer, "\0"); // using strtok to get numbers
    var = strtok(var, " ");
    int result = atoi(var);
    if (var == NULL || (var[0]!='0' && result==0)) {
        errno = EINVAL;
        perror("Non-numerical input");
        _exit(-3);
    }
    var = strtok(NULL, " ");
    while(var != NULL) {
        int del = atoi(var);
        if (del == 0) {
            errno = EINVAL;
            if (var[0]=='0') perror("Zero divisor");
            else perror("Non-numerical input");
            _exit(-4);
        }
        result /= del;
        var = strtok(NULL, " ");
    }
    int num_len = 1, num = result/10;
    while (num>0) {
        num/=10;
        ++num_len;
    }
    char res_str[num_len];
    sprintf(res_str, "%d", result);
    memcpy(buffer, res_str, num_len); // writing the result in file
    ftruncate(file_rw, num_len); // shrinking the file
}