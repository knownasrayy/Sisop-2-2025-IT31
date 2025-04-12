#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>     // a. Untuk mkdir()
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define QUARANTINE_DIR "quarantine"
#define STARTERKIT_DIR "starter_kit"
#define LOG_FILE "activity.log"
#define PID_FILE "decrypt.pid"

// g. Logging aktivitas ke activity.log
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

// b. Dekripsi base64 dari nama file
void base64_decode(const char *src, char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo %s | base64 -d", src);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        fgets(dest, 256, fp);
        pclose(fp);
    }
}

// b. Menjalankan proses daemon dekripsi nama file dari base64
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
    setsid(); // buat proses baru agar menjadi daemon

    while (1) {
        DIR *dir = opendir(QUARANTINE_DIR);
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    char decoded[256] = {0};
                    base64_decode(ent->d_name, decoded);
                    if (strlen(decoded) == 0) continue;

                    // Menghapus newline hasil decode
                    decoded[strcspn(decoded, "\n")] = 0;

                    char old_path[512], new_path[512];
                    snprintf(old_path, sizeof(old_path), "%s/%s", QUARANTINE_DIR, ent->d_name);
                    snprintf(new_path, sizeof(new_path), "%s/%s", QUARANTINE_DIR, decoded);

                    if (rename(old_path, new_path) == 0) {
                        char log_msg[512];
                        sprintf(log_msg, "%s renamed to %s", ent->d_name, decoded);
                        write_log(log_msg);
                    }
                }
            }
            closedir(dir);
        }
        sleep(5); // jalan tiap 5 detik
    }
}

// Digunakan untuk c, d
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

// c. Memindahkan file dari starter_kit ke quarantine
void move_to_quarantine() {
    move_files(STARTERKIT_DIR, QUARANTINE_DIR, "moved to quarantine directory");
}

// d. Mengembalikan file dari quarantine ke starter_kit
void return_to_starterkit() {
    move_files(QUARANTINE_DIR, STARTERKIT_DIR, "returned to starter kit directory");
}

// e. Menghapus semua file dari folder quarantine
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

// f. Mematikan proses daemon dan menghapus decrypt.pid
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

    // Untuk apakah proses masih aktif
    if (kill(pid, 0) == -1) {
        printf("No such process with PID %d.\n", pid);
        write_log("PID file found but no such process exists. Removing stale PID file.");
        remove(PID_FILE);
        return;
    }

    // Untuk mengirim sinyal SIGTERM untuk menghentikan daemon
    if (kill(pid, SIGTERM) == 0) {
        char msg[128];
        sprintf(msg, "Successfully shut off decryption process with PID %d.", pid);
        write_log(msg);
        remove(PID_FILE);
    } else {
        perror("Failed to send SIGTERM");
    }
}

// Untuk Menampilkan bantuan perintah
void print_usage() {
    printf("Usage:\n");
    printf("./starterkit --decrypt\n");
    printf("./starterkit --quarantine\n");
    printf("./starterkit --return\n");
    printf("./starterkit --eradicate\n");
    printf("./starterkit --shutdown\n");
}

int main(int argc, char *argv[]) {
    // a. Membuat direktori starter_kit, quarantine, dan decrypted (kalau belum ada)
    mkdir(STARTERKIT_DIR, 0777);
    mkdir(QUARANTINE_DIR, 0777);
    mkdir("decrypted", 0777); // meskipun belum digunakan,tetapi sesuai soal

    if (argc != 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) run_daemon_decrypt();        // b
    else if (strcmp(argv[1], "--quarantine") == 0) move_to_quarantine(); // c
    else if (strcmp(argv[1], "--return") == 0) return_to_starterkit();   // d
    else if (strcmp(argv[1], "--eradicate") == 0) delete_quarantine_contents(); // e
    else if (strcmp(argv[1], "--shutdown") == 0) shutdown_daemon();      // f
    else print_usage();

    return 0;
}
