#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <fstream>
#include <string>
#include <vector>

#include "scene/Mesh.h"

namespace cat
{
	class Pipeline final
	{
	public:
		struct PipelineInfo
		{
			// DYNAMIC STATE
			std::vector<VkDynamicState> dynamicStates{};
			VkPipelineDynamicStateCreateInfo dynamicState{};

			// VERTEX INPUT
			std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions{};
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions{};

			// INPUT ASSEMBLY
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

			// VIEWPORT & SCISSORS
			VkPipelineViewportStateCreateInfo viewportState{};

			// RASTERIZER
			VkPipelineRasterizationStateCreateInfo rasterizer{};

			// MULTISAMPLING
			VkPipelineMultisampleStateCreateInfo multisampling{};

			// DEPTH & STENCIL TESTING
			VkPipelineDepthStencilStateCreateInfo depthStencil{};

			// COLOR BLENDING
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};

			VkPipelineColorBlendStateCreateInfo colorBlending{};

			// PUSH CONSTANTS
			VkPushConstantRange pushConstantRanges{};


			// PIPELINE LAYOUT
			VkPipelineLayout pipelineLayout;
			void CreatePipelineLayout(const Device& device,const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
				pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRanges;
				if (vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create pipeline layout!");
				}
			}

			// ATTACHEMENTS
			std::vector<VkFormat> colorAttachments = { VK_FORMAT_UNDEFINED };
			VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
			VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;


			void SetDefault()
			{
				// DYNAMIC STATE
				dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
				dynamicState = VkPipelineDynamicStateCreateInfo{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
					.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
					.pDynamicStates = dynamicStates.data()
				};

				// VERTEX INPUT
				vertexBindingDescriptions = { Mesh::Vertex::getBindingDescription() };
				vertexAttributeDescriptions = Mesh::Vertex::getAttributeDescriptions();

				// INPUT ASSEMBLY
				inputAssembly = VkPipelineInputAssemblyStateCreateInfo{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
					.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
					.primitiveRestartEnable = VK_FALSE
				};

				// VIEWPORT & SCISSORS
				viewportState = VkPipelineViewportStateCreateInfo{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.pViewports = nullptr, // will be set dynamically
					.scissorCount = 1,
					.pScissors = nullptr // will be set dynamically
				};

				// RASTERIZER
				rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer.depthClampEnable = VK_FALSE;
				rasterizer.rasterizerDiscardEnable = VK_FALSE;
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizer.lineWidth = 1.0f;
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
				rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
				rasterizer.depthBiasEnable = VK_FALSE;

				// MULTISAMPLING
				multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.sampleShadingEnable = VK_FALSE;
				multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

				// DEPTH & STENCIL TESTING
				depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depthStencil.depthTestEnable = VK_TRUE;
				depthStencil.depthWriteEnable = VK_TRUE;
				depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
				depthStencil.depthBoundsTestEnable = VK_FALSE;
				depthStencil.minDepthBounds = 0.0f;
				depthStencil.maxDepthBounds = 1.0f;


				// COLOR BLENDING
				colorBlendAttachments.resize(1);
				colorBlendAttachments[0].blendEnable = VK_FALSE;
				colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

				colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				colorBlending.logicOpEnable = VK_FALSE;
				colorBlending.attachmentCount = colorBlendAttachments.size();
				colorBlending.pAttachments = colorBlendAttachments.data();
				colorBlending.blendConstants[0] = 0.0f;
				colorBlending.blendConstants[1] = 0.0f;
				colorBlending.blendConstants[2] = 0.0f;
				colorBlending.blendConstants[3] = 0.0f;


				// PUSH CONSTANTS
				pushConstantRanges = VkPushConstantRange{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset = 0,
					.size = sizeof(glm::mat4)
				};

				// PIPELINE LAYOUT
				pipelineLayout = VK_NULL_HANDLE;

				// ATTACHEMENTS
				colorAttachments = { VK_FORMAT_B8G8R8A8_SRGB };
				depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
				stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
			}
		};


		// CTOR & DTOR
		//--------------------
		Pipeline(Device& device, const std::string& vertPath, const std::string& fragPath, const PipelineInfo& pipelineInfo);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;


		// Methods
		//--------------------
		void Bind(VkCommandBuffer commandBuffer) const
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
		}

		// Getters & Setters
		VkPipeline GetGraphicsPipeline() const { return m_GraphicsPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }


	private:
		// Private Methods
		//--------------------
		void CreateGraphicsPipeline(const PipelineInfo& pipelineInfo);
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
		static std::vector<char> ReadFile(const std::string& filename);



		// Private Members
		//--------------------
		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;

		const std::string m_VertPath;
		const std::string m_FragPath;

		Device& m_Device;
		SwapChain* m_pSwapChain;
		VkExtent2D m_SwapChainExtent;
		VkDescriptorSetLayout m_DescriptorSetLayout;
	};

}
