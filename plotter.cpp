//
// Created by calata on 14/02/19.
//
#include <cstdio>
#include "plotter.h"
#include <math.h>
#include <vector>


#define abs(n1,n2) sqrt(pow(n1,2)+pow(n2,2))

bool block = false;
bool getBlock(){
    return block;
}
void setBlock(bool state){
    block = state;
}

void Plotter::plotAbs(int windowNumber, fftwf_complex *data, int len) {
    std::vector<double> time;
    std::vector<double> plotValues;

    plotValues = genAbs(data,len);
    //Generate timeStamps
    for(double i = 0; i < len; i++) {
        time.push_back(i);
    }
    plot(windowNumber,&plotValues);
}

Plotter::Plotter(int numberOfWindows) {
    this->numberOfWindows = numberOfWindows;
    for(size_t i = 0; i < this->numberOfWindows; i++){
        windows.emplace_back(new Gnuplot());
    }
}

std::vector<double> Plotter::genAbs(fftwf_complex *data, int len) {
    std::vector<double> absValues;
    for(size_t i = 0; i < len; i++){
        absValues.push_back(abs(data[i][0],data[i][1]));
    }
    return absValues;
}

Plotter::~Plotter() {
    for (size_t i = 0; i < numberOfWindows; i++) {
        *(windows[i]) << ("exit");
        delete windows[i];
    }
}

void Plotter::plot(int windowNumber, std::vector<double>* dataToPlot) {
    Gnuplot* pGnuplot;
    if(windowNumber > this->numberOfWindows){
        printf("Error in plotting impossible window number");
        return;
    }
    pGnuplot = windows[windowNumber];
    (*pGnuplot) << "plot '-' with lines notitle \n";
    pGnuplot->send1d(*dataToPlot);

}

void Plotter::plotFft(int windowNumber, fftwf_complex *data, int len) {
    std::vector<double>* plotValues;
    //printf("Bloqueo \n");
    //setBlock(true);
    plotValues = genFft(data,len);
    //TODO: GENERATE FREQUENCIES
    plot(windowNumber,plotValues);
    delete plotValues;
    //setBlock(false);
    //printf("Desbloqueo \n");
}


std::vector<double>* Plotter::genFft(fftwf_complex *data, int len) {
    fftwf_plan my_plan;
    std::vector<double>* fftAbs = new std::vector<double>();
    fftwf_complex* fftData = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*len);
    my_plan = fftwf_plan_dft_1d(len, data, fftData, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(my_plan);
    fftwf_destroy_plan(my_plan);
    int c =  (int) floor((float)len/2);
    for(size_t i = 0; i < len; i ++){
        fftAbs->push_back(abs(fftData[i][0],fftData[i][1]));
    }
    return fftAbs;
}

void Plotter::swap(void *u1, void *u2, size_t i) {
    uint8_t temp;		// address for buffering one byte
    uint8_t * a = (uint8_t*)u1;	// bytewise moving pointer to va
    uint8_t * b = (uint8_t*)u2;	// bytewise moving pointer to vb
    // Counting down from the length of the objects to 1
    // Reaching zero stops the while loop, all addresses
    // copied one to one.
    while ( i-- ) temp = a[i], a[i] = b[i], b[i] = temp;

}

void Plotter::closePlots() {
    for (size_t i = 0; i < numberOfWindows; i++) {
        *(windows[i]) << ("reset");
    }
}

