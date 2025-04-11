#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void downloadClues() {
	struct stat st = {0};

	if (stat("Clues", &st) == -1) {
    	system("wget --no-check-certificate 'https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK' -O Clues.zip");
    	mkdir("Clues", 0777);
    	system("unzip Clues.zip -d Clues");

    	struct stat nested;
    	if (stat("Clues/Clues", &nested) == 0) {
        	system("mv Clues/Clues/* Clues/");
        	system("rm -r Clues/Clues");
    	}

    	system("rm Clues.zip");
    	printf("Berhasil download dan extract Clues.zip\n");
	} else {
    	printf("Folder Clues sudah ada, skip download.\n");
	}
}

void filterFiles() {
	DIR *dir = opendir("./Clues");
	if (!dir) {
    	printf("Folder Clues tidak ditemukan!\n");
    	return;
	}

	system("rm -rf Filtered");
	mkdir("Filtered", 0777);

	struct dirent *de, *sub;
	char path[256];

	while ((de = readdir(dir)) != NULL) {
    	if (de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
        	snprintf(path, sizeof(path), "./Clues/%s", de->d_name);
        	DIR *subdir = opendir(path);
        	if (!subdir) continue;

        	while ((sub = readdir(subdir)) != NULL) {
            	if (sub->d_type == DT_REG) {
                	int len = strlen(sub->d_name);
                	if (len == 5 && ((sub->d_name[0] >= 'a' && sub->d_name[0] <= 'z') || (sub->d_name[0] >= '0' && sub->d_name[0] <= '9')) &&
                    	strcmp(&sub->d_name[1], ".txt") == 0) {
                    	char src[256], dest[256];
                    	snprintf(src, sizeof(src), "%s/%s", path, sub->d_name);
                    	snprintf(dest, sizeof(dest), "mv '%s' Filtered/", src);
                    	system(dest);
                	} else {
                    	char to_delete[256];
                    	snprintf(to_delete, sizeof(to_delete), "%s/%s", path, sub->d_name);
                    	remove(to_delete);
                	}
            	}
        	}
        	closedir(subdir);
    	}
	}

	closedir(dir);
	printf("Filtering selesai!\n");
}

void combineFiles() {
	DIR *dir = opendir("./Filtered");
	if (!dir) {
    	printf("Folder Filtered tidak ditemukan!\n");
    	return;
	}

	FILE *combined = fopen("Combined.txt", "w");
	if (!combined) {
    	printf("Gagal membuat Combined.txt\n");
    	return;
	}

	struct dirent *de;
	char angka[100][100], huruf[100][100];
	int idxA = 0, idxH = 0;

	while ((de = readdir(dir)) != NULL) {
    	if (de->d_type == DT_REG) {
        	if (de->d_name[0] >= '0' && de->d_name[0] <= '9')
            	strcpy(angka[idxA++], de->d_name);
        	else if (de->d_name[0] >= 'a' && de->d_name[0] <= 'z')
            	strcpy(huruf[idxH++], de->d_name);
    	}
	}
	closedir(dir);

	for (int i = 0; i < idxA - 1; i++) {
    	for (int j = i + 1; j < idxA; j++) {
        	if (atoi(angka[i]) > atoi(angka[j])) {
            	char temp[100];
            	strcpy(temp, angka[i]);
            	strcpy(angka[i], angka[j]);
            	strcpy(angka[j], temp);
        	}
    	}
	}

	for (int i = 0; i < idxH - 1; i++) {
    	for (int j = i + 1; j < idxH; j++) {
        	if (strcmp(huruf[i], huruf[j]) > 0) {
            	char temp[100];
            	strcpy(temp, huruf[i]);
            	strcpy(huruf[i], huruf[j]);
            	strcpy(huruf[j], temp);
        	}
    	}
	}

	int i = 0, j = 0;
	while (i < idxA || j < idxH) {
    	if (i < idxA) {
        	char path[256], buf[256];
        	snprintf(path, sizeof(path), "./Filtered/%s", angka[i++]);
        	FILE *fp = fopen(path, "r");
        	if (fp) {
            	while (fgets(buf, sizeof(buf), fp))
                	fputs(buf, combined);
            	fclose(fp);
            	remove(path);
        	}
    	}
    	if (j < idxH) {
        	char path[256], buf[256];
        	snprintf(path, sizeof(path), "./Filtered/%s", huruf[j++]);
        	FILE *fp = fopen(path, "r");
        	if (fp) {
            	while (fgets(buf, sizeof(buf), fp))
                	fputs(buf, combined);
            	fclose(fp);
            	remove(path);
        	}
    	}
	}

	fclose(combined);
	printf("Combine selesai!\n");
}

char rot13(char c) {
	if ('a' <= c && c <= 'z') return 'a' + (c - 'a' + 13) % 26;
	if ('A' <= c && c <= 'Z') return 'A' + (c - 'A' + 13) % 26;
	return c;
}

void decodeFile() {
	FILE *in = fopen("Combined.txt", "r");
	FILE *out = fopen("Decoded.txt", "w");
	if (!in || !out) {
    	printf("Gagal membuka file\n");
    	return;
	}

	char c;
	while ((c = fgetc(in)) != EOF) {
    	fputc(rot13(c), out);
	}

	fclose(in);
	fclose(out);
	printf("Decode selesai!\n");
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
    	downloadClues();
	} else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
    	if (strcmp(argv[2], "Filter") == 0) {
        	filterFiles();
    	} else if (strcmp(argv[2], "Combine") == 0) {
        	combineFiles();
    	} else if (strcmp(argv[2], "Decode") == 0) {
        	decodeFile();
    	} else {
        	printf("Usage:\n");
        	printf("./action         	--> Download dan extract Clues\n");
        	printf("./action -m Filter   --> Filter file\n");
        	printf("./action -m Combine  --> Combine file\n");
        	printf("./action -m Decode   --> Decode file\n");
    	}
	} else {
    	printf("Usage:\n");
    	printf("./action         	--> Download dan extract Clues\n");
    	printf("./action -m Filter   --> Filter file\n");
    	printf("./action -m Combine  --> Combine file\n");
    	printf("./action -m Decode   --> Decode file\n");
	}

	return 0;
}
