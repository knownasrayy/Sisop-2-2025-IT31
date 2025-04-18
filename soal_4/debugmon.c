#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>

void list_processes(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
        printf("Error: User '%s' not found\n", username);
        exit(1);
    }

    printf("%-10s %-20s %-10s %-10s\n", "PID", "COMMAND", "CPU%", "MEM%");

    int pipefd[2];
    if (pipe(pipefd) == -1) { 
        perror("pipe error");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        char *args[] = {
            "ps",
            "-u", (char *)username,
            "-o", "pid,comm:20,pcpu,pmem",
            "--no-headers",
            NULL
        };
        execvp("ps", args);
        perror("execvp error");
        exit(1);
    } else {
        close(pipefd[1]);
        char buffer[1024];
        
        while (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            unsigned int pid;
            char comm[50];
            float cpu, mem;
            
            if (sscanf(buffer, "%u %49s %f %f", &pid, comm, &cpu, &mem) == 4) {
                printf("%-10u %-20s %-10.1f %-10.1f%%\n", pid, comm, cpu, mem);
            }
        }
        close(pipefd[0]);
        wait(NULL);
    }
}

void run_as_daemon(const char *username) {
    pid_t pid = fork();
    if (pid < 0) {
        exit(1);
    }
    if (pid > 0) {
        exit(0);
    }

    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    char logfile[256];
    snprintf(logfile, sizeof(logfile), "/tmp/debugmon_%s.log", username);

    while (1) {
        FILE *log = fopen(logfile, "a");
        if (!log) {
            exit(1);
        }

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[50];
        strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", tm_info);
        fprintf(log, "%s_debugmon_%s_STATUS(RUNNING)\n", timestamp, username);

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            exit(1);
        }

        pid_t child_pid = fork();
        if (child_pid == 0) {
            close(pipefd[0]);
            dup2(fileno(log), STDOUT_FILENO);
            list_processes(username);
            exit(0);
        } else {
            close(pipefd[1]);
            close(pipefd[0]);
            wait(NULL);
        }

        fclose(log);
        sleep(10);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        if (strcmp(argv[1], "list") == 0) {
            list_processes(argv[2]);
        } 
        else if (strcmp(argv[1], "daemon") == 0) {
            run_as_daemon(argv[2]);
        }
    } else {
        printf("Usage:\n");
        printf("  %s list <user>\n", argv[0]);
        printf("  %s daemon <user>\n", argv[0]);
        return 1;
    }
    return 0;
}
