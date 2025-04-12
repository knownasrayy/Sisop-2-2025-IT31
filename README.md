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
