#include "resourceProvider_block.h"

#include <include/wrapper/cef_stream_resource_handler.h>
#include <iostream>

string DumpRequestContents(CefRefPtr<CefRequest> request) {
    std::stringstream ss;

    ss << "<h1>Blocked!</h1>";
    std::cout << "blocked " << std::string(request->GetURL()) << std::endl;
    ss << "\nURL: " << std::string(request->GetURL());
    ss << "\nMethod: " << std::string(request->GetMethod());

    CefRequest::HeaderMap headerMap;
    request->GetHeaderMap(headerMap);
    if (headerMap.size() > 0) {
        ss << "\nHeaders:";
        CefRequest::HeaderMap::const_iterator it = headerMap.begin();
        for (; it != headerMap.end(); ++it) {
            ss << "\n\t" << std::string((*it).first) << ": "
               << std::string((*it).second);
        }
    }

    CefRefPtr<CefPostData> postData = request->GetPostData();
    if (postData.get()) {
        CefPostData::ElementVector elements;
        postData->GetElements(elements);
        if (elements.size() > 0) {
            ss << "\nPost Data:";
            CefRefPtr<CefPostDataElement> element;
            CefPostData::ElementVector::const_iterator it = elements.begin();
            for (; it != elements.end(); ++it) {
                element = (*it);
                if (element->GetType() == PDE_TYPE_BYTES) {
                    // the element is composed of bytes
                    ss << "\n\tBytes: ";
                    if (element->GetBytesCount() == 0) {
                        ss << "(empty)";
                    } else {
                        // retrieve the data.
                        size_t size = element->GetBytesCount();
                        char *bytes = new char[size];
                        element->GetBytes(size, bytes);
                        ss << std::string(bytes, size);
                        delete[] bytes;
                    }
                } else if (element->GetType() == PDE_TYPE_FILE) {
                    ss << "\n\tFile: " << std::string(element->GetFile());
                }
            }
        }
    }

    return ss.str();
}

bool ResourceProvider_block::OnRequest(
    scoped_refptr<CefResourceManager::Request> request) {
    CEF_REQUIRE_IO_THREAD();

    const std::string &url = request->url();
    if (boost::regex_match(url, whitelist)) {
        // no blocking
        return false;
    }

    const std::string &dump = DumpRequestContents(request->request());
    std::string str =
        "<html><body bgcolor=\"white\"><pre>" + dump + "</pre></body></html>";
    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
        static_cast<void *>(const_cast<char *>(str.c_str())), str.size());
    DCHECK(stream.get());
    request->Continue(new CefStreamResourceHandler("text/html", stream));
    return true;
}