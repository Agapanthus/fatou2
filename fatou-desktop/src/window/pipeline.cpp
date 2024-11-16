#include "pipeline.h"
#include <iostream>
#include "buffer.h"

void PipelineBase::createPipeline(vk::ImageLayout finalLayout,
                                  vk::ImageLayout initialLayout,
                                  vector<vk::DynamicState> dynamicStates) {
    assert(bool(finalLayout));

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vert->getInfo(),
                                                        frag->getInfo()};

    createRenderPass(finalLayout, initialLayout);

    // The vk::PipelineVertexInputStateCreateInfo structure describes the format
    // of the vertex data that will be passed to the vertex shader. It describes
    // this in roughly two ways:
    //    -  Bindings: spacing between data and whether the data is per-vertex
    //    or per-instance (see instancing)
    //    -  Attribute descriptions: type of the attributes passed to the vertex
    //    shader, which binding to load them from and at which offset
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        vk::StructureType::ePipelineVertexInputStateCreateInfo;

    vk::VertexInputBindingDescription bindingDescription =
        Vertex2::getBindingDescription();
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions =
        Vertex2::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    ///////////////

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    setupInputAssembly(inputAssembly);
    vk::Rect2D scissor;
    vk::Viewport viewport;
    vk::PipelineViewportStateCreateInfo viewportState;
    setupViewport(extent, scissor, viewport, viewportState);
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    setupRasterizer(rasterizer);
    vk::PipelineMultisampleStateCreateInfo multisampling;
    setupMultisampling(multisampling);
    vk::PipelineDepthStencilStateCreateInfo stencil;
    setupStencil(stencil);
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo colorBlending;
    setupBlending(colorBlendAttachment, colorBlending);

    // A limited amount of the state that we've specified in the previous
    // structs can actually be changed without recreating the pipeline. Examples
    // are the size of the viewport, line width and blend constants.
    // vector<vk::DynamicState> dynamicStates = {//vk::DynamicState::eViewport,
    //                                          vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    // This will cause the configuration of these values to be ignored and you
    // will be required to specify the data at drawing time.

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState =
        stencilMode == StencilMode::eNone ? nullptr : &stencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = 0;

    // Vulkan allows you to create a new graphics pipeline by deriving from an
    // existing pipeline. The idea of pipeline derivatives is that it is less
    // expensive to set up pipelines when they have much functionality in common
    // with an existing pipeline and switching between pipelines from the same
    // parent can also be done quicker. You can either specify the handle of an
    // existing pipeline with basePipelineHandle or reference another pipeline
    // that is about to be created by index with basePipelineIndex. Right now
    // there is only a single pipeline, so we'll simply specify a null handle
    // and an invalid index. These values are only used if the
    // VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags
    // field of vk::GraphicsPipelineCreateInfo.
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    // It is designed to take multiple vk::GraphicsPipelineCreateInfo objects
    // and create multiple vk::Pipeline objects in a single call. The second
    // parameter, for which we've passed the VK_NULL_HANDLE argument, references
    // an optional vk::PipelineCache object. A pipeline cache can be used to
    // store and reuse data relevant to pipeline creation across multiple calls
    // to vkCreateGraphicsPipelines and even across program executions if the
    // cache is stored to a file. This makes it possible to significantly speed
    // up pipeline creation at a later time.
    graphicsPipeline =
        device->device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);
}

void PipelineBase::createRenderPass(vk::ImageLayout finalLayout,
                                    vk::ImageLayout initialLayout) {
    assert(bool(finalLayout));
    vector<vk::AttachmentDescription> attachments;
    // In our case we'll have just a single color buffer attachment represented
    // by one of the images from the swap chain.

    // The format of the color attachment should match the format of the swap
    // chain images, and we're not doing anything with multisampling yet, so
    // we'll stick to 1 sample.
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;

    // The loadOp and storeOp determine what to do with the data in the
    // attachment before rendering and after rendering.
    // - VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the
    //   attachment
    // - VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the
    //   start
    // - VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we
    //   don't care about them
    // when initial layout is undefined, clear contents; otherwise: keep
    // contents.
    colorAttachment.loadOp = (initialLayout == vk::ImageLayout::eUndefined)
                                 ? vk::AttachmentLoadOp::eClear
                                 : vk::AttachmentLoadOp::eLoad;

    // - VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in
    // memory and can be read later
    // - VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be
    // undefined after the rendering operation
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

    // we don't use stencil data
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

    // Some of the most common layouts are:
    // - vk::ImageLayout::_COLOR_ATTACHMENT_OPTIMAL: Images used as color
    // attachment
    // - vk::ImageLayout::_PRESENT_SRC_KHR: Images to be presented in the swap
    // chain
    // - vk::ImageLayout::eTransferDstOptimal: Images to be used as destination
    // for a memory copy operation
    // - vk::ImageLayout::_UNDEFINED: we don't care what previous layout the
    // image was in. The caveat of this special value is that the contents of
    // the image are not guaranteed to be preserved, but that doesn't matter
    // since we're going to clear it anyway
    colorAttachment.initialLayout = initialLayout;
    colorAttachment.finalLayout = finalLayout;

    // A single render pass can consist of multiple subpasses. Subpasses are
    // subsequent rendering operations that depend on the contents of
    // framebuffers in previous passes, for example a sequence of
    // post-processing effects that are applied one after another. If you group
    // these rendering operations into one render pass, then Vulkan is able to
    // reorder the operations and conserve memory bandwidth for possibly better
    // performance. For our very first triangle, however, we'll stick to a
    // single subpass.
    vk::AttachmentReference colorAttachmentRef{};

    // the attachment parameter specifies which attachment to reference by its
    // index in the attachment descriptions array. Our array consists of a
    // single vk::AttachmentDescription, so its index is 0.
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    // Remember that the subpasses in a render pass automatically take care of
    // image layout transitions. These transitions are controlled by subpass
    // dependencies, which specify memory and execution dependencies between
    // subpasses. We have only a single subpass right now, but the operations
    // right before and right after this subpass also count as implicit
    // "subpasses".
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    // There are two built-in dependencies that take care of the transition at
    // the start of the render pass and at the end of the render pass, but the
    // former does not occur at the right time. It assumes that the transition
    // occurs at the start of the pipeline, but we haven't acquired the image
    // yet at that point! There are two ways to deal with this problem. We could
    // change the waitStages for the imageAvailableSemaphore to
    // vk::PipelineStageFlagBits::eTopOfPipe to ensure that the render passes
    // don't begin until the image is available, or we can make the render pass
    // wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.
    vk::SubpassDependency dependency{};

    // The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass
    // before or after the render pass depending on whether it is specified in
    // srcSubpass or dstSubpass. The index 0 refers to our subpass, which is the
    // first and only one. The dstSubpass must always be higher than srcSubpass
    // to prevent cycles in the dependency graph (unless one of the subpasses is
    // VK_SUBPASS_EXTERNAL).
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    // The next two fields specify the operations to wait on and the stages in
    // which these operations occur. We need to wait for the swap chain to
    // finish reading from the image before we can access it. This can be
    // accomplished by waiting on the color attachment output stage itself.
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = {};

    // The operations that should wait on this are in the color attachment stage
    // and involve the writing of the color attachment. These settings will
    // prevent the transition from happening until it's actually necessary (and
    // allowed): when we want to start writing colors to it.
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    attachments.push_back(colorAttachment);

    //////////////////////////////////

    vk::AttachmentDescription stencilAttachment{};
    vk::AttachmentReference stencilAttachmentRef{};

    if (stencilMode != StencilMode::eNone) {
        stencilAttachment.flags = {};

        stencilAttachment.format = depthFormat;
        stencilAttachment.samples = vk::SampleCountFlagBits::e1;

        if (stencilMode == StencilMode::eRead) {
            stencilAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
            stencilAttachment.storeOp = vk::AttachmentStoreOp::eNone;
            stencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eLoad;
            stencilAttachment.stencilStoreOp = vk::AttachmentStoreOp::eNone;
            stencilAttachment.initialLayout = vk::ImageLayout::
                eDepthStencilAttachmentOptimal; // TODO: Change to
                                                // eDepthStencilReadOnlyOptimal?
            stencilAttachment.finalLayout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

            stencilAttachmentRef.attachment = 1;
            stencilAttachmentRef.layout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

        } else if (stencilMode == StencilMode::eWrite) {
            stencilAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            stencilAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            stencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
            stencilAttachment.stencilStoreOp =
                vk::AttachmentStoreOp::eStore; // TODO: is eDontCare also fine
                                               // here?
            stencilAttachment.initialLayout = vk::ImageLayout::eUndefined;
            stencilAttachment.finalLayout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

            stencilAttachmentRef.attachment = 1;
            stencilAttachmentRef.layout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

        } else if (stencilMode == StencilMode::eIgnore) {
            stencilAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
            stencilAttachment.storeOp = vk::AttachmentStoreOp::eNone;
            stencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eLoad;
            stencilAttachment.stencilStoreOp = vk::AttachmentStoreOp::eNone;
            stencilAttachment.initialLayout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;
            stencilAttachment.finalLayout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

            stencilAttachmentRef.attachment = 1;
            stencilAttachmentRef.layout =
                vk::ImageLayout::eDepthStencilAttachmentOptimal;

        } else {
            assert(false);
        }

        attachments.push_back(stencilAttachment);
    }

    //////////////////////////////////

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    // The index of the attachment in this array is directly referenced from the
    // fragment shader with the layout(location = 0) out vec4 outColor
    // directive!
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    subpass.pDepthStencilAttachment = nullptr;
    if (stencilMode != StencilMode::eNone)
        subpass.pDepthStencilAttachment = &stencilAttachmentRef;

    // The following other types of attachments can be referenced by a subpass:
    // - pInputAttachments: Attachments that are read from a shader
    // - pResolveAttachments: Attachments used for multisampling color
    // attachments
    // - pDepthStencilAttachment: Attachment for depth and stencil data
    // - pPreserveAttachments: Attachments that are not used by this subpass,
    // but for which the data must be preserved
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    renderPass = device->device.createRenderPass(renderPassInfo);
}

void PipelineBase::createPipelineLayout(const vk::DescriptorSetLayout *myDsl) {
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = myDsl;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    pipelineLayout = device->device.createPipelineLayout(pipelineLayoutInfo);
}

void PipelineBase::setupInputAssembly(
    vk::PipelineInputAssemblyStateCreateInfo &inputAssembly) {
    inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{};
    inputAssembly.sType =
        vk::StructureType::ePipelineInputAssemblyStateCreateInfo;

    // topology:
    // - VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
    // - VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without
    // reuse
    // - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used
    // as start vertex for the next line
    // - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices
    // without reuse
    // - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of
    // every triangle are used as first two vertices of the next triangle
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    // If you set the primitiveRestartEnable member to VK_TRUE, then it's
    // possible to break up lines and triangles in the _STRIP topology modes by
    // using a special index of 0xFFFF or 0xFFFFFFFF
    inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBase::setupViewport(
    const Extent2D &swapChainExtent, vk::Rect2D &scissor,
    vk::Viewport &viewport,
    vk::PipelineViewportStateCreateInfo &viewportState) {

    // A viewport basically describes the region of the framebuffer that the
    // output will be rendered to.
    viewport = vk::Viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor ~ Mask for the framebuffer,
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    scissor = vk::Rect2D{};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapChainExtent;

    // It's also possible to use multiple viewports and scissor rectangles on
    // some graphics cards
    viewportState = vk::PipelineViewportStateCreateInfo{};
    viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
}

void PipelineBase::setupStencil(
    vk::PipelineDepthStencilStateCreateInfo &depthStencil) {
    depthStencil.sType =
        vk::StructureType::ePipelineDepthStencilStateCreateInfo;

    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    // The depthCompareOp field specifies the comparison that is performed to
    // keep or discard fragments. We're sticking to the convention of lower
    // depth = closer, so the depth of new fragments should be less.
    depthStencil.depthCompareOp = vk::CompareOp::eLess;

    // The depthBoundsTestEnable, minDepthBounds and maxDepthBounds fields are
    // used for the optional depth bound test. Basically, this allows you to
    // only keep fragments that fall within the specified depth range. We won't
    // be using this functionality.
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    // The last three fields configure stencil buffer operations. If you want to
    // use these operations, then you will have to make sure that the format of
    // the depth/stencil image contains a stencil component.
    depthStencil.stencilTestEnable = VK_TRUE;

    if (stencilMode == StencilMode::eWrite) {
        // write everywhere
        depthStencil.back.compareOp = vk::CompareOp::eAlways;
        depthStencil.back.failOp = vk::StencilOp::eReplace;
        depthStencil.back.depthFailOp = vk::StencilOp::eReplace;
        depthStencil.back.passOp = vk::StencilOp::eReplace;
    } else if (stencilMode == StencilMode::eRead) {
        // write where 1
        depthStencil.back.compareOp = vk::CompareOp::eEqual;
        depthStencil.back.failOp = vk::StencilOp::eKeep;
        depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
        depthStencil.back.passOp = vk::StencilOp::eReplace;
    } else if (stencilMode == StencilMode::eIgnore) {
        // write where 1
        depthStencil.back.compareOp = vk::CompareOp::eAlways;
        depthStencil.back.failOp = vk::StencilOp::eKeep;
        depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
        depthStencil.back.passOp = vk::StencilOp::eKeep;
    }

    depthStencil.back.reference = 1; // compare with this one
    depthStencil.back.compareMask =
        0xff; // AND this mask to both reference and compared value
    depthStencil.back.writeMask = 0xff;
    depthStencil.front = depthStencil.back;
}

void PipelineBase::setupRasterizer(
    vk::PipelineRasterizationStateCreateInfo &rasterizer) {
    // The rasterizer takes the geometry that is shaped by the vertices from the
    // vertex shader and turns it into fragments to be colored by the fragment
    // shader. It also performs depth testing, face culling and the scissor
    // test, and it can be configured to output fragments that fill entire
    // polygons or just the edges (wireframe rendering).
    rasterizer = vk::PipelineRasterizationStateCreateInfo{};
    rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;

    // If depthClampEnable is set to VK_TRUE, then fragments that are beyond the
    // near and far planes are clamped to them as opposed to discarding them.
    // This is useful in some special cases like shadow maps. Using this
    // requires enabling a GPU feature
    rasterizer.depthClampEnable = VK_FALSE;

    // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes
    // through the rasterizer stage. This basically disables any output to the
    // framebuffer.
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    // The polygonMode determines how fragments are generated for geometry. The
    // following modes are available:
    // - VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
    // - VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
    // - VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
    // Using any mode other than fill requires enabling a GPU feature.
    rasterizer.polygonMode = vk::PolygonMode::eFill;

    // The lineWidth member is straightforward, it describes the thickness of
    // lines in terms of number of fragments. The maximum line width that is
    // supported depends on the hardware and any line thicker than 1.0f requires
    // you to enable the wideLines GPU feature.
    rasterizer.lineWidth = 1.0f;

    // The cullMode variable determines the type of face culling to use. You can
    // disable culling, cull the front faces, cull the back faces or both. The
    // frontFace variable specifies the vertex order for faces to be considered
    // front-facing and can be clockwise or counterclockwise.
    rasterizer.cullMode = vk::CullModeFlagBits::eNone; // VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

    // The rasterizer can alter the depth values by adding a constant value or
    // biasing them based on a fragment's slope. This is sometimes used for
    // shadow mapping
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional
}

void PipelineBase::setupMultisampling(
    vk::PipelineMultisampleStateCreateInfo &multisampling) {
    // MSAA. Enabling it requires enabling a GPU feature.
    multisampling = vk::PipelineMultisampleStateCreateInfo{};
    multisampling.sType =
        vk::StructureType::ePipelineMultisampleStateCreateInfo;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional
}

void PipelineBase::setupBlending(
    vk::PipelineColorBlendAttachmentState &colorBlendAttachment,
    vk::PipelineColorBlendStateCreateInfo &colorBlending) {

    // After a fragment shader has returned a color, it needs to be combined
    // with the color that is already in the framebuffer. This transformation is
    // known as color blending and there are two ways to do it:
    // - Mix the old and new value to produce a final color
    // - Combine the old and new value using a bitwise operation

    // contains the configuration per attached framebuffer
    /** Basically does the following:
     * if (blendEnable) {
     *     - finalColor.rgb = (srcColorBlendFactor * newColor.rgb)
     *       <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
     *     - finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp>
     *       (dstAlphaBlendFactor * oldColor.a);
     * } else {
     *     - finalColor = newColor;
     * }
     * finalColor = finalColor & colorWriteMask;
     */
    // The settings below are for alpha blending, i.e.
    /**
     * - finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
     *  - finalColor.a = newAlpha.a;
     */
    colorBlendAttachment = vk::PipelineColorBlendAttachmentState{};
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = hasBlend ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor =
        vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    // The second structure references the array of structures for all of the
    // framebuffers and allows you to set blend constants that you can use as
    // blend factors in the aforementioned calculations.

    // set logicOpEnable to VK_TRUE. The bitwise operation can then be specified
    // in the logicOp field. Note that this will automatically disable the first
    // method, as if you had set blendEnable to VK_FALSE for every attached
    // framebuffer! The colorWriteMask will also be used in this mode to
    // determine which channels in the framebuffer will actually be affected. It
    // is also possible to disable both modes, in which case the fragment colors
    // will be written to the framebuffer unmodified.
    colorBlending = vk::PipelineColorBlendStateCreateInfo{};
    colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
}
