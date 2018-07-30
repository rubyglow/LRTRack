#pragma once

#include <string.h>
#include "dsp/ringbuffer.hpp"
#include "dsp/fir.hpp"


namespace dsp {

    /**
     * @brief Base class for all signal processors
     */
    struct DSPEffect {

    protected:


    public:

        float sr = 44100.0;


        explicit DSPEffect(float sr) : sr(sr) {
            init();
        }


        float getSamplerate() const {
            return sr;
        }


        void setSamplerate(float sr) {
            DSPEffect::sr = sr;
            invalidate();
        }


        virtual void init() {};


        /**
         * @brief Method for mark parameters as invalidate to trigger recalculation
         */
        virtual void invalidate() {};


        /**
         * @brief Process one step and return the computed sample
         * @return
         */
        virtual void process() {};
    };


    struct Decimator {
        float inBuffer[512];
        float kernel[512];
        int inIndex;
        int oversample, quality;
        float cutoff = 0.9;


        Decimator(int oversample, int quality) {
            Decimator::oversample = oversample;
            Decimator::quality = quality;

            rack::boxcarLowpassIR(kernel, oversample * quality, cutoff * 0.5f / oversample);
            rack::blackmanHarrisWindow(kernel, oversample * quality);
            reset();
        }


        void reset() {
            inIndex = 0;
            memset(inBuffer, 0, sizeof(inBuffer));
        }


        /** `in` must be length OVERSAMPLE */
        float process(float *in) {
            // Copy input to buffer
            memcpy(&inBuffer[inIndex], in, oversample * sizeof(float));
            // Advance index
            inIndex += oversample;
            inIndex %= oversample * quality;
            // Perform naive convolution
            float out = 0.f;
            for (int i = 0; i < oversample * quality; i++) {
                int index = inIndex - 1 - i;
                index = (index + oversample * quality) % (oversample * quality);
                out += kernel[i] * inBuffer[index];
            }
            return out;
        }
    };


    struct Upsampler {
        float inBuffer[512];
        float kernel[512];
        int inIndex;
        int oversample, quality;
        float cutoff = 0.9;


        Upsampler(int oversample, int quality) {
            Upsampler::oversample = oversample;
            Upsampler::quality = quality;

            rack::boxcarLowpassIR(kernel, oversample * quality, cutoff * 0.5f / oversample);
            rack::blackmanHarrisWindow(kernel, oversample * quality);
            reset();
        }


        void reset() {
            inIndex = 0;
            memset(inBuffer, 0, sizeof(inBuffer));
        }


        /** `out` must be length OVERSAMPLE */
        void process(float in, float *out) {
            // Zero-stuff input buffer
            inBuffer[inIndex] = oversample * in;
            // Advance index
            inIndex++;
            inIndex %= quality;
            // Naively convolve each sample
            // TODO replace with polyphase filter hierarchy
            for (int i = 0; i < oversample; i++) {
                float y = 0.f;
                for (int j = 0; j < quality; j++) {
                    int index = inIndex - 1 - j;
                    index = (index + quality) % quality;
                    int kernelIndex = oversample * j + i;
                    y += kernel[kernelIndex] * inBuffer[index];
                }
                out[i] = y;
            }
        }
    };


/**
 * @brief NEW oversampling class
 */
    template<int CHANNELS>
    struct Resampler {
        float up[CHANNELS][512] = {};
        float data[CHANNELS][512] = {};

        Decimator decimator[CHANNELS];
        Upsampler interpolator[CHANNELS];

        int oversample;


        /**
         * @brief Constructor
         * @param factor Oversampling factor
         */
        Resampler(int oversample) {
            Resampler::oversample = oversample;
        }


        int getFactor() {
            return oversample;
        }


        /**
         * @brief Create up-sampled data out of two basic values
         */
        void doUpsample(int channel, float in) {
            interpolator[channel].process(in, up[channel]);
        }


        /**
         * @brief Downsampled data from a given channel
         * @param channel Channel to proccess
         * @return Downsampled point
         */
        float getDownsampled(int channel) {
            return decimator[channel].process(data[channel]);
        }


        /**
         * @brief Upsampled data from a given channel
         * @param channel Channel to retrieve
         * @return Pointer to the upsampled data
         */
        float *getUpsampled(int channel) {
            return up[channel];
        }
    };

}