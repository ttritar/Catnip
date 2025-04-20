#include "Pipeline.h"
#include "Structs.h"

namespace cat
{
	Pipeline::Pipeline(const VkDevice& device,const std::string& vertPath, const std::string& fragPath, const VkRenderPass& renderPass, const VkExtent2D& swapChainExtent, const VkDescriptorSetLayout& descriptorSetLayout)
		:   m_VertPath(vertPath), m_FragPath(fragPath),
		m_Device(device), m_RenderPass(renderPass), m_SwapChainExtent(swapChainExtent), m_DescriptorSetLayout(descriptorSetLayout)
	{
		CreateGraphicsPipeline();
	}

    Pipeline::~Pipeline()
	{
		vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    }


    void Pipeline::CreateGraphicsPipeline()
    {
        auto vertShaderCode = ReadFile("shaders/shader.vert.spv");
        auto fragShaderCode = ReadFile("shaders/shader.frag.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);



        // Shader stage creation
        //-------------------

        //VERTEX
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;    //obligatory
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;    // tells vulkan in which pipeline stage the shader is going to be used

        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";   // the function to invoke = entry point (multip frag shaders can be combined into a single shader module and use diff entry point to differentiate between the behaviours)
        // pSpecializationInfo   ->>>>   It allows you to specify values for shader constants.(autom nullptr -> no constants)


        //FRAGMENT
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";


        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };


        // Fixed Functions
        //------------------

        // DYNAMIC STATE
        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // VERTEX INPUT
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; //optional   -> point to array of struct that describe the aforementioned details for loading vertex data
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> (attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); //optional     -> point to array of struct that describe the aforementioned details for loading vertex data

        // INPUT ASSEMBLY
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // VIEWPORT AND SCISSORS
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_SwapChainExtent.width;
        viewport.height = (float)m_SwapChainExtent.height;
        viewport.minDepth = 0.0f; // specify rrange of depth values for the framebuffer
        viewport.maxDepth = 1.0;

        VkRect2D scissor{};
        scissor.offset = { 0,0 };
        scissor.extent = m_SwapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // RASTERIZER
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;     // clampsm fragments outside of near and far plane, instead of discarding them

        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; //optional
        rasterizer.depthBiasClamp = 0.0f; //optional
        rasterizer.depthBiasSlopeFactor = 0.0f; //optinal

        // MULTISAMPLING   -> disabled
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; //optional
        multisampling.pSampleMask = nullptr; //optional
        multisampling.alphaToCoverageEnable = VK_FALSE; //optional
        multisampling.alphaToOneEnable = VK_FALSE; //optional

        // DEPTH AND STENCIL TESTING
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE; // specifies if the depth of new fragments should be compared to the depth buffer to see if they should be discarded
        depthStencil.depthWriteEnable = VK_TRUE; // specifies if the new depth of fragments that pass the depth test should actually be written to the depth buffer
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // specifies the comparison to use for the depth test
        depthStencil.depthBoundsTestEnable = VK_FALSE; //optional
        depthStencil.minDepthBounds = 0.0f; //optional
        depthStencil.maxDepthBounds = 1.0f; //optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; //optional
        depthStencil.back = {}; //optional

        // COLOR BLENDING
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        // no blending
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; //optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //optional

        // alpha blending
    //colorBlendAttachment.blendEnable = VK_TRUE;
    //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    //colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    //colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    //colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; //optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; //optional
        colorBlending.blendConstants[1] = 0.0f; //optional
        colorBlending.blendConstants[2] = 0.0f; //optional
        colorBlending.blendConstants[3] = 0.0f; //optional


        // PIPELINE LAYOUT
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; //optional
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout; //optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; //optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; //optional

        if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }



        // Creating the Graphics Pipeline (Conclusion)
        //-------------------
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.layout = m_PipelineLayout;

        pipelineInfo.renderPass = m_RenderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.renderPass = m_RenderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; //optional
        pipelineInfo.basePipelineIndex = -1; //optional

        if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }


        // Module deletion
        //-------------------
        vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
    }

    VkShaderModule Pipeline::CreateShaderModule(const std::vector<char>& code) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }

        return shaderModule;
    }

	std::vector<char> Pipeline::ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}
