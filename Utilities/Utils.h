/*
    Utils.h
    Created: 14 Jun 2023 9:29:30pm
    Author:  Quentin Prost
*/

#pragma once

inline void limitOutput(float* buffer, int sampleCount)
{
    if (buffer == nullptr) { return; }
    bool firstWarning = true;
    for (int i = 0; i < sampleCount; ++i) {
        float x = buffer[i];
        bool silence = false;
        if (std::isnan(x)) {
            DBG("!!! Nan detected in audio buffer, silencing !!!");
            silence = true;
        } else if (std::isinf(x)) {
            DBG("!!! WARNING: inf detected in audio buffer, silencing !!!");
            silence = true;
        } else if (x < -2.0f || x > 2.0f) {
            DBG("WARNING: sample out of range, silencing");
            silence = true;
        } else if (x < -1.0f) {
            if (firstWarning) {
                DBG("sample out of range, hard clipping");
                firstWarning = false;
            }
            buffer[i] = -1.0f;
        } else if (x > 1.0f) {
            if (firstWarning) {
                DBG("WARNING: sample out of range, hard clipping");
                firstWarning = false;
            }
            buffer[i] = 1.0f;
        }
        if (silence) {
            memset(buffer, 0, sampleCount * sizeof(float));
            return;
        }
    }
};
