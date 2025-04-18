#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define STARTERKIT_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define DECRYPTED_DIR "decrypted"
#define LOG_FILE "activity.log"
#define PID_FILE "decrypt.pid"
#define ZIP_FILE "starter_kit.zip"
#define DOWNLOAD_LINK "https://drive.google.com/uc?export=download&id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS"

// Logging aktivitas
void write_log(const char *msg) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "[%02d-%02d-%d][%02d:%02d:%02d] - %s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec, msg);
    fclose(log);
}

// Download & unzip starter kit jika belum ada atau kosong
void download_and_extract() {
    struct stat st;
    int need_download = 0;

    if (stat(STARTERKIT_DIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        // Folder ada, cek apakah kosong
        DIR *dir = opendir(STARTERKIT_DIR);
        struct dirent *entry;
        int file_count = 0;

        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        file_count++;
                    }
                }
            }
            closedir(dir);
        }

        if (file_count == 0) {
            need_download = 1;
        } else {
            printf("starter_kit already exists and is not empty. Skipping download.\n");
            return;
        }
    } else {
        need_download = 1;
    }

    if (need_download) {
        printf("Downloading starter_kit.zip...\n");
        char command[1024];
        snprintf(command, sizeof(command),
            "command -v wget >/dev/null 2>&1 && wget --no-check-certificate \"%s\" -O %s && unzip -o %s -d %s && rm %s || command -v curl >/dev/null 2>&1 && curl -L \"%s\" -o %s && unzip -o %s -d %s && rm %s",
            DOWNLOAD_LINK, ZIP_FILE, ZIP_FILE, STARTERKIT_DIR, ZIP_FILE,
            DOWNLOAD_LINK, ZIP_FILE, ZIP_FILE, STARTERKIT_DIR, ZIP_FILE);

        int result = system(command);
        if (result == 0) {
            printf("Download and extraction complete.\n");
        } else {
            fprintf(stderr, "Failed to download or extract files.\n");
            exit(EXIT_FAILURE);
        }
    }
}

// Dekripsi nama file dari base64
void base64_decode(const char *src, char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo %s | base64 -d", src);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        fgets(dest, 256, fp);
        pclose(fp);
    }
}

// Daemon dekripsi
void run_daemon_decrypt() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        FILE *f = fopen(PID_FILE, "w");
        if (f) {
            fprintf(f, "%d", pid);
            fclose(f);
        }
        char msg[128];
        sprintf(msg, "Successfully started decryption process with PID %d.", pid);
        write_log(msg);
        return;
    }

    setsid(); // jadi daemon

    while (1) {
        DIR *dir = opendir(STARTERKIT_DIR);
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    char decoded[256] = {0};
                    base64_decode(ent->d_name, decoded);
                    if (strlen(decoded) == 0) continue;
                    decoded[strcspn(decoded, "\n")] = 0;

                    char old_path[512], new_path[512];
                    snprintf(old_path, sizeof(old_path), "%s/%s", STARTERKIT_DIR, ent->d_name);
                    snprintf(new_path, sizeof(new_path), "%s/%s", STARTERKIT_DIR, decoded);

                    if (rename(old_path, new_path) == 0) {
                        char log_msg[512];
                        sprintf(log_msg, "%s renamed to %s", ent->d_name, decoded);
                        write_log(log_msg);
                    }
                }
            }
            closedir(dir);
        }
        sleep(5);
    }
}

// Pindah file dari from -> to
void move_files(const char *from, const char *to, const char *action_msg) {
    DIR *dir = opendir(from);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            char src[512], dest[512];
            snprintf(src, sizeof(src), "%s/%s", from, ent->d_name);
            snprintf(dest, sizeof(dest), "%s/%s", to, ent->d_name);
            if (rename(src, dest) == 0) {
                char log_msg[512];
                sprintf(log_msg, "%s - Successfully %s.", ent->d_name, action_msg);
                write_log(log_msg);
            }
        }
    }
    closedir(dir);
}

void move_to_quarantine() {
    move_files(STARTERKIT_DIR, QUARANTINE_DIR, "moved to quarantine directory");
}

void return_to_starterkit() {
    move_files(QUARANTINE_DIR, STARTERKIT_DIR, "returned to starter kit directory");
}

void delete_quarantine_contents() {
    DIR *dir = opendir(QUARANTINE_DIR);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", QUARANTINE_DIR, ent->d_name);
            if (remove(path) == 0) {
                char log_msg[512];
                sprintf(log_msg, "%s - Successfully deleted.", ent->d_name);
                write_log(log_msg);
            }
        }
    }
    closedir(dir);
}

void shutdown_daemon() {
    FILE *f = fopen(PID_FILE, "r");
    if (!f) {
        printf("No PID file found. Decryption process might not be running.\n");
        return;
    }

    int pid;
    if (fscanf(f, "%d", &pid) != 1) {
        fclose(f);
        printf("Failed to read PID from file.\n");
        return;
    }
    fclose(f);

    if (kill(pid, 0) == -1) {
        printf("No such process with PID %d.\n", pid);
        write_log("PID file found but no such process exists. Removing stale PID file.");
        remove(PID_FILE);
        return;
    }

    if (kill(pid, SIGTERM) == 0) {
        char msg[128];
        sprintf(msg, "Successfully shut off decryption process with PID %d.", pid);
        write_log(msg);
        remove(PID_FILE);
    } else {
        perror("Failed to send SIGTERM");
    }
}

void print_usage() {
    printf("Usage:\n");
    printf("./starterkit --decrypt\n");
    printf("./starterkit --quarantine\n");
    printf("./starterkit --return\n");
    printf("./starterkit --eradicate\n");
    printf("./starterkit --shutdown\n");
}

int main(int argc, char *argv[]) {
    mkdir(STARTERKIT_DIR, 0777);
    mkdir(QUARANTINE_DIR, 0777);
    mkdir(DECRYPTED_DIR, 0777);

    download_and_extract();

    if (argc != 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) run_daemon_decrypt();
    else if (strcmp(argv[1], "--quarantine") == 0) move_to_quarantine();
    else if (strcmp(argv[1], "--return") == 0) return_to_starterkit();
    else if (strcmp(argv[1], "--eradicate") == 0) delete_quarantine_contents();
    else if (strcmp(argv[1], "--shutdown") == 0) shutdown_daemon();
    else print_usage();

    return 0;
}
