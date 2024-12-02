#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

void list_dir_recursively(const char *path, int depth) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("디렉토리를 열 수 없습니다");
        return;
    }

    printf("%*s[%s]:\n", depth * 2, "", path); // 디렉토리 출력 (들여쓰기 포함)

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // 현재 디렉토리(.)와 상위 디렉토리(..) 건너뛰기
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (stat(full_path, &entry_stat) == 0) {
            if (S_ISDIR(entry_stat.st_mode)) {
                // 디렉토리인 경우 재귀 호출
                list_dir_recursively(full_path, depth + 1);
            } else {
                // 파일인 경우 출력
                printf("%*s- %s\n", (depth + 1) * 2, "", full_path);
            }
        }
    }

    closedir(dir);
}

int main() {
    char dir_path[1024];

    // 사용자로부터 디렉토리 경로 입력받기
    printf("탐색할 디렉토리 경로를 입력하세요: ");
    if (fgets(dir_path, sizeof(dir_path), stdin) == NULL) {
        perror("입력 오류");
        return 1;
    }

    // 입력값에서 개행 문자 제거
    dir_path[strcspn(dir_path, "\n")] = '\0';

    // 디렉토리 재귀적으로 나열
    list_dir_recursively(dir_path, 0);

    return 0;
}
