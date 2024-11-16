#include "resourceProvider_local.h"
#include <iostream>

#include <include/wrapper/cef_resource_manager.h>
#include <include/wrapper/cef_stream_resource_handler.h>

#include "../../window/console.h"
#include "../../shaderc/database.h"

CefRefPtr<CefStreamReader> GetResourceReader(const std::string &resource_path) {
    std::cout << "Reading resource " << resource_path << std::endl;

    // TODO: sanitize path!
    size_t pos = std::min(resource_path.find("?"), resource_path.length());
    string fp = resource_path.substr(0, pos);
    if (fp[0] == '/')
        fp = fp.substr(1, fp.length());
    std::replace(fp.begin(), fp.end(), '\\', '/');

    std::cout << "Loading " << fp << std::endl;

    // vector<unsigned char> buffer = readFile2<unsigned char>(fp);

    auto buffer = fatouDB->getFile(fp.c_str());

    // TODO: Free?
    unsigned char *buf = new unsigned char[buffer.size()];
    memcpy(buf, buffer.data(), buffer.size());
    return CefStreamReader::CreateForHandler(new CefByteReadHandler(
        (const unsigned char *)buf, buffer.size(), nullptr));
}

ResourceProvider_local::ResourceProvider_local(const string &root_url)
    : root_url_(root_url) {
    DCHECK(!root_url.empty());

    createFatouDB();
}

bool ResourceProvider_local::OnRequest(
    scoped_refptr<CefResourceManager::Request> request) {
    CEF_REQUIRE_IO_THREAD();

    const std::string &url = request->url();
    if (url.rfind(root_url_, 0) != 0L) {
        std::cout << "Passing " << url << std::endl;
        // Not handled by this provider.
        return false;
    }

    CefRefPtr<CefResourceHandler> handler;

    const std::string &relative_path = url.substr(root_url_.length());
    if (!relative_path.empty()) {

        CefRefPtr<CefStreamReader> stream =
            GetResourceReader(relative_path.data());
        if (stream.get()) {
            handler = new CefStreamResourceHandler(
                request->mime_type_resolver().Run(url), stream);
        }
    }

    request->Continue(handler);
    return true;
}
