#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// 종료 시 호출될 함수
void on_exit_function(void) {
    printf("[종료] 프로그램이 종료됩니다.\n");
}

int main() {
    pid_t pid;  // 프로세스 ID
    int status; // 자식 프로세스 종료 상태

    // 종료 핸들러 등록
    if (atexit(on_exit_function) != 0) {
        perror("atexit 실패");
        return EXIT_FAILURE;
    }

    printf("[부모] 프로세스를 시작합니다. PID: %d\n", getpid());

    // 프로세스 생성
    pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // 자식 프로세스
        printf("[자식] 자식 프로세스가 시작되었습니다. PID: %d\n", getpid());
        printf("[자식] 부모 PID: %d\n", getppid());

        // 다른 프로그램 실행 (예: `/bin/ls`)
        printf("[자식] 다른 프로그램을 실행합니다.\n");
        execl("/bin/ls", "ls", "-l", NULL);

        // exec 함수가 실패할 경우
        perror("[자식] exec 실패");
        exit(EXIT_FAILURE); // 자식 프로세스 종료
    } else {
        // 부모 프로세스
        printf("[부모] 자식 프로세스(PID: %d)를 생성했습니다.\n", pid);

        // 자식 프로세스 종료 대기
        pid_t waited_pid = waitpid(pid, &status, 0);
        if (waited_pid == -1) {
            perror("[부모] waitpid 실패");
            return EXIT_FAILURE;
        }

        // 자식 프로세스 종료 상태 확인
        if (WIFEXITED(status)) {
            printf("[부모] 자식 프로세스가 종료되었습니다. 종료 코드: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[부모] 자식 프로세스가 신호 %d로 종료되었습니다.\n", WTERMSIG(status));
        }

        printf("[부모] 모든 작업이 완료되었습니다.\n");
    }

    return EXIT_SUCCESS;
}
