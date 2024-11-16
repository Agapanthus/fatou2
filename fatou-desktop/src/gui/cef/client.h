#pragma once

#include <include/cef_client.h>
#include <include/cef_display_handler.h>
#include <include/cef_request_handler.h>
#include "renderHandler.h"
#include <include/cef_resource_request_handler.h>
#include <include/wrapper/cef_resource_manager.h>

#include <iostream>

// for manual render handler, OGRE
class BrowserClient : public CefClient,
                      public CefDisplayHandler,
                      public CefRequestHandler,
                      public CefResourceRequestHandler {
  public:
    BrowserClient(RenderHandler *renderHandler);

    // CefClient methods:
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return m_renderHandler;
    }

    void OnTitleChange(CefRefPtr<CefBrowser> browser,
                       const CefString &title) override {
        CEF_REQUIRE_UI_THREAD();

        std::cout << "Title changed " << title << std::endl;
    }

    CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, bool is_navigation, bool is_download,
        const CefString &request_initiator,
        bool &disable_default_handling) override {
        CEF_REQUIRE_IO_THREAD();
        return this;
    }

    cef_return_value_t
    OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefRequest> request,
                         CefRefPtr<CefCallback> callback) override {
        CEF_REQUIRE_IO_THREAD();

        return resource_manager_->OnBeforeResourceLoad(browser, frame, request,
                                                       callback);
    }

    CefRefPtr<CefResourceHandler>
    GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       CefRefPtr<CefRequest> request) override {
        CEF_REQUIRE_IO_THREAD();

        return resource_manager_->GetResourceHandler(browser, frame, request);
    }

    bool OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor,
                        cef_cursor_type_t type,
                        const CefCursorInfo &custom_cursor_info) override;

    bool
    OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefProcessId source_process,
                             CefRefPtr<CefProcessMessage> message) override;

    /*
        static void sendMessage(CefRefPtr<CefBrowser> browser) {
              // Create the message object.
              CefRefPtr<CefProcessMessage> msg =
                  CefProcessMessage::Create("my_message");

              // Retrieve the argument list object.
              CefRefPtr<CefListValue> args = msg->GetArgumentList();

              // Populate the argument values.
              args->SetString(0, "my string");
              args->SetInt(0, 10);

              // Send the process message to the main frame in the render
       process.
              // Use PID_BROWSER instead when sending a message to the browser
              // process.
              browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);

        }*/

  private:
    CefRefPtr<CefRenderHandler> m_renderHandler;
    CefRefPtr<CefResourceManager> resource_manager_;

    IMPLEMENT_REFCOUNTING(BrowserClient);
    DISALLOW_COPY_AND_ASSIGN(BrowserClient);
};
