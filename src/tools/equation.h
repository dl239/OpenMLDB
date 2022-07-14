// Equation.h: interface for the CEquation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EQUATION_H__FC845E0F_FDDF_48D5_86B6_B7F4A88C7807__INCLUDED_)
#define AFX_EQUATION_H__FC845E0F_FDDF_48D5_86B6_B7F4A88C7807__INCLUDED_
namespace openmldb {
namespace tools {

class CEquation  {
  public:
	CEquation();
	virtual ~CEquation();
	//全选主元高斯消去法(P87)
	int cagaus(double* a,double* b, int n,double* x);

	//一般实矩阵的奇异值分解(P169)
	int dluav(double* a,int m,int n,double* u,double* v,double eps,int ka);
	void ppp(double* a,double* e,double* s,double* v,int m,int n);
	void sss(double* fg,double* cs);

	//实矩阵相乘(P133)
	void damul(double* a,double* b,int m,int n,int k,double* c);
	//求广义逆的奇异值分解法(P155)
	int dginv(double *a,int m,int n,double *aa,double eps,double *u,double *v,int ka);
	//求解线性最小二乘问题的广义逆法(P130)
	int cmgm(double *a,int m,int n,double *b,double *x,double *aa,double eps,double *u,double *v,int ka);
	//矩形域的最小二乘曲面拟合(P369)
	void icpcir(double* x,double* y,double* z,int n,int m,double* a,int p,int q,double* dt);
	// 实数冒泡排序 P492
	void mbbub(double* p,int n);
	// 求实对称矩阵特征值及特征向量的雅可比法 P204
	int eejcb(double* a,int n,double *v,double eps,int jt);
	
	static void kbfft(double* pr,double *pi,int n,int k,double* fr,double* fi,int l,int il);

    //一元线性回归
    void regres(double* x,double* y,int n,double* a,double* dt);
    void regres(float* x,float* y,int n,double* a,double* dt);
};
}  // namespace tools
}  // namespace openmldb
#endif // !defined(AFX_EQUATION_H__FC845E0F_FDDF_48D5_86B6_B7F4A88C7807__INCLUDED_)
