#pragma once

#include <include/cef_browser.h>
#include <include/cef_render_handler.h>

class RenderHandler : public CefRenderHandler {
  public:
    RenderHandler(int width, int height) : width(width), height(height) { ; }

    void resize(int width, int height) {
        this->width = width;
        this->height = height;
    }

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override {
        rect = CefRect(0, 0, width, height);
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                 const RectList &dirtyRects, const void *buffer, int width,
                 int height) override;

  public:
    IMPLEMENT_REFCOUNTING(RenderHandler);

  private:
    int width;
    int height;
};