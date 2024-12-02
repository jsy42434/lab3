#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_TEXT 256

// 메시지 구조체 정의
struct message {
    long msg_type;          // 메시지 타입 (1: 부모 -> 자식, 2: 자식 -> 부모)
    char text[MAX_TEXT];    // 메시지 내용
};

int main() {
    int msgid;
    pid_t pid;
    struct message msg;
    key_t key = ftok(".", 'A'); // 고유 키 생성

    // 메시지 큐 생성
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("메시지 큐 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 자식 프로세스 생성
    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // 자식 프로세스 (수신 역할)
        while (1) {
            // 메시지 읽기
            if (msgrcv(msgid, &msg, sizeof(msg.text), 1, 0) == -1) {
                perror("메시지 수신 실패");
                exit(EXIT_FAILURE);
            }

            // "exit" 메시지 처리
            if (strncmp(msg.text, "exit", 4) == 0) {
                printf("[자식] 채팅 종료 요청을 받았습니다.\n");
                break;
            }

            printf("[자식] 부모로부터 받은 메시지: %s\n", msg.text);

            // 부모로 받은 메시지를 임시 변수에 저장
            char received_msg[MAX_TEXT];
            strncpy(received_msg, msg.text, MAX_TEXT - 1);
            received_msg[MAX_TEXT - 1] = '\0'; // Null-terminate

            // 자식 -> 부모로 메시지 보내기
            msg.msg_type = 2;
            snprintf(msg.text, MAX_TEXT, "자식이 응답: %.230s", received_msg);
            if (msgsnd(msgid, &msg, sizeof(msg.text), 0) == -1) {
                perror("메시지 송신 실패");
                exit(EXIT_FAILURE);
            }
        }
    } else {  // 부모 프로세스 (송신 역할)
        while (1) {
            printf("[부모] 메시지 입력: ");
            fgets(msg.text, MAX_TEXT, stdin);
            msg.text[strcspn(msg.text, "\n")] = '\0'; // 개행 문자 제거

            // "exit" 메시지 처리
            if (strncmp(msg.text, "exit", 4) == 0) {
                msg.msg_type = 1;
                if (msgsnd(msgid, &msg, sizeof(msg.text), 0) == -1) {
                    perror("메시지 송신 실패");
                }
                break;
            }

            // 부모 -> 자식으로 메시지 보내기
            msg.msg_type = 1;
            if (msgsnd(msgid, &msg, sizeof(msg.text), 0) == -1) {
                perror("메시지 송신 실패");
                exit(EXIT_FAILURE);
            }

            // 자식 -> 부모로 메시지 수신
            if (msgrcv(msgid, &msg, sizeof(msg.text), 2, 0) == -1) {
                perror("메시지 수신 실패");
                exit(EXIT_FAILURE);
            }
            printf("[부모] 자식으로부터 응답: %s\n", msg.text);
        }

        // 자식 프로세스 종료 대기
        wait(NULL);

        // 메시지 큐 삭제
        if (msgctl(msgid, IPC_RMID, NULL) == -1) {
            perror("메시지 큐 삭제 실패");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
