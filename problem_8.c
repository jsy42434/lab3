#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// 시그널 핸들러 함수
void signal_handler(int signo) {
    switch (signo) {
        case SIGINT:
            printf("\n[핸들러] SIGINT (Ctrl+C) 시그널을 받았습니다.\n");
            break;
        case SIGTERM:
            printf("[핸들러] SIGTERM 시그널을 받았습니다. 프로그램을 종료합니다.\n");
            exit(EXIT_SUCCESS);
            break;
        case SIGUSR1:
            printf("[핸들러] SIGUSR1 사용자 정의 시그널을 받았습니다.\n");
            break;
        default:
            printf("[핸들러] 알 수 없는 시그널: %d\n", signo);
            break;
    }
}

int main() {
    printf("프로그램 PID: %d\n", getpid());
    printf("SIGINT (Ctrl+C), SIGTERM, SIGUSR1 시그널을 처리합니다.\n\n");

    // 시그널 핸들러 등록
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("SIGINT 핸들러 등록 실패");
        return EXIT_FAILURE;
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("SIGTERM 핸들러 등록 실패");
        return EXIT_FAILURE;
    }

    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("SIGUSR1 핸들러 등록 실패");
        return EXIT_FAILURE;
    }

    // 무한 루프: 시그널 수신 대기
    while (1) {
        printf("프로그램이 실행 중입니다... (Ctrl+C로 종료 가능)\n");
        sleep(5); // 5초 대기
    }

    return EXIT_SUCCESS;
}
