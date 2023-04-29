#include <dirent.h>
#include <stdio.h>

// int main() {
//     DIR* dir;
//     struct dirent* ent;

//     // Open the current directory
//     dir = opendir(".");
//     if (dir == NULL) {
//         printf("Failed to open directory.\n");
//         return -1;
//     }

//     // Read each entry in the directory
//     while ((ent = readdir(dir)) != NULL) {
//         printf("%s\n", ent->d_name);
//     }

//     // Close the directory
//     closedir(dir);
//     return 0;
// }



#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void print_directory_contents(const char* path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[512];

    dir = opendir(path);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", path);
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) == -1) {
            printf("Failed to get file information for %s\n", full_path);
            continue;
        }

        if (S_ISREG(file_stat.st_mode)) {
            printf("File: %s\n", entry->d_name);
        } else if (S_ISDIR(file_stat.st_mode)) {
            printf("Directory: %s\n", entry->d_name);
        }
    }

    closedir(dir);
}


int main() {

    print_directory_contents(".");

}