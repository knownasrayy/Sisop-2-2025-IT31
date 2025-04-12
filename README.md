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
