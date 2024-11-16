#pragma once

#include "texture.h"
#include "swapChain.h"
#include "compositor.h"
#include "renderPass.h"
#include "interlacedRenderer.h"

#include "sharedTexture.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

template <class DSL> class Fractal {
  protected:
    Fractal(shared_ptr<LogicalDevice> device, const path &shaderPath,
            Extent2D extent, shared_ptr<CommandPool> commandPool, size_t phases)
        : device(device), extent(extent) {
        renderer = make_shared<InterlacedRenderer<DSL>>(
            device, shaderPath,
            Extent2D(superSampling * extent.width,
                     superSampling * extent.height),
            commandPool, phases);
    }

  public:
    void makeDP(Compositor &compositor) { renderer->makeDP(compositor); }
    void present(vk::CommandBuffer commandBuffer, Compositor &compositor,
                 IRect r) {
        renderer->present(commandBuffer, compositor, r);
    }
    Extent2D getExtent() const { return extent; }

    void renderStep(const CommandBufferRecorder &rec,
                    vk::CommandBuffer commandBuffer, size_t bufferIndex) {

        if (!presetLoader.empty()) {

            iGamma = .7;
            play = 0.;
            shift = 0.;
            contrast = 3.0;
            phase = 1.;
            radius = 4.0;
            smoothing = 1.0;

            const string preset = presetLoader.dequeue();
            try {
                std::stringstream ss(preset);
                boost::property_tree::ptree pt;
                boost::property_tree::read_json(ss, pt);
                using boost::property_tree::ptree;
                ptree::const_iterator end = pt.end();
                for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
                    std::cout << it->first << ": "
                              << it->second.get_value<std::string>()
                              << std::endl;

                    if (it->first == "x") {
                        navi.x = it->second.get_value<double>();
                    }
                    if (it->first == "y") {
                        navi.y = it->second.get_value<double>();
                    }
                    if (it->first == "zoom") {
                        navi.z = it->second.get_value<double>();
                    }
                    if (it->first == "iterations") {
                        maxiter = it->second.get_value<int>();
                    }
                    if (it->first == "iGamma") {
                        iGamma = it->second.get_value<double>();
                    }
                    if (it->first == "play") {
                        play = it->second.get_value<double>();
                    }
                    if (it->first == "shift") {
                        shift = it->second.get_value<double>();
                    }
                    if (it->first == "contrast") {
                        contrast = it->second.get_value<double>();
                    }
                    if (it->first == "phase") {
                        phase = it->second.get_value<double>();
                    }
                    if (it->first == "radius") {
                        radius = it->second.get_value<double>();
                    }
                    if (it->first == "smoothing") {
                        smoothing = it->second.get_value<double>();
                    }

                    //   print(it->second);
                }

            } catch (std::exception const &e) {
                std::cerr << e.what() << std::endl;
            }

            renderer->hardInvalidate();
        }

        UniformBufferObject2 ubo2{};
        double ax, ay;
        navi.getAdjustedPos(ax, ay);
        ubo2.pos.x = ax;
        ubo2.pos.y = ay;
        ubo2.zoom = navi.z;

        parametersChanged |= ax != axo;
        parametersChanged |= ay != ayo;
        parametersChanged |= navi.z != az;
        axo = ax;
        ayo = ay;
        az = navi.z;

        ubo2.iter = maxiter;
        ubo2.iGamma = iGamma;
        ubo2.play = play;
        ubo2.shift = shift;
        ubo2.contrast = contrast;
        ubo2.phase = phase;
        ubo2.radius = radius;
        ubo2.smoothing = smoothing;

        if (parametersChanged) {
            renderer->invalidate();
            parametersChanged = false;
        }

        renderer->renderStep(rec, commandBuffer, ubo2, bufferIndex);
    }

    const Extent2D extent;

  protected:
    shared_ptr<LogicalDevice> device;
    shared_ptr<CommandPool> commandPool;
    shared_ptr<InterlacedRenderer<DSL>> renderer;

    const int superSampling = 2;

    bool parametersChanged = true;
    double axo, ayo, az;

    int maxiter = 100;

    float iGamma = .7;
    float play = .0;
    float shift = 0;
    float contrast = 3.0;
    float phase = 1.0;
    float radius = 4.0;
    float smoothing = 1.0;
};

class Fractal_Mandel : public Fractal<MandelDescriptorSetLayout> {
  public:
    Fractal_Mandel(shared_ptr<LogicalDevice> device, Extent2D e,
                   shared_ptr<CommandPool> commandPool, size_t phases)
        : Fractal(device, shaderPath / "playground" / "mandeld.frag", e,
                  commandPool, phases) {}

  private:
};
