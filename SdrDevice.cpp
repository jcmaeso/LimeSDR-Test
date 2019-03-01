//
// Created by calata on 1/03/19.
//

#include "SdrDevice.h"
#include <cstring>
#include <iostream>

SdrDevice::SdrDevice(char *serialId, int deviceId) {
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    connected = false;
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        errorLogger(1);

    if (n < 1) {
        return;
    }

    for(int i = 0; i < 8; i++){
        if(strstr(list[i],serialId) != NULL){
            strcpy(deviceInfo,list[i]);
        }
    }
    if (LMS_Open(&device, deviceInfo, NULL)){
        errorLogger(2);
        return;
    }
    connected = true;
    if (LMS_Init(device) != 0) {
        errorLogger(3);
        return;
    }
    rxChannels = LMS_GetNumChannels(device, LMS_CH_RX);
    txChannels = LMS_GetNumChannels(device, LMS_CH_TX);
    std::cout << "Number of RX channels: " << rxChannels << std::endl;
    std::cout << "Number of TX channels: " << txChannels << std::endl;

}

void SdrDevice::errorLogger(int errorCode) {
    switch(errorCode){
        case 1:
            std::cout << "Error connecting to device, NOT CONNECTED" << std::endl;
            break;
        case 2:
            std::cout << "Error openning device" << std::endl;
            break;
        case 10:
            std::cout << "Oversampling invalid rate. Only 0,1,2,4,8,16,32 are valid" << std::endl;
            break;
        case 14:
            std::cout << "RX LPF invalid bw. Only 0.75,2,5,15.5,35,75 Mhz are valid" << std::endl;
            break;
        case 15:
            std::cout << "TX LPF invalid bw. Only 2.5,5,10,20,40,75 Mhz are valid" << std::endl;
            break;
        case 18:
            std::cout << "RX gain invalid. Range is from 0 to 89 dB" << std::endl;
            break;
        case 19:
            std::cout << "TX gain invalid. Range is from 0 to 70 dB" << std::endl;
            break;
        default:
            std::cout << "Unknown error in device" << std::endl;
    }
}

int SdrDevice::EnableChannels(bool way) {
    int channel;
    int nChannels = way == LMS_CH_RX ? rxChannels : txChannels;
    int error = 0;
    for(channel = 0; channel < nChannels; channel++) {
        if (LMS_EnableChannel(device, way, channel, true) != 0) {
            errorLogger(way == LMS_CH_RX ? 4 : 5);
            error |= 1 << channel;
        }
    }
    return error;
}

int SdrDevice::EnableRxChannels() {
    return EnableChannels(LMS_CH_RX);
}

int SdrDevice::EnableTxChannels() {
    return EnableChannels(LMS_CH_TX);
}

int SdrDevice::SetLOFrecuency(bool way,double LO_frequency) {
    int channel;
    int nChannels = way == LMS_CH_RX ? rxChannels : txChannels;
    int error = 0;
    for(channel = 0; channel < nChannels; channel++) {
        if (LMS_SetLOFrequency(device, way, channel, LO_frequency) != 0) {
            errorLogger(way == LMS_CH_RX ? 6 : 7);
            error |= 1 << channel;
        }
    }
    return error;
}

int SdrDevice::SetRxLOFrecuency(double LO_frequency) {
    return SetLOFrecuency(LMS_CH_RX,LO_frequency);
}

int SdrDevice::SetTxLOFrecuency(double LO_frequency) {
    return SetLOFrecuency(LMS_CH_TX,LO_frequency);
}

int SdrDevice::SetRxNormalizedGain(double gain) {
    return SetNormalizedGain(LMS_CH_RX,gain);
}

int SdrDevice::SetTxNormalizedGain(double gain) {
    return SetNormalizedGain(LMS_CH_TX,gain);
}
int SdrDevice::SetNormalizedGain(bool way, double gain) {
    int channel;
    int nChannels = way == LMS_CH_RX ? rxChannels : txChannels;
    int error = 0;
    for(channel = 0; channel < nChannels; channel++) {
        if (LMS_SetNormalizedGain(device, way, channel, gain) != 0) {
            errorLogger(way == LMS_CH_RX ? 8 : 9);
            error |= 1 << channel;
        }
    }
    return error;
}

int SdrDevice::SetSampleRate(double rate, size_t oversampling) {
    //Check oversampling rate
    if(oversampling > 32 && !((oversampling & (oversampling - 1)) == 0)) {
        errorLogger(10);
    }
}

int SdrDevice::SetupRxStream() {
    int error;
    int channel = 0;
    bufferRxSize = 1024 * 8;
    rxStreams = new lms_stream_t[rxChannels];
    rxBuffers = new samplesBuffer*[rxChannels];
    for(channel = 0; channel < rxChannels; channel++){
        rxStreams[channel].channel = channel; //channel number
        rxStreams[channel].fifoSize = 1024 * 1024; //fifo size in samples
        rxStreams[channel].throughputVsLatency = 0.5; //some middle ground
        rxStreams[channel].isTx = false; //RX channel
        rxStreams[channel].dataFmt = lms_stream_t::LMS_FMT_F32; //Floats
        if (LMS_SetupStream(device, &rxStreams[channel]) != 0){
            errorLogger(11);
            error |= 1 << channel;
        }
        rxBuffers[channel] = new samplesBuffer[bufferRxSize*2]; //I-Q SAMPLES
    }
    //Metadata config
    rxMetadata.flushPartialPacket = false; //currently has no effect in RX
    rxMetadata.waitForTimestamp = false; //currently has no effect in RX
    return error;
}

int SdrDevice::SetupTxStream() {
    return 0;
}

void SdrDevice::StartRxStream() {
    int channel = 0;
    for(channel = 0; channel < rxChannels; channel++){
        LMS_StartStream(&rxStreams[channel]);
    }
}

samplesBuffer **SdrDevice::GetRxBuffers() {
    return rxBuffers;
}

void SdrDevice::PollGetSamples() {
    int channel = 0;
    int samplesRead = 0; //Case we need it for deep debugging
    for(channel = 0; channel < rxChannels; channel++){
        samplesRead = LMS_RecvStream(&rxStreams[channel], rxBuffers[channel], bufferRxSize, &rxMetadata, 1000);
    }
}

void SdrDevice::StopAndDestroy() {
    int channel = 0;
    for (channel = 0; channel < rxChannels; channel++)
    {
        LMS_StopStream(&rxStreams[channel]); //stream is stopped but can be started again with LMS_StartStream()
        LMS_DestroyStream(device, &rxStreams[channel]); //stream is deallocated and can no longer be used
        delete[] rxBuffers [channel];
    }
/*
    for (channel = 0; channel < txChannels; channel++)
    {
        LMS_StopStream(&txStreams[channel]);
        LMS_DestroyStream(device, &txStreams[channel]);
        delete[] txBuffers [channel];
    }*/
    //Close device
    LMS_Close(device);
}

void SdrDevice::PrintRxStats() {
    lms_stream_status_t status;
    LMS_GetStreamStatus(rxStreams, &status); //Obtain RX stream stats
    std::cout << "RX rate: " << status.linkRate / 1e6 << " MB/s\n"; //link data rate (both channels))
    std::cout << "RX 0 FIFO: " << 100 * status.fifoFilledCount / status.fifoSize << "%" << std::endl; //percentage of RX 0 fifo filled
}

void SdrDevice::CalibrateRx(double bw) {
    int channel = 0;
    for (channel = 0; channel < rxChannels; channel++) {
        LMS_Calibrate(device, LMS_CH_RX, channel, bw, 0);
    }
}

void SdrDevice::PrintRxAntennas() {
    PrintAntennas(LMS_CH_RX);
}
void SdrDevice::PrintTxAntennas() {
    PrintAntennas(LMS_CH_TX);
}

void SdrDevice::PrintAntennas(bool way) {
    int index,channel;
    for(channel = 0; channel < (way==LMS_CH_RX ? rxChannels : txChannels); channel++){
        index = LMS_GetAntennaList(device,LMS_CH_RX,channel,antennaList);
        std::cout << (way==LMS_CH_RX ? "RX" : "TX") << " antennas from SDR " << id  << "in ch " << channel << std::endl;
        for (int i = 0; i < index; i++)
            std::cout << i << ": " << antennaList[i] << std::endl;
        index = LMS_GetAntenna(device, LMS_CH_RX, 0);
        std::cout << "Selected" << (way==LMS_CH_RX ? "RX" : "TX") << "antenna from SDR" << id << " in channel " << channel << " is " << antennaList[index] << std::endl;
    }
}

void SdrDevice::SetRxHfAntenna() {
    SetAntenna(LMS_CH_RX,LMS_PATH_LNAH);
}

void SdrDevice::SetRxWfAntenna() {
    SetAntenna(LMS_CH_RX,LMS_PATH_LNAW);
}

void SdrDevice::SetRxLfAntenna() {
    SetAntenna(LMS_CH_RX,LMS_PATH_LNAL);
}

void SdrDevice::SetAntenna(bool way, size_t antenna) {
    int channel;
    for(channel = 0; channel < (way==LMS_CH_RX ? rxChannels : txChannels); channel++){
        if(LMS_SetAntenna(device,way,channel,antenna) != 0){
            errorLogger(12);
        }
    }
}

void SdrDevice::SetRxLpfBandwidth(double bw) {
    if(bw != 75e6 && bw != 35e6 && bw != 15.5e6 && bw != 5e6 && bw != 2e6 && bw != 0.75e6){
        errorLogger(14);
        return;
    }
    SetLpfBandwidth(LMS_CH_RX,bw);
}

void SdrDevice::SetTxLpfBandwidth(double bw) {
    if(bw != 75e6 && bw != 40e6 && bw != 20e6 && bw != 10e6 && bw != 5e6 && bw != 2.5e6){
        errorLogger(15);
        return;
    }
    SetLpfBandwidth(LMS_CH_TX,bw);
}

void SdrDevice::SetLpfBandwidth(bool way, double bw) {
    int channel;
    for(channel = 0; channel < (way==LMS_CH_RX ? rxChannels : txChannels); channel++){
        if(LMS_SetLPFBW(device,way,channel,bw) != 0){
            errorLogger(16);
        }
    }
}

void SdrDevice::SetGainDB(bool way, double gain) {
    int channel;
    for (channel = 0; channel < (way == LMS_CH_RX ? rxChannels : txChannels); channel++) {
        if (LMS_SetGaindB(device, way, channel, gain) != 0) {
            errorLogger(17);
        }
    }
}

void SdrDevice::SetRxGainDB(double gain) {
    if(gain > 89 || gain < 0){
        errorLogger(18);
        return;
    }
    SetGainDB(LMS_CH_RX,gain);
}

void SdrDevice::SetTxGainDB(double gain) {
    if(gain > 70 || gain < 0){
        errorLogger(19);
        return;
    }
    SetGainDB(LMS_CH_TX,gain);
}



