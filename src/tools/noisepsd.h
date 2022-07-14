#ifndef NOISEPSD_H
#define NOISEPSD_H

#include "tools/equation.h"
namespace openmldb {
namespace tools {
const int MAX_SECT_SAMP_NUM=220000;
//const int MAX_SECT_SAMP_NUM=1300;

#define MAX_SITE_NUM 200

#define MAX_FREQ_CENTER_NUM 200
#define MAX_SECT_NUM 1000   //48*17
#define MAX_HOUR_SECT 50   //48*17
#define MAX_PRO_HOUR  50   //48*17
#define MAX_ZERO_POLE 100   //48*17
#define MAX_SITESEL_KEYDATA 7   //台站评价关键数据个数
class CPsdProMainParams
{
public:
    int LenCalPsdStep;   //计算Psd值的步长，单位：秒
    double LenCalPsdWin;    //计算Psd值的窗长2^17，单位：秒
    double LenWaveBuffer;   //波形缓存的长度，单位：秒
    double MinFreq;         //最低频率，单位Hz
    double MaxFreq;         //最高频率，单位Hz
};

class CTranFunction  //仪器传递函数
{
public:
    char Type[10];
    char Signal_input_unit[10];
    char Signal_output_unit[10];
    double Normalization_factor;
    double Normalization_frequency;
    double Sensitivity0;  //第0级灵敏度
    double Sensitivity1;  //第1级灵敏度
    double Sensitivity2;  //第2级灵敏度
    int zeroNum;
    int poleNum;
    double zeroRealList[MAX_ZERO_POLE];
    double zeroImagList[MAX_ZERO_POLE];
    double poleRealList[MAX_ZERO_POLE];
    double poleImagList[MAX_ZERO_POLE];
    double AmpFactor[MAX_FREQ_CENTER_NUM];

};


class CNoisePsd {
 public:
  explicit CNoisePsd();
  ~CNoisePsd();

    void GetFreqCenter(double MinFrequence, double MaxFrequence);

    void PsdCalculate(const std::string& device, double* CurData,int DataNum,int sampRate,double Sensitivity,/*double* AmpFactor,*/double* PsdFreqCenter);

    int GetFreqNum() { return m_FreqNum; }

    //int m_LenCalPsdStep;        //计算Psd值的步长，单位：秒
    //double m_LenCalPsdWin;      //计算Psd值的窗长，单位：秒
    //int m_LenWaveBuffer;        //波形缓存的长度，单位：秒
    double m_OctaveStep;        //对线性坐标中的Psd值，在对数坐标中以1/9倍频程为“步长”进行平均
    double m_OctaveWin;         //对线性坐标中的Psd值，在对数坐标中以1/9倍频程为“窗长”进行平均
    //double m_MinFreq;           //中心频率的最小值
    //double m_MaxFreq;           //中心频率的最da值

    int m_SectSampNum;   //每个数据段的采样点个数=窗长*采样率,为2^m_SectPower
    int m_SectPower;     //采样点个数=2^m_SectPower
    double m_StandardFreq;  ////国标中标准中心频率, 为1000Hz
    double m_FreqLow[MAX_FREQ_CENTER_NUM],m_FreqCenter[MAX_FREQ_CENTER_NUM],m_FreqHigh[MAX_FREQ_CENTER_NUM];  //f0中心频率；f1
    double m_PsdData[MAX_FREQ_CENTER_NUM];
    int m_FreqExpStart;      //倍频程滤波，最低频端对应的频率指数x

    double m_RBW;
    int m_FreqNum;       //频点数
    float* m_WaveDataFloat; //波形数据
    double* m_WaveDataALL; //波形数据
    double* m_PsdDataALL; //进行对数坐标离散前的PSD值
    double* m_FourierReal;  //傅立叶变换的实部
    double* m_FourierImag;  //傅立叶变换的需部


    double zeroRealList[MAX_ZERO_POLE];
    double zeroImagList[MAX_ZERO_POLE];
    double poleRealList[MAX_ZERO_POLE];
    double poleImagList[MAX_ZERO_POLE];
    double AmpFactor[MAX_FREQ_CENTER_NUM];
};
}  // namespace tools
}  // namespace openmldb
#endif // NOISEPSD_H
