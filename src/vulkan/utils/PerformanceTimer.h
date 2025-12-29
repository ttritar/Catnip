// PerformanceTimer.h
#pragma once
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <atomic>
#include <mutex>

namespace cat
{
    struct FrameMetrics
    {
        uint32_t frameNumber = 0;
        double frameTime = 0.0;         // Total frame time (ms)
        double depthPrepassTime = 0.0;  // Depth prepass GPU time
        double shadowPassTime = 0.0;    // Shadow pass GPU time
        double geometryPassTime = 0.0;  // Geometry pass GPU time
        double lightingPassTime = 0.0;  // Lighting pass GPU time
        double volumetricPassTime = 0.0;// Volumetric pass GPU time
        double blitPassTime = 0.0;      // Blit pass GPU time
        double totalGpuTime = 0.0;      // Sum of all pass times
        double cpuOverhead = 0.0;       // Frame time - total GPU time
        double fps = 0.0;               // Calculated FPS
        uint32_t triangleCount = 0;     // Triangles rendered
        uint32_t drawCalls = 0;         // Number of draw calls
        
        std::string GetAsCSV() const
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3)
                << frameNumber << ","
                << frameTime << ","
                << depthPrepassTime << ","
                << shadowPassTime << ","
                << geometryPassTime << ","
                << lightingPassTime << ","
                << volumetricPassTime << ","
                << blitPassTime << ","
                << totalGpuTime << ","
                << cpuOverhead << ","
                << fps << ","
                << triangleCount << ","
                << drawCalls;
            return ss.str();
        }
    };

    class PerformanceTimer
    {
    public:
        PerformanceTimer();
        ~PerformanceTimer() = default;

        // Start recording first N frames
        void StartRecording(uint32_t maxFrames = 1000);
        void StopRecording();

        // Frame timing
        void BeginFrame() ;
        void EndFrame();

        // Pass timing (GPU passes)
        void BeginPass(const std::string& passName);
        void EndPass(const std::string& passName);

        // Set additional metrics
        void SetTriangleCount(uint32_t count) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_IsRecording && m_CurrentFrameMetrics.frameNumber <= m_MaxFrames) {
                m_CurrentFrameMetrics.triangleCount = count;
            }
        }

        void SetDrawCalls(uint32_t calls) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_IsRecording && m_CurrentFrameMetrics.frameNumber <= m_MaxFrames) {
                m_CurrentFrameMetrics.drawCalls = calls;
            }
        }

        // Save results
        void SaveToCSV(const std::string& filename = "performance.csv", bool includeSummary = true);

        // Status
        bool IsRecording() const { return m_IsRecording; }
        bool HasReachedMaxFrames() const { return m_FrameCount >= m_MaxFrames; }
        uint32_t GetRecordedFrames() const { return m_FrameCount; }
        uint32_t GetMaxFrames() const { return m_MaxFrames; }

        // Quick toggle for debugging
        void ToggleRecording(uint32_t maxFrames = 1000);

    private:
        std::mutex m_Mutex;

        // Recording state
        std::atomic<bool> m_IsRecording{ false };
        std::atomic<uint32_t> m_MaxFrames{ 1000 };
        std::atomic<uint32_t> m_FrameCount{ 0 };

        // Timing data
        FrameMetrics m_CurrentFrameMetrics;
        std::vector<FrameMetrics> m_FrameMetrics;

        // Timers
        std::chrono::high_resolution_clock::time_point m_FrameStart;
        std::chrono::high_resolution_clock::time_point m_PassStart;

        // Current pass tracking
        std::string m_CurrentPassName;

        // Statistics for summary
        struct SummaryStats
        {
            double avgFrameTime = 0.0;
            double minFrameTime = std::numeric_limits<double>::max();
            double maxFrameTime = 0.0;
            double avgFPS = 0.0;
            double totalTime = 0.0;

            // Per-pass averages
            double avgDepthTime = 0.0;
            double avgShadowTime = 0.0;
            double avgGeometryTime = 0.0;
            double avgLightingTime = 0.0;
            double avgVolumetricTime = 0.0;
            double avgBlitTime = 0.0;
        };

        SummaryStats CalculateSummary() const;

        // Helper to map pass names to metrics
        double* GetPassMetricPtr(const std::string& passName);
    };
}