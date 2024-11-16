
#pragma once

#include <include/cef_v8.h>

//
// helper function to convert a
// CefDictionaryValue to a CefV8Object
//
CefRefPtr<CefV8Value>
to_v8object(CefRefPtr<CefDictionaryValue> const &dictionary);

//
// V8 handler for our 'mixer' object available to javascript
// running in a page within this application
//
class MixerHandler : public CefV8Accessor {
  public:
    MixerHandler(CefRefPtr<CefBrowser> const &browser,
                 CefRefPtr<CefV8Context> const &context)
        : browser_(browser), context_(context) {
        auto window = context->GetGlobal();
        auto const obj = CefV8Value::CreateObject(this, nullptr);
        obj->SetValue("requestStats", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
        window->SetValue("mixer", obj, V8_PROPERTY_ATTRIBUTE_NONE);
    }

    void update(CefRefPtr<CefDictionaryValue> const &dictionary) {
        if (!request_stats_) {
            return;
        }

        context_->Enter();
        CefV8ValueList values;
        values.push_back(to_v8object(dictionary));
        request_stats_->ExecuteFunction(request_stats_, values);
        context_->Exit();
    }

    bool Get(const CefString &name, const CefRefPtr<CefV8Value> object,
             CefRefPtr<CefV8Value> &retval,
             CefString & /*exception*/) override {

        if (name == "requestStats" && request_stats_ != nullptr) {
            retval = request_stats_;
            return true;
        }

        // Value does not exist.
        return false;
    }

    bool Set(const CefString &name, const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString & /*exception*/) override {
        if (name == "requestStats") {
            request_stats_ = value;

            // notify the browser process that we want stats
            auto message = CefProcessMessage::Create("mixer-request-stats");
            if (message != nullptr && browser_ != nullptr) {
                throw runtime_error("here!");
                // TODO: enable this again!
                // browser_->SendProcessMessage(PID_BROWSER, message);
            }
            return true;
        }
        return false;
    }

    IMPLEMENT_REFCOUNTING(MixerHandler);

  private:
    CefRefPtr<CefBrowser> const browser_;
    CefRefPtr<CefV8Context> const context_;
    CefRefPtr<CefV8Value> request_stats_;
};
