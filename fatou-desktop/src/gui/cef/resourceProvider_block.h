#pragma once

#include <include/cef_resource_handler.h>
#include <include/wrapper/cef_resource_manager.h>

#include <boost/regex.hpp>

// Demonstrate a custom Provider implementation by dumping the request
class ResourceProvider_block : public CefResourceManager::Provider {
  public:
    explicit ResourceProvider_block(const boost::regex &whitelist)
        : whitelist(whitelist) {
        DCHECK(!whitelist.empty());
    }

    bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override;

  private:
    const boost::regex whitelist;

    DISALLOW_COPY_AND_ASSIGN(ResourceProvider_block);
};