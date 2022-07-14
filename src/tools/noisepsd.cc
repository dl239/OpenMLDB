#include <cmath>
#include <cstdio>
#include "base/strings.h"
#include "tools/equation.h"
#include "tools/noisepsd.h"
namespace openmldb {
namespace tools {

constexpr double PI = 3.141592653589793;

CNoisePsd::CNoisePsd() {
    m_WaveDataFloat=new float[MAX_SECT_SAMP_NUM];
    m_WaveDataALL=new double[MAX_SECT_SAMP_NUM];
    m_PsdDataALL=new double[MAX_SECT_SAMP_NUM];
    m_FourierReal=new double[MAX_SECT_SAMP_NUM];
    m_FourierImag=new double[MAX_SECT_SAMP_NUM];

    m_StandardFreq=1000.0;
}

CNoisePsd::~CNoisePsd() {
    //m_DataBasePro.DisConnectDB();
    delete[] m_WaveDataFloat;
    delete[] m_WaveDataALL;
    delete[] m_PsdDataALL;
    delete[] m_FourierReal;
    delete[] m_FourierImag;

}

void CNoisePsd::PsdCalculate(const std::string& device, double* CurData,int DataNum,int sampRate,double Sensitivity,/*double* AmpFactor,*/double* PsdFreqCenter) {
    int i,j;
    double samp=(double)sampRate;
    double Dt=1.0/samp;     //每个采样点的时间间隔
    int secSampNum = 0;     //每段采样点的个数，为2^n
    int secPower = 0;
    for(i=0;i<100;i++)
    {
        int sampNum=std::pow(2.0,(double)i);
        if(sampNum>DataNum)
        {
            secSampNum=sampNum/2;   //满足反映最大周期的最小2的整数倍
            secPower=i-1;
            break;
        }
        else if(sampNum==DataNum)
        {
            secSampNum=sampNum;   //满足反映最大周期的最小2的整数倍
            secPower=i;
            break;
        }
    }
    double Tr=secSampNum*Dt;          //每段的时间长度

    //处理步骤3：为了减少长期成份的影响，要对原始数据进行零飘与线性长周期趋势去除。

    //求均值
    //average slope method
    double DataSum=0;
    double DataAvgSum=0;
    for(i=0;i<secSampNum;i++)
    {
        m_PsdDataALL[i]=0;
        DataAvgSum+=CurData[i];
        if(i<(secSampNum/3))		DataSum-=CurData[i]*Dt;
        if(i>=(secSampNum*2/3))	DataSum+=CurData[i]*Dt;
    }
    double DataAve=DataAvgSum/(double)secSampNum;  //均值
    double Tlp=DataSum*(double)(9.0/(2.0*Tr*Tr));
    //去零漂与长周期成份
    //处理步骤4：为了抑制FFT变换的边界效应，对两端10%的数据进行正弦衰减。
    for(i=0;i<secSampNum;i++)
    {
        CurData[i]-=(int)(DataAve+Tlp*(i*Dt-Tr/2));
        if(i>=0 && i<=secSampNum/10)
        {
            CurData[i]=(int)((double)CurData[i]*sin(PI/2*10/Tr*i*Dt));
        }
        if(i>=secSampNum*9/10 && i<secSampNum)
        {
            CurData[i]=(int)((double)CurData[i]*sin(PI/2*10/Tr*(Tr-i*Dt)));
        }
    }
    //处理步骤5：计算Fourier变换
    //CurData返回后存放模，m_PsdDataALL返回后存放幅角
    CEquation::kbfft(CurData,m_PsdDataALL,secSampNum,secPower,m_FourierReal,m_FourierImag,0,1);
    //处理步骤5：计算PSD
    for(i=0;i<secSampNum/2;i++)
    {
        //FftMoSqure[i]=pr[i]*pr[i];
        m_PsdDataALL[i]=2.0*Dt*CurData[i]*CurData[i]/(double)secSampNum;
        m_PsdDataALL[i]*=1.142857;          //10%正弦衰减
    }

    //计算各频点的PSD值
    for(i=0;i<m_FreqNum;i++)
    {
        PsdFreqCenter[i]=0;
        int PsdSum=0;
        for(j=0;j<secSampNum/2;j++)
        {
            double MyFrequence=(double)(j+1)*(double)samp/(double)secSampNum;  //当前点的频率
            if(MyFrequence>=this->m_FreqLow[i] && MyFrequence<m_FreqHigh[i])      //在该中心频率的频带宽度内
            {
                PsdFreqCenter[i]+=m_PsdDataALL[j];  //累加
                PsdSum++;
            }
        }
        if(PsdSum>0)
        {
            PsdFreqCenter[i]/=(double)PsdSum;    //求平均
        }
    }
    for(i=0;i<m_FreqNum;i++)
    {
        //仪器效正
        double Sensitivity0=Sensitivity;
        //double factor=AmpFactor[i];
        double factor=1;
        PsdFreqCenter[i]/=((Sensitivity0*factor)*(Sensitivity0*factor));

        /*QString chn=QString(QLatin1String(newPsd.Chn_code));
        if(chn.contains("EN"))//wlh 待处理*/
        std::vector<std::string> result;
        ::openmldb::base::SplitString(device, ".", result);
        if (result.back().find("EN") != std::string::npos)
        {
            PsdFreqCenter[i]=10*log10(PsdFreqCenter[i]);
        }
        else
        {
            PsdFreqCenter[i]=10*log10(PsdFreqCenter[i])-20*log10(1/(m_FreqCenter[i]*2*PI));  //计算DB值，并转换为加速度PSD值
        }
    }
}

void CNoisePsd::GetFreqCenter(double MinFrequence, double MaxFrequence)
{
    // m_OctaveStep, m_OctaveWin inited by dl239
    m_OctaveStep = 1.0 / 3;
    m_OctaveWin = 1.0 / 6;

    m_FreqNum=0;
    for(int k=-1000;k<=1000;k++)
    {
        double freqCenter=m_StandardFreq*pow(2.0,(double)(k)*m_OctaveStep);	//f0=fr*2^(x/3)  fr=1000Hz
        if(freqCenter>MaxFrequence)  break;
        if(freqCenter>=MinFrequence)
        {
            if(m_FreqNum==0)  m_FreqExpStart=k;   //倍频程滤波，最低频端对应的频率指数x

            m_FreqCenter[m_FreqNum]=freqCenter;
            m_FreqLow[m_FreqNum]=freqCenter*pow(2.0,-1.0*m_OctaveWin/2.0);	//f1=f0*2^(-1/6)
            m_FreqHigh[m_FreqNum]=freqCenter*pow(2.0,1.0*m_OctaveWin/2.0);		//f1=f0*2^(1/6)
            m_FreqNum++;
        }
    }
    // m_RBW=pow(2.0,1*m_OctaveWin/2.0)-pow(2.0,-1*m_OctaveWin/2.0);  //(fn-f1)/f0

    //	//查找5Hz频点的序号
    //	double diff_1,diff_5,diff_20;
    //    diff_1=diff_5=diff_20=100;

    //    for(int i=0;i<m_FreqNum;i++)
    //	{
    //        if(fabs(m_f0[i]-1)<diff_1)		{            m_f0_1=i;diff_1=fabs(m_f0[i]-1);		}
    //        if(fabs(m_f0[i]-5)<diff_5)		{            m_f0_5=i;diff_5=fabs(m_f0[i]-5);		}
    //        if(fabs(m_f0[i]-20)<diff_20)	{            m_f0_20=i;diff_20=fabs(m_f0[i]-20);	}
    //	}

    //	double Psdv=-106.7+20*log10(0.17/(1*2*PI));
    //	double RawPsdv=pow(10,(Psdv)/10);
    //	double rms=sqrt(2.0*RawPsdv*1/0.17*1*m_RBW);         //计算Rms值


}

}  // namespace tools
}  // namespace openmldb
