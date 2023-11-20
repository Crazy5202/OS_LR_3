#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "sys/wait.h"
#include "signal.h"
#define filename "file.txt"

int main() {
    int file_write = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0777); // creating file
    if (file_write == -1) {
        perror("File creation failure");
        return 1;
    }
    posix_fallocate(file_write, 0, 4096); // ensuring that file has a size of one page
    char* buffer = mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, file_write, 0); // buffering the file
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
        int stop_signal = raise(SIGSTOP); //stopping the child to avoid mapping intersection
        if (stop_signal != 0) {
            perror("Child process stopping failure");
            return 4;
        }
        execle("divider", "divider", filename, NULL); // executing program to divide the numbers
        perror("Exec error");
        return 5;
    } else { // going in parent process
        waitpid(pid, NULL, WUNTRACED); // waiting for a child to be stopped
        char c = getchar();
        for(int i=0;; ++i) { // writing arguments from input to buffer
            buffer[i] = c;
            c = getchar();
            if (c=='\n') {
                buffer[i+1] = '\0';
                break;
            }
        }
        if (munmap(buffer, 4096) == -1) { // checking for error in child
            return 6;
        }
        int cont_signal = kill(pid, SIGCONT); // sending the signal for child to continue
        if (cont_signal != 0) {
            perror("Child process continuation failure");
            return 7;
        }
        int status;
        wait(&status); // waiting for child process to finish
        int return_value = WEXITSTATUS(status);
        if (return_value!=0) { // stopping the program in case the child process had an error
            return 8;
        }
        int file_read = open(filename, O_RDONLY); // opening file for reading result
        if (file_read == -1) {
            perror("File opening for result failure");
            return 9;
        }
        buffer = mmap(NULL, 4096, PROT_READ, MAP_SHARED, file_read, 0);
        if (buffer == MAP_FAILED) {
            perror("Parent mapping for reading failure");
            return 10;
        }
        printf("Result: %s\n", buffer);
    }
    return 0;
}