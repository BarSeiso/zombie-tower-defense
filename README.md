# Pos Terakhir

Game tower defense 2D bertema wabah zombie, ditulis dengan C99 dan raylib 5.5.
Pertahankan markas selama 10 wave dengan mengisi 8 pos tetap. Semua teks permainan
dan komentar penjelasan memakai bahasa Indonesia.

## Menjalankan

Executable hasil build berada di `build/bin/zombie_td.exe`. Jalankan dengan klik dua
kali atau melalui PowerShell. Folder `assets` harus berada di samping executable.

## Build di Windows

Gunakan GCC MinGW 64-bit dan CMake 3.24 atau lebih baru. Keduanya harus tersedia
di PATH. Konfigurasi pertama memerlukan internet untuk mengunduh raylib dari
repositori resminya. CMake membangun raylib bersama game.

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_TESTING=ON
cmake --build build -j 4
ctest --test-dir build --output-on-failure
.\build\bin\zombie_td.exe
```

## Kontrol

| Aksi | Kontrol |
|---|---|
| Pilih pos | Klik tanda + atau unit |
| Beli, upgrade, ganti | Klik kartu di panel bawah |
| Lihat statistik pilihan | Arahkan mouse ke kartu |
| Mulai wave | Tombol MULAI WAVE atau Spasi |
| Jeda dan lanjut | Tombol JEDA/LANJUT atau P |
| Kecepatan 1x/2x | Tombol kecepatan atau F |
| Batalkan pilihan pos | Esc atau klik tanah kosong |
| Keluar | Tombol KELUAR di menu atau tutup jendela |

## Aturan

- Markas mulai dengan 20 ketahanan dan 180 Scrap. Zombie yang lolos merusak markas.
- Pemanah berharga 70 Scrap, Penembak 115 Scrap, dan Meriam 165 Scrap.
- Setiap unit memiliki jangkauan berbeda. Meriam meledak dalam radius 24 piksel.
- Semua unit memprioritaskan zombie yang paling jauh menyusuri jalur.
- Klik kartu unit yang sedang terpasang untuk upgrade hingga level 3.
  Biaya upgrade level 2 sebesar 50% harga dasar, lalu level 3 sebesar 75%.
  Semua pembulatan biaya ke bawah. Damage level 2 sebesar 1,5 kali damage dasar
  dan level 3 sebesar 2 kali, dibulatkan ke bawah. Jangkauan bertambah 7 per level.
- Klik kartu jenis lain untuk mengganti isi pos. Biaya penggantian mendapat
  potongan 40% harga dasar unit lama, dibulatkan ke bawah. Unit pengganti mulai
  di level 1. Biaya upgrade lama tidak ikut dihitung dalam potongan.
- Pembelian dan upgrade tersedia saat wave berjalan. Saat jeda, transaksi berhenti.
- Wave 1 sampai 9 memberi bonus 25 + 5 kali nomor wave setelah semua zombie selesai.
- Mutant pada wave 10 mempunyai 620 HP dan merusak markas sebesar 20 jika lolos.
- Pilih MAIN LAGI setelah menang atau kalah untuk mengulang dari awal.

## Membaca kode

- `src/game.h` mendefinisikan data unit, zombie, wave, dan keadaan permainan.
- `src/game.c` berisi tabel statistik, ekonomi, pergerakan, dan pertempuran.
- `src/view.c` menggambar peta, sprite, tombol, jangkauan, dan efek serangan.
- `src/main.c` menghubungkan input mouse/keyboard dengan simulasi 60 langkah per detik.
- `tests/game_checks.c` memeriksa aturan dan satu strategi yang menyelesaikan 10 wave.
- `assets/sprites.png` adalah atlas pixel art orisinal, 2 frame untuk setiap jenis unit
  dan zombie. Peta dan elemen antarmuka digambar menggunakan fungsi raylib.

Statistik ada di `UNITS` dan `ENEMIES`. Susunan wave ada di `WAVES`.
Wave 10 memisahkan Rage menjadi dua kelompok melalui `SpawnType`.
Efek panah dan peluru bersifat visual. Damage langsung diterapkan saat unit menembak.
Resolusi gambar 480 x 270 ditampilkan pada jendela 960 x 540 dengan filter piksel tajam.

Untuk mengambil gambar pemeriksaan menu atau pertempuran, jalankan
`zombie_td.exe --screenshot` atau `zombie_td.exe --screenshot battle`.
Gambar disimpan pada direktori kerja. Preview pertempuran memakai kondisi khusus
untuk memperlihatkan unit dan musuh, terpisah dari sesi permainan biasa.

raylib memakai lisensi zlib/libpng. Sumber dan lisensinya tersedia di
[repositori raylib](https://github.com/raysan5/raylib/tree/5.5).
