/* functions to design parametric equalizer based on 
the audio EQ Cookbook by Robert Bristow-Johnson
https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt

version 1.0
(c) J. Bitzer @ TGM, Jade Hochschule, BSD 3-Clause License
*/

#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
/*
    This enumeration contains all possible error codes that can be returned by the designPeakEqualizer function.
    NO_ERROR: The function executed successfully without any problems.
    SAMPLING_RATE_TOO_LOW: The given sampling rate is lower than required.
    F0_TOO_HIGH: The frequency 'f0' is higher than the limit (fs/2).
    Q_SETTING_OUT_OF_RANGE: The q-factor provided is not within the acceptable range (0.1 - 10).
    GAIN_SETTING_OUT_OF_RANGE: The gain provided is not within the acceptable range (-24dB - 24dB). 
*/
enum EqualizerErrorCode
{
    NO_ERROR = 0,
    SAMPLING_RATE_TOO_LOW = -1,
    F0_TOO_HIGH = -2,
    Q_SETTING_OUT_OF_RANGE = -3,
    GAIN_SETTING_OUT_OF_RANGE = -4
};

/*
    This function designs a peak equalizer filter, given certain parameters.
    @param b The return vector to store the numerator coefficients of the IIR filter.
    @param a The return vector to store the denominator coefficients of the IIR filter.
    @param f0 The center frequency in Hz of the filter.
    @param Q The Q factor, determining the quality of the filter.
    @param gain The desired gain in decibels (dB).
    @param fs The sampling frequency in Hz.

    @return The error code of the function. If the function executed successfully, the return value is NO_ERROR.
            for all other cases, the return value is an error code given in the EqualizerErrorCode enumeration.
*/
EqualizerErrorCode designPeakEqualizer(std::vector<double>& b, std::vector<double>& a, double f0, double Q, double gain, double fs)
{
    if (fs < f0*0.5 && fs < 0)
    {
        return SAMPLING_RATE_TOO_LOW;
    }
    if (f0>fs*0.5)
    {
        return F0_TOO_HIGH;
    }
    if (Q < 0.1 || Q > 10)
    {
        return Q_SETTING_OUT_OF_RANGE;
    }
    if (gain < -24.0 || gain > 24.0)
    {
        return GAIN_SETTING_OUT_OF_RANGE;
    }

    double w0 = 2.0 * M_PI * f0 / fs;
    double alpha = sin(w0) / (2.0 * Q);
    double A = pow(10.0, gain / 40.0);

    b.resize(3);
    a.resize(3);
    
    double norm = 1.0 + alpha / A;

    b[0] = (1.0 + alpha * A)/norm;
    b[1] = (-2.0 * cos(w0))/norm;
    b[2] = (1.0 - alpha * A)/norm;

    a[0] = 1.0;
    a[1] = (-2.0 * cos(w0))/norm;
    a[2] = (1.0 - alpha / A)/norm;

    return NO_ERROR;
}