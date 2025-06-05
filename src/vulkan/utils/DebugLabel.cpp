#include "DebugLabel.h"
namespace cat
{
	
    void cat::DebugLabel::Init(VkDevice device)
{
    std::call_once(initFlag, [device]() 
        {
        vkCmdBeginDebugUtilsLabelEXT =
            reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
                vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));

        vkCmdEndDebugUtilsLabelEXT =
            reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
                vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));

        vkSetDebugUtilsObjectNameEXT =
            reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));

        s_device = device; });
}
    
    bool DebugLabel::IsAvailable()
{
    return vkCmdBeginDebugUtilsLabelEXT && vkCmdEndDebugUtilsLabelEXT && vkSetDebugUtilsObjectNameEXT;
}
    
    DebugLabel::ScopedCmdLabel::ScopedCmdLabel(VkCommandBuffer cmdBuffer, const std::string& name, const glm::vec4& color) : cmdBuffer(cmdBuffer)
    	{
        if (vkCmdBeginDebugUtilsLabelEXT) 
        {
            VkDebugUtilsLabelEXT labelInfo = 
            {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                name.c_str(),
                {color[0], color[1], color[2], color[3]}
            };
            vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
        }
    }
    
    DebugLabel::ScopedCmdLabel::~ScopedCmdLabel() 
    {
    	vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
        
    }
    
    void DebugLabel::BeginCmdLabel(VkCommandBuffer cmdBuffer, const std::string& name, glm::vec4 color)
	{
        if (vkCmdBeginDebugUtilsLabelEXT) 
        {
            VkDebugUtilsLabelEXT labelInfo = 
            {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                name.c_str(),
                {color[0], color[1], color[2], color[3]}
            };
            vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
        }
    }
    
    void DebugLabel::EndCmdLabel(VkCommandBuffer cmdBuffer)
	{
        if (vkCmdEndDebugUtilsLabelEXT) 
        {
            vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
        }
    }
    
    void DebugLabel::SetObjectName(uint64_t objectHandle, VkObjectType objectType, const std::string& name)
	{
        if (vkSetDebugUtilsObjectNameEXT && s_device) 
        {
            VkDebugUtilsObjectNameInfoEXT nameInfo = 
            {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                nullptr,
                objectType,
                objectHandle,
                name.c_str()
            };
            vkSetDebugUtilsObjectNameEXT(s_device, &nameInfo);
        }
    }
    
    void DebugLabel::NameBuffer(VkBuffer buffer, const std::string& name)
	{
        SetObjectName((uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, name);
    }
    
    void DebugLabel::NameImage(VkImage image, const std::string& name)
	{
        SetObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE, name);
    }
    
    void DebugLabel::NameCommandBuffer(VkCommandBuffer cmdBuffer, const std::string& name)
	{
        SetObjectName((uint64_t)cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
    }
    
}
