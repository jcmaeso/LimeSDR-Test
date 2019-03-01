//
// Created by calata on 1/03/19.
//

#ifndef LIMESUITERX_SDRDEVICE_H
#define LIMESUITERX_SDRDEVICE_H

#include "lime/LimeSuite.h"

typedef float samplesBuffer;

class SdrDevice {
public:
    SdrDevice(char* serialId, int deviceId);
    int EnableRxChannels();
    int EnableTxChannels();
    int SetRxLOFrecuency(double LO_frequency);
    int SetTxLOFrecuency(double LO_frequency);
    int SetRxNormalizedGain(double gain);
    int SetTxNormalizedGain(double gain);
    int SetSampleRate(double rate, size_t oversampling);
    int SetupRxStream();
    int SetupTxStream();
    void StartRxStream();
    samplesBuffer** GetRxBuffers();
    void PollGetSamples();
    void PrintRxStats();
    void StopAndDestroy();
    void CalibrateRx(double bw);
    void PrintRxAntennas();
    void PrintTxAntennas();
    void SetRxHfAntenna();
    void SetRxWfAntenna();
    void SetRxLfAntenna();
    void SetRxLpfBandwidth(double bw);
    void SetTxLpfBandwidth(double bw);
    void SetRxGainDB(double gain);
    void SetTxGainDB(double gain);
private:
    int id;
    int bufferRxSize;
    int rxChannels, txChannels;
    bool connected;
    lms_device_t* device;
    lms_info_str_t deviceInfo;
    lms_stream_t* rxStreams;
    lms_stream_t* txStreams;
    lms_stream_meta_t rxMetadata;
    lms_stream_meta_t txMetadata;
    lms_name_t antennaList[10];
    samplesBuffer** rxBuffers;
    samplesBuffer** txBuffers;
    void errorLogger(int errorCode);
    void PrintAntennas(bool way);
    int EnableChannels(bool way);
    int SetNormalizedGain(bool way, double gain);
    int SetLOFrecuency(bool way,double LO_frequency);
    void SetAntenna(bool way, size_t  antenna);
    void SetLpfBandwidth(bool way, double bw);
    void SetGainDB(bool way, double gain);
};


#endif //LIMESUITERX_SDRDEVICE_H
