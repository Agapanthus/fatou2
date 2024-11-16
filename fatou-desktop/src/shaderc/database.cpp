#include "database.h"

#include <blosc/blosc.h>
#include <shlobj_core.h>

#include <filesystem>
#include <boost/regex.hpp>
#include "../window/console.h"

unique_ptr<DatabaseManager> fatouDB = nullptr;

const int BLOSC_BUFFER_SIZE = 1024 * 1024 * 10;
static uint8_t BLOSC_BUFFER[BLOSC_BUFFER_SIZE];

#define USE_LOCAL_FILES true

path getIncPath() {
    const static vector<path> tryme = {fs::current_path() / "public",
                                       "C:/code/clouds/public"};

    for (const path p : tryme) {
        if (fs::exists(p / "shaders/playground/mandeld.frag")) {
            return p;
        }
    }

    fatalBox("Couldn't find resources!");
}

DatabaseManager::DatabaseManager() {
    blosc_init();
    // zstd = very slow, strongest compression
    // lz4hc - fast
    // blosclz - fastest

    blosc_set_compressor("lz4hc");
    blosc_set_nthreads(4);

    const fs::path path = getAppData();
    const fs::path dbPath = path / "db";

    // create if it doesn't exist
    if (!fs::is_directory(path) || !fs::exists(path)) {
        fs::create_directory(path);
    }

#if !USE_LOCAL_FILES
    bool recreate = false || !fs::exists(dbPath);
#ifndef NDEBUG
    if (recreate && fs::exists(dbPath)) {
        std::uintmax_t n = fs::remove(dbPath);
    }
#endif

    if (sqlite3_open_v2(dbPath.string().c_str(), &db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) {
        // TODO
        std::cerr << "Can't open database: \n"
                  << sqlite3_errmsg(db) << std::endl;
        fatalBox("TODO: Can't open database!");
        db = nullptr;
        return;
    }

    if (recreate)
        this->recreate();

    createStmts();
#endif
}

vector<uint8_t> DatabaseManager::getFile(const path &filename) {

#if (USE_LOCAL_FILES)
    const path incPath = getIncPath();
    return readFile2<uint8_t>((incPath / filename).string());
#else
    SQLiteMutex _(db);

    stmtFile->bind(1, filename.string());
    stmtFile->exe();
    auto blob = stmtFile->blob(0);
    int uncompressed = stmtFile->integer(1);
    stmtFile->reset();

    if (uncompressed > 0) {
        int dsize =
            blosc_decompress(blob.data(), BLOSC_BUFFER, BLOSC_BUFFER_SIZE);
        if (dsize < 0) {
            printf("Decompression error.  Error code: %d\n", dsize);
            return {};
        }
        return vector<uint8_t>(BLOSC_BUFFER, BLOSC_BUFFER + dsize);
    } else if (uncompressed < 0) {
        const auto res = debrotli.decompress(blob, -uncompressed);
        return res;
    } else {
        return blob;
    }
#endif
}

bool DatabaseManager::fileExists(const path &filename) {
#if (USE_LOCAL_FILES)
    const path incPath = getIncPath();
    return fs::exists(incPath / filename);
#else
    stmtFile->bind(1, filename.string());
    bool found = stmtFile->tryExe();
    int size = stmtFile->integer(1);
    stmtFile->reset();
    return size != 0;
#endif
}

void DatabaseManager::storeFile(const path &name,
                                const vector<uint8_t> &content) {

#if (USE_LOCAL_FILES)
    const path incPath = getIncPath();
    std::ofstream file((incPath / name), std::ios::binary);
    file.write((char *)content.data(), content.size() * sizeof(uint8_t));
    file.close();
#else
    SQLiteMutex _(db);

    string filename = name.string();
    std::replace(filename.begin(), filename.end(), '\\', '/');

    if (content.size() >= BLOSC_BUFFER_SIZE) {
        fatalBox("blosc buffer too small!");
    }

    int csize = blosc_compress(5, 1, sizeof(uint8_t), content.size(),
                               content.data(), BLOSC_BUFFER, BLOSC_BUFFER_SIZE);
    if (csize < 0) {
        printf("Compression error.  Error code: %d\n", csize);
        return;
    }

    const auto res = brotli.bestCompress(content);
    std::cout << res.size() / float(csize) << " "
              << res.size() / float(content.size()) << " " << filename
              << std::endl;

    // std::cout << csize / float(content.size()) << " " << filename <<
    // std::endl;

    stmtStore->bind(1, filename);
    if (csize <= res.size() && csize / float(content.size()) < 0.95) {
        stmtStore->bind(2, content.size());
        stmtStore->bind(3, (const void *)BLOSC_BUFFER, csize);
    } else if (res.size() <= csize &&
               res.size() / float(content.size()) < 0.95) {
        stmtStore->bind(2, -content.size());
        stmtStore->bind(3, (const void *)res.data(), res.size());
    } else {
        stmtStore->bind(2, 0);
        stmtStore->bind(3, (const void *)content.data(), content.size());
    }
    stmtStore->exe();
    stmtStore->reset();
#endif
}

fs::path DatabaseManager::getAppData() {
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL,
                                  0, szPath))) {
        return fs::path(szPath) / "fatou";
    }

    fatalBox("TODO: Can't find app data!");

    return fs::current_path();
}

void DatabaseManager::recreate() {

#ifndef NDEBUG
#if !USE_LOCAL_FILES

    {
        SQLiteMutex _(db);

        const char *sql = "CREATE TABLE FILES("
                          "ID INTEGER PRIMARY KEY AUTOINCREMENT     NOT NULL,"
                          "NAME           TEXT                      NOT NULL,"
                          "UNCOMPRESSED   INT                       NOT NULL,"
                          "DATA           BLOB                      NOT NULL);"
                          "CREATE UNIQUE INDEX idx_files_name ON FILES (NAME);";

        char *zErrMsg = 0;
        if (sqlite3_exec(db, sql, nullptr, 0, &zErrMsg) != SQLITE_OK) {
            fprintf(stderr, "TODO: SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    }
    createStmts();

    const path incPath = getIncPath();
    for (const auto &entry : fs::recursive_directory_iterator(incPath)) {
        if (fs::is_regular_file(entry)) {
            string s = entry.path().string();
            string s2 = s.substr(incPath.string().length() + 1);

            auto content = readFile2<uint8_t>(entry.path().string());
            storeFile(s2, content);
        }
    }
    {
        SQLiteMutex _(db);
        sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, nullptr);
    }
#endif
#endif
}

void DatabaseManager::createStmts() {

    SQLiteMutex _(db);
    if (!stmtStore.get())
        stmtStore = std::make_unique<DatabaseStatement>(
            db, "INSERT INTO FILES (NAME, UNCOMPRESSED, DATA) VALUES (?,?,?);");

    if (!stmtFile.get())
        stmtFile = std::make_unique<DatabaseStatement>(
            db, "SELECT DATA, UNCOMPRESSED from FILES where NAME = ?");
}