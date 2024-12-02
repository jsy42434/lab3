#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_ARGS 50

int getargs(char *cmd, char **argv);
void handle_builtin_commands(char **argv);
void execute_command(char **argv);

int main() {
    char buf[MAX_LINE];
    char *argv[MAX_ARGS];
    pid_t pid;

    while (1) {
        printf("shell> ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) break; // EOF 처리
        buf[strcspn(buf, "\n")] = '\0'; // 줄바꿈 제거

        if (strlen(buf) == 0) continue; // 빈 입력 무시

        int narg = getargs(buf, argv);

        if (strcmp(argv[0], "exit") == 0) break; // 종료 명령어 처리
        //여기서부터

        /*pid = fork();

        if (pid == 0) { // 자식 프로세스
            execvp(argv[0], argv);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) { // 부모 프로세스
            wait(NULL); // 자식 대기
        } else {
            perror("fork failed");
        }
        */

        //여기까지 지우고 및에 함수 두개 추가해줘
        //execute_command(argv)에 위에 코드 들어가있고 2번 백그라운드 처리 포함돼있음

        // rmdir, ln, cp 명령어 직접 처리
        handle_builtin_commands(argv);

        // 시스템 명령 실행
        execute_command(argv);
    }
    return 0;
}

int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        while (*cmd == ' ' || *cmd == '\t') *cmd++ = '\0'; // 공백 제거
        if (*cmd) argv[narg++] = cmd;
        while (*cmd && *cmd != ' ' && *cmd != '\t') cmd++;
    }
    argv[narg] = NULL;
    return narg;
}


// argv 배열을 받아 rmdir, ln, cp, rm, mv, cat 명령어 실행
void handle_builtin_commands(char **argv) {
    // rmdir 명령어 구현
    if (strcmp(argv[0], "rmdir") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "rmdir: missing operand\n");
            return;
        }
        if (rmdir(argv[1]) != 0) {
            perror("rmdir");
        }
        return;
    }
    // ln 명령어 구현
    if (strcmp(argv[0], "ln") == 0) {
        int symbolic = 0; // 심볼릭 링크 여부 플래그
        // 옵션 검사
        if (argv[1] != NULL && strcmp(argv[1], "-s") == 0) {
            symbolic = 1; // 심볼릭 링크 생성
            argv++;       // 옵션 제거
        }
        if (argv[1] == NULL || argv[2] == NULL) {
            fprintf(stderr, "ln: missing operand\n");
            return;
        }
        if (symbolic) {
            // 심볼릭 링크 생성
            if (symlink(argv[1], argv[2]) != 0) {
                perror("ln (symbolic)");
            }
        } else {
            // 하드 링크 생성
            if (link(argv[1], argv[2]) != 0) {
                perror("ln (hard)");
            }
        }
        return;
    }

    // cp 명령어 구현
    if (strcmp(argv[0], "cp") == 0) {
        if (argv[1] == NULL || argv[2] == NULL) {
            fprintf(stderr, "cp: missing operand\n");
            return;
        }

        int src_fd = open(argv[1], O_RDONLY);
        if (src_fd < 0) {
            perror("cp: open source");
            return;
        }

        int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dest_fd < 0) {
            perror("cp: open destination");
            close(src_fd);
            return;
        }

        char buffer[4096];
        ssize_t bytes;
        while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
            if (write(dest_fd, buffer, bytes) != bytes) {
                perror("cp: write");
                close(src_fd);
                close(dest_fd);
                return;
            }
        }

        close(src_fd);
        close(dest_fd);

        if (bytes < 0) {
            perror("cp: read");
            return;
        }
        return;
    }
    // rm 명령어 구현
    if (strcmp(argv[0], "rm") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "rm: missing operand\n");
            return;
        }
        if (unlink(argv[1]) != 0) {
            perror("rm");
        }
        return;
    }

    // mv 명령어 구현
    if (strcmp(argv[0], "mv") == 0) {
        if (argv[1] == NULL || argv[2] == NULL) {
            fprintf(stderr, "mv: missing operand\n");
            return;
        }
        if (rename(argv[1], argv[2]) != 0) {
            perror("mv");
        }
        return;
    }

    // cat 명령어 구현
    if (strcmp(argv[0], "cat") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cat: missing operand\n");
            return;
        }
        for (int i = 1; argv[i] != NULL; i++) {
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0) {
                perror("cat");
                continue;
            }
            char buffer[1024];
            ssize_t bytes;
            while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
                write(STDOUT_FILENO, buffer, bytes);
            }
            close(fd);
        }
        return;
    }
}

// 명령어 실행 함수
void execute_command(char **argv) {
    pid_t pid;
    int background = 0;

    // '&' 처리: 백그라운드 실행
    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "&") == 0) {
            background = 1;
            argv[i] = NULL; // '&' 제거
            break;
        }
    }

    pid = fork();
    if (pid == 0) { // 자식 프로세스
        execvp(argv[0], argv);
        perror("exec failed");
        exit(1);
    } else if (pid > 0) { // 부모 프로세스
        if (!background) {
            waitpid(pid, NULL, 0); // 포그라운드 실행
        } else {
            printf("[Background process started with PID %d]\n", pid);
        }
    } else {
        perror("fork failed");
    }
}
  
