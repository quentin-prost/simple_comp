#include "CompAhr.h"

template <typename SampleType>
void CompAhr<SampleType>::prepare(const juce::dsp::ProcessSpec& spec) {
    jassert(spec.sampleRate > 0);
    jassert(spec.maximumBlockSize > 0);
    mSampleRate = spec.sampleRate;
    mEnvelope.resize(spec.maximumBlockSize);
    std::fill(mEnvelope.begin(), mEnvelope.end(), static_cast<SampleType>(1.0));
    setAttack(mParams.attack);
    setRelease(mParams.release);
    setHold(mParams.hold);
}

template <typename SampleType>
void CompAhr<SampleType>::setAttack(SampleType attack) {
    mAttack.time = attack;
    mAttack.counter = 0;
    mAttack.samples = (unsigned int) ceil(attack * mSampleRate);
    mAttack.value = static_cast<SampleType>(1.0 - exp(-2.2 / (attack * (float) mSampleRate)));
    mAttack.coefs[0] = static_cast<SampleType>(1.0 - mAttack.value);
    mAttack.coefs[1] = mAttack.value;
}

template <typename SampleType>
void CompAhr<SampleType>::setHold(SampleType hold) {
    mHold.time = hold;
    mHold.counter = 0;
    mHold.samples = (unsigned int) ceil(hold * mSampleRate);
    mHold.value = static_cast<SampleType>(1.0 - exp(-2.2 / (hold * (float) mSampleRate)));
    mHold.coefs[0] = static_cast<SampleType>(1.0 - mHold.value);
    mHold.coefs[1] = mHold.value;
}

template <typename SampleType>
void CompAhr<SampleType>::setRelease(SampleType release) {
    mRelease.time = release;
    mRelease.counter = 0;
    mRelease.samples = (unsigned int) ceil(release * mSampleRate);
    mRelease.value = static_cast<SampleType>(1.0 - exp(-2.2 / (release * (float) mSampleRate)));
    mRelease.coefs[0] = static_cast<SampleType>(1.0 - mRelease.value);
    mRelease.coefs[1] = mRelease.value;
}

template <typename SampleType>
void CompAhr<SampleType>::setThreshold(SampleType threshold) {
    mThreshold.db = threshold;
    mThreshold.linear = juce::Decibels::decibelsToGain(threshold);
    mKnee.top = static_cast<SampleType>(mThreshold.db + 0.5*mKnee.width);
    mKnee.bottom = static_cast<SampleType>(mThreshold.db - 0.5*mKnee.width);
}

template <typename SampleType>
void CompAhr<SampleType>::setKnee(SampleType knee) {
    mKnee.type = knee < __FLT_EPSILON__ ? COMP_HARD_KNEE : COMP_SOFT_KNEE;
    mKnee.width = knee;
    mKnee.bottom = mThreshold.db - static_cast<SampleType>(0.5)*mKnee.width;
    mKnee.top = mThreshold.db + static_cast<SampleType>(0.5)*mKnee.width;
}

template <typename SampleType>
void CompAhr<SampleType>::setRatio(SampleType ratio) {
    mRatio.value = ratio;
    mRatio.slope = static_cast<SampleType>(1.0 / mRatio.value - 1.0);
}

template <typename SampleType>
void CompAhr<SampleType>::setMakeUpGain(SampleType makeUpGain) {
    mMakeUpGain.db = makeUpGain;
    mMakeUpGain.linear = juce::Decibels::decibelsToGain(makeUpGain);
}

template <typename SampleType>
void CompAhr<SampleType>::setParams(CompAhrParams<SampleType> *params) {
    setAttack(params->attack);
    setHold(params->hold);
    setRelease(params->release);
    setKnee(params->knee);
    setRatio(params->ratio);
    setMakeUpGain(params->makeUpGain);
}

template <typename SampleType>
void CompAhr<SampleType>::applyHardKnee(const juce::dsp::ProcessContextNonReplacing<SampleType>& context) {
    auto output = context.getOutputBlock();
    auto blockSize = output.getNumSamples();
    for (unsigned int n = 0; n < blockSize; n++) {
        output.setSample(0, n, applyHardKneeSample(mEnvelope[n]));
    }
}

template <typename SampleType>
SampleType CompAhr<SampleType>::applyHardKneeSample(SampleType input) {
    SampleType gainDb;
    SampleType envelopeDb = juce::Decibels::gainToDecibels(input);
    if (envelopeDb < mThreshold.db) {
        return (1.0 * mMakeUpGain.linear);
    } else {
        gainDb = mRatio.slope * (envelopeDb - mThreshold.db);
        return juce::Decibels::decibelsToGain(gainDb + mMakeUpGain.db);
    }
}

template <typename SampleType>
void CompAhr<SampleType>::applySoftKnee(const juce::dsp::ProcessContextNonReplacing<SampleType>& context) {
    auto output = context.getOutputBlock();
    auto blockSize = output.getNumSamples();
    for (unsigned int n = 0; n < blockSize; n++) {
        output.setSample(0, n, applySoftKneeSample(mEnvelope[n]));
        //printf("gains ahr %i %f\n", n, output.getSample(0, n));
    }
}

template <typename SampleType>
SampleType CompAhr<SampleType>::applySoftKneeSample(SampleType input) {
    SampleType gainDb;
    SampleType envelopeDb = juce::Decibels::gainToDecibels(input);
    if (envelopeDb < mKnee.bottom) {
        return (1.0 * mMakeUpGain.linear);
    } else if (envelopeDb > mKnee.top) {
        gainDb = mRatio.slope * (envelopeDb - mThreshold.db);
        return juce::Decibels::decibelsToGain(gainDb + mMakeUpGain.db);
    } else {
        gainDb = mRatio.slope * (envelopeDb - mThreshold.db + 0.5f * mKnee.width * (1.0f - cos((envelopeDb - mThreshold.db) / mKnee.width * M_PI)));
        return (juce::Decibels::decibelsToGain(gainDb + mMakeUpGain.db));
    }
}

template <typename SampleType>
void CompAhr<SampleType>::processBlock(const juce::dsp::ProcessContextNonReplacing<SampleType>& context) {
    SampleType input;
    const auto& input_block = context.getInputBlock();
    size_t blockSize = input_block.getNumSamples();
    
    for (size_t n = 0; n < blockSize; n++) {
        input = input_block.getSample(0, (int) n);
        if (input > current_envelope)
            mState = STATE_ATTACK;
        
        switch (mState) {
            case STATE_ATTACK:
                mState = STATE_HOLD;
                mHold.counter = 0;
                current_envelope = mAttack.coefs[0] * current_envelope + mAttack.coefs[1] * input;
                break;
            case STATE_HOLD:
                if (++mHold.counter > mHold.samples)
                    mState = STATE_RELEASE;
                break;
            case STATE_RELEASE:
                mHold.counter = 0;
                current_envelope = mRelease.coefs[0] * current_envelope + mRelease.coefs[1] * input;
                break;
        }
        mEnvelope[n] = current_envelope;
        //printf("mEnvelope[%i]: %f\n", n, mEnvelope[n]);
    }
        
    switch (mKnee.type) {
        case COMP_HARD_KNEE:
            applyHardKnee(context);
            break;
        default:
            applySoftKnee(context);
            break;
    }
    
}

template <typename SampleType>
SampleType CompAhr<SampleType>::processSample(SampleType input) {
    
    if (input > current_envelope)
        mState = STATE_ATTACK;
        
    switch (mState) {
        case STATE_ATTACK:
            mState = STATE_HOLD;
            mHold.counter = 0;
            current_envelope = mAttack.coefs[0] * current_envelope + mAttack.coefs[1] * input;
            break;
        case STATE_HOLD:
            if (++mHold.counter > mHold.samples)
                mState = STATE_RELEASE;
            break;
        case STATE_RELEASE:
            mHold.counter = 0;
            current_envelope = mRelease.coefs[0] * current_envelope + mRelease.coefs[1] * input;
            break;
    }
    
    
    switch (mKnee.type) {
        case COMP_HARD_KNEE:
            return applyHardKneeSample(input);
            break;
        default:
            return applySoftKneeSample(input);
            break;
    }
}

template class CompAhr<float>;
template class CompAhr<double>;
