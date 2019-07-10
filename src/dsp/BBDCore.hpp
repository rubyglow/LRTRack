/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */
#pragma once

#include "DSPEffect.hpp"

namespace lrt {

/**
 * @brief Helper class representing a fractional-index access to an array with interpolation
 * @tparam T Floating point type
 */
template<class T>
struct FracVector {
    std::vector<T> data;


    FracVector(int size) {
        data.resize(size);
    }


    T get(float position) {
        // get the fractional part of the float
        int c0 = (int) truncf(position);
        T c1 = position - (T) c0;
        // get counterpart
        T c2 = 1 - c1;
        T v1 = data[c0];
        T v2 = data[c0 + 1];

        return c1 * v1 + c2 + v2;
    }


    void set(float position, T value) {
        // get the fractional part of the float
        int c0 = (int) truncf(position);
        T c1 = position - (T) c0;
        // get counterpart
        T c2 = 1 - c1;

        data[c0] = value * c1;
        data[c0 + 1] = value * c2;
    }

};


/**
 * Interpolation type used for value computation
 */
enum BBDInterpolation {
    NONE,
    LINEAR
};


/**
 * @brief Bucket Brigade Device model
 */
struct BBDCore : DSPEffect {
private:
    float *data;

    // current index in array and memory stages
    unsigned int index, stages;

    // delay length in seconds
    float length;

    // current sampling frequency
    float samplefrq;


    float stepsize;
public:

    float in, out;


    /**
     * @brief Construct a new BBD memory core object
     * @param size Size of the memory chip in bytes
     */
    BBDCore(float sr, unsigned int size = 4096) : DSPEffect(sr) {
        data = new float[size];
        index = 0;
        this->stages = size;
    }


    virtual ~BBDCore() {
        delete[] data;
    }


    void init() override {
        DSPEffect::init();
    }


    /**
     * @brief Compute basic variables depending on the setup
     */
    void invalidate() override {
        samplefrq = stages / length;
        stepsize = sr / samplefrq;

    }


    void process() override {
        DSPEffect::process();
    }


    /**
     * @brief Set the delay length in seconds
     * @param length
     */
    inline void setLength(float length) {
        this->length = length;
        invalidate();
    }

};


}
