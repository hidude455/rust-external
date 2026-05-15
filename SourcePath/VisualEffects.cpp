#include "VisualEffects.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Graphics {
    
    CVisualEffects::CVisualEffects() 
        : m_totalTime(0.0f), m_maxParticles(500), m_effectsEnabled(true), m_effectIntensity(1.0f) {
        m_lastUpdate = std::chrono::high_resolution_clock::now();
        m_startTime = m_lastUpdate;
        
        // Initialize background animation
        m_background.waveAmplitude = 20.0f;
        m_background.waveFrequency = 0.02f;
        m_background.waveSpeed = 1.0f;
        m_background.primaryColor = ImVec4(0.1f, 0.1f, 0.2f, 0.8f);
        m_background.secondaryColor = ImVec4(0.2f, 0.1f, 0.3f, 0.8f);
        m_background.gradientOffset = 0.0f;
        m_background.isActive = true;
        
        InitializeWavePoints();
        
        // Reserve memory for particles
        m_particles.reserve(m_maxParticles);
    }
    
    CVisualEffects::~CVisualEffects() {
        Shutdown();
    }
    
    void CVisualEffects::Initialize() {
        // Pre-allocate particles
        m_particles.clear();
        m_particles.reserve(m_maxParticles);
        
        // Initialize glow effects
        m_glowEffects.clear();
        m_glowEffects.reserve(10);
        
        // Reset timing
        m_lastUpdate = std::chrono::high_resolution_clock::now();
        m_startTime = m_lastUpdate;
        m_totalTime = 0.0f;
    }
    
    void CVisualEffects::Shutdown() {
        m_particles.clear();
        m_glowEffects.clear();
        m_keyframes.clear();
    }
    
    void CVisualEffects::Update() {
        if (!m_effectsEnabled) return;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - m_lastUpdate).count();
        m_lastUpdate = currentTime;
        
        m_totalTime += deltaTime;
        
        // Update all effect systems
        UpdateParticles(deltaTime);
        UpdateBackground(deltaTime);
        UpdateGlowEffects(deltaTime);
        UpdateAnimations(deltaTime);
    }
    
    void CVisualEffects::Render(const ImVec2& windowPos, const ImVec2& windowSize) {
        if (!m_effectsEnabled) return;
        
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;
        
        // Render background first
        if (m_background.isActive) {
            RenderBackground(drawList, windowPos, windowSize);
        }
        
        // Render glow effects
        RenderGlowEffects(drawList);
        
        // Render particles
        RenderParticles(drawList);
    }
    
    void CVisualEffects::UpdateParticles(float deltaTime) {
        for (auto& particle : m_particles) {
            if (!particle.isActive) continue;
            
            // Update particle life
            particle.life -= deltaTime;
            if (particle.life <= 0.0f) {
                particle.isActive = false;
                continue;
            }
            
            // Update position
            particle.position.x += particle.velocity.x * deltaTime;
            particle.position.y += particle.velocity.y * deltaTime;
            
            // Apply gravity
            particle.velocity.y += 98.0f * deltaTime; // Gravity effect
            
            // Fade out based on life
            float lifeRatio = particle.life / particle.maxLife;
            particle.color.w = lifeRatio * m_effectIntensity;
        }
        
        // Remove inactive particles
        m_particles.erase(
            std::remove_if(m_particles.begin(), m_particles.end(), 
                          [](const Particle& p) { return !p.isActive; }),
            m_particles.end()
        );
    }
    
    void CVisualEffects::UpdateBackground(float deltaTime) {
        if (!m_background.isActive) return;
        
        // Update wave animation
        m_background.gradientOffset += m_background.waveSpeed * deltaTime * 0.1f;
        if (m_background.gradientOffset > 1.0f) {
            m_background.gradientOffset -= 1.0f;
        }
    }
    
    void CVisualEffects::UpdateGlowEffects(float deltaTime) {
        for (auto& glow : m_glowEffects) {
            if (glow.pulsing) {
                glow.currentPulse += glow.pulseSpeed * deltaTime;
                if (glow.currentPulse > 1.0f) {
                    glow.currentPulse -= 1.0f;
                }
                
                // Calculate pulse intensity
                float pulseValue = (sin(glow.currentPulse * 3.14159f * 2.0f) + 1.0f) * 0.5f;
                glow.intensity = 0.5f + pulseValue * 0.5f;
            }
        }
    }
    
    void CVisualEffects::UpdateAnimations(float deltaTime) {
        if (m_keyframes.empty()) return;
        
        // Find current keyframe based on time
        float currentTime = fmod(m_totalTime, m_keyframes.back().time);
        
        for (size_t i = 0; i < m_keyframes.size() - 1; ++i) {
            if (currentTime >= m_keyframes[i].time && currentTime <= m_keyframes[i + 1].time) {
                // Interpolate between keyframes
                float t = (currentTime - m_keyframes[i].time) / (m_keyframes[i + 1].time - m_keyframes[i].time);
                t = EaseInOut(t);
                
                // Apply interpolated values
                // This would affect UI elements
                break;
            }
        }
    }
    
    void CVisualEffects::RenderParticles(ImDrawList* drawList) {
        for (const auto& particle : m_particles) {
            if (!particle.isActive) continue;
            
            ImU32 color = IM_COL32(
                static_cast<int>(particle.color.x * 255),
                static_cast<int>(particle.color.y * 255),
                static_cast<int>(particle.color.z * 255),
                static_cast<int>(particle.color.w * 255)
            );
            
            drawList->AddCircleFilled(particle.position, particle.size, color);
        }
    }
    
    void CVisualEffects::RenderBackground(ImDrawList* drawList, const ImVec2& windowPos, const ImVec2& windowSize) {
        RenderGradientBackground(drawList, windowPos, windowSize);
        
        // Render wave effect
        if (!m_background.wavePoints.empty()) {
            ImVec4 waveColor = InterpolateColor(m_background.primaryColor, m_background.secondaryColor, m_background.gradientOffset);
            ImU32 color = IM_COL32(
                static_cast<int>(waveColor.x * 255),
                static_cast<int>(waveColor.y * 255),
                static_cast<int>(waveColor.z * 255),
                static_cast<int>(waveColor.w * 255 * 0.3f)
            );
            
            for (size_t i = 0; i < m_background.wavePoints.size() - 1; ++i) {
                ImVec2 p1 = m_background.wavePoints[i];
                ImVec2 p2 = m_background.wavePoints[i + 1];
                
                // Apply wave animation
                float waveOffset = sin(p1.x * m_background.waveFrequency + m_totalTime * m_background.waveSpeed) * m_background.waveAmplitude;
                p1.y += waveOffset;
                
                waveOffset = sin(p2.x * m_background.waveFrequency + m_totalTime * m_background.waveSpeed) * m_background.waveAmplitude;
                p2.y += waveOffset;
                
                drawList->AddLine(p1, p2, color, 2.0f);
            }
        }
    }
    
    void CVisualEffects::RenderGlowEffects(ImDrawList* drawList) {
        for (const auto& glow : m_glowEffects) {
            ImVec4 color = glow.color;
            color.w *= glow.intensity * m_effectIntensity;
            
            ImU32 glowColor = IM_COL32(
                static_cast<int>(color.x * 255),
                static_cast<int>(color.y * 255),
                static_cast<int>(color.z * 255),
                static_cast<int>(color.w * 255)
            );
            
            // Render multiple layers for glow effect
            for (int i = 3; i > 0; --i) {
                float alpha = 0.1f / i;
                ImU32 layerColor = (glowColor & ~IM_COL32_A_MASK) | static_cast<ImU32>(alpha * 255);
                
                drawList->AddRect(
                    ImVec2(glow.position.x - i, glow.position.y - i),
                    ImVec2(glow.position.x + glow.size.x + i, glow.position.y + glow.size.y + i),
                    layerColor, 4.0f, 0, 2.0f
                );
            }
        }
    }
    
    void CVisualEffects::RenderGradientBackground(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size) {
        // Create animated gradient background
        ImVec4 topColor = InterpolateColor(m_background.primaryColor, m_background.secondaryColor, m_background.gradientOffset);
        ImVec4 bottomColor = InterpolateColor(m_background.secondaryColor, m_background.primaryColor, m_background.gradientOffset);
        
        ImU32 topColor32 = IM_COL32(
            static_cast<int>(topColor.x * 255),
            static_cast<int>(topColor.y * 255),
            static_cast<int>(topColor.z * 255),
            static_cast<int>(topColor.w * 255)
        );
        
        ImU32 bottomColor32 = IM_COL32(
            static_cast<int>(bottomColor.x * 255),
            static_cast<int>(bottomColor.y * 255),
            static_cast<int>(bottomColor.z * 255),
            static_cast<int>(bottomColor.w * 255)
        );
        
        // Draw gradient rectangle
        drawList->AddRectFilledMultiColor(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            topColor32, topColor32, bottomColor32, bottomColor32
        );
    }
    
    void CVisualEffects::EmitParticles(const ImVec2& position, const ImVec4& color, int count) {
        if (m_particles.size() + count > m_maxParticles) {
            return; // Don't exceed max particles
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 3.14159f * 2.0f);
        std::uniform_real_distribution<float> speedDist(50.0f, 150.0f);
        std::uniform_real_distribution<float> sizeDist(2.0f, 5.0f);
        std::uniform_real_distribution<float> lifeDist(0.5f, 2.0f);
        
        for (int i = 0; i < count; ++i) {
            Particle particle = CreateParticle(position, color, sizeDist(gen));
            
            float angle = angleDist(gen);
            float speed = speedDist(gen);
            particle.velocity.x = cos(angle) * speed;
            particle.velocity.y = sin(angle) * speed;
            particle.life = lifeDist(gen);
            particle.maxLife = particle.life;
            
            m_particles.push_back(particle);
        }
    }
    
    void CVisualEffects::CreateExplosion(const ImVec2& position, const ImVec4& color) {
        EmitParticles(position, color, 30);
    }
    
    void CVisualEffects::CreateTrail(const ImVec2& from, const ImVec2& to, const ImVec4& color) {
        int particleCount = static_cast<int>(abs(to.x - from.x) + abs(to.y - from.y)) / 10;
        
        for (int i = 0; i < particleCount; ++i) {
            float t = static_cast<float>(i) / particleCount;
            ImVec2 pos = ImVec2(
                from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t
            );
            
            Particle particle = CreateParticle(pos, color, 3.0f);
            particle.velocity = ImVec2(0, -20); // Slight upward movement
            particle.life = 1.0f;
            particle.maxLife = 1.0f;
            
            m_particles.push_back(particle);
        }
    }
    
    void CVisualEffects::EnableBackgroundAnimation(bool enable) {
        m_background.isActive = enable;
    }
    
    void CVisualEffects::SetBackgroundColors(const ImVec4& primary, const ImVec4& secondary) {
        m_background.primaryColor = primary;
        m_background.secondaryColor = secondary;
    }
    
    void CVisualEffects::SetWaveParameters(float amplitude, float frequency, float speed) {
        m_background.waveAmplitude = amplitude;
        m_background.waveFrequency = frequency;
        m_background.waveSpeed = speed;
    }
    
    void CVisualEffects::AddGlowEffect(const ImVec2& position, const ImVec2& size, const ImVec4& color) {
        GlowEffect glow;
        glow.position = position;
        glow.size = size;
        glow.color = color;
        glow.intensity = 1.0f;
        glow.radius = 10.0f;
        glow.pulsing = false;
        
        m_glowEffects.push_back(glow);
    }
    
    void CVisualEffects::CreatePulsingGlow(const ImVec2& position, const ImVec2& size, const ImVec4& color, float pulseSpeed) {
        AddGlowEffect(position, size, color);
        m_glowEffects.back().pulsing = true;
        m_glowEffects.back().pulseSpeed = pulseSpeed;
        m_glowEffects.back().currentPulse = 0.0f;
    }
    
    void CVisualEffects::StartAnimation(const std::vector<AnimationKeyframe>& keyframes) {
        m_keyframes = keyframes;
        m_totalTime = 0.0f;
    }
    
    void CVisualEffects::StopAnimation() {
        m_keyframes.clear();
    }
    
    bool CVisualEffects::IsAnimationPlaying() const {
        return !m_keyframes.empty();
    }
    
    void CVisualEffects::SetEffectIntensity(float intensity) {
        m_effectIntensity = std::clamp(intensity, 0.0f, 1.0f);
    }
    
    void CVisualEffects::SetMaxParticles(size_t maxCount) {
        m_maxParticles = maxCount;
        m_particles.reserve(maxCount);
    }
    
    void CVisualEffects::EnableEffects(bool enable) {
        m_effectsEnabled = enable;
    }
    
    bool CVisualEffects::AreEffectsEnabled() const {
        return m_effectsEnabled;
    }
    
    void CVisualEffects::OptimizeForPerformance() {
        SetMaxParticles(200);
        SetEffectIntensity(0.7f);
    }
    
    size_t CVisualEffects::GetActiveParticleCount() const {
        return m_particles.size();
    }
    
    float CVisualEffects::GetAverageFrameTime() const {
        return 16.67f; // Placeholder for 60 FPS
    }
    
    Particle CVisualEffects::CreateParticle(const ImVec2& position, const ImVec4& color, float size) {
        Particle particle;
        particle.position = position;
        particle.velocity = ImVec2(0, 0);
        particle.color = color;
        particle.size = size;
        particle.life = 1.0f;
        particle.maxLife = 1.0f;
        particle.isActive = true;
        return particle;
    }
    
    ImVec4 CVisualEffects::InterpolateColor(const ImVec4& color1, const ImVec4& color2, float t) {
        return ImVec4(
            color1.x + (color2.x - color1.x) * t,
            color1.y + (color2.y - color1.y) * t,
            color1.z + (color2.z - color1.z) * t,
            color1.w + (color2.w - color1.w) * t
        );
    }
    
    float CVisualEffects::EaseInOut(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
    
    void CVisualEffects::InitializeWavePoints() {
        const int pointCount = 50;
        m_background.wavePoints.clear();
        m_background.wavePoints.reserve(pointCount);
        
        for (int i = 0; i < pointCount; ++i) {
            m_background.wavePoints.push_back(ImVec2(i * 20.0f, 200.0f));
        }
    }
    
} // namespace Graphics
