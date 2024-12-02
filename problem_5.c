#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define PASSWORDSIZE 12
#define NUM_SENTENCES 3

int main(void) {
    int fd;
    int nread, cnt, errcnt, total_errors = 0;
    char ch;
    double total_time = 0.0;
    char *sentences[] = {
        "The quick brown fox jumps over the lazy dog.",
        "C programming is fun and educational.",
        "Practice makes perfect, so keep typing."
    };
    struct termios init_attr, new_attr;

    fd = open(ttyname(fileno(stdin)), O_RDWR);
    tcgetattr(fd, &init_attr);
    new_attr = init_attr;
    new_attr.c_lflag &= ~ICANON;
    new_attr.c_lflag &= ~ECHO;
    new_attr.c_cc[VMIN] = 1;
    new_attr.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &new_attr) != 0) {
        fprintf(stderr, "터미널 속성을 설정할 수 없음.\n");
        return 1;
    }

    for (int i = 0; i < NUM_SENTENCES; i++) {
        printf("\n%s\n", sentences[i]);

        cnt = 0;
        errcnt = 0;
        char *text = sentences[i];
        int text_length = strlen(text);

        clock_t start_time = clock();

        while ((nread = read(fd, &ch, 1)) > 0 && ch != '\n') {
            if (ch == text[cnt]) {
                write(fd, &ch, 1); // 올바른 문자는 출력
            } else {
                write(fd, "*", 1); // 잘못 입력한 문자는 '*' 출력
                errcnt++;
            }
            cnt++;
            if (cnt >= text_length) {
                break;
            }
        }

        clock_t end_time = clock();
        double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        //printf("\n문장 %d 결과: 타이핑 오류 %d회, 소요 시간 %.2f초\n", i + 1, errcnt, elapsed_time);

        total_errors += errcnt;
        total_time += elapsed_time;
    }

    double average_wpm = ((NUM_SENTENCES * 50.0) / total_time) * 60.0;

    printf("\n--- 전체 결과 ---\n");
    printf("총 오타 횟수: %d회\n", total_errors);
    printf("평균 분당 타자수(WPM): %.2f\n", average_wpm);

    tcsetattr(fd, TCSANOW, &init_attr);
    close(fd);

    return 0;
}
