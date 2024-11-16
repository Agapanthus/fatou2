
#pragma once

class ShaderCache {
  public:
    ShaderCache();

    //   optional<vector<uint32_t>> getCompiled(const string &path);
    optional<vector<uint8_t>> getCompiled(const path &path);
    string getSource(const path &path);
    void store(const path &path, const vector<uint32_t> &data);
    void store(const path &path, const vector<uint8_t> &data);
};

extern ShaderCache cache;