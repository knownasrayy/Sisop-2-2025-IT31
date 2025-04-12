
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


//      FUNCTION DECLARATIONS

void exec_cmd(char *cmd, char *arg1, char *arg2, char *arg3);
void downloadClues();     // a. Downloading the Clues
void filterFiles();       // b. Filtering the Files
void combineFiles();      // c. Combine the File Content
void decodeFile();        // d. Decode the file


//   EXEC COMMAND HELPER FUNC

void exec_cmd(char *cmd, char *arg1, char *arg2, char *arg3) {
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {cmd, arg1, arg2, arg3, NULL};
        execvp(cmd, argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork failed");
    }
}


// a. DOWNLOADING THE CLUES SECTION

void downloadClues() {
    struct stat st = {0};

    // Jika folder "Clues" belum ada, lakukan proses download dan extract
    if (stat("Clues", &st) == -1) {
        pid_t pid = fork();
        if (pid == 0) {
            // Download file Clues.zip dari link menggunakan wget
            execl("/bin/sh", "sh", "-c",
                  "wget --no-check-certificate --content-disposition \"https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download\" -O Clues.zip",
                  NULL);
            perror("wget failed");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }

        // Buat folder Clues
        mkdir("Clues", 0777);

        // Extract Clues.zip ke folder Clues
        pid = fork();
        if (pid == 0) {
            char *argv[] = {"unzip", "Clues.zip", "-d", "Clues", NULL};
            execvp("unzip", argv);
            perror("unzip failed");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }

        // Jika hasil unzip berada dalam nested folder (Clues/Clues), pindahkan isinya
        struct stat nested;
        if (stat("Clues/Clues", &nested) == 0) {
            pid = fork();
            if (pid == 0) {
                execl("/bin/sh", "sh", "-c", "mv Clues/Clues/* Clues/", NULL);
                perror("mv failed");
                exit(EXIT_FAILURE);
            } else {
                wait(NULL);
                rmdir("Clues/Clues");
            }
        }

        // Hapus zip setelah extract
        unlink("Clues.zip");
        printf("Berhasil download dan extract Clues.zip\n");
    } else {
        printf("Folder Clues sudah ada, skip download.\n");
    }
}


// b. FILTERING THE FILES SECTION

void filterFiles() {
    DIR *dir = opendir("./Clues");
    if (!dir) {
        printf("Folder Clues tidak ditemukan!\n");
        return;
    }

    // Bersihkan folder Filtered jika sudah ada
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", "rm -rf Filtered", NULL);
        exit(0);
    } else {
        wait(NULL);
    }
    mkdir("Filtered", 0777);

    struct dirent *de, *sub;
    char path[256];

    // Loop setiap folder di dalam Clues/
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            snprintf(path, sizeof(path), "./Clues/%s", de->d_name);
            DIR *subdir = opendir(path);
            if (!subdir) continue;

            // Loop setiap file dalam subfolder
            while ((sub = readdir(subdir)) != NULL) {
                if (sub->d_type == DT_REG) {
                    int len = strlen(sub->d_name);
                    // Syarat: hanya file dengan nama 1 huruf/angka + ".txt" (panjang 5 karakter)
                    if (len == 5 && ((sub->d_name[0] >= 'a' && sub->d_name[0] <= 'z') || (sub->d_name[0] >= '0' && sub->d_name[0] <= '9')) &&
                        strcmp(&sub->d_name[1], ".txt") == 0) {
                        char src[256], dest[256];
                        snprintf(src, sizeof(src), "%s/%s", path, sub->d_name);
                        snprintf(dest, sizeof(dest), "Filtered/%s", sub->d_name);
                        rename(src, dest); // Pindahkan ke folder Filtered
                    } else {
                        char to_delete[256];
                        snprintf(to_delete, sizeof(to_delete), "%s/%s", path, sub->d_name);
                        remove(to_delete); // Hapus jika tidak sesuai kriteria
                    }
                }
            }
            closedir(subdir);
        }
    }

    closedir(dir);
    printf("Filtering selesai!\n");
}




// c. COMBINE THE FILE CONTENTS TO ONE TXT

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

    // Pisahkan file nama angka dan huruf
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

    // Urutkan angka (numerik)
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

    // Urutkan huruf (alphabet)
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

    // Gabung isi file dengan urutan angka → huruf → angka → huruf...
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
                remove(path); // Hapus setelah isi diproses
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

// d. DECODE COMBINED.TXT

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

    // Lakukan decode rot13 karakter demi karakter
    char c;
    while ((c = fgetc(in)) != EOF) {
        fputc(rot13(c), out);
    }

    fclose(in);
    fclose(out);
    printf("Decode selesai!\n");
}

//      MAIN FUNCTION
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
            printf("./action              --> Download dan extract Clues\n");
            printf("./action -m Filter    --> Filter file\n");
            printf("./action -m Combine   --> Combine file\n");
            printf("./action -m Decode    --> Decode file\n");
        }
    } else {
        printf("Usage:\n");
        printf("./action              --> Download dan extract Clues\n");
        printf("./action -m Filter    --> Filter file\n");
        printf("./action -m Combine   --> Combine file\n");
        printf("./action -m Decode    --> Decode file\n");
    }

    return 0;
}

