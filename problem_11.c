#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SHM_SIZE 1024 // 공유 메모리 크기

// 세마포어 연산 구조체
struct sembuf sem_lock = {0, -1, 0}; // 세마포어 잠금
struct sembuf sem_unlock = {0, 1, 0}; // 세마포어 해제

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <원본 파일> <복사본 파일>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *src_file = argv[1];
    const char *dest_file = argv[2];

    // 공유 메모리 및 세마포어 생성
    key_t key = ftok(".", 'A');
    int shm_id = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("공유 메모리 생성 실패");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(key, 1, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("세마포어 생성 실패");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // 세마포어 초기화
    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("세마포어 초기화 실패");
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    // 공유 메모리 연결
    char *shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("공유 메모리 연결 실패");
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // 자식 프로세스 (쓰기)
        int fd_out = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out < 0) {
            perror("출력 파일 열기 실패");
            exit(EXIT_FAILURE);
        }

        while (1) {
            // 세마포어 잠금
            semop(sem_id, &sem_lock, 1);

            // 종료 조건
            if (strcmp(shm_ptr, "EOF") == 0) {
                semop(sem_id, &sem_unlock, 1); // 세마포어 해제
                break;
            }

            // 공유 메모리에서 읽어서 파일에 쓰기
            write(fd_out, shm_ptr, strlen(shm_ptr));

            // 세마포어 해제
            semop(sem_id, &sem_unlock, 1);
        }

        close(fd_out);
        exit(EXIT_SUCCESS);
    } else { // 부모 프로세스 (읽기)
        int fd_in = open(src_file, O_RDONLY);
        if (fd_in < 0) {
            perror("입력 파일 열기 실패");
            exit(EXIT_FAILURE);
        }

        while (1) {
            // 세마포어 잠금
            semop(sem_id, &sem_lock, 1);

            // 파일에서 읽어서 공유 메모리에 쓰기
            ssize_t bytes_read = read(fd_in, shm_ptr, SHM_SIZE - 1);
            if (bytes_read < 0) {
                perror("파일 읽기 실패");
                semop(sem_id, &sem_unlock, 1); // 세마포어 해제
                break;
            }

            shm_ptr[bytes_read] = '\0'; // null-terminate

            // 파일의 끝이면 "EOF" 신호
            if (bytes_read == 0) {
                strcpy(shm_ptr, "EOF");
                semop(sem_id, &sem_unlock, 1); // 세마포어 해제
                break;
            }

            // 세마포어 해제
            semop(sem_id, &sem_unlock, 1);
        }

        close(fd_in);

        // 자식 프로세스 종료 대기
        wait(NULL);

        // 공유 메모리 및 세마포어 해제
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
    }

    return 0;
}
