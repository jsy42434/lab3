#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// 사용자 정의 system 함수
int my_system(const char *command) {
    if (command == NULL) {
        return -1; // 명령어가 NULL인 경우 에러
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        return -1;
    }

    if (pid == 0) {
        // 자식 프로세스: 명령 실행
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl 실패");
        exit(EXIT_FAILURE); // exec 호출 실패 시 종료
    } else {
        // 부모 프로세스: 자식 프로세스 대기
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid 실패");
            return -1;
        }
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "사용법: %s <쉘 명령>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 명령어 합치기
    char command[1024] = {0};
    for (int i = 1; i < argc; i++) {
        strcat(command, argv[i]);
        if (i < argc - 1) {
            strcat(command, " ");
        }
    }

    printf("명령어 실행: %s\n", command);

    // 사용자 정의 system 함수 호출
    int result = my_system(command);
    if (result == -1) {
        fprintf(stderr, "명령어 실행 실패\n");
    } else {
        printf("명령어 실행 성공, 종료 코드: %d\n", result);
    }

    return 0;
}
