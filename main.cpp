#include "SdrDevice.h"
#include "plotter.h"
#include "DSP.h"
#include <iostream>
#include <chrono>
#include <cstring>

using namespace std;

const size_t channels = 2;
const size_t len = 1024 * 8;

int main(int argc, char** argv)
{
    //Find devices
    int n;
    DSP dsp(channels,len);
    fftwf_complex* plotbuff1 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    fftwf_complex* plotbuff2 = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * len);
    samplesBuffer * buffs[channels];
    lms_info_str_t serialDev = "serial=00090726074D1F35";
    SdrDevice* sdr1 = new SdrDevice(serialDev,1);
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;
    sdr1->EnableRxChannels();
    sdr1->SetRxLOFrecuency(2.4e9);
    sdr1->SetSampleRate(10e6,4);
    //sdr1->SetTxNormalizedGain(0.7);
    sdr1->SetRxGainDB(0);
    sdr1->CalibrateRx(10e6);
    sdr1->SetRxHfAntenna();
    sdr1->PrintRxAntennas();
    sdr1->SetupRxStream();
    auto buffers = sdr1->GetRxBuffers();
    sdr1->StartRxStream();
    buffs[0] = buffers[0];
    buffs[1] = buffers[1];

    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(100)) //run for 10 seconds
    {
        sdr1->PollGetSamples();
        //Parser to FFT LIb format
        for(int i = 0; i < 1024 * 8; i++){
            plotbuff1[i][0] = buffers[0][2*i];
            plotbuff1[i][1] = buffers[0][2*i+1];
            plotbuff2[i][0] = buffers[1][2*i];
            plotbuff2[i][1] = buffers[1][2*i+1];
        }
        //Print stats every 1s
        if (chrono::high_resolution_clock::now() - t2 > chrono::milliseconds(100))
        {
            t2 = chrono::high_resolution_clock::now();
            //Print stats
            dsp.ParseAndPlot(buffs);
            sdr1->PrintRxStats();
        }
    }


    sdr1->StopAndDestroy();
    free(plotbuff1);
    free(plotbuff2);

    return 0;
}
