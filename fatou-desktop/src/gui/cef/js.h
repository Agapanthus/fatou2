
#pragma once

#include <map>

#ifdef WITH_GUI
#include <include/cef_client.h>
extern CefRefPtr<CefBrowser> mainBrowser;

void sendJS(const string &js);
string jsStr(const string &inp);

#else

inline void sendJS(const string &js) {}
inline string jsStr(const string &inp) { return ""; }

#endif

inline string jsStr(size_t value) { return to_string(value); }
inline string jsStrD(double value) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(18) << value;
    return stream.str();
}
inline string jsStr(const std::map<string, string> &values) {
    string res = "{";
    for (const auto value : values) {
        res += jsStr(value.first) + ":" + value.second + ",";
    }
    return res + "}";
}

inline void simpleJS(const string &key, const string &def, const string &op) {
    sendJS("window.app = window.app || {};window.app[\"" + key +
           "\"] = window.app[\"" + key + "\"]||" + def + ";window.app[\"" +
           key + "\"]" + op + ";");
}

inline void simpleJSSet(const string &key, const string &def,
                        const string &value) {
    simpleJS(key, def, "=" + value);
}

inline void simpleJSAppend(const string &key, const string &value) {
    simpleJS(key, "[]", ".push(" + value + ")");
}

inline void setJS(const string &key, size_t value) {
    simpleJSSet(key, "0", jsStr(value));
}

inline void setJS(const string &key, const string &value) {
    simpleJS(key, "0", jsStr(value));
}

inline void appendJS(const string &key, const string &value) {
    simpleJSAppend(key, jsStr(value));
}

inline void appendJS(const string &key, const std::map<string, string> &value) {
    simpleJSAppend(key, jsStr(value));
}

inline void commitJS(const string &key, const string &value) {
    sendJS("window.app.store.commit(\"" + key + "\", " + value + ");");
}

inline void commitJS(const string &key, const std::map<string, string> &value) {
    commitJS(key, jsStr(value));
}
