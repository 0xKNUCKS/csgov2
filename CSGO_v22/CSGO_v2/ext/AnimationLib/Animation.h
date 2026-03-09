// Src: https://github.com/0xKNUCKS/cpp-animation-lib
#pragma once
#include <algorithm>
#include <chrono>
#include "ext/easing.h"

class Animation
{
public:
    Animation() : m_flAnimationDuration(1.0f), m_eEaseIn(EaseInQuad), m_eEaseOut(EaseOutQuad) {};
    Animation(float AnimDuration) : m_flAnimationDuration(AnimDuration) {};
    Animation(float AnimDuration, easing_functions In, easing_functions out) : m_flAnimationDuration(AnimDuration), m_eEaseIn(In), m_eEaseOut(out) {};

    // Has to be called every tick
    void Update()
    {
        // Reset the elapsed time if the bool switches, store current value as starting point
        if (m_bSwitch != m_bLastSwitch) {
            m_flElapsedTime = 0;
            m_flStartValue = m_flValue;
        }

        m_flElapsedTime = std::clamp(m_flElapsedTime, 0.0f, m_flAnimationDuration);
        float t = m_flElapsedTime / m_flAnimationDuration;

        // Determine the target value based on the current state
        float targetValue = m_bSwitch ? 1.0f : 0.0f;

        // Select the appropriate easing function based on the current state
        easing_functions EaseInOrOut = m_bSwitch ? m_eEaseIn : m_eEaseOut;

        // Apply easing with lerp from start value to target
        m_flValue = m_flStartValue + (targetValue - m_flStartValue) * static_cast<float>(getEasingFunction(EaseInOrOut)(t));

        m_flElapsedTime += getDeltaTime();
        m_bLastSwitch = m_bSwitch;
    }

    float   getValue() { return m_flValue; }
    float   getValue(float scale) { return m_flValue * scale; }
    int     getValue(int baseValue) { return static_cast<int>(m_flValue * baseValue); }

    bool& getSwitch() { return m_bSwitch; }
    bool Switch() { return m_bSwitch = !m_bSwitch; }
    void Switch(bool value) { m_bSwitch = value; }
protected:
    bool m_bSwitch = false;
    bool m_bLastSwitch = m_bSwitch;

    float m_flAnimationDuration = 1.0f;
    float m_flElapsedTime = 0.f;

    float m_flValue = 0.f;
    float m_flStartValue = 0.f;

    easing_functions m_eEaseIn = EaseInQuad;
    easing_functions m_eEaseOut = EaseOutQuad;

    using clock = std::chrono::high_resolution_clock;
    clock::time_point m_tLastTime = clock::now();

    float getDeltaTime()
    {
        auto now = clock::now();
        std::chrono::duration<float> duration = now - m_tLastTime;
        m_tLastTime = now;
        return duration.count();
    }
};
