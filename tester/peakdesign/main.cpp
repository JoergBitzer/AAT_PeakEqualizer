#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#include "EqualizerDesign.h"

int main()
{
    std::vector<double> b, a;
    double f0 = 2000.0;
    double Q = 1.0;
    double gain = 8.0;
    double fs = 44100.0;

    EqualizerErrorCode errorCode = designPeakEqualizer(b, a, f0, Q, gain, fs);

    if (errorCode == NO_ERROR)
    {
        std::cout << "b = [" << b[0] << ", " << b[1] << ", " << b[2] << "]" << std::endl;
        std::cout << "a = [" << a[0] << ", " << a[1] << ", " << a[2] << "]" << std::endl;
    }
    else
    {
        std::cout << "Error code: " << errorCode << std::endl;
    }

    return 0;
}