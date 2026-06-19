/*
 * ============================================================
 *  MAZE GAME  -  Game puzzle terminal
 *  Cross-platform: Linux & Windows
 *  Compile : g++ MazeAmaze.cpp -o MazeAmaze -static
 * (-static) itu buat copy library-nya ke .exenya juga, jaga-jaga kalo pc lain ga pake gcc versi terbaru
 *  Kontrol : W A S D untuk bergerak  |  / untuk perintah
 * ============================================================
 */


#include <iostream>
#include <fstream>          // ifstream, ofstream, getline
#include <vector>           // vector<T>, push_back, size, operator[]
#include <list>             // list<T>, push_back, sort
#include <string>           // string, getline, stoi, substr, find_first_not_of, find_last_not_of, empty
#include <algorithm>        // transform, max
#include <cctype>           // tolower
#include <ctime>            // time, srand, rand, time_t
#include <cstdlib>          // system
#include <stdexcept>        // runtime_error 
#include <functional>       // function  
#include <sstream>          // istringstream
#include <iterator>         // distance
#include <thread>           // this_thread::sleep_for	
#include <chrono>           // chrono::milliseconds	

/*
 * =============================================================================
 *                         PENJELASAN HEADER DAN FUNGSI
 * =============================================================================
 * 	
 * <iostream>        
 * - cout / cin    : input output standar
 * - endl          : newline dan flush buffer
 * - flush         : membersihkan buffer output
 
 * <fstream>
 * - ifstream      : membaca file
 * - ofstream      : menulis file
 * - getline       : membaca baris dari file atau input standar
 
 * <vector>
 * - vector<T>     : container dinamis
 * - push_back     : menambahkan elemen ke akhir vector
 * - size          : mendapatkan jumlah elemen dalam vector
 * - operator[]    : mengakses elemen pada indeks tertentu
 
 * <list>
 * - list<T>       : container double-linked list
 * - push_back     : menambahkan elemen ke akhir list
 * - sort          : mengurutkan elemen dalam list (method sendiri, karena tidak bisa diakses acak)
 
 * <string>
 * - string        : menyimpan teks
 * - getline       : membaca satu baris dari input (cin atau file) ke objek string
 * - stoi          : mengonversi string ke integer
 * - substr        : mengambil bagian dari string
 * - find_first_not_of  : mencari karakter pertama yang tidak cocok (untuk trim)
 * - find_last_not_of   : mencari karakter terakhir yang tidak cocok (untuk trim)
 * - empty         : memeriksa apakah string kosong
 
 * <algorithm>
 * - transform     : menerapkan fungsi ke setiap elemen dalam rentang dan menyimpan hasilnya
 *                   Contoh: mengubah string menjadi huruf kecil semua
 * - max           : mengembalikan nilai terbesar dari dua nilai (atau lebih)
 
 * <cctype>
 * - tolower       : mengubah karakter huruf besar menjadi huruf kecil
 *                   Berguna untuk membuat perbandingan string tidak case-sensitive
 
 * <ctime>
 * - time(nullptr) : mengembalikan waktu saat ini dalam detik sejak epoch (1 Jan 1970)
 * - srand()       : menginisialisasi generator angka acak dengan seed
 *                   srand(time(nullptr)) dijalankan SEKALI di awal program
 *                   agar hasil acak berbeda setiap program dijalankan
 * - time_t        : tipe data untuk menyimpan waktu (biasanya integer besar)
 * - rand()        : menghasilkan angka acak berdasarkan seed dari srand()
 *                   Jika srand tidak dipanggil, rand menghasilkan urutan angka yang sama
 
 * <cstdlib>
 * - system        : menjalankan perintah dari sistem operasi
 *                   Contoh: system("cls") untuk membersihkan layar di Windows
 
 * <stdexcept>
 * - runtime_error : kelas exception untuk kesalahan saat program berjalan (runtime)
 *                   Digunakan dengan throw
 
 * <sstream>
 * - istringstream : membaca string seperti stream (mirip cin)
 *                   Berguna untuk mem-parsing baris berisi beberapa kata terpisah spasi
 

 * <functional>
 * - function      : pembungkus untuk menyimpan dan memanggil fungsi, lambda, atau functor
 *                   Memungkinkan fungsi disimpan sebagai objek

 * <iterator>
 * - distance      : menghitung jumlah langkah antara dua iterator
 *                   Contoh: jarak antara begin dan end pada container

 * <thread>           
 * - this_thread::sleep_for	: Menunda thread selama durasi tertentu


 * <chrono>           
 * - chrono::milliseconds   : Menyatakan durasi dalam milidetik untuk diberikan ke sleep_for

 * =============================================================================
 */




#ifdef _WIN32
    #include <conio.h>      // _getch() untuk baca karakter tanpa enter di Windows
    #include <windows.h>    // untuk fungsi khusus Windows seperti Clear Screen dan Enable ANSI
#else
    #include <termios.h>    // untuk fungsi khusus Linux/Unix seperti baca karakter tanpa enter dan Clear Screen
    #include <unistd.h>     // untuk STDIN_FILENO dan write() di Linux/Unix
#endif

using namespace std;

// ============================================================
//  SECTION 1 : PLATFORM LAYER
// ============================================================

#ifdef _WIN32   // jika berjalan di OS windows
void aktifkanANSIWindows() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); 
    // HANDLE = tipe data untuk menyimpan "kunci akses" ke console
    // GetStdHandle(STD_OUTPUT_HANDLE) = mengambil kunci akses ke layar output (STD_OUTPUT_HANDLE)
    //h = variabel penyimpan kunci tersebut
    DWORD  mode = 0; 
    // DWORD = tipe angka bulat 32 bit khusus windows.h
    //mode = 0 -> variabel menyimpan data pengaturan console saat ini
    GetConsoleMode(h, &mode);   // menyimpan data console melalui handle h ke variabel mode
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);   // memasukkan rule untuk enable ANSI
}

// fungsi jika menggunakan Windows
namespace platform {
    int  bacaChar()      { return ::_getch(); }

    // Clear penuh: hapus layar + pindah kursor ke atas
    // Dipakai saat berpindah tampilan (menu, bantuan, level baru, dsb)
    void bersihLayar()   { system("cls"); }

    // Refresh ringan: hanya pindah kursor ke pojok kiri atas tanpa hapus
    // Dipakai saat render ulang in-game agar tidak flicker
    void refreshLayar()  {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD pos = {0, 0};
        SetConsoleCursorPosition(h, pos);
    }

    void inisialisasi()  { aktifkanANSIWindows(); }
}

#else // jika bukan Windows (LINUX/MACOS)

// namespace untuk linux terminal
namespace linux_term {
    int bacaChar() {
        struct termios lama, baru;      // struktur data untuk menyinpan setting terminal
        int ch;
        tcgetattr(STDIN_FILENO, &lama);
        // tcgetattr()	    Fungsi untuk membaca pengaturan terminal saat ini
        //STDIN_FILENO	    Nomor file untuk input standar (keyboard) = 0
        // &lama	        Alamat variabel lama tempat hasil bacaan disimpan
        baru = lama;    // copy setting terminal lama, simpan di variabel baru
        baru.c_lflag &= ~(ICANON | ECHO);
        // c_lflag	        Field dalam termios yang menyimpan mode lokal (c = control, lflag = local flags)
        // ICANON	        Mode canonical = input masuk hanya setelah Enter ditekan
        // ECHO	            karakter yang diketik langsung ditampilkan di layar
        // ~(ICANON | ECHO) menghasilkan bitmask(aturan on/off) dengan bit ICANON dan ECHO bernilai 0, sisanya 1
        // baru.c_lflag &=  menyimpan bit ICANON dan ECHO berubah menjadi 0 (mati)
        // Flag (settings) lainnya tetap sama seperti semula
        tcsetattr(STDIN_FILENO, TCSANOW, &baru);
        // tcsetattr()	    Fungsi untuk menerapkan pengaturan terminal
        // STDIN_FILENO	    Targetnya adalah input standar
        // TCSANOW	        Terapkan perubahan sekarang juga (tanpa menunggu)
        // &baru	        Pengaturan baru yang akan dipakai
        ch = getchar();     // membaca 1 karakter dari input keybaord dan simpan ke ch
        tcsetattr(STDIN_FILENO, TCSANOW, &lama);
        // tcsetattr()	    Fungsi untuk menerapkan pengaturan terminal
        // STDIN_FILENO	    Targetnya adalah input standar
        // TCSANOW	        Terapkan perubahan sekarang juga (tanpa menunggu)
        // &baru	        Pengaturan baru yang akan dipakai
        return ch;
        // return karakter yang diketik
    }
}
// fungsi jika tidak menggunakan Windows (LINUX/MACOS)
namespace platform {
    int  bacaChar()     { return linux_term::bacaChar(); }

    // Clear penuh: hapus seluruh layar + pindah kursor ke pojok kiri atas
    // Dipakai saat berpindah tampilan (menu, bantuan, level baru, dsb)
    void bersihLayar()  {
        const char* cls = "\033[2J\033[H";
        if (write(STDOUT_FILENO, cls, 7)) {}
    }

    // Refresh ringan: hanya pindah kursor ke pojok kiri atas tanpa hapus
    // Dipakai saat render ulang in-game agar tidak flicker
    void refreshLayar() {
        const char* ke_atas = "\033[H";
        if (write(STDOUT_FILENO, ke_atas, 4)) {}
    }

    void inisialisasi() {}
}

#endif

// ============================================================
//  SECTION 2 : WARNA ANSI
// ============================================================

#define RESET       "\033[0m"
#define TEBAL       "\033[1m"
#define MERAH       "\033[91m"
#define HIJAU       "\033[92m"
#define KUNING      "\033[93m"
#define CYAN        "\033[96m"
#define ABU         "\033[90m"
#define PUTIH       "\033[97m"
#define JUDUL       "\033[1;95m"
#define PERINGATAN  "\033[1;93m"
#define REDUP       "\033[2m"
#define BIRU        "\033[94m"
#define MAGENTA     "\033[95m"

// ============================================================
//  SECTION 3 : KONSTANTA SEL MAP
// ============================================================

const char SEL_DINDING      = 'X';
const char SEL_JALAN        = '*';
const char SEL_MULAI        = '.';
const char SEL_SELESAI      = '!';
const char SEL_SPESIAL      = '?';
const char SEL_PEMAIN       = '@';
const char SEL_CHECKPOINT   = '+';   // checkpoint aktif (dari '?' yang diinjak)

// ============================================================
//  SECTION 4 : STRUKTUR DATA
// ============================================================


struct Map {
    vector<vector<char>> grid;
    int baris     = 0;
    int kolom     = 0;
    int barisMulai= -1, kolomMulai = -1;
    int barisAkhir= -1, kolomAkhir = -1;
    bool valid    = false;
};

struct Level {
    string       nama;
    int          nomorLevel = 0;
    vector<Map>  daftarMap;
};

struct EntriLeaderboard {
    string  nickname;
    int     totalDetik = 0;
};

// ============================================================
//  SECTION 5 : FUNGSI UTILITAS
// ============================================================

// Trim string (inline)
inline string trim(const string& s) { 
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Template: cetak nilai apapun
template<typename T>
inline void cetakBaris(const T& nilai) {
    cout << nilai << "\n";
}

// Overloading: buat garis pemisah dengan char default '='
inline string garisPemisah(int panjang, char karakter = '=') {
    return string(panjang, karakter);
}
// Overloading: buat garis pemisah dengan string kustom
string garisPemisah(const string& kiri, const string& isi, const string& kanan, int lebar) {
    int isiLen = lebar - (int)kiri.size() - (int)kanan.size();
    if (isiLen < 0) isiLen = 0;
    return kiri + string(isiLen, isi.empty() ? ' ' : isi[0]) + kanan;
}

// Konversi detik ke format MM:SS 
string formatWaktu(int detik) {
    int menit = detik / 60;
    int sisa  = detik % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", menit, sisa);
    return string(buf);
}

// Cek apakah karakter valid di map
inline bool adalahSelValid(char c) {
    return c == SEL_DINDING || c == SEL_JALAN ||
           c == SEL_MULAI   || c == SEL_SELESAI || c == SEL_SPESIAL;
}

// Baca satu baris input (dengan restore terminal di Linux)
string bacaBaris() {
    string cmd;
    #ifndef _WIN32  // jika menggunakan OS Windows
        struct termios lama, baru; // variabel bertipe termios
        tcgetattr(STDIN_FILENO, &lama);     // ambil pengaturan di terminal, simpan di 'lama'
        baru = lama;                        // simpen di variabel sama
        baru.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &baru);
        getline(cin, cmd);
        tcsetattr(STDIN_FILENO, TCSANOW, &lama);
    #else
        getline(cin, cmd);
    #endif
        return cmd;
}

// ============================================================
//  SECTION 6 : PARSER MAP FILE
// ============================================================

// Mengubah teks menjadi objek Map
Map parseMap(const vector<string>& barisRaw) {
    Map m;
    int maxKolom = 0;

    // Menyaring setiap baris mentah 
    for (const auto& baris : barisRaw) { 
        if (trim(baris).empty()) continue;
        vector<char> brs;
        for (char c : baris) {
            if (adalahSelValid(c)) brs.push_back(c);
        }
        if (!brs.empty()) {
            m.grid.push_back(brs); // hanya karakter yang lolos fungsi adalahSelValid yang dimasukkan ke m.grid
            maxKolom = max(maxKolom, (int)brs.size());
        }
    }

    m.baris = (int)m.grid.size();
    m.kolom = maxKolom;

    // Pemindaian ulang seluruh grid yang sudah jadi untuk mencari koordinat pasti
    for (int r = 0; r < m.baris; r++) { 
        for (int c = 0; c < (int)m.grid[r].size(); c++) {
            if (m.grid[r][c] == SEL_MULAI)   { m.barisMulai = r; m.kolomMulai = c; }
            if (m.grid[r][c] == SEL_SELESAI) { m.barisAkhir = r; m.kolomAkhir = c; }
        }
    }

    m.valid = (m.baris > 0 && m.barisMulai >= 0 && m.barisAkhir >= 0);
    return m;
}

// Ekstrak nomor level dari nama (misal "LEVEL 7" -> 7)
int ekstrakNomorLevel(const string& nama) {
    istringstream iss(nama);
    string kata;
    int nomor = 0;
    while (iss >> kata) {
        try {
            nomor = stoi(kata);
            break;
        } catch (...) {}
    }
    return nomor;
}

// Fungsi memuat file
vector<Level> muatFilePeta(const string& namaFile) {
    vector<Level> daftarLevel;
    ifstream file(namaFile);

    if (!file.is_open()) {
        throw runtime_error("Gagal membuka file peta: '" + namaFile + "'");
    }

    Level         levelSaatIni;
    vector<string> barisMap;
    bool          dalamLevel = false;
    string        baris;

    // Lambda: commit map saat ini ke level
    auto simpanMap = [&]() { // [&] berarti fungsi ini diizinkan mengubah variabel apa saja yang ada di sekitarnya
        if (!barisMap.empty()) {
            Map m = parseMap(barisMap);
            if (m.valid) levelSaatIni.daftarMap.push_back(m);
            barisMap.clear();
        }
    };

    // Lambda: commit level saat ini ke daftar
    auto simpanLevel = [&]() {
        simpanMap();
        if (!levelSaatIni.daftarMap.empty()) {
            daftarLevel.push_back(levelSaatIni);
        }
        levelSaatIni = Level();
        dalamLevel = false;
    };

    // Membaca file peta baris per baris dari atas ke bawah
    while (getline(file, baris)) {
        if (!baris.empty() && baris.back() == '\r') baris.pop_back();
        string t = trim(baris);
        if (t.empty() || t[0] == '#') continue; // mengabaikan baris kosong atau komentar ditandai #

        // Tanda bahwa ini adalah level baru. Program akan memanggil lambda simpanLevel() untuk menyimpan level sebelumnya, lalu membuat level baru
        if (t[0] == '>') {
            if (dalamLevel) simpanLevel();
            levelSaatIni.nama       = trim(t.substr(1));
            levelSaatIni.nomorLevel = ekstrakNomorLevel(levelSaatIni.nama);

            // Baca titik spesial untuk level 6-10 dari baris berikutnya bertag #SPESIAL
            // (akan di-handle lewat komentar khusus di file, tapi di sini kita generate otomatis)
            dalamLevel = true;
        } else if (t == "EndMap") {
            if (dalamLevel) simpanLevel();
        } else if (t == ",") {
            if (dalamLevel) simpanMap();
        } else if (dalamLevel) {
            barisMap.push_back(baris);
        }
    }

    if (dalamLevel) simpanLevel();

    return daftarLevel;
}

// ============================================================
//  SECTION 7 : LEADERBOARD
// ============================================================

namespace leaderboard {

    const string FILE_LB = "leaderboard.txt"; // nama file teks tujuan penyimpanan data
    const int    MAKS_TAMPIL = 10;            // jumlah maksimal entri peringkat yang akan ditampilkan

    // Muat leaderboard dari file ke list
    list<EntriLeaderboard> muat() {
        list<EntriLeaderboard> daftar; 
        ifstream f(FILE_LB);             // membuka file leaderboard
        if (!f.is_open()) return daftar; // jika file gagal dibuka, fungsi langsung berhenti

        string baris;
        while (getline(f, baris)) {
            if (baris.empty() || baris[0] == '#') continue; // jika baris kosong atau diawali # maka baris tersebut diabaikan
            istringstream iss(baris);                       // mengubah baris teks menjadi stream agar mudah dibagi-bagi isinya
            EntriLeaderboard e;
            string detikStr;
            if (getline(iss, e.nickname, '|') && getline(iss, detikStr)) {
                e.nickname = trim(e.nickname); // membersihkan spasi berlebih di awal atau akhir nickname pemain
                try { e.totalDetik = stoi(trim(detikStr)); } catch (...) { continue; } // mengubah teks waktu menjadi integer, jika gagal baris akan dilewati
                if (!e.nickname.empty()) daftar.push_back(e);
            }
        }

        // Sort berdasarkan waktu terkecil
        daftar.sort([](const EntriLeaderboard& a, const EntriLeaderboard& b){
            return a.totalDetik < b.totalDetik; // mengurutkan isi daftar dari waktu terkecil ke terbesar (menggunakan lambda)
        });

        return daftar;
    }

    // Simpan leaderboard ke file
    void simpan(const list<EntriLeaderboard>& daftar) {
        ofstream f(FILE_LB); // membuka file leaderboard.txt untuk proses writing
        if (!f.is_open()) throw runtime_error("Gagal menyimpan leaderboard.");
        f << "# nickname|detik\n"; // menulis dengan format tersebut di baris pertama file baru
        for (const auto& e : daftar) {
            f << e.nickname << "|" << e.totalDetik << "\n";
        }
    }

    // Tambah entri baru dan simpan (hanya top 100 disimpan)
    void tambah(const string& nick, int detik) {
        list<EntriLeaderboard> daftar = muat(); // memanggil fungsi muat untuk mengambil data yang sudah ada sebelumnya
        EntriLeaderboard baru; 
        baru.nickname   = nick;
        baru.totalDetik = detik;
        daftar.push_back(baru);
        daftar.sort([](const EntriLeaderboard& a, const EntriLeaderboard& b){
            return a.totalDetik < b.totalDetik;
        });
        // Batasi 100 entri
        while ((int)daftar.size() > 100) daftar.pop_back();
        simpan(daftar);
    }

    // Tampilkan leaderboard (top N)
    void tampilkan(int topN = MAKS_TAMPIL) {
        list<EntriLeaderboard> daftar = muat();
        int lebar = 44;
        cout << JUDUL
             << garisPemisah("+", "=", "+", lebar) << "\n"
                    << "|           TOP " << topN << " LEADERBOARD             |\n"
             << garisPemisah("+", "=", "+", lebar) << "\n"
             << RESET;

        if (daftar.empty()) {
            cout << "|             (Belum ada data)             |\n";
        } else {
            int peringkat = 1;
            auto it = daftar.begin();
            while (it != daftar.end() && peringkat <= topN) {
                string entri = to_string(peringkat) + ". " + it->nickname; // menggabungkan string antara nomor peringkat dan nickname
                string waktuStr = formatWaktu(it->totalDetik); // mengubah detik menjadi teks waktu
                // Pad entri
                int padLen = lebar - 4 - (int)entri.size() - (int)waktuStr.size();
                if (padLen < 1) padLen = 1;
                cout << KUNING << "| " << RESET
                     << entri << string(padLen, ' ') << waktuStr
                     << KUNING << " |" << RESET << "\n";
                ++it;
                ++peringkat;
            }
        }
        cout << JUDUL << garisPemisah("+", "=", "+", lebar) << RESET << "\n";
    }

} // namespace leaderboard

// ============================================================
//  SECTION 8 : RENDERER
// ============================================================

// ============================================================
//  Callback untuk render sel (mendukung mode debug/normal)
// ============================================================
using FungsiRenderSel = function<string(char, bool)>;

// Render sel mode NORMAL (jalan '*' dan spesial '?' tidak terlihat)
string renderSelNormal(char sel, bool isPemain) {
    if (isPemain) return string(CYAN) + TEBAL + SEL_PEMAIN + RESET;
    switch (sel) {
        case SEL_DINDING:   return string(MERAH) + SEL_DINDING + RESET;
        case SEL_MULAI:     return string(HIJAU) + SEL_MULAI + RESET;
        case SEL_SELESAI:   return string(KUNING) + TEBAL + SEL_SELESAI + RESET;
        case SEL_CHECKPOINT:return string(HIJAU) + TEBAL + SEL_CHECKPOINT + RESET;
        case SEL_JALAN:     return " ";          // tersembunyi
        case SEL_SPESIAL:   return " ";          // tersembunyi
        default:            return string(1, sel);
    }
}

// Render sel mode DEBUG (jalan '*' dan spesial '?' ditampilkan)
string renderSelDebug(char sel, bool isPemain) {
    if (isPemain) return string(CYAN) + TEBAL + SEL_PEMAIN + RESET;
    switch (sel) {
        case SEL_DINDING:   return string(MERAH) + SEL_DINDING + RESET;
        case SEL_MULAI:     return string(HIJAU) + SEL_MULAI + RESET;
        case SEL_SELESAI:   return string(KUNING) + TEBAL + SEL_SELESAI + RESET;
        case SEL_CHECKPOINT:return string(HIJAU) + TEBAL + SEL_CHECKPOINT + RESET;
        case SEL_JALAN:     return string(ABU) + SEL_JALAN + RESET;      // tampilkan '*'
        case SEL_SPESIAL:   return string(PUTIH) + TEBAL + SEL_SPESIAL + RESET; // tampilkan '?'
        default:            return string(1, sel);
    }
}


// ============================================================
//  Fungsi renderMap buffer tunggal
//  Menampilkan seluruh peta, posisi pemain, header, status,
//  waktu, dan pesan dalam satu operasi output ke terminal.
//  menghilangkan flicker dan mempercepat rendering.
// ============================================================
void renderMap(const Map& mp,
               int barisP, int kolomP,
               const string& namaLevel,
               int idxMap, int totalMap,
               const string& pesan,
               int totalDetik,
               FungsiRenderSel renderSel)  // <-- tambahkan callback
{
    // 1. Sembunyikan kursor agar tidak berkedip selama render
    cout << "\033[?25l";          // ANSI escape: hide cursor
    platform::refreshLayar();     // Pindah kursor ke pojok kiri atas tanpa clear

    // 2. Tentukan lebar tampilan (minimal 44 karakter,
    //    atau sesuai lebar peta + padding)
    int lebarBar = max(44, mp.kolom * 2 + 8);

    // 3. Buat buffer string untuk merakit seluruh frame
    ostringstream buf;

    // ---------- HEADER ----------
    // Nama level dan progress map (misal: "LEVEL 7  |  Map 2 / 5")
    buf << JUDUL
        << " MAZE GAME  |  " << namaLevel
        << "  |  Map " << (idxMap + 1) << " / " << totalMap
        << RESET << "\n"
        << garisPemisah(lebarBar) << "\n";   // garis "=" sepanjang lebarBar

    // Jika ada total waktu (lebih dari 0), tampilkan
    if (totalDetik > 0) {
        buf << BIRU << TEBAL
            << "  TOTAL WAKTU = " << totalDetik << " detik"
            << RESET << "\n";
    }
    buf << "\n";  // baris kosong sebelum grid

    
    // GRID PETA (gunakan callback)
    for (int r = 0; r < mp.baris; r++) {
        buf << "  ";
        for (int c = 0; c < (int)mp.grid[r].size(); c++) {
            if (r == barisP && c == kolomP) {
                buf << renderSel(SEL_PEMAIN, true) << " ";  // pemain via callback
                continue;
            }
            char sel = mp.grid[r][c];
            buf << renderSel(sel, false) << " ";
        }
        buf << "\n";
    }





    // ---------- STATUS & PESAN ----------
    // Garis pemisah
    buf << "\n" << garisPemisah(lebarBar, '-') << "\n";

    // Informasi posisi pemain dan tujuan akhir
    buf << "  " << TEBAL << "Posisi : " << RESET
        << "[" << barisP << "][" << kolomP << "]"
        << "   "
        << TEBAL << "Tujuan: " << RESET
        << "[" << mp.barisAkhir << "][" << mp.kolomAkhir << "]\n";

    // Tampilkan pesan (contoh: ">> MAP BERUBAH! ...") jika ada
    buf << "  " << PERINGATAN
        << (pesan.empty() ? "" : ">> " + pesan)  // hanya tampilkan ">> " jika pesan tidak kosong
        << RESET << "\n";

    // Bantuan kontrol di bagian bawah
    buf << "\n" << REDUP
        << "   [Arrow Key] Atau [W] [A] [S] [D] : gerak   [/] : perintah"
        << CYAN << REDUP << "\n   perbesar terminal / zoom out jika tampilan rusak dan refesh dengan '/' lalu Enter\n" << RESET;

    // 4. CETAK SELURUH FRAME SEKALIGUS ke terminal
    cout << buf.str();
    cout.flush();   // langsung kirim ke layar
}

// ============================================================
//  SECTION 9 : STATE PERMAINAN
// ============================================================

struct StatusPermainan {
    vector<Level>  daftarLevel;
    int  idxLevel     = 0;
    int  idxMap       = 0;    // map yang sedang aktif (hasil random)
    int  barisP       = 0;
    int  kolomP       = 0;
    bool berjalan     = true;
    bool menang       = false;
    bool kembaliMenu  = false;
    string pesan;
    string nickname;
    int  barisCheckpoint = -1;   // posisi checkpoint aktif (-1 = belum ada)
    int  kolomCheckpoint = -1;

    // Waktu per level (dalam detik)
    vector<int> waktuPerLevel;
    time_t      waktuMulaiLevel = 0;
    int         totalDetikSaatIni = 0; // akumulasi level sebelumnya

    // Acak map untuk level saat ini
    int pilihanMap = 0; // indeks map yang dipilih secara random

    // ---- Aksesor ----
    Level& levelSaatIni() { return daftarLevel[idxLevel]; }
    const Level& levelSaatIni() const { return daftarLevel[idxLevel]; }

    Map& mapSaatIni() { return daftarLevel[idxLevel].daftarMap[pilihanMap]; }
    const Map& mapSaatIni() const { return daftarLevel[idxLevel].daftarMap[pilihanMap]; }

    int jumlahMapDiLevel() const {
        return (int)daftarLevel[idxLevel].daftarMap.size();
    }

    // ---- Pilih map secara random ----
    void acakMap() {
        int jml = jumlahMapDiLevel();
        if (jml <= 0) throw runtime_error("Level tidak memiliki map valid.");
        pilihanMap = (jml == 1) ? 0 : (rand() % jml);
    }

    // ---- Posisikan pemain di titik mulai ----
    void spawnPemain() {
        // jika sudah pernah menginjak checkpoint
        if (barisCheckpoint >= 0) {
            // spawn di checkpoint jika ada
            barisP = barisCheckpoint;
            kolomP = kolomCheckpoint;
        } else {    // jika belum
            const Map& m = mapSaatIni();
            barisP = m.barisMulai;
            kolomP = m.kolomMulai;
        }
    }

    // ---- Mulai level baru ----
    // reset checkpoint saat ganti level
    void mulaiLevel() {
        platform::bersihLayar();
        barisCheckpoint = -1;   // reset checkpoint di level baru
        kolomCheckpoint = -1;
        acakMap();
        spawnPemain();
        waktuMulaiLevel = time(nullptr);
        pesan = "";
    }

    // ---- Ganti map karena injak titik spesial ----
    void gantimapSpesial(int barisInjak, int kolomInjak) {
        // Tandai ? yang diinjak sebagai checkpoint di semua map level ini
        for (auto& mp : daftarLevel[idxLevel].daftarMap) {
            // Hapus checkpoint lama jika ada
            if (barisCheckpoint >= 0 &&
                barisCheckpoint < mp.baris &&
                kolomCheckpoint < (int)mp.grid[barisCheckpoint].size()) {
                char& selLama = mp.grid[barisCheckpoint][kolomCheckpoint];
                if (selLama == SEL_CHECKPOINT) selLama = SEL_SPESIAL; // kembalikan jadi ?
            }
            // Pasang checkpoint baru
            if (barisInjak < mp.baris &&
                kolomInjak < (int)mp.grid[barisInjak].size()) {
                mp.grid[barisInjak][kolomInjak] = SEL_CHECKPOINT;
            }
        }
        barisCheckpoint = barisInjak;
        kolomCheckpoint = kolomInjak;

        // Ganti map
        int jml = jumlahMapDiLevel();
        if (jml > 1) {
            int mapLama = pilihanMap;
            while (pilihanMap == mapLama) pilihanMap = rand() % jml;
            pesan = "MAP BERUBAH! Checkpoint tersimpan di sini.";
        } else {
            pesan = "Checkpoint tersimpan! (Hanya 1 map di level ini)";
        }
        // Posisi pemain tetap (koordinat ? sama di semua map)
    }

    // ---- Coba bergerak ----
    // Returns: true jika berhasil
    bool cobaGerak(int dr, int dc) {
        int barisB = barisP + dr;
        int kolomB = kolomP + dc;
        const Map& m = mapSaatIni();

        if (barisB < 0 || barisB >= m.baris) return false;
        if (kolomB < 0 || kolomB >= (int)m.grid[barisB].size()) return false;

        char target = m.grid[barisB][kolomB];

        if (target == SEL_DINDING) {
            // platform::bersihLayar();
            // Restart ke titik mulai (bukan blocking saja)
            spawnPemain();
            pesan = "Nabrak dinding! Kembali ke titik awal.";
            return false;
        }

        barisP = barisB;
        kolomP = kolomB;

        if (target == SEL_SELESAI) {
            majuLevel();
        } else if (target == SEL_SPESIAL) {
            gantimapSpesial(barisB, kolomB);
        }
        return true;
    }

    // ---- Maju ke level berikutnya ----
    void majuLevel() {
        // Catat waktu level saat ini
        int detikLevel = (int)(time(nullptr) - waktuMulaiLevel);
        waktuPerLevel.push_back(detikLevel);
        totalDetikSaatIni += detikLevel;

        int levelBerikut = idxLevel + 1;
        if (levelBerikut < (int)daftarLevel.size()) {
            idxLevel = levelBerikut;
            pesan = "Level selesai! Memasuki " + daftarLevel[idxLevel].nama + "...";
            mulaiLevel();
        } else {
            menang   = true;
            berjalan = false;
        }
    }

    bool debugMode = false;   // mode debug aktif/non-aktif

    // Method untuk melewati satu level (hanya untuk debug)
    void skipLevel() {
        int detikLevel = (int)(time(nullptr) - waktuMulaiLevel);
        waktuPerLevel.push_back(detikLevel);
        totalDetikSaatIni += detikLevel;
        int levelBerikut = idxLevel + 1;
        if (levelBerikut < (int)daftarLevel.size()) {
            idxLevel = levelBerikut;
            pesan = "Level dilewati! Memasuki " + daftarLevel[idxLevel].nama + "...";
            mulaiLevel();  // reset checkpoint, acak map, spawn
        } else {
            menang = true;
            berjalan = false;
        }
    }
};



// ============================================================
//  SECTION 10 : SISTEM PERINTAH
// ============================================================

void tampilkanBantuan() {
    platform::bersihLayar(); // Memanggil fungsi dari scope platform untuk membersihkan terminal 
    cout << JUDUL << "=== MAZE GAME - BANTUAN ===" << RESET << "\n\n"; // Menampilkan judul dari menu bantuan

    cout << TEBAL << "  Gerakan\n" << RESET; // Menampilkan daftar tombol kontrol pemain  
    cout << "    W  -  Bergerak ke atas\n"
         << "    A  -  Bergerak ke kiri\n"
         << "    S  -  Bergerak ke bawah\n"
         << "    D  -  Bergerak ke kanan\n\n";

    cout << TEBAL << "  Simbol Peta\n" << RESET; // Menampilkan maksud dari simbol yang ada pada peta
    cout << "    " << MERAH   << "X" << RESET << "  -  Dinding      (nabrak = restart ke awal)\n";
    cout << "    " << RESET   << "*"          << "  -  Jalan        (bisa dilewati, tampil kosong)\n";
    cout << "    " << HIJAU   << "." << RESET << "  -  Mulai        (titik awal)\n";
    cout << "    " << KUNING  << "!" << RESET << "  -  Selesai      (titik finish)\n";
    cout << "    " << CYAN    << "@" << RESET << "  -  Pemain       (player)\n";
    cout << "    " << RESET   << "?"          << "  -  Special      (tersembunyi, tampil kosong, hanya di level 6-10)\n";
    cout << "    " << HIJAU   << "+" << RESET << "  -  Checkpoint   (Special Field yang terinjak menjadi Checkpoint)\n\n";

    cout << TEBAL << "  Special Field (Level 6-10)\n" << RESET; // Menampilkan penjelasan tentang special field yang berada pada level 6-10
    cout << "    Tidak semua terlihat di layar, tampil sama seperti jalan biasa.\n";
    cout << "    Diatur manual di file maps.txt menggunakan simbol '?'.\n";
    cout << "    Special Field '?' terletak sama di semua map dalam satu level.\n";
    cout << "    Jika diinjak, map berganti secara acak ke map lain di level yang sama!\n\n";

    cout << TEBAL << "  Perintah (tekan '/' lalu ketik + Enter)\n" << RESET; // Menampilkan tutorial untuk memanggil perintah
    cout << "    /help  -  Tampilkan bantuan ini\n" // Perintah untuk melihat bantuan 
         << "    /q     -  Keluar ke menu utama\n" // Perintah untuk keluar dari game di tengah permainan
         << "    /     -  Kembali ke permainan\n\n"; // Kembali ke game jika tidak jadi memanggil perintah 

    cout << TEBAL << "  Tujuan\n" << RESET; // Menmpilkan penjelasan objektif dari game mazeAmaze
    cout << "    Capai " << KUNING << "!" << RESET
         << " di setiap level. Selesaikan 10 level untuk masuk leaderboard.\n"
         << "    Map di setiap level dipilih secara acak.\n\n";

    cout << garisPemisah(44, '-') << "\n"; // Membuat garis pemisah untuk footer
    cout << "  Tekan sembarang tombol untuk kembali...\n"; // Menampilkan perintah menekan tombol untuk keluar menu bantuan 
    if (platform::bacaChar()) {platform::bersihLayar();} // Membaca inputan karakter untuk keluar menu bantuan
}

void tanganiPerintah(const string& rawCmd, StatusPermainan& status) { // Fungsi untuk memperoses perintah setelah command '/'
    string cmd = trim(rawCmd); // Menghapus spasi pada awal dan akhir kalimat
    // mengubah input dari command '/' menjadi kecil semua
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower); 

    if (cmd == "q" || cmd == "quit") { // Jika perintah adalah q atau quit
        status.berjalan    = false; // Menghentikan permainan
        status.kembaliMenu = true; // Kembali ke menu
    } else if (cmd == "help") { // Jika perintah adalah help
        tampilkanBantuan(); // Memanggil fungsi untuk menampilkan bantuan
    } else if (cmd == "debug") {
        status.debugMode = true;
        status.pesan = "DEBUG MODE AKTIF: * dan ? terlihat. Gunakan /normal untuk keluar, /skip untuk skip level.";
    }
    else if (cmd == "normal") {
        status.debugMode = false;
        status.pesan = "Mode normal. * dan ? tersembunyi.";
    }
    else if (cmd == "skip") {
        if (status.debugMode) {
            status.skipLevel();
            // pesan sudah di set di skipLevel()
        } else {
            status.pesan = "Perintah /skip hanya tersedia dalam mode debug. Aktifkan dengan /debug.";
        }
    }
    else if (cmd == "c" || cmd.empty()) {
        // kembali ke permainan
    }
    else {
        status.pesan = "Perintah tidak dikenal '/" + cmd + "'  |  Coba /help";
    }
}

// ============================================================
//  SECTION 11 : LOOP PERMAINAN
// ============================================================

void loopPermainan(StatusPermainan& status) { // Loop utama permainan
    while (status.berjalan) { // Ulangi selama status permainan masih berjalan
        const Map& mp = status.mapSaatIni(); // Mengambil map yang sedang dimainkan
        const Level& lv = status.levelSaatIni(); // Mengambil data level yang sedang dimainkan

        FungsiRenderSel callback = status.debugMode ? renderSelDebug : renderSelNormal;
        renderMap(mp, status.barisP, status.kolomP,
                lv.nama,
                status.pilihanMap, status.jumlahMapDiLevel(),
                status.pesan,
                status.totalDetikSaatIni,
                callback);   // <-- kirim callback
        status.pesan = ""; // menghapus pesan setelah ditampilkan

        int  tombol = platform::bacaChar(); // Membaca satu karakter atau tombol dari keyboard
        char ch     = (char)tolower((unsigned char)tombol); // Mengubah tombol menjadi huruf kecil

        // Arrow key: Windows kirim 0x00/0xE0 lalu kode arah
        // Linux kirim ESC [ A/B/C/D
        bool sudahGerak = false; // Menandakan apakah pemain sudah bergerak
        if (tombol == 0 || tombol == 0xE0) {          // Windows prefix
            int kode = platform::bacaChar(); // Membaca karakter untuk arah gerak pemain
            if      (kode == 72) { status.cobaGerak(-1,  0); sudahGerak = true; } // atas
            else if (kode == 80) { status.cobaGerak( 1,  0); sudahGerak = true; } // bawah
            else if (kode == 75) { status.cobaGerak( 0, -1); sudahGerak = true; } // kiri
            else if (kode == 77) { status.cobaGerak( 0,  1); sudahGerak = true; } // kanan
        // ESC — mungkin arrow Linux
        } else if (tombol == 27) {                    
            // Cek apakah ada karakter berikutnya dalam ~1ms (non-blocking peek)
        #ifndef _WIN32  // jika bukan windows
            struct termios oldt, newt; // menyimpan konfigurasi terminal lama
            tcgetattr(STDIN_FILENO, &oldt); 
            newt = oldt; // menyalin konfigurasi lama
            newt.c_lflag &= ~(ICANON | ECHO); // Menonaktifkan mode canonical dan echo
            newt.c_cc[VMIN]  = 0; // input tidak perlu menggunakan enter 
            newt.c_cc[VTIME] = 1; // 100ms timeout
            tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Menerapakan konfigurasi baru
            int c2 = getchar(); // Membaca karakter kedua 
            if (c2 == '[') { // Arrow linux membentuk ESC [
                int c3 = getchar(); // Membaca karakter arah
                if      (c3 == 'A') { status.cobaGerak(-1,  0); sudahGerak = true; } // Panah atas
                else if (c3 == 'B') { status.cobaGerak( 1,  0); sudahGerak = true; } // Panah bawah
                else if (c3 == 'D') { status.cobaGerak( 0, -1); sudahGerak = true; } // Panah kiri
                else if (c3 == 'C') { status.cobaGerak( 0,  1); sudahGerak = true; } // Panah kanan
            }
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Mengendalikan setting terminal semula
        #endif
        }

        if (!sudahGerak) { // Jika belum gerak menggunakan arrow key
            if      (ch == 'w') { status.cobaGerak(-1,  0); } // tombol w = atas
            else if (ch == 's') { status.cobaGerak( 1,  0); } // tombol S = bawah
            else if (ch == 'a') { status.cobaGerak( 0, -1); } // tombol A = kiri
            else if (ch == 'd') { status.cobaGerak( 0,  1); } // tombol D = kanan
            else if (ch == '/') { // Masuk ke mode command                            
                // Clear penuh saat membuka input perintah agar prompt tampil bersih
                platform::bersihLayar();
                        renderMap(mp, status.barisP, status.kolomP,
                lv.nama,
                status.pilihanMap, status.jumlahMapDiLevel(),
                status.pesan,
                status.totalDetikSaatIni,
                callback);   // gunakan callback yang sama
                // Menampilkan prompt command
                cout << "\033[?25h";      // tampilkan kembali kursor saat masuk command
                cout << "  " << TEBAL << "['/HELP' = bantuan] ['/q' = keluar] ['/ ' = kembali]\n  Perintah :" << RESET << "  /"; 
                cout.flush(); // Memastikan prompt langsung tampil 

                string cmd = bacaBaris(); // Membaca command sampai Enter 
                // Clear penuh setelah selesai input agar layar game kembali bersih
                platform::bersihLayar();
                tanganiPerintah(cmd, status); // Memproses command
            }
        } // end !sudahGerak
    }
}
// ============================================================
//  SECTION 12 : MENU UTAMA
// ============================================================

void tampilkanMenuUtama() { // Menampilkan menu utama dari game
    platform::bersihLayar(); // Membersihkan seluruh terminal sebelum menampilkan menu

    // Leaderboard top 10
    leaderboard::tampilkan(10); 
    // R sebelum "" agar bisa print sama persis dengan yang ditulis tanpa perlu /n atau " atau /t
    cout << "\n" << JUDUL << R"(
  +============================+
  |      M A Z E  G A M E      |
  +============================+
)" << RESET;

    cout << "\n"; // Memberi baris kosong
    cout << HIJAU << "  1. " << RESET << "Mulai Bermain\n"; // opsi 1
    cout << HIJAU << "  2. " << RESET << "Leaderboard Lengkap\n"; // opsi 2
    cout << HIJAU << "  3. " << RESET << "Bantuan\n"; // opsi 3
    cout << HIJAU << "  9. " << RESET << "the Gentlemen\n"; // opsi 9
    cout << HIJAU << "  0. " << RESET << "Keluar\n"; // opsi 4
    cout << "\n  Pilih: "; // posisi input
}

// ============================================================
//  SECTION 13 : MULAI BERMAIN
// ============================================================
void credit();      // deklarasi untuk menggunakan fungsi credit

void mulaiPermainan(vector<Level>& daftarLevel) { // Fungsi sesi mulai permainan baru
    if (daftarLevel.empty()) { // Mengecek apakah ada level yang berhasil dimuat
        cout << MERAH << "\n  Error: Tidak ada level yang dimuat!\n" << RESET; // Menampilkan pesan error
        platform::bacaChar();  // Menunggu pengguna menekan tombol sembarang
        return;
    }

    platform::bersihLayar(); // Membersihkan terminal sebelum mulai permainan
    cout << JUDUL << "=== MULAI BERMAIN ===\n" << RESET << "\n"; // Menampilkan Header menu mulai bermain
    cout << "  Masukkan nickname kamu: "; // Meminta inputan nama game
    string nick = bacaBaris(); // Membaca input nama 
    nick = trim(nick); // Menghapus spasi di awal dan akhir
    if (nick.empty()) nick = "Pemain"; // jika nick name dikosongkan maka menggunakan default nick name
    platform::bersihLayar(); // Membersihkan terminal 

    // Inisialisasi status
    StatusPermainan status;
    status.daftarLevel = daftarLevel;
    status.nickname    = nick;
    status.idxLevel    = 0;

    try {  // menggunakan exception handling untuk mulai permainan
        status.mulaiLevel(); // Mulai level pertama
    } catch (const exception& e) { // Jika terjadi error maka perintah selanjut nya tidak dijalankan
        cout << MERAH << "Error: " << e.what() << RESET << "\n";
        platform::bacaChar();
        return;
    }

    // Jalankan loop
    loopPermainan(status);

    // Setelah selesai
    platform::bersihLayar();

    if (status.menang) {
        int totalDetik = status.totalDetikSaatIni;
        // R sebelum "" agar bisa print sama persis dengan yang ditulis tanpa perlu /n atau " atau /t
        // Menampilkan pesan kemenangan
        cout << JUDUL << R"( 
  +==================================+
  |  SELAMAT! KAMU MENYELESAIKAN     |
  |  SEMUA 10 LEVEL! AMAZING!        |
  +==================================+
)" << RESET;
        cout << BIRU << "\n  Total Waktu : " << totalDetik // Menampilkan total waktu format menit:detik
             << " detik (" << formatWaktu(totalDetik) << ")\n" << RESET;
        cout << "  Nickname    : " << nick << "\n\n"; // Menampilkan nickname

        // Simpan ke leaderboard
        try {
            leaderboard::tambah(nick, totalDetik);
            cout << HIJAU << "  Hasil tersimpan ke leaderboard!\n" << RESET;
        } catch (const exception& e) {
            cout << MERAH << "  Gagal simpan leaderboard: " << e.what() << RESET << "\n";
        }
        cout << "\n  Tekan sembarang tombol...\n";
        platform::bacaChar();
        platform::bersihLayar(); // Membersihkan layar terminal
        credit();


    } else if (status.kembaliMenu) { // Jika pemain menggunakan command /q
        cout << "\n  Kamu keluar ke menu. Progress tidak disimpan.\n";
        cout << "  (Hanya tersimpan jika menyelesaikan semua 10 level)\n";
        cout << "\n  Tekan sembarang tombol...\n";
        platform::bacaChar();
    }
}

// ============================================================
//  SECTION 14 : LEADERBOARD LENGKAP
// ============================================================

void tampilkanLeaderboardLengkap() { // Menampilkan seluruh isi leaderboard
    platform::bersihLayar(); // Membersihkan layar terminal sebelum menampilkan leaderboard
    cout << JUDUL << "=== LEADERBOARD LENGKAP ===\n" << RESET << "\n"; // Menampilkan header menu leaderboard

    list<EntriLeaderboard> daftar = leaderboard::muat(); // Mengambil data leaderboard dari file

    if (daftar.empty()) { // Jika leaderboard kosong
        cout << "  Belum ada data leaderboard.\n"; // Menampilkan pesan belum ada data leaderboard
    } else {
        // Hitung dengan iterator
        int total = (int)distance(daftar.begin(), daftar.end()); // Menghitung jumlah entri pada list distance() menghitung jarak dari begin() ke end()
        cout << "  Total entri: " << total << "\n\n"; // Menampilkan jumlah total data

        int urutan = 1; // Nomor urut rangking
        for (auto it = daftar.begin(); it != daftar.end(); ++it, ++urutan) { // Traversal seluruh data leaderboard menggunakan iterator
            cout << KUNING << "  " << urutan << ". " << RESET
                 << it->nickname // Menampilkan nickname pemain
                 << string(max(1, 20 - (int)it->nickname.size()), ' ')  // Menambahkan spasi agar kolom waktu rata
                 << formatWaktu(it->totalDetik) // Menampilkan waktu dalam format mm:ss
                 << " (" << it->totalDetik << " detik)\n";   // Menampilkan waktu asli dalam detik
        }
    }

    cout << "\n" << garisPemisah(44, '-') << "\n"; // Menampilkan garis pemisah
    cout << "  Tekan sembarang tombol untuk kembali...\n"; // Pesan untuk kembali ke menu
    platform::bacaChar(); // Menunggu pengguna menekan tombol pada keyboard
}




// Fungsi untuk menampilkan teks dengan efek per baris (bukan per karakter)
// teks            : string yang akan ditampilkan (bisa multi-baris)
// delayPerBarisMs : jeda waktu antar baris dalam milidetik (default 50ms)
void typeLine(const string& teks, int delayPerBarisMs = 50) {
    if (teks.empty()) return;  // jika teks kosong, langsung keluar fungsi
    
    istringstream iss(teks);   // konversi string menjadi stream agar mudah dipecah per baris
    string baris;              // variabel penyimpan sementara untuk setiap baris
    
    while (getline(iss, baris)) {           // baca satu baris dari stream (pisah berdasarkan newline)
        cout << baris << endl;              // cetak baris utuh sekaligus, lalu pindah baris
        cout.flush();                       // paksa output langsung ditampilkan ke layar
        
        if (!iss.eof()) {                   // jika masih ada baris berikutnya (bukan baris terakhir)
            this_thread::sleep_for(chrono::milliseconds(delayPerBarisMs));  // jeda antar baris
        }
    }
}


void credit(){ // Menmpilkan credit 
    int delay = 500; //milisec
    platform::bersihLayar();   // Membersihkan layar sebelum masuk fungsi menampilkan credit
    cout << "\033[999;1H" << flush;  // pindah ke baris akhir, kolom pertama
    cout << MERAH << TEBAL; // set warna merah dan tebal
    typeLine(R"(
                                                                                                                                    ***         ***
                                                                                                                                  *******     *******
                                                __  __           _        ____ _   __  _                                         *********   *********
                                               |  \/  | __ _  __| | ___  | __ ) \ / / | |    _____   _____                      ***********************
                                               | |\/| |/ _` |/ _` |/ _ \ |  _ \\ V /  | |   / _ \ \ / / _ \                     ***********************
                                               | |  | | (_| | (_| |  __/ | |_) || |   | |__| (_) \ V /  __/                      *********************
                                               |_|  |_|\__,_|\__,_|\___| |____/ |_|   |_____\___/ \_/ \___|                       *******************
                                                                                                                                   *****************
                                                                                                                                    ***************
                                                                                                                                     *************
                                                                                                                                      ***********
                                                                                                                                       *********
                                                                                                                                        *******
                                                                                                                                         *****
                                                                                                                                          ***
                                                                                                                                           *)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris
    


    cout << KUNING << JUDUL;
    typeLine(R"(
                                     ________  ___  ________  ___  ___  ________  ___  ___  ________                 ________  ___       _______   ___  ___
                                     |\   ___ \|\  \|\   ____\|\  \|\  \|\   ____\|\  \|\  \|\   ___  \              |\   __  \|\  \     |\  ___ \ |\  \|\  \
                                     \ \  \_|\ \ \  \ \  \___|\ \  \\\  \ \  \___|\ \  \\\  \ \  \\ \  \             \ \  \|\  \ \  \    \ \   __/|\ \  \\\  \
                                      \ \  \ \\ \ \  \ \_____  \ \  \\\  \ \_____  \ \  \\\  \ \  \\ \  \             \ \  \\\  \ \  \    \ \  \_|/_\ \   __  \
                                       \ \  \_\\ \ \  \|____|\  \ \  \\\  \|____|\  \ \  \\\  \ \  \\ \  \             \ \  \\\  \ \  \____\ \  \_|\ \ \  \ \  \
                                        \ \_______\ \__\____\_\  \ \_______\____\_\  \ \_______\ \__\\ \__\             \ \_______\ \_______\ \_______\ \__\ \__\
                                         \|_______|\|__|\_________\|_______|\_________\|_______|\|__| \|__|              \|_______|\|_______|\|_______|\|__|\|__|
                                                       \|_________|        \|_________| 
                     
                          )"); // Menampilkan informasi nama pembuat game 
    cout << JUDUL << BIRU;
    typeLine(R"(
                                                    ____     __  ___  _______  __        ______   .___  ___. .______     ______    __  ___     __      ____
                                                   |    |   |  |/  / |   ____||  |      /  __  \  |   \/   | |   _  \   /  __  \  |  |/  /    |  |    |    |
                                                   |  |-`   |  '  /  |  |__   |  |     |  |  |  | |  \  /  | |  |_)  | |  |  |  | |  '  /     |  |    `-|  |
                                                   |  |     |    <   |   __|  |  |     |  |  |  | |  |\/|  | |   ___/  |  |  |  | |    <      |  |      |  |
                                                   |  |     |  .  \  |  |____ |  `----.|  `--'  | |  |  |  | |  |      |  `--'  | |  .  \     |  |      |  |
                                                   |  |-.   |__|\__\ |_______||_______| \______/  |__|  |__| | _|       \______/  |__|\__\    |__|    .-|  |
                                                   |____|                                                                                             |____|

        )");
    
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris
    cout << PUTIH << TEBAL;

    typeLine(R"DELIM(


                                           db    88""Yb .dP"Y8 Yb  dP     8b    d8    db    88   88 88        db    88b 88    db        .dP"Y8 88 8888b.  88  dP"Yb
                                          dPYb   88__dP `Ybo."  YbdP      88b  d88   dPYb   88   88 88       dPYb   88Yb88   dPYb       `Ybo." 88  8I  Yb 88 dP   Yb
                                         dP__Yb  88"Yb  o.`Y8b   8P       88YbdP88  dP__Yb  Y8   8P 88  .o  dP__Yb  88 Y88  dP__Yb      o.`Y8b 88  8I  dY 88 Yb b dP
                                        dP""""Yb 88  Yb 8bodP'  dP        88 YY 88 dP""""Yb `YbodP' 88ood8 dP""""Yb 88  Y8 dP""""Yb     8bodP' 88 8888Y"  88  `"YoYo))
            )DELIM");
    typeLine(R"(
                                                              __  _____   _____   __    _____   _____   __    __    __    __    _____  __
                                                             / / / __  \ |  ___| /  |  |  _  | |  ___| /  |  /  |  /  |  /  |  |  _  | \ \
                                                            | |  `' / /' |___ \  `| |  | |/' | |___ \  `| |  `| |  `| |  `| |  | |_| |  | |
                                                            | |    / /       \ \  | |  |  /| |     \ \  | |   | |   | |   | |  \____ |  | |
                                                            | |  ./ /___ /\__/ / _| |_ \ |_/ / /\__/ / _| |_ _| |_ _| |_ _| |_ .___/ /  | |
                                                            | |  \_____/ \____/  \___/  \___/  \____/  \___/ \___/ \___/ \___/ \____/   | |
                                                             \_\                                                                       /_/)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris
    
    

    typeLine(R"DELIM(


                                              _____          ______ ______        _   _         _    _ ______      _        _______    _ _    _ _____ _____
                                             |  __ \   /\   |  ____|  ____/\     | \ | |   /\  | |  | |  ____/\   | |      |___  / |  | | |  | |  __ \_   _|
                                             | |  | | /  \  | |__  | |__ /  \    |  \| |  /  \ | |  | | |__ /  \  | |         / /| |  | | |__| | |  | || |
                                             | |  | |/ /\ \ |  __| |  __/ /\ \   | . ` | / /\ \| |  | |  __/ /\ \ | |        / / | |  | |  __  | |  | || |
                                             | |__| / ____ \| |    | | / ____ \  | |\  |/ ____ \ |__| | | / ____ \| |____   / /__| |__| | |  | | |__| || |_
                                             |_____/_/    \_\_|    |_|/_/    \_\ |_| \_/_/    \_\____/|_|/_/    \_\______| /_____|\____/|_|  |_|_____/_____|
                                             )DELIM");
                                             
    typeLine(R"(
                                                              __  _____   _____   __    _____   _____   __    __    __    _____   _____  __
                                                             / / / __  \ |  ___| /  |  |  _  | |  ___| /  |  /  |  /  |  |  _  | |  _  | \ \
                                                            | |  `' / /' |___ \  `| |  | |/' | |___ \  `| |  `| |  `| |  | |/' |  \ V /   | |
                                                            | |    / /       \ \  | |  |  /| |     \ \  | |   | |   | |  |  /| |  / _ \   | |
                                                            | |  ./ /___ /\__/ / _| |_ \ |_/ / /\__/ / _| |_ _| |_ _| |_ \ |_/ / | |_| |  | |
                                                            | |  \_____/ \____/  \___/  \___/  \____/  \___/ \___/ \___/  \___/  \_____/  | |
                                                             \_\                                                                         /_/)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris

    typeLine(R"DELIM(

        
                             _   _  _____  ___    _  _______  _   _            _____  _   _  _      _____  _   _  _____     _____  _         ___    _   _  _  ___    _____  ___    _     _
                            ( ) ( )(  _  )(  _`\ (_)(_____  )( ) ( )   /'\_/`\(  _  )( ) ( )( )    (  _  )( ) ( )(  _  )   (  _  )( )       (  _`\ ( ) ( )(_)(  _`\ (  _  )|  _`\ ( )   ( )
                            | |_| || (_) || (_(_)| |     /'/'| |_| |   |     || (_) || | | || |    | (_) || `\| || (_) |   | (_) || |       | ( (_)| |_| || || (_(_)| (_) || (_) )`\`\_/'/'
                            |  _  ||  _  ||  _)  | |   /'/'  |  _  |   | (_) ||  _  || | | || |  _ |  _  || , ` ||  _  |   |  _  || |  _    | |___ |  _  || ||  _)  |  _  || ,  /   `\ /'
                            | | | || | | || |    | | /'/'___ | | | |   | | | || | | || (_) || |_( )| | | || |`\ || | | |   | | | || |_( )   | (_, )| | | || || |    | | | || |\ \    | |
                            (_) (_)(_) (_)(_)    (_)(_______)(_) (_)   (_) (_)(_) (_)(_____)(____/'(_) (_)(_) (_)(_) (_)   (_) (_)(____/'   (____/'(_) (_)(_)(_)    (_) (_)(_) (_)   (_) 
            )DELIM");
    typeLine(R"(
                                                               __  _____   _____   __    _____   _____   __    __    __    __    _____  __
                                                              / / / __  \ |  ___| /  |  |  _  | |  ___| /  |  /  |  /  |  /  |  / __  \ \ \
                                                             | |  `' / /' |___ \  `| |  | |/' | |___ \  `| |  `| |  `| |  `| |  `' / /'  | |
                                                             | |    / /       \ \  | |  |  /| |     \ \  | |   | |   | |   | |    / /    | |
                                                             | |  ./ /___ /\__/ / _| |_ \ |_/ / /\__/ / _| |_ _| |_ _| |_ _| |_ ./ /___  | |
                                                             | |  \_____/ \____/  \___/  \___/  \____/  \___/ \___/ \___/ \___/ \_____/  | |
                                                              \_\                                                                       /_/)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris

    
    typeLine(R"DELIM(


                                                     __   ________   __    __       ___      .______          __    __   __   __       __  ___   ______
                                                    |  | |       /  |  |  |  |     /   \     |   _  \        |  |  |  | |  | |  |     |  |/  /  /  __  \
                                                    |  | `---/  /   |  |__|  |    /  ^  \    |  |_)  |       |  |__|  | |  | |  |     |  '  /  |  |  |  |
                                                    |  |    /  /    |   __   |   /  /_\  \   |      /        |   __   | |  | |  |     |    <   |  |  |  |
                                                    |  |   /  /----.|  |  |  |  /  _____  \  |  |\  \--.     |  |  |  | |  | |  `----.|  .  \  |  `--'  |
                                                    |__|  /________||__|  |__| /__/     \__\ | _| `.___|     |__|  |__| |__| |_______||__|\__\  \______/
            )DELIM");
            
    typeLine(R"(
                                                               __  _____   _____   __    _____   _____   __    __    __    _____   __   __
                                                              / / / __  \ |  ___| /  |  |  _  | |  ___| /  |  /  |  /  |  / __  \ /  |  \ \
                                                             | |  `' / /' |___ \  `| |  | |/' | |___ \  `| |  `| |  `| |  `' / /' `| |   | |
                                                             | |    / /       \ \  | |  |  /| |     \ \  | |   | |   | |    / /    | |   | |
                                                             | |  ./ /___ /\__/ / _| |_ \ |_/ / /\__/ / _| |_ _| |_ _| |_ ./ /___ _| |_  | |
                                                             | |  \_____/ \____/  \___/  \___/  \____/  \___/ \___/ \___/ \_____/ \___/  | |
                                                              \_\                                                                       /_/)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris
    
    typeLine(R"DELIM(


                                 ___________         _    _ _         _    _          ____      ____           _____ ______ _        _______ ____  _       _____ _______ ______     __
                                |___  /_   _|   /\  | |  | | |       | |  | |   /\   / __ \    |  _ \   /\    / ____|  ____| |      |__   __/ __ \| |     / ____|__   __/ __ \ \   / /
                                   / /  | |    /  \ | |  | | |       | |__| |  /  \ | |  | |   | |_) | /  \  | (___ | |__  | |         | | | |  | | |    | (___    | | | |  | \ \_/ /
                                  / /   | |   / /\ \| |  | | |       |  __  | / /\ \| |  | |   |  _ < / /\ \  \___ \|  __| | |         | | | |  | | |     \___ \   | | | |  | |\   /
                                 / /__ _| |_ / ____ \ |__| | |____   | |  | |/ ____ \ |__| |   | |_) / ____ \ ____) | |____| |____     | | | |__| | |____ ____) |  | | | |__| | | |
                                /_____|_____/_/    \_\____/|______|  |_|  |_/_/    \_\___\_\   |____/_/    \_\_____/|______|______|    |_|  \____/|______|_____/   |_|  \____/  |_|
    )DELIM");

    typeLine(R"(
                                                              __  _____   _____   __    _____   _____   __    __    __    _____   _____  __
                                                             / / / __  \ |  ___| /  |  |  _  | |  ___| /  |  /  |  /  |  / __  \ |____ | \ \
                                                            | |  `' / /' |___ \  `| |  | |/' | |___ \  `| |  `| |  `| |  `' / /'     / /  | |
                                                            | |    / /       \ \  | |  |  /| |     \ \  | |   | |   | |    / /       \ \  | |
                                                            | |  ./ /___ /\__/ / _| |_ \ |_/ / /\__/ / _| |_ _| |_ _| |_ ./ /___ .___/ /  | |
                                                            | |  \_____/ \____/  \___/  \___/  \____/  \___/ \___/ \___/ \_____/ \____/   | |
                                                             \_\                                                                         /_/)");
    this_thread::sleep_for(chrono::milliseconds(delay));  // jeda antar baris
    

    
    cout << RESET;
    cout << "\n\n\n";
    cout << "  Tekan sembarang tombol untuk kembali...";
    platform::bacaChar();
}




// ============================================================
//  SECTION 15 : ENTRY POINT
// ============================================================

//=====================================
//========== Fungsi Utama =============
//=====================================

int main(int argc, char* argv[]) {   // agar program bisa terima argumen untuk devoloping dengan penggunaan map yg berbeda
    platform::inisialisasi(); // Inisialisasi Ansi untuk membaca kode warna 
    srand((unsigned)time(nullptr)); // Menginisalisasi random number generator untuk random map

    string filePeta = "maps.txt"; // Inisialisasi default nama peta
    if (argc > 1) filePeta = argv[1]; // Jika dijalankan dengan argumen maka menggunakan file yang diberikan 

    // Muat level dari file
    vector<Level> daftarLevel; 
    try { // Menggunakan exception handling untuk error saat membaca file
        daftarLevel = muatFilePeta(filePeta); // Membaca seluruh file peta
    } catch (const exception& e) {
        cout << MERAH << "Error: " << e.what() << RESET // Menampilkan pesan error jika file gagal dimuat
             << "\nGunakan: " << argv[0] << " [file_peta.txt]\n";
        return 1;
    }

    if (daftarLevel.empty()) { // Memastikan file peta berisi level yang valid
        cout << MERAH << "Error:" << RESET
             << " File peta '" << filePeta << "' tidak berisi level yang valid.\n";
        return 1;
    }

    // Loop menu utama
    bool jalan = true;
    while (jalan) {
        tampilkanMenuUtama(); // Menampilkan menu utama 
 
        string pilihan = bacaBaris(); // Membaca pilihan pengguna
        pilihan = trim(pilihan); // Menghapus spasi di awal dan akhir input

        try { // Menggunakan exception handling untuk error pemilihan menu
            if (pilihan == "1") { // Jika memilih menu satu
                platform::bersihLayar();   // Membersihkan layar
                cout << JUDUL << KUNING << R"(
    JIKA TAMPILAN RUSAK, ZOOM OUT TERMINAL, LALU TEKAN '/' LALU ENTER
    )" << RESET ;
                platform::bacaChar();
                platform::bersihLayar();   // Membersihkan layar
                mulaiPermainan(daftarLevel); // Maka memanggil fungsi mulai permainan
            } else if (pilihan == "2") { // jika memilih 2 
                tampilkanLeaderboardLengkap(); // Maka akan menampilkan Leaderboard lengkap
            } else if (pilihan == "3") { // Jika memilih 3 
                tampilkanBantuan(); // Maka memanggil fungsi menu bantuan 
            } else if (pilihan == "9") {
                credit(); // memanggil fungsi credit
            } else if (pilihan == "0") { // Jika Pilihan 0 
                jalan = false; // Program tidak berjalan atau berhenti (Keluar program)
            } else {
                // Lambda validasi pilihan
                auto pesanInvalid = [&](const string& p) {
                    cout << MERAH << "\n  Pilihan '" << p << "' tidak valid.\n" << RESET;
                    cout << "  Tekan sembarang tombol...\n";
                    platform::bacaChar();
                };
                pesanInvalid(pilihan);
            }
        } catch (const exception& e) { // Menangkap error yang terjadi pada pilihan
            cout << MERAH << "\n  Terjadi error: " << e.what() << RESET << "\n";
            cout << "  Tekan sembarang tombol...\n";
            platform::bacaChar();
        }
    }

    platform::bersihLayar(); // Membersihkan seluruh layar terminal sebelum keluar
    cout << "\n  Terima kasih telah bermain Maze Game!\n\n"; // Menampilkan pesan penutup
    return 0;
}
