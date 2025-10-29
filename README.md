# Vulkan Renderer [![wakatime](https://wakatime.com/badge/user/9ddd942c-b6f3-4a8e-aab7-1f2d3af43b2c/project/30cea435-6d91-4bae-9567-9d6072ece5e7.svg)](https://wakatime.com/badge/user/9ddd942c-b6f3-4a8e-aab7-1f2d3af43b2c/project/30cea435-6d91-4bae-9567-9d6072ece5e7)


## Description
This is my Vulkan renderer for the Graphics Programming 2 course of Howest - Digital Arts and Entertainment. 
The renderer supports the following Vulkan specific techniques or concepts:
- Graphics Pipeline
- Images and Buffers, including texture loading
- Depth Buffering
- An interactive camera using frame independent controls.
- A non-forward rendering pipeline technique with a Deferred Rendering Pipeline with a Depth Prepass.
- Physical Based Lighting using:
  - Cook-Torrance BRDF materials (using textures).
  - Light Types using physically based light units:
      - Omni/Point Light
      - Directional Light
  - Image Based Lighting (IBL) for Diffuse Irradiance only using HDR images.
- Post Processing chain with Tone Mapping:
  - Exposure determined by Physical Camera settings
  - Reinhard and Uncharted 2
- The rendering pipeline uses High Dynamic Range (HDR) and only maps to Low Dynamic Range (LDR) towards the end of the post processing chain


## Sources
For this project alot of sources different sources were used. But I'll list the most impactful ones here.
- [The Vulkan Documentation](https://docs.vulkan.org/spec/latest/index.html)
- [https://vulkan-tutorial.com/](https://vulkan-tutorial.com/)
- [https://learnopengl.com/](https://learnopengl.com/)
- [https://vkguide.dev/](https://vkguide.dev/)
- [https://github.com/blurrypiano/littleVulkanEngine/tree/main/src](https://github.com/blurrypiano/littleVulkanEngine/tree/main/src)
- The slides and classes given by Matthieu Delaere in Graphics Programming 2.
- The help and guidance from friends and classmates.
- ...

## Models
The models are fetched by CMake from [my models repository](https://github.com/ttritar/GP2_Models). I strongly suggest anyone to go read its README.md for info about the artists and more.

## Author
**[Thalia Tritar]**
[GitHub Repository](https://github.com/ttritar/GP2_VulkanProject.git)
