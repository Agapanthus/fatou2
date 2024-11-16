#include "renderHandler.h"

#include "../../window/sharedTexture.h"

#include "webp/encode.h"
#include <fstream>
#include <sstream>

int globalI = 0;

inline bool fileExists(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type, const RectList &dirtyRects,
                            const void *buffer, int width, int height) {
    /*Ogre::HardwarePixelBufferSharedPtr texBuf =
        m_renderTexture->getBuffer();
    texBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
    memcpy(texBuf->getCurrentLock().data, buffer, width * height * 4);
    texBuf->unlock();*/

    if (!(width == this->width && height == this->height)) {
        // TODO: Why does this sometimes happen?
        std::cout << "Incompatible size: " << width << "," << height << " vs. "
                  << this->width << "," << this->height << std::endl;
        return;
        // throw runtime_error("Incompatible image size!");
    }

    std::unique_lock lk(targetMutex);

    if (!targetTexture) {
        // throw runtime_error("Texture not defined!");
        std::cerr << "Texture not defined!" << std::endl;
        return;
    }

    // TODO: There is sometimes an access violation exception during startup
    // when vkcopy (or transition image layout?) is run from here! Fix this! Not
    // noticing this anymore

    // TODO: use a shared D3D11-Texture instead! VK_NV_external_memory allows
    // importing d3d11 memory to vulkan
    targetTexture->update((const uint8_t *)buffer);

    lk.unlock();

    /*
    const string path("./frames/");

    if (!fileExists("frames/file.webp")) {
        int stride = width * 4;
        uint8_t *output;
        size_t outSize = WebPEncodeBGRA((const uint8_t *)buffer, width,
                                        height, stride, 100, &output);
        std::cout << "OnPaint " << width << ", " << height << " " << outSize
                  << std::endl;

        globalI += 1;
        const string fname = "file"; // to_string(globalI);
        std::ofstream myfile =
            std::ofstream(path + fname + ".webp", std::fstream::binary);
        if (!myfile.good()) {
            throw runtime_error("couldn't write file");
        }
        myfile.write((const char *)output, outSize);
        myfile.close();

        delete[] output;
    }*/
}