//
// Created by calata on 1/03/19.
//

#include "DSP.h"


DSP::DSP(size_t channels,size_t len) {
    size_t channel;
    this->channels = channels;
    this->len = len;
    buffs = new fftwf_complex* [channels];
    p = new Plotter(channels);
    for(channel = 0; channel < channels; channel++){
        buffs[channel] = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    }
}

void DSP::GenerateFft(float **buffers) {
    LimeParser(buffers);

}

void DSP::LimeParser(float **buffers) {
    for(size_t channel = 0; channel < channels; channel++) {
        for (size_t i = 0; i < len; i++) {
            buffs[channel][i][0] = buffers[0][2 * i];
            buffs[channel][i][1] = buffers[0][2 * i + 1];
        }
    }
}

DSP::~DSP() {
    for(size_t channel = 0; channel < channels; channel++) {
        delete[] buffs[channel];
    }
    delete p;
}

void DSP::calcThreshold(void) {

}

void DSP::ParseAndPlot(float **buffers) {
    LimeParser(buffers);
    for(size_t channel = 0; channel < channels; channel++){
        p->plotFft(channel,buffs[channel],len);
    }
}
