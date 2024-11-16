#pragma once

#include <fstream>

#include <sqlite3.h>
#include "../window/console.h"
#include <brotli/encode.h>
#include <brotli/decode.h>

namespace fs = std::filesystem;

class Brotli {
  public:
    Brotli() { state = BrotliEncoderCreateInstance(0, 0, 0); }

    ~Brotli() { BrotliEncoderDestroyInstance(state); }

    vector<uint8_t>
    compress(const vector<uint8_t> data,
             BrotliEncoderMode mode = BrotliEncoderMode::BROTLI_MODE_GENERIC) {
        vector<uint8_t> output(BrotliEncoderMaxCompressedSize(data.size()));
        int quality = BROTLI_DEFAULT_QUALITY;
        int lgwin = BROTLI_DEFAULT_WINDOW;
        size_t encodedSize = output.size();
        BrotliEncoderCompress(quality, lgwin, mode, data.size(), data.data(),
                              &encodedSize, output.data());
        output.resize(encodedSize);
        return output;
    }

    vector<uint8_t> bestCompress(const vector<uint8_t> data) {

        const auto res1 = compress(data, BROTLI_MODE_FONT);
        const auto res2 = compress(data, BROTLI_MODE_GENERIC);
        const auto res3 = compress(data, BROTLI_MODE_TEXT);
        if (res1.size() <= res2.size() && res1.size() <= res3.size()) {
            return res1;
        }
        if (res2.size() <= res3.size()) {
            return res2;
        }
        return res3;
    }

  private:
    BrotliEncoderState *state;
};

class Debrotli {
  public:
    Debrotli() { state = BrotliDecoderCreateInstance(0, 0, 0); }

    ~Debrotli() { BrotliDecoderDestroyInstance(state); }

    vector<uint8_t> decompress(const vector<uint8_t> data,
                               size_t targetOutputSize) {

        vector<uint8_t> output(targetOutputSize);
        size_t decodedSize = targetOutputSize;
        // TODO: error checking!
        BrotliDecoderDecompress(data.size(), data.data(), &decodedSize,
                                output.data());
        if (decodedSize != targetOutputSize) {
            fatalBox("TODO: decode error!");
        }
        return output;
    }

  private:
    BrotliDecoderState *state;
};

template <typename T> vector<T> readFile2(const string &filename) {
    // The advantage of starting to read at the end of the file is that we can
    // use the read position to determine the size of the file and allocate a
    // buffer:
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        fatalBox("TODO: couldn't open file " + filename);
        throw runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    if (fileSize % sizeof(T) != 0) {
        fatalBox("TODO: fail has wrong size");
        throw runtime_error("fail has wrong size!");
    }
    vector<T> buffer(fileSize / sizeof(T));
    file.seekg(0);
    file.read((char *)buffer.data(), fileSize);
    file.close();
    return buffer;
}

inline bool fileExists(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}

class SQLiteMutex {
  public:
    SQLiteMutex(sqlite3 *db) : db(db) {
        sqlite3_mutex_enter(sqlite3_db_mutex(db));
    }
    ~SQLiteMutex() { sqlite3_mutex_leave(sqlite3_db_mutex(db)); }

  private:
    sqlite3 *db;
};

class DatabaseStatement {
  public:
    DatabaseStatement(sqlite3 *db, const char *txt) {
        if (sqlite3_prepare_v3(db, txt, -1, SQLITE_PREPARE_PERSISTENT, &stmt,
                               nullptr) != SQLITE_OK) {
            string s = sqlite3_errmsg(db);
            // fprintf(stderr, "SQL error: %s\n", s);
            fatalBox("sql prepare failed!");
        }
    }
    ~DatabaseStatement() { sqlite3_finalize(stmt); }

    void bind(int i, const string &s) { bind(i, s.c_str()); }

    void bind(int i, const char *s) {
        sqlite3_bind_text(stmt, i, s, strlen(s), nullptr);
    }

    void bind(int i, const void *data, size_t n) {
        sqlite3_bind_blob(stmt, i, data, n, nullptr);
    }

    void bind(int i, int data) { sqlite3_bind_int(stmt, i, data); }

    bool tryExe() {
        int res = sqlite3_step(stmt);
        return (res == SQLITE_DONE || res == SQLITE_ROW);
    }

    void exe() {
        if (!tryExe()) {
            fatalBox("TODO: sql step failed!");
        }
    }

    vector<string> allStrings(int i) {
        vector<string> r;
        int res;
        while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
            r.push_back(text(i));
        }
        return r;
    }

    void reset() {
        sqlite3_clear_bindings(stmt);
        if (sqlite3_reset(stmt) != SQLITE_OK) {
            fatalBox("TODO: sql reset failed!");
        }
    }

    string text(int i) {
        const unsigned char *blob = sqlite3_column_text(stmt, i);
        size_t blob_bytes = sqlite3_column_bytes(stmt, i);
        return string(blob, blob + blob_bytes);
    }

    vector<uint8_t> blob(int i) {
        const uint8_t *blob = (const uint8_t *)sqlite3_column_blob(stmt, i);
        size_t blob_bytes = sqlite3_column_bytes(stmt, i);
        return vector<uint8_t>(blob, blob + blob_bytes);
    }

    int integer(int i) { return sqlite3_column_int(stmt, i); }

  public:
    sqlite3 *db;
    sqlite3_stmt *stmt;
};

class DatabaseManager {
  public:
    DatabaseManager();

    void createStmts();

    bool fileExists(const path &filename);

    vector<uint8_t> getFile(const path &filename);
    /* vector<uint32_t> getFile32(const char* filename) {
         auto tmp = getFile(filename);
         const uint32_t* d = (const uint32_t * )tmp.data();
         vector< uint32_t> res(d, d + tmp.size() / sizeof(uint32_t));
         return res;
     }*/

    void storeFile(const path &name, const vector<uint8_t> &content);
    void storeFile(const path &name, const vector<uint32_t> &content) {
        const uint8_t *d = (const uint8_t *)content.data();
        const vector<uint8_t> content2(d,
                                       d + (content.size() * sizeof(uint32_t)));
        storeFile(name, content2);
    }
    void recreate();

    fs::path getAppData();

    ~DatabaseManager() {
        if (db)
            sqlite3_close(db);
    }

  private:
    sqlite3 *db = nullptr;
    unique_ptr<DatabaseStatement> stmtFile;
    unique_ptr<DatabaseStatement> stmtStore;

    Brotli brotli;
    Debrotli debrotli;
};

extern unique_ptr<DatabaseManager> fatouDB;

inline void createFatouDB() {
    if (!fatouDB.get())
        fatouDB = make_unique<DatabaseManager>();
}