// PerformanceTimer.cpp
#include "PerformanceTimer.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace cat
{
    PerformanceTimer::PerformanceTimer()
    {
        m_FrameMetrics.reserve(5000); // Pre-allocate for 5000 frames
    }

    void PerformanceTimer::StartRecording(uint32_t maxFrames)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        m_IsRecording = true;
        m_MaxFrames = maxFrames;
        m_FrameCount = 0;
        m_FrameMetrics.clear();
        m_FrameMetrics.reserve(maxFrames);

        std::cout << "\n=== PERFORMANCE RECORDING STARTED ===" << std::endl;
        std::cout << "Will record first " << maxFrames << " frames" << std::endl;
        std::cout << "Press P to stop early and save" << std::endl;
    }

    void PerformanceTimer::StopRecording()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (!m_IsRecording) return;

        m_IsRecording = false;
        std::cout << "\n=== PERFORMANCE RECORDING STOPPED ===" << std::endl;
        std::cout << "Recorded " << m_FrameCount << " frames" << std::endl;

        if (!m_FrameMetrics.empty())
        {
            auto summary = CalculateSummary();
            std::cout << "Average FPS: " << std::fixed << std::setprecision(1) << summary.avgFPS << std::endl;
            std::cout << "Average frame time: " << std::setprecision(3) << summary.avgFrameTime << " ms" << std::endl;
        }
    }

    void PerformanceTimer::ToggleRecording(uint32_t maxFrames)
    {
        if (m_IsRecording)
        {
            StopRecording();
			std::cout << "Performance recording toggled off." << std::endl;
        }
        else
        {
            StartRecording(maxFrames);
			std::cout << "Performance recording toggled on." << std::endl;
        }
    }

    void PerformanceTimer::BeginFrame()
    {
        if (!m_IsRecording || m_FrameCount >= m_MaxFrames) return;
        
        m_FrameStart = std::chrono::high_resolution_clock::now();

        // Reset current frame metrics
        m_CurrentFrameMetrics = FrameMetrics();
        m_CurrentFrameMetrics.frameNumber = m_FrameCount + 1;
    }

    void PerformanceTimer::EndFrame()
    {
        if (!m_IsRecording || m_FrameCount >= m_MaxFrames) return;

        auto frameEnd = std::chrono::high_resolution_clock::now();
        m_CurrentFrameMetrics.frameTime = std::chrono::duration<double, std::milli>(frameEnd - m_FrameStart).count();

        // Calculate derived metrics
        m_CurrentFrameMetrics.totalGpuTime = m_CurrentFrameMetrics.depthPrepassTime +
            m_CurrentFrameMetrics.shadowPassTime +
            m_CurrentFrameMetrics.geometryPassTime +
            m_CurrentFrameMetrics.lightingPassTime +
            m_CurrentFrameMetrics.volumetricPassTime +
            m_CurrentFrameMetrics.blitPassTime;

        m_CurrentFrameMetrics.cpuOverhead = m_CurrentFrameMetrics.frameTime - m_CurrentFrameMetrics.totalGpuTime;
        m_CurrentFrameMetrics.fps = (m_CurrentFrameMetrics.frameTime > 0) ?
            1000.0 / m_CurrentFrameMetrics.frameTime : 0.0;

        // Store this frame's data
        m_FrameMetrics.push_back(m_CurrentFrameMetrics);
        m_FrameCount++;

        // Auto-stop if we've reached max frames
        if (m_FrameCount >= m_MaxFrames)
        {
            StopRecording();
        }
    }

    void PerformanceTimer::BeginPass(const std::string& passName)
    {
        if (!m_IsRecording || m_FrameCount >= m_MaxFrames) return;

        m_CurrentPassName = passName;
        m_PassStart = std::chrono::high_resolution_clock::now();
    }

    void PerformanceTimer::EndPass(const std::string& passName)
    {
        if (!m_IsRecording || m_FrameCount >= m_MaxFrames || passName != m_CurrentPassName) return;

        auto passEnd = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(passEnd - m_PassStart).count();

        // Update the appropriate pass time
        double* metricPtr = GetPassMetricPtr(passName);
        if (metricPtr)
        {
            *metricPtr = duration;
        }
    }

    double* PerformanceTimer::GetPassMetricPtr(const std::string& passName)
    {
        if (passName == "DepthPrepass") return &m_CurrentFrameMetrics.depthPrepassTime;
        if (passName == "ShadowPass") return &m_CurrentFrameMetrics.shadowPassTime;
        if (passName == "GeometryPass") return &m_CurrentFrameMetrics.geometryPassTime;
        if (passName == "LightingPass") return &m_CurrentFrameMetrics.lightingPassTime;
        if (passName == "VolumetricPass") return &m_CurrentFrameMetrics.volumetricPassTime;
        if (passName == "BlitPass") return &m_CurrentFrameMetrics.blitPassTime;
        return nullptr;
    }

    PerformanceTimer::SummaryStats PerformanceTimer::CalculateSummary() const
    {
        SummaryStats stats;

        if (m_FrameMetrics.empty()) return stats;

        // Calculate frame time statistics
        std::vector<double> frameTimes;
        frameTimes.reserve(m_FrameMetrics.size());

        for (const auto& frame : m_FrameMetrics)
        {
            frameTimes.push_back(frame.frameTime);
            stats.totalTime += frame.frameTime;

            // Track min/max
            if (frame.frameTime < stats.minFrameTime) stats.minFrameTime = frame.frameTime;
            if (frame.frameTime > stats.maxFrameTime) stats.maxFrameTime = frame.frameTime;

            // Sum pass times for averages
            stats.avgDepthTime += frame.depthPrepassTime;
            stats.avgShadowTime += frame.shadowPassTime;
            stats.avgGeometryTime += frame.geometryPassTime;
            stats.avgLightingTime += frame.lightingPassTime;
            stats.avgVolumetricTime += frame.volumetricPassTime;
            stats.avgBlitTime += frame.blitPassTime;
        }

        // Calculate averages
        size_t count = m_FrameMetrics.size();
        stats.avgFrameTime = stats.totalTime / count;
        stats.avgFPS = 1000.0 / stats.avgFrameTime;

        stats.avgDepthTime /= count;
        stats.avgShadowTime /= count;
        stats.avgGeometryTime /= count;
        stats.avgLightingTime /= count;
        stats.avgVolumetricTime /= count;
        stats.avgBlitTime /= count;

        return stats;
    }

    void PerformanceTimer::SaveToCSV(const std::string& filename, bool includeSummary)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_FrameMetrics.empty())
        {
            std::cout << "No performance data to save!" << std::endl;
            return;
        }

        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open CSV file: " << filename << std::endl;
            return;
        }

        std::cout << "Saving performance data to: " << filename << std::endl;
        std::cout << "Frames recorded: " << m_FrameMetrics.size() << std::endl;

        // Write CSV header
        file << "Frame,FrameTime(ms),DepthPrepass(ms),ShadowPass(ms),GeometryPass(ms),"
            << "LightingPass(ms),VolumetricPass(ms),BlitPass(ms),TotalGPU(ms),"
            << "CPUOverhead(ms),FPS,Triangles,DrawCalls\n";

        // Write frame data (first X frames only)
        for (const auto& frame : m_FrameMetrics)
        {
            file << frame.GetAsCSV() << "\n";
        }

        // Optional: Add summary statistics
        if (includeSummary)
        {
            auto stats = CalculateSummary();

            file << "\n\nSUMMARY STATISTICS\n";
            file << "Metric,Value\n";
            file << "Total Frames," << m_FrameMetrics.size() << "\n";
            file << "Total Time (s)," << std::fixed << std::setprecision(3) << (stats.totalTime / 1000.0) << "\n";
            file << "Average Frame Time (ms)," << stats.avgFrameTime << "\n";
            file << "Min Frame Time (ms)," << stats.minFrameTime << "\n";
            file << "Max Frame Time (ms)," << stats.maxFrameTime << "\n";
            file << "Average FPS," << std::setprecision(1) << stats.avgFPS << "\n";
            file << "\nAverage Pass Times (ms)\n";
            file << "Depth Prepass," << std::setprecision(3) << stats.avgDepthTime << "\n";
            file << "Shadow Pass," << stats.avgShadowTime << "\n";
            file << "Geometry Pass," << stats.avgGeometryTime << "\n";
            file << "Lighting Pass," << stats.avgLightingTime << "\n";
            file << "Volumetric Pass," << stats.avgVolumetricTime << "\n";
            file << "Blit Pass," << stats.avgBlitTime << "\n";
            file << "Total GPU," << (stats.avgDepthTime + stats.avgShadowTime + stats.avgGeometryTime +
                stats.avgLightingTime + stats.avgVolumetricTime + stats.avgBlitTime) << "\n";
            file << "CPU Overhead," << (stats.avgFrameTime - (stats.avgDepthTime + stats.avgShadowTime +
                stats.avgGeometryTime + stats.avgLightingTime +
                stats.avgVolumetricTime + stats.avgBlitTime)) << "\n";
        }

        file.close();

        // Print summary to console
        auto stats = CalculateSummary();
        std::cout << "\n=== PERFORMANCE SUMMARY ===" << std::endl;
        std::cout << "Average FPS: " << std::fixed << std::setprecision(1) << stats.avgFPS << std::endl;
        std::cout << "Average frame time: " << std::setprecision(3) << stats.avgFrameTime << " ms" << std::endl;
        std::cout << "Frame time range: " << stats.minFrameTime << " - " << stats.maxFrameTime << " ms" << std::endl;
        std::cout << "\nPass breakdown (ms):" << std::endl;
        std::cout << "  Depth prepass: " << stats.avgDepthTime << std::endl;
        std::cout << "  Shadow pass: " << stats.avgShadowTime << std::endl;
        std::cout << "  Geometry pass: " << stats.avgGeometryTime << std::endl;
        std::cout << "  Lighting pass: " << stats.avgLightingTime << std::endl;
        std::cout << "  Volumetric pass: " << stats.avgVolumetricTime << std::endl;
        std::cout << "  Blit pass: " << stats.avgBlitTime << std::endl;
        std::cout << "  Total GPU: " << (stats.avgDepthTime + stats.avgShadowTime + stats.avgGeometryTime +
            stats.avgLightingTime + stats.avgVolumetricTime + stats.avgBlitTime) << std::endl;
        std::cout << "  CPU overhead: " << (stats.avgFrameTime - (stats.avgDepthTime + stats.avgShadowTime +
            stats.avgGeometryTime + stats.avgLightingTime +
            stats.avgVolumetricTime + stats.avgBlitTime)) << std::endl;
    }
}