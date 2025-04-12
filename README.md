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
#### Soal : Menyaring file yang hanya memiliki 1 huruf atau angka (tanpa karakter spesial) dan menyimpannya ke folder Filtered/. File lain dihapus.
