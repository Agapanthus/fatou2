#include "shaderCache.h"
#include <fstream>
#include "../database.h"

/*
inline bool fileExists(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();

template <typename T> vector<T> readFile(const string &filename) {
    // The advantage of starting to read at the end of the file is that we can
    // use the read position to determine the size of the file and allocate a
    // buffer:
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    if (fileSize % sizeof(T) != 0) {
        throw runtime_error("fail has wrong size!");
    }
    vector<T> buffer(fileSize / sizeof(T));
    file.seekg(0);
    file.read((char *)buffer.data(), fileSize);
    file.close();
    return buffer;
}*/

ShaderCache cache;

ShaderCache::ShaderCache() { createFatouDB(); }

/*
optional<vector<uint32_t>> ShaderCache::getCompiled(const string &path) {
    optional<vector<uint32_t>> spv;
    if (fatouDB->fileExists((path + ".spv").c_str())) {
        spv = fatouDB->getFile32((path + ".spv").c_str());
        // readFile<uint32_t>(path + ".spv");
        return spv;
    }
    return spv;
}*/

optional<vector<uint8_t>> ShaderCache::getCompiled(const path &p) {
    optional<vector<uint8_t>> spv;
    const path p2 = p.string() + ".spv";
    if (fatouDB->fileExists(p2)) {
        spv = fatouDB->getFile(p2);
        // readFile<uint32_t>(path + ".spv");
        return spv;
    }
    return spv;
}

string ShaderCache::getSource(const path &path) {
    auto buf = fatouDB->getFile(path); // readFile<char>(path);
    return string(buf.begin(), buf.end());
}

void ShaderCache::store(const path &path, const vector<uint32_t> &data) {
    fatouDB->storeFile(path.string() + ".spv", data);
}

void ShaderCache::store(const path &path, const vector<uint8_t> &data) {
    fatouDB->storeFile(path.string() + ".spv", data);
}
