#include "Pipeline.h"

namespace cat
{
	Pipeline::Pipeline(Device& device, const std::string& vertPath, const std::string& fragPath, const PipelineInfo& pipelineInfo)
		:   m_VertPath(vertPath), m_FragPath(fragPath),
		m_Device(device), m_PipelineLayout(pipelineInfo.pipelineLayout)
	{
		CreateGraphicsPipeline(pipelineInfo);
	}

    Pipeline::~Pipeline()
	{
		vkDestroyPipeline(m_Device.GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }


    void Pipeline::CreateGraphicsPipeline(const PipelineInfo& pipelineInfo)
    {
        // Shader stage creation
        //-------------------
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        //VERTEX
        VkShaderModule vertShaderModule {VK_NULL_HANDLE};
        if (!m_VertPath.empty())
        {
            auto vertShaderCode = ReadFile(m_VertPath);
            vertShaderModule = CreateShaderModule(vertShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;    //obligatory
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;    // tells vulkan in which pipeline stage the shader is going to be used

            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";   // the function to invoke = entry point (multip frag shaders can be combined into a single shader module and use diff entry point to differentiate between the behaviours)
            // pSpecializationInfo   ->>>>   It allows you to specify values for shader constants.(autom nullptr -> no constants)

			shaderStages.push_back(vertShaderStageInfo);
        }

        //FRAGMENT
        VkShaderModule fragShaderModule{ VK_NULL_HANDLE };
        if (!m_FragPath.empty())
		{
			auto fragShaderCode = ReadFile(m_FragPath);
        	fragShaderModule = CreateShaderModule(fragShaderCode);

        	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        	fragShaderStageInfo.module = fragShaderModule;
        	fragShaderStageInfo.pName = "main";

			shaderStages.push_back(fragShaderStageInfo);
		}


        // Fixed Functions
        //------------------

        // VERTEX INPUT
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = pipelineInfo.vertexBindingDescriptions.size();
        vertexInputInfo.pVertexBindingDescriptions = pipelineInfo.vertexBindingDescriptions.data(); //optional   -> point to array of struct that describe the aforementioned details for loading vertex data
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> (pipelineInfo.vertexAttributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = pipelineInfo.vertexAttributeDescriptions.data(); //optional     -> point to array of struct that describe the aforementioned details for loading vertex data

        // RENDERING INFO
		VkPipelineRenderingCreateInfoKHR renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		renderingInfo.pNext = nullptr;
        renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = pipelineInfo.colorAttachments.size();
        renderingInfo.pColorAttachmentFormats = pipelineInfo.colorAttachments.data();
        renderingInfo.depthAttachmentFormat = pipelineInfo.depthAttachmentFormat;
        renderingInfo.stencilAttachmentFormat = pipelineInfo.stencilAttachmentFormat;

        // Creating the Graphics Pipeline (Conclusion)
        //-------------------
        VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
        graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineInfo.stageCount = shaderStages.size();
        graphicsPipelineInfo.pStages = shaderStages.data();

        graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
        graphicsPipelineInfo.pInputAssemblyState = &pipelineInfo.inputAssembly;
        graphicsPipelineInfo.pViewportState = &pipelineInfo.viewportState;
        graphicsPipelineInfo.pRasterizationState = &pipelineInfo.rasterizer;
        graphicsPipelineInfo.pMultisampleState = &pipelineInfo.multisampling;
        graphicsPipelineInfo.pDepthStencilState = &pipelineInfo.depthStencil;
        graphicsPipelineInfo.pColorBlendState = &pipelineInfo.colorBlending;
        graphicsPipelineInfo.pDynamicState = &pipelineInfo.dynamicState;

        graphicsPipelineInfo.layout = pipelineInfo.pipelineLayout;

        graphicsPipelineInfo.pNext = &renderingInfo;
        graphicsPipelineInfo.renderPass = VK_NULL_HANDLE;
        graphicsPipelineInfo.subpass = 0;

        graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE; //optional
        graphicsPipelineInfo.basePipelineIndex = -1; //optional

        if (vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }


        // Module deletion
        //-------------------
			vkDestroyShaderModule(m_Device.GetDevice(), fragShaderModule, nullptr);
			vkDestroyShaderModule(m_Device.GetDevice(), vertShaderModule, nullptr);
    }

    VkShaderModule Pipeline::CreateShaderModule(const std::vector<char>& code) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
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
