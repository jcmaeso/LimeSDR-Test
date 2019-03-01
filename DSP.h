//
// Created by calata on 1/03/19.
//

#ifndef LIMESUITERX_DSP_H
#define LIMESUITERX_DSP_H


#include <fftw3.h>
#include "plotter.h"


class DSP {
public:
    DSP(size_t channels,size_t len);

    virtual ~DSP();

    void GenerateFft(float** buffers);
    void ParseAndPlot(float** buffers);

private:
    Plotter* p;
    size_t len;
    size_t channels;
    fftwf_complex** buffs;
    float threshold;
    void LimeParser(float** buffers);
    void calcThreshold(void);
};


#endif //LIMESUITERX_DSP_H
