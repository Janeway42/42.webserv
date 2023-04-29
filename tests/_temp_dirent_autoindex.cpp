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
#include <string>
#include <iostream>

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

	std::string folders;
	std::string files;
	std::string allFolderContent;

	while ((entry = readdir(dir)) != NULL) {                                    //      . /    filename
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);   // joins path + d_name and stores it into full_path
	//  printf("--- %s\n", full_path);

		if (stat(full_path, &file_stat) == -1) {
			printf("Failed to get file information for %s\n", full_path);
			continue;
		}
		if (S_ISREG(file_stat.st_mode)) {
			files.append("F:").append(entry->d_name).append("\n");
			// printf("F: %s\n", entry->d_name);
			std::cout << files << "\n";
		} else if (S_ISDIR(file_stat.st_mode)) {
			folders.append("D:").append(entry->d_name).append("\n");
			// printf("D: %s\n", entry->d_name);
			std::cout << folders << "\n";
		}
	}
	closedir(dir);
	allFolderContent.append(folders).append(files);
	std::cout << "ALL CONTENT:\n" << allFolderContent;
}


int main() {

	print_directory_contents(".");

}