#pragma once
#include "Common.h"
#include "../MenuPath/imgui/imgui.h"
#include <vector>
#include <chrono>
#include <memory>

namespace Graphics {
    
    // Particle structure for visual effects
    struct Particle {
        ImVec2 position;
        ImVec2 velocity;
        ImVec4 color;
        float size;
        float life;
        float maxLife;
        bool isActive;
    };
    
    // Animation keyframe for smooth transitions
    struct AnimationKeyframe {
        float time;
        ImVec4 color;
        ImVec2 position;
        float scale;
        float rotation;
    };
    
    // Background animation data
    struct BackgroundAnimation {
        std::vector<ImVec2> wavePoints;
        float waveAmplitude;
        float waveFrequency;
        float waveSpeed;
        ImVec4 primaryColor;
        ImVec4 secondaryColor;
        float gradientOffset;
        bool isActive;
    };
    
    // Glow effect parameters
    struct GlowEffect {
        ImVec2 position;
        ImVec2 size;
        ImVec4 color;
        float intensity;
        float radius;
        bool pulsing;
        float pulseSpeed;
        float currentPulse;
    };
    
    class CVisualEffects {
    private:
        std::vector<Particle> m_particles;
        std::vector<AnimationKeyframe> m_keyframes;
        std::vector<GlowEffect> m_glowEffects;
        BackgroundAnimation m_background;
        
        // Animation timing
        std::chrono::high_resolution_clock::time_point m_lastUpdate;
        std::chrono::high_resolution_clock::time_point m_startTime;
        float m_totalTime;
        
        // Performance optimization
        size_t m_maxParticles;
        bool m_effectsEnabled;
        float m_effectIntensity;
        
        // Internal helper functions
        void UpdateParticles(float deltaTime);
        void UpdateBackground(float deltaTime);
        void UpdateGlowEffects(float deltaTime);
        void UpdateAnimations(float deltaTime);
        
        // Rendering functions
        void RenderParticles(ImDrawList* drawList);
        void RenderBackground(ImDrawList* drawList, const ImVec2& windowPos, const ImVec2& windowSize);
        void RenderGlowEffects(ImDrawList* drawList);
        void RenderGradientBackground(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size);
        
        // Utility functions
        Particle CreateParticle(const ImVec2& position, const ImVec4& color, float size);
        ImVec4 InterpolateColor(const ImVec4& color1, const ImVec4& color2, float t);
        float EaseInOut(float t);
        void InitializeWavePoints();
        
    public:
        CVisualEffects();
        ~CVisualEffects();
        
        // Initialization
        void Initialize();
        void Shutdown();
        
        // Update and render
        void Update();
        void Render(const ImVec2& windowPos, const ImVec2& windowSize);
        
        // Particle system
        void EmitParticles(const ImVec2& position, const ImVec4& color, int count = 10);
        void CreateExplosion(const ImVec2& position, const ImVec4& color);
        void CreateTrail(const ImVec2& from, const ImVec2& to, const ImVec4& color);
        
        // Background effects
        void EnableBackgroundAnimation(bool enable);
        void SetBackgroundColors(const ImVec4& primary, const ImVec4& secondary);
        void SetWaveParameters(float amplitude, float frequency, float speed);
        
        // Glow effects
        void AddGlowEffect(const ImVec2& position, const ImVec2& size, const ImVec4& color);
        void CreatePulsingGlow(const ImVec2& position, const ImVec2& size, const ImVec4& color, float pulseSpeed);
        
        // Animation system
        void StartAnimation(const std::vector<AnimationKeyframe>& keyframes);
        void StopAnimation();
        bool IsAnimationPlaying() const;
        
        // Configuration
        void SetEffectIntensity(float intensity);
        void SetMaxParticles(size_t maxCount);
        void EnableEffects(bool enable);
        bool AreEffectsEnabled() const;
        
        // Performance
        void OptimizeForPerformance();
        size_t GetActiveParticleCount() const;
        float GetAverageFrameTime() const;
    };
    
} // namespace Graphics
