#pragma once

#include <include/cef_stream.h>
#include <include/wrapper/cef_byte_read_handler.h>
#include <include/wrapper/cef_resource_manager.h>

// Provider implementation for loading BINARY resources from the current
// executable.
class ResourceProvider_local : public CefResourceManager::Provider {
  public:
    explicit ResourceProvider_local(const string &root_url);

    bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override;

  private:
    string root_url_;

    DISALLOW_COPY_AND_ASSIGN(ResourceProvider_local);
};
