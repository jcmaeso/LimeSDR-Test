#include "SdrDevice.h"
#include "plotter.h"
#include "DSP.h"
#include <iostream>
#include <chrono>
#include <cstring>

using namespace std;

const size_t channels = 4;
const size_t len = 1024 * 8;

int main(int argc, char** argv)
{
    //Find devices
    int n;
    DSP dsp(channels,len);
    Plotter plotter(4);
    fftwf_complex* plotbuff1 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    fftwf_complex* plotbuff2 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    fftwf_complex* plotbuff3 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    fftwf_complex* plotbuff4 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);

    samplesBuffer * buffs[channels];
    lms_info_str_t serialDev1 = "serial=00090726074D1F35";
    lms_info_str_t serialDev2 = "serial=0009072C02881C32";
    SdrDevice* sdr1 = new SdrDevice(serialDev1,1);
    SdrDevice* sdr2 = new SdrDevice(serialDev2,2);

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    sdr1->EnableRxChannels();
    sdr2->EnableRxChannels();
    sdr1->SetRxLOFrecuency(2.4e9);
    sdr2->SetRxLOFrecuency(2.4e9);
    sdr1->SetSampleRate(10e6,4);
    sdr2->SetSampleRate(10e6,4);
    //sdr1->SetTxNormalizedGain(0.7);
    sdr1->SetTxNormalizedGain(0.7);
    sdr2->SetTxNormalizedGain(0.7);
    sdr1->SetRxHfAntenna();
    sdr2->SetRxHfAntenna();
    sdr1->CalibrateRx(10e6);
    sdr2->CalibrateRx(10e6);
    sdr1->SetupRxStream();
    sdr2->SetupRxStream();
    auto buffers1 = sdr1->GetRxBuffers();
    auto buffers2 = sdr2->GetRxBuffers();
    sdr1->StartRxStream();
    sdr2->StartRxStream();
    buffs[0] = buffers1[0];
    buffs[1] = buffers1[1];
    buffs[2] = buffers2[0];
    buffs[3] = buffers2[1];

    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(100)) //run for 10 seconds
    {
        sdr1->PollGetSamples();
        sdr2->PollGetSamples();
        //Parser to FFT LIb format
        for(int i = 0; i < 1024 * 8; i++){
            plotbuff1[i][0] = buffers1[0][2*i];
            plotbuff1[i][1] = buffers1[0][2*i+1];
            plotbuff2[i][0] = buffers1[1][2*i];
            plotbuff2[i][1] = buffers1[1][2*i+1];
            plotbuff3[i][0] = buffers2[0][2*i];
            plotbuff3[i][1] = buffers2[0][2*i+1];
            plotbuff4[i][0] = buffers2[1][2*i];
            plotbuff4[i][1] = buffers2[1][2*i+1];
        }
        //Print stats every 1s
        if (chrono::high_resolution_clock::now() - t2 > chrono::milliseconds(100))
        {
            t2 = chrono::high_resolution_clock::now();
            //Print stats
            plotter.plotFft(0,plotbuff1,1024 * 8);
            plotter.plotFft(1,plotbuff2,1024 * 8);
            plotter.plotFft(2,plotbuff3,1024 * 8);
            plotter.plotFft(3,plotbuff4,1024 * 8);
            sdr1->PrintRxStats();
        }
    }


    sdr1->StopAndDestroy();
    sdr2->StopAndDestroy();
    free(plotbuff1);
    free(plotbuff2);

    return 0;
}
