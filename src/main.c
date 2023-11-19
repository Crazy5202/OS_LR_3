#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "sys/wait.h"
#include "signal.h"

int main() {
    int file_write = open("file.txt", O_CREAT | O_RDWR | O_TRUNC, 0777); // opening file
    if (file_write == -1) {
        perror("File creation failure");
        return 1;
    }
    posix_fallocate(file_write, 0, 4096);
    char* buffer = mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, file_write, 0);
    if (buffer == MAP_FAILED) {
        perror("Parent mapping for writing failure");
        return 2;
    }
    close(file_write);
    pid_t pid = fork(); // creating child process
    if (pid == -1) {
        perror("Child process creation failure");
        return 3;
    } else if (pid == 0) { // going in child process
        raise(SIGSTOP);
        execle("divider", "divider", "output.txt", NULL); // executing program to divide the numbers
        perror("Exec error");
        return 4;
    } else { // going in parent process
        waitpid(pid, NULL, WUNTRACED);
        char c = getchar();
        for(int i=0;; ++i) {
            buffer[i] = c;
            c = getchar();
            if (c=='\n') {
                buffer[i+1] = '\0';
                break;
            }
        }
        if (munmap(buffer, 4096) == -1) { // checking for error in child
            return 5;
        }
        kill(pid, SIGCONT);

        int status;
        wait(&status); // waiting for child process to finish
        if (WEXITSTATUS(status)!=0) { // checking for error in child
            return 6;
        }
        int file_read = open("file.txt", O_RDONLY); // opening file
        if (file_read == -1) {
            perror("File opening for result failure");
            return 7;
        }
        buffer = mmap(NULL, 4096, PROT_READ, MAP_SHARED, file_read, 0);
        if (buffer == MAP_FAILED) {
            perror("Parent mapping for reading failure");
            return 8;
        }
        printf("Result: %s\n", buffer);
    }
    return 0;
}