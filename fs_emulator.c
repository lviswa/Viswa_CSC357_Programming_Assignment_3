#define _GNU_SOURCE
#define INODE_LIST "inodes_list_2"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>


void loadInodeList(const char *path);
void saveInodeList(const char *path);
void listContents();
void changeDirectory(char *name);
void createFile(char *name, int name_length);
void createDirectory(char *name);

typedef struct {
	uint32_t inodeNum;
	uint32_t parentInodeNum;
	char type;
	char name[32];
} inode;

inode inodeList[1024];
size_t inodeCount = 0;
uint32_t currentInode = 0;

char *uint32_to_str(uint32_t i) {
	int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
	char* str = (char*) malloc(sizeof(char) * (length + 1));       // allocate space for the actual string
	if(str == NULL) {
		exit(1);
	}   
	snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string
	return str;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Invalid number of arguments\n");
		return 1;
	}
	if (chdir(argv[1]) != 0) {
                perror("Error changing directory\n");
                return 1;
        }

        printf("Current working directory changed to: %s\n",  argv[1]);
	
	loadInodeList(INODE_LIST);

	char *buffer = NULL;
	size_t bufsize = 0;
	ssize_t characters_read;
	char *token;
	int i;

	while(characters_read = getline(&buffer, &bufsize, stdin) != -1) {
		if (strcmp(buffer, "exit\n\0", 6) == 0 || feof(stdin)) {
			free(buffer);
			saveInodeList(INODE_LIST);
			exit(0);
		}
		else if (strncmp(buffer, "ls\n\0", 4) == 0) {
			listContents();
                }
		else if (strncmp(buffer, "cd ", 3) == 0) {
                	token = strtok(buffer, " ");
			token = strtok(NULL, " ");
			i = 0;
			while(token[i] != '\n') {
                                i++;
                        }
                        token[i] = '\0';
			changeDirectory(token);
		}
		else if (strncmp(buffer, "touch ", 6) == 0) {
			token = strtok(buffer, " ");    
                        token = strtok(NULL, " ");
			i = 0;
			while(token[i] != '\n') {
                                i++;
                        }
                        token[i] = '\0';
			if (strlen(token) > 32) {
				createFile(token, 32);
			}
			else {
				createFile(token, strlen(token));
			}
                }
		else if (strncmp(buffer, "mkdir ", 6) == 0) {
                	token = strtok(buffer, " ");    
                        token = strtok(NULL, " ");
			i = 0;
			while(token[i] != '\n') {
				i++;
			}
			token[i] = '\0';
			createDirectory(token);
		}
		else {
			printf("Invalid command\n");
		}
		free(buffer);
    		buffer = NULL;
	}

	return 0;
}

void loadInodeList(const char *path) {
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		perror("Error opening file");
		return;
	}

	inode tempInodeList[1024];
	size_t tempInodeCount = 0;

	tempInodeCount = fread(tempInodeList, sizeof(inode), 1024, file);

	if (tempInodeCount == 0) {
		perror("Error reading file");
		fclose(file);
		return;
	}

	for (int i = 0; i < tempInodeCount; i++) {
		if(tempInodeList[i].inodeNum < 1024 && (tempInodeList[i].type == 'f' || tempInodeList[i].type == 'd')) {
			strcpy(inodeList[inodeCount].name, tempInodeList[i].name);
        		inodeList[inodeCount].type = tempInodeList[i].type;
        		inodeList[inodeCount].parentInodeNum = tempInodeList[i].parentInodeNum;
        		inodeList[inodeCount].inodeNum = tempInodeList[i].inodeNum;
			inodeCount++;
		}
	}

	fclose(file);
}

void saveInodeList(const char *path) {
	FILE *file = fopen(path, "wb");
        if (file == NULL) {
                perror("Error opening file");
                return;
        }
	
	size_t elements_written = fwrite(inodeList, sizeof(inode), inodeCount, file);
	
	if (elements_written < inodeCount) {
		perror("Error writing inodeList to file");
		fclose(file);
		return;
	}

	fclose(file);	
}

void listContents() {
	for (int i = 0; i < inodeCount; i++) {
		if (inodeList[i].parentInodeNum == currentInode) {
			printf("%s\t%u\n", inodeList[i].name, inodeList[i].inodeNum);
		}
	}
}

void changeDirectory(char *name) {
	char *curr_dir = ".";
	char *parent_dir = ".."; 
	if (strcmp(curr_dir, name) == 0) {
		printf("Current working directory changed to: %s\n",  inodeList[currentInode].name);
		return;
	}
	if (strcmp(parent_dir, name) == 0) {
		currentInode = inodeList[currentInode].parentInodeNum;
		printf("Current working directory changed to: %s\n",  inodeList[currentInode].name);
		return;
	}
	for(int i = 0; i < inodeCount; i++) {
		if(strcmp(inodeList[i].name, name) == 0 && inodeList[i].type == 'd') {
			currentInode = inodeList[i].inodeNum;
			printf("Current working directory changed to: %s\n",  inodeList[i].name);
			return;
		}
	}
	printf("Cannot change into entered directory\n");
}

void createFile(char *name, int name_length) {
	if (inodeCount == 1024) {
		printf("inodeList has reached capacity");
		return;
	}

	for (int i = currentInode; i < inodeCount; i++) {
                if (strcmp(inodeList[i].name, name) == 0 && inodeList[i].parentInodeNum == currentInode) {
                        printf("File already exists");
                        return;
                }
        }
	
	strncpy(inodeList[inodeCount].name, name, 32);
	inodeList[inodeCount].type = 'f';
	inodeList[inodeCount].parentInodeNum = currentInode;
	inodeList[inodeCount].inodeNum = inodeCount; 
	
	char *inode_num_str = uint32_to_str(inodeList[inodeCount].inodeNum);
        char *parent_inode_num_str = uint32_to_str(inodeList[inodeCount].parentInodeNum);
        
	FILE *new_file = fopen(inode_num_str, "wb"); 
	if (new_file == NULL) {
                perror("Error opening new file");
                return;
        }

        FILE *directory = fopen(parent_inode_num_str, "ab");
        if (directory == NULL) {
                perror("Error opening file");
                return;
        }

        free(inode_num_str);
        free(parent_inode_num_str);

	size_t temp_elem_check;

	temp_elem_check = fwrite(inodeList[inodeCount].name, sizeof(char), name_length, new_file);
	if (temp_elem_check < name_length) {
		perror("Error writing name to new file");
		fclose(new_file);
		fclose(directory);
		return;
	}

	temp_elem_check = fwrite(&inodeList[inodeCount].inodeNum, sizeof(uint32_t), 1, directory);
	if (temp_elem_check < 1) {
                perror("Error writing new file inodeNum to directory file");
                fclose(new_file);
                fclose(directory);
                return;
        }
	temp_elem_check = fwrite(inodeList[inodeCount].name, sizeof(char), 32, directory);
	if (temp_elem_check < 32) {
                perror("Error writing new file name to directory");
                fclose(new_file);
                fclose(directory);
                return; 
        }
	inodeCount++;

	fclose(new_file);
	fclose(directory);	
}

void createDirectory(char *name) {
	if (inodeCount == 1024) {
                printf("inodeList has reached capacity");
                exit(1);
        }

	for (int i = 0; i < inodeCount; i++) {
		if (strcmp(inodeList[i].name, name) == 0 && inodeList[i].parentInodeNum == currentInode) {
			printf("Directory already exists");
			exit(1);
		}
	}
	
        strcpy(inodeList[inodeCount].name, name);
        inodeList[inodeCount].type = 'd';
        inodeList[inodeCount].parentInodeNum = currentInode;
        inodeList[inodeCount].inodeNum = inodeCount;
	
	char *inode_num_str = uint32_to_str(inodeList[inodeCount].inodeNum);
	char *parent_inode_num_str = uint32_to_str(inodeList[inodeCount].parentInodeNum);
	
	FILE *new_directory = fopen(inode_num_str, "wb");
	if (new_directory == NULL) {
		perror("Error opening file for new directory");
		return;
	}
	
	FILE *parent_directory = fopen(parent_inode_num_str, "ab");
	if (parent_directory == NULL) {
                perror("Error opening parent directory file");
                return;
        }

	free(inode_num_str);
	free(parent_inode_num_str);
	char cur_dir[32];
	char par_dir[32];
	strncpy(cur_dir, ".", 32);
	strncpy(par_dir, "..", 32);

	size_t temp_elem_check = 0;
	temp_elem_check += fwrite(cur_dir, sizeof(char), 32, new_directory);
	temp_elem_check += fwrite(&inodeList[inodeCount].inodeNum, sizeof(uint32_t), 1, new_directory);
	temp_elem_check += fwrite(par_dir, sizeof(char), 32, new_directory);
	temp_elem_check += fwrite(&inodeList[inodeCount].parentInodeNum, sizeof(uint32_t), 1, new_directory);
	
	if (temp_elem_check < 66) {
                perror("Error writing new file name to directory");
                fclose(new_directory);
                fclose(parent_directory);
                return;
        }

	temp_elem_check = fwrite(&inodeList[inodeCount].inodeNum, sizeof(uint32_t), 1, parent_directory);
	temp_elem_check += fwrite(inodeList[inodeCount].name, sizeof(char), 32, parent_directory);	

	if (temp_elem_check < 33) {
                perror("Error writing new file name to directory");
                fclose(new_directory);
                fclose(parent_directory);
                return;
        }
	inodeCount++;	
	fclose(new_directory);
	fclose(parent_directory);
}
	
