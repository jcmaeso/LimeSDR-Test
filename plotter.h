//
// Created by calata on 14/02/19.
//

#ifndef SOAPY2RXCPP_PLOTTER_H
#define SOAPY2RXCPP_PLOTTER_H

#include "gnuplot-iostream.h"
#include <fftw3.h>

bool getBlock();
void setBlock(bool state);

class Plotter {
public:
    Plotter(int numberOfWindows);
    void plotAbs(int windowNumber, fftwf_complex* data, int len);
    void plotFft(int windowNumber, fftwf_complex* data, int len);
    void closePlots();
    void justPlot(int windowNumber, std::vector<double> *dataToPlot);
    ~Plotter();
private:
    void swap(void *u1, void *u2, size_t i);
    void plot(int windowNumber, std::vector<double>* dataToPlot);
    std::vector<double> genAbs(fftwf_complex* data,int len);
    std::vector<double>* genFft(fftwf_complex* data,int len);
    int numberOfWindows;
    std::vector<Gnuplot*> windows;
};


#endif //SOAPY2RXCPP_PLOTTER_H
