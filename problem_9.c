#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define BUFFER_SIZE 256

int main() {
    int pipefd[2];                // 파이프 파일 디스크립터
    pid_t pid;                    // 프로세스 ID
    char write_msg[] = "자식 프로세스 hi";  // 부모 -> 자식으로 보낼 메시지
    char read_msg[BUFFER_SIZE];   // 자식 -> 부모로 보낼 메시지

    // 파이프 생성
    if (pipe(pipefd) == -1) {
        perror("파이프 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 프로세스 생성
    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // 자식 프로세스
        close(pipefd[1]); // 쓰기 끝 닫기 (자식은 읽기만 함)
        read(pipefd[0], read_msg, sizeof(read_msg)); // 파이프에서 데이터 읽기
        printf("자식 프로세스가 받은 메시지: %s\n", read_msg);
        close(pipefd[0]); // 읽기 끝 닫기
        exit(EXIT_SUCCESS);
    } else {  // 부모 프로세스
        close(pipefd[0]); // 읽기 끝 닫기 (부모는 쓰기만 함)
        write(pipefd[1], write_msg, strlen(write_msg) + 1); // 파이프에 데이터 쓰기
        close(pipefd[1]); // 쓰기 끝 닫기
        wait(NULL); // 자식 프로세스 종료 대기
    }

    return 0;
}
