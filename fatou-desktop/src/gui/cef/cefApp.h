#pragma once

#include "mixerHandler.h"
#include <include/cef_app.h>

class MyV8Handler : public CefV8Handler {
  public:
    MyV8Handler(CefRefPtr<CefBrowser> browser) : browser(browser) {}

    virtual bool Execute(const CefString &name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList &arguments,
                         CefRefPtr<CefV8Value> &retval,
                         CefString &exception) override {
        if (name == "browserReady") {

            CefRefPtr<CefProcessMessage> msg =
                CefProcessMessage::Create("guiReady");

            /*
                // Retrieve the argument list object.
                CefRefPtr<CefListValue> args = msg->GetArgumentList();

                // Populate the argument values.
                args->SetString(0, "my string");
                args->SetInt(0, 10);*/

            // Send the process message to the main frame in therender process.
            // Use PID_BROWSER instead when sending a message to the browser
            // process.
            browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);

            // Return my string value.
            retval = CefV8Value::CreateString("ok");
            return true;
        } else if (name == "renderAreaResize") {

            CefRefPtr<CefProcessMessage> msg =
                CefProcessMessage::Create("renderAreaResize");

            // Retrieve the argument list object.
            CefRefPtr<CefListValue> args = msg->GetArgumentList();

            // Populate the argument values.
            args->SetInt(0, arguments[0]->GetIntValue());
            args->SetInt(1, arguments[1]->GetIntValue());
            args->SetInt(2, arguments[2]->GetIntValue());
            args->SetInt(3, arguments[3]->GetIntValue());

            // Send the process message to the main frame in therender process.
            // Use PID_BROWSER instead when sending a message to the browser
            // process.
            browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);

            // Return my string value.
            retval = CefV8Value::CreateString("ok");
            return true;
        } else if (name == "loadPreset") {
            CefRefPtr<CefProcessMessage> msg =
                CefProcessMessage::Create("loadPreset");
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            const string strToSend =
                std::string(arguments[0]->GetStringValue());
            args->SetString(0, strToSend);
            browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
            retval = CefV8Value::CreateString("ok");
            return true;
        }

        // Function does not exist.
        return false;
    }

    // Provide the reference counting implementation for this class.
    IMPLEMENT_REFCOUNTING(MyV8Handler);

  private:
    CefRefPtr<CefBrowser> browser;
};

/*
inline void createFunction(CefRefPtr<CefV8Handler> handler,
                           const string &funName,
                           CefRefPtr<CefV8Context> context) {
    // Create the "myfunc" function.
    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(funName, handler);

    // add function
    CefRefPtr<CefV8Value> object = context->GetGlobal();
    object->SetValue(funName, func, V8_PROPERTY_ATTRIBUTE_NONE);
}*/

class MyCefApp : public CefApp,
                 public CefBrowserProcessHandler,
                 public CefRenderProcessHandler {
  public:
    MyCefApp() {}

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    void OnBeforeCommandLineProcessing(
        CefString const & /*process_type*/,
        CefRefPtr<CefCommandLine> command_line) override;

    virtual void OnContextInitialized() override {}

    // CefRenderProcessHandler::OnContextCreated
    //
    // Adds our custom 'mixer' object to the javascript context running
    // in the render process
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override {
        mixer_handler_ = new MixerHandler(browser, context);

        CefRefPtr<CefV8Handler> handler = new MyV8Handler(browser);

        // createFunction(handler, "browserReady", context);

        {
            const string funName = "browserReady";
            // Create the "myfunc" function.
            CefRefPtr<CefV8Value> func =
                CefV8Value::CreateFunction(funName, handler);

            // add function
            CefRefPtr<CefV8Value> object = context->GetGlobal();
            object->SetValue(funName, func, V8_PROPERTY_ATTRIBUTE_NONE);
        }

        {
            const string funName = "renderAreaResize";
            // Create the "myfunc" function.
            CefRefPtr<CefV8Value> func =
                CefV8Value::CreateFunction(funName, handler);

            // add function
            CefRefPtr<CefV8Value> object = context->GetGlobal();
            object->SetValue(funName, func, V8_PROPERTY_ATTRIBUTE_NONE);
        }

        {
            const string funName = "loadPreset";
            // Create the "myfunc" function.
            CefRefPtr<CefV8Value> func =
                CefV8Value::CreateFunction(funName, handler);

            // add function
            CefRefPtr<CefV8Value> object = context->GetGlobal();
            object->SetValue(funName, func, V8_PROPERTY_ATTRIBUTE_NONE);
        }
    }

    // CefRenderProcessHandler::OnBrowserDestroyed
    void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override {
        mixer_handler_ = nullptr;
    }

    // CefRenderProcessHandler::OnProcessMessageReceived
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefProcessId /*source_process*/,
                                  CefRefPtr<CefProcessMessage> message);

  private:
    IMPLEMENT_REFCOUNTING(MyCefApp);

    CefRefPtr<MixerHandler> mixer_handler_;
};
