#include "mixerHandler.h"

CefRefPtr<CefV8Value>
to_v8object(CefRefPtr<CefDictionaryValue> const &dictionary) {
    auto const obj = CefV8Value::CreateObject(nullptr, nullptr);
    if (dictionary) {
        auto const attrib = V8_PROPERTY_ATTRIBUTE_READONLY;
        CefDictionaryValue::KeyList keys;
        dictionary->GetKeys(keys);
        for (auto const &k : keys) {
            auto const type = dictionary->GetType(k);
            switch (type) {
            case VTYPE_BOOL:
                obj->SetValue(k, CefV8Value::CreateBool(dictionary->GetBool(k)),
                              attrib);
                break;
            case VTYPE_INT:
                obj->SetValue(k, CefV8Value::CreateInt(dictionary->GetInt(k)),
                              attrib);
                break;
            case VTYPE_DOUBLE:
                obj->SetValue(
                    k, CefV8Value::CreateDouble(dictionary->GetDouble(k)),
                    attrib);
                break;
            case VTYPE_STRING:
                obj->SetValue(
                    k, CefV8Value::CreateString(dictionary->GetString(k)),
                    attrib);
                break;

            default:
                break;
            }
        }
    }
    return obj;
}
