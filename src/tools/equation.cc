// Equation.cpp: implementation of the CEquation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "stdlib.h"
#include "tools/equation.h"
#include "math.h"

namespace openmldb {
namespace tools {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEquation::CEquation() { }

CEquation::~CEquation() { }

///////////////////////////////////////////////
//全选主元高斯消去法(P87)
int CEquation::cagaus(double* a,double* b, int n,double* x) 
{
    int *js,l,k,i,j,is,p,q;
    double d,t;
    js=(int *)malloc(n*sizeof(int));
    l=1;
    for (k=0;k<=n-2;k++)
    { d=0.0;
        for (i=k;i<=n-1;i++)
            for (j=k;j<=n-1;j++)
            { t=fabs(a[i*n+j]);
                if (t>d) { d=t; js[k]=j; is=i;}
            }
        if (d+1.0==1.0) l=0;
        else
        { if (js[k]!=k)
                for (i=0;i<=n-1;i++)
                { p=i*n+k; q=i*n+js[k];
                    t=a[p]; a[p]=a[q]; a[q]=t;
                }
            if (is!=k)
            { for (j=k;j<=n-1;j++)
                { p=k*n+j; q=is*n+j;
                    t=a[p]; a[p]=a[q]; a[q]=t;
                }
                t=b[k]; b[k]=b[is]; b[is]=t;
            }
        }
        if (l==0)
        { free(js); printf("fail");
            return(0);
        }
        d=a[k*n+k];
        for (j=k+1;j<=n-1;j++)
        { p=k*n+j; a[p]=a[p]/d;}
        b[k]=b[k]/d;
        for (i=k+1;i<=n-1;i++)
        { for (j=k+1;j<=n-1;j++)
            { p=i*n+j;
                a[p]=a[p]-a[i*n+k]*a[k*n+j];
            }
            b[i]=b[i]-a[i*n+k]*b[k];
        }
    }
    d=a[(n-1)*n+n-1];
    if (fabs(d)+1.0==1.0)
    { free(js); printf("fail");
        return(0);
    }
    x[n-1]=b[n-1]/d;
    for (i=n-2;i>=0;i--)
    { t=0.0;
        for (j=i+1;j<=n-1;j++)
            t=t+a[i*n+j]*x[j];
        x[i]=b[i]-t;
    }
    js[n-1]=n-1;
    for (k=n-1;k>=0;k--)
        if (js[k]!=k)
        { t=x[k]; x[k]=x[js[k]]; x[js[k]]=t;}
    free(js);
    return(1);
}	
//////////////////////////////////////
//一般实矩阵的奇异值分解(P169)
int CEquation::dluav(double* a,int m,int n,double* u,double* v,double eps,int ka)
{ 
    int i,j,k,l,it,ll,kk,ix,iy,mm,nn,iz,m1,ks;
    double d,dd,t,sm,sm1,em1,sk,ek,b,c,shh,fg[2],cs[2];
    double *s,*e,*w;

    s=(double *)malloc(ka*sizeof(double));
    e=(double *)malloc(ka*sizeof(double));
    w=(double *)malloc(ka*sizeof(double));
    it=60; k=n;
    if (m-1<n) k=m-1;
    l=m;
    if (n-2<m) l=n-2;
    if (l<0) l=0;
    ll=k;
    if (l>k) ll=l;
    if (ll>=1)
    { for (kk=1; kk<=ll; kk++)
        { if (kk<=k)
            { d=0.0;
                for (i=kk; i<=m; i++)
                { ix=(i-1)*n+kk-1; d=d+a[ix]*a[ix];}
                s[kk-1]=sqrt(d);
                if (s[kk-1]!=0.0)
                { ix=(kk-1)*n+kk-1;
                    if (a[ix]!=0.0)
                    { s[kk-1]=fabs(s[kk-1]);
                        if (a[ix]<0.0) s[kk-1]=-s[kk-1];
                    }
                    for (i=kk; i<=m; i++)
                    { iy=(i-1)*n+kk-1;
                        a[iy]=a[iy]/s[kk-1];
                    }
                    a[ix]=1.0+a[ix];
                }
                s[kk-1]=-s[kk-1];
            }
            if (n>=kk+1)
            { for (j=kk+1; j<=n; j++)
                { if ((kk<=k)&&(s[kk-1]!=0.0))
                    { d=0.0;
                        for (i=kk; i<=m; i++)
                        { ix=(i-1)*n+kk-1;
                            iy=(i-1)*n+j-1;
                            d=d+a[ix]*a[iy];
                        }
                        d=-d/a[(kk-1)*n+kk-1];
                        for (i=kk; i<=m; i++)
                        { ix=(i-1)*n+j-1;
                            iy=(i-1)*n+kk-1;
                            a[ix]=a[ix]+d*a[iy];
                        }
                    }
                    e[j-1]=a[(kk-1)*n+j-1];
                }
            }
            if (kk<=k)
            { for (i=kk; i<=m; i++)
                { ix=(i-1)*m+kk-1; iy=(i-1)*n+kk-1;
                    u[ix]=a[iy];
                }
            }
            if (kk<=l)
            { d=0.0;
                for (i=kk+1; i<=n; i++)
                    d=d+e[i-1]*e[i-1];
                e[kk-1]=sqrt(d);
                if (e[kk-1]!=0.0)
                { if (e[kk]!=0.0)
                    { e[kk-1]=fabs(e[kk-1]);
                        if (e[kk]<0.0) e[kk-1]=-e[kk-1];
                    }
                    for (i=kk+1; i<=n; i++)
                        e[i-1]=e[i-1]/e[kk-1];
                    e[kk]=1.0+e[kk];
                }
                e[kk-1]=-e[kk-1];
                if ((kk+1<=m)&&(e[kk-1]!=0.0))
                { for (i=kk+1; i<=m; i++) w[i-1]=0.0;
                    for (j=kk+1; j<=n; j++)
                        for (i=kk+1; i<=m; i++)
                            w[i-1]=w[i-1]+e[j-1]*a[(i-1)*n+j-1];
                    for (j=kk+1; j<=n; j++)
                        for (i=kk+1; i<=m; i++)
                        { ix=(i-1)*n+j-1;
                            a[ix]=a[ix]-w[i-1]*e[j-1]/e[kk];
                        }
                }
                for (i=kk+1; i<=n; i++)
                    v[(i-1)*n+kk-1]=e[i-1];
            }
        }
    }
    mm=n;
    if (m+1<n) mm=m+1;
    if (k<n) s[k]=a[k*n+k];
    if (m<mm) s[mm-1]=0.0;
    if (l+1<mm) e[l]=a[l*n+mm-1];
    e[mm-1]=0.0;
    nn=m;
    if (m>n) nn=n;
    if (nn>=k+1)
    { for (j=k+1; j<=nn; j++)
        { for (i=1; i<=m; i++)
                u[(i-1)*m+j-1]=0.0;
            u[(j-1)*m+j-1]=1.0;
        }
    }
    if (k>=1)
    { for (ll=1; ll<=k; ll++)
        { kk=k-ll+1; iz=(kk-1)*m+kk-1;
            if (s[kk-1]!=0.0)
            { if (nn>=kk+1)
                    for (j=kk+1; j<=nn; j++)
                    { d=0.0;
                        for (i=kk; i<=m; i++)
                        { ix=(i-1)*m+kk-1;
                            iy=(i-1)*m+j-1;
                            d=d+u[ix]*u[iy]/u[iz];
                        }
                        d=-d;
                        for (i=kk; i<=m; i++)
                        { ix=(i-1)*m+j-1;
                            iy=(i-1)*m+kk-1;
                            u[ix]=u[ix]+d*u[iy];
                        }
                    }
                for (i=kk; i<=m; i++)
                { ix=(i-1)*m+kk-1; u[ix]=-u[ix];}
                u[iz]=1.0+u[iz];
                if (kk-1>=1)
                    for (i=1; i<=kk-1; i++)
                        u[(i-1)*m+kk-1]=0.0;
            }
            else
            { for (i=1; i<=m; i++)
                    u[(i-1)*m+kk-1]=0.0;
                u[(kk-1)*m+kk-1]=1.0;
            }
        }
    }
    for (ll=1; ll<=n; ll++)
    { kk=n-ll+1; iz=kk*n+kk-1;
        if ((kk<=l)&&(e[kk-1]!=0.0))
        { for (j=kk+1; j<=n; j++)
            { d=0.0;
                for (i=kk+1; i<=n; i++)
                { ix=(i-1)*n+kk-1; iy=(i-1)*n+j-1;
                    d=d+v[ix]*v[iy]/v[iz];
                }
                d=-d;
                for (i=kk+1; i<=n; i++)
                { ix=(i-1)*n+j-1; iy=(i-1)*n+kk-1;
                    v[ix]=v[ix]+d*v[iy];
                }
            }
        }
        for (i=1; i<=n; i++)
            v[(i-1)*n+kk-1]=0.0;
        v[iz-n]=1.0;
    }
    for (i=1; i<=m; i++)
        for (j=1; j<=n; j++)
            a[(i-1)*n+j-1]=0.0;
    m1=mm; it=60;
    while (1==1)
    { if (mm==0)
        { ppp(a,e,s,v,m,n);
            free(s); free(e); free(w); return(1);
        }
        if (it==0)
        { ppp(a,e,s,v,m,n);
            free(s); free(e); free(w); return(-1);
        }
        kk=mm-1;
        while ((kk!=0)&&(fabs(e[kk-1])!=0.0))
        { d=fabs(s[kk-1])+fabs(s[kk]);
            dd=fabs(e[kk-1]);
            if (dd>eps*d) kk=kk-1;
            else e[kk-1]=0.0;
        }
        if (kk==mm-1)
        { kk=kk+1;
            if (s[kk-1]<0.0)
            { s[kk-1]=-s[kk-1];
                for (i=1; i<=n; i++)
                { ix=(i-1)*n+kk-1; v[ix]=-v[ix];}
            }
            while ((kk!=m1)&&(s[kk-1]<s[kk]))
            { d=s[kk-1]; s[kk-1]=s[kk]; s[kk]=d;
                if (kk<n)
                    for (i=1; i<=n; i++)
                    { ix=(i-1)*n+kk-1; iy=(i-1)*n+kk;
                        d=v[ix]; v[ix]=v[iy]; v[iy]=d;
                    }
                if (kk<m)
                    for (i=1; i<=m; i++)
                    { ix=(i-1)*m+kk-1; iy=(i-1)*m+kk;
                        d=u[ix]; u[ix]=u[iy]; u[iy]=d;
                    }
                kk=kk+1;
            }
            it=60;
            mm=mm-1;
        }
        else
        { ks=mm;
            while ((ks>kk)&&(fabs(s[ks-1])!=0.0))
            { d=0.0;
                if (ks!=mm) d=d+fabs(e[ks-1]);
                if (ks!=kk+1) d=d+fabs(e[ks-2]);
                dd=fabs(s[ks-1]);
                if (dd>eps*d) ks=ks-1;
                else s[ks-1]=0.0;
            }
            if (ks==kk)
            { kk=kk+1;
                d=fabs(s[mm-1]);
                t=fabs(s[mm-2]);
                if (t>d) d=t;
                t=fabs(e[mm-2]);
                if (t>d) d=t;
                t=fabs(s[kk-1]);
                if (t>d) d=t;
                t=fabs(e[kk-1]);
                if (t>d) d=t;
                sm=s[mm-1]/d; sm1=s[mm-2]/d;
                em1=e[mm-2]/d;
                sk=s[kk-1]/d; ek=e[kk-1]/d;
                b=((sm1+sm)*(sm1-sm)+em1*em1)/2.0;
                c=sm*em1; c=c*c; shh=0.0;
                if ((b!=0.0)||(c!=0.0))
                { shh=sqrt(b*b+c);
                    if (b<0.0) shh=-shh;
                    shh=c/(b+shh);
                }
                fg[0]=(sk+sm)*(sk-sm)-shh;
                fg[1]=sk*ek;
                for (i=kk; i<=mm-1; i++)
                { sss(fg,cs);
                    if (i!=kk) e[i-2]=fg[0];
                    fg[0]=cs[0]*s[i-1]+cs[1]*e[i-1];
                    e[i-1]=cs[0]*e[i-1]-cs[1]*s[i-1];
                    fg[1]=cs[1]*s[i];
                    s[i]=cs[0]*s[i];
                    if ((cs[0]!=1.0)||(cs[1]!=0.0))
                        for (j=1; j<=n; j++)
                        { ix=(j-1)*n+i-1;
                            iy=(j-1)*n+i;
                            d=cs[0]*v[ix]+cs[1]*v[iy];
                            v[iy]=-cs[1]*v[ix]+cs[0]*v[iy];
                            v[ix]=d;
                        }
                    sss(fg,cs);
                    s[i-1]=fg[0];
                    fg[0]=cs[0]*e[i-1]+cs[1]*s[i];
                    s[i]=-cs[1]*e[i-1]+cs[0]*s[i];
                    fg[1]=cs[1]*e[i];
                    e[i]=cs[0]*e[i];
                    if (i<m)
                        if ((cs[0]!=1.0)||(cs[1]!=0.0))
                            for (j=1; j<=m; j++)
                            { ix=(j-1)*m+i-1;
                                iy=(j-1)*m+i;
                                d=cs[0]*u[ix]+cs[1]*u[iy];
                                u[iy]=-cs[1]*u[ix]+cs[0]*u[iy];
                                u[ix]=d;
                            }
                }
                e[mm-2]=fg[0];
                it=it-1;
            }
            else
            { if (ks==mm)
                { kk=kk+1;
                    fg[1]=e[mm-2]; e[mm-2]=0.0;
                    for (ll=kk; ll<=mm-1; ll++)
                    { i=mm+kk-ll-1;
                        fg[0]=s[i-1];
                        sss(fg,cs);
                        s[i-1]=fg[0];
                        if (i!=kk)
                        { fg[1]=-cs[1]*e[i-2];
                            e[i-2]=cs[0]*e[i-2];
                        }
                        if ((cs[0]!=1.0)||(cs[1]!=0.0))
                            for (j=1; j<=n; j++)
                            { ix=(j-1)*n+i-1;
                                iy=(j-1)*n+mm-1;
                                d=cs[0]*v[ix]+cs[1]*v[iy];
                                v[iy]=-cs[1]*v[ix]+cs[0]*v[iy];
                                v[ix]=d;
                            }
                    }
                }
                else
                { kk=ks+1;
                    fg[1]=e[kk-2];
                    e[kk-2]=0.0;
                    for (i=kk; i<=mm; i++)
                    { fg[0]=s[i-1];
                        sss(fg,cs);
                        s[i-1]=fg[0];
                        fg[1]=-cs[1]*e[i-1];
                        e[i-1]=cs[0]*e[i-1];
                        if ((cs[0]!=1.0)||(cs[1]!=0.0))
                            for (j=1; j<=m; j++)
                            { ix=(j-1)*m+i-1;
                                iy=(j-1)*m+kk-2;
                                d=cs[0]*u[ix]+cs[1]*u[iy];
                                u[iy]=-cs[1]*u[ix]+cs[0]*u[iy];
                                u[ix]=d;
                            }
                    }
                }
            }
        }
    }

    return(1);
}

void CEquation::ppp(double* a,double* e,double* s,double* v,int m,int n)
{ 
    int i,j,p,q;
    double d;
    if (m>=n) i=n;
    else i=m;
    for (j=1; j<=i-1; j++)
    { a[(j-1)*n+j-1]=s[j-1];
        a[(j-1)*n+j]=e[j-1];
    }
    a[(i-1)*n+i-1]=s[i-1];
    if (m<n) a[(i-1)*n+i]=e[i-1];
    for (i=1; i<=n-1; i++)
        for (j=i+1; j<=n; j++)
        { p=(i-1)*n+j-1; q=(j-1)*n+i-1;
            d=v[p]; v[p]=v[q]; v[q]=d;
        }
    return;
}

void CEquation::sss(double* fg,double* cs)
{
    double r,d;
    if ((fabs(fg[0])+fabs(fg[1]))==0.0)
    { cs[0]=1.0; cs[1]=0.0; d=0.0;}
    else
    { d=sqrt(fg[0]*fg[0]+fg[1]*fg[1]);
        if (fabs(fg[0])>fabs(fg[1]))
        { d=fabs(d);
            if (fg[0]<0.0) d=-d;
        }
        if (fabs(fg[1])>=fabs(fg[0]))
        { d=fabs(d);
            if (fg[1]<0.0) d=-d;
        }
        cs[0]=fg[0]/d; cs[1]=fg[1]/d;
    }
    r=1.0;
    if (fabs(fg[0])>fabs(fg[1])) r=cs[1];
    else
        if (cs[0]!=0.0) r=1.0/cs[0];
    fg[0]=d; fg[1]=r;
    return;
}
//////////////////////////////////
//实矩阵相乘(P133)
void CEquation::damul(double* a,double* b,int m,int n,int k,double* c)
{
    int i,j,l,u;
    for (i=0; i<=m-1; i++)
        for (j=0; j<=k-1; j++)
        { u=i*k+j; c[u]=0.0;
            for (l=0; l<=n-1; l++)
                c[u]=c[u]+a[i*n+l]*b[l*k+j];
        }
    return;
}

//////////////////////////////////////
//求广义逆的奇异值分解法(P155)
int CEquation::dginv(double *a,int m,int n,double *aa,double eps,double *u,double *v,int ka)
{
    int i,j,k,l,t,p,q,f;

    i=dluav(a,m,n,u,v,eps,ka);
    if (i<0) return(-1);
    j=n;
    if (m<n) j=m;
    j=j-1;
    k=0;
    while ((k<=j)&&(a[k*n+k]!=0.0)) k=k+1;
    k=k-1;
    for (i=0; i<=n-1; i++)
        for (j=0; j<=m-1; j++)
        { t=i*m+j; aa[t]=0.0;
            for (l=0; l<=k; l++)
            { f=l*n+i; p=j*m+l; q=l*n+l;
                aa[t]=aa[t]+v[f]*u[p]/a[q];
            }
        }
    return(1);
}
/////////////////////////////////////////////////
//求解线性最小二乘问题的广义逆法(P130)
int CEquation::cmgm(double *a,int m,int n,double *b,double *x,double *aa,double eps,double *u,double *v,int ka)
{
    int i,j;

    i=dginv(a,m,n,aa,eps,u,v,ka);
    if (i<0) return(-1);
    for (i=0; i<=n-1; i++)
    { x[i]=0.0;
        for (j=0; j<=m-1; j++)
            x[i]=x[i]+aa[i*m+j]*b[j];
    }
    return(1);
}
/////////////////////////////////////////////////
//矩形域的最小二乘曲面拟合(P369)
void CEquation::icpcir(double* x,double* y,double* z,int n,int m,double* a,int p,int q,double* dt)
{
    int i,j,k,l,kk;
    double apx[20],apy[20],bx[20],by[20],u[20][20];
    double t[20],t1[20],t2[20],xx,yy,d1,d2,g,g1,g2;
    double x2,dd,y1,x1,*v;

    v=(double *)malloc(20*m*sizeof(double));
    for (i=0; i<=p-1; i++)
    { l=i*q;
        for (j=0; j<=q-1; j++) a[l+j]=0.0;
    }
    if (p>n) p=n;
    if (p>20) p=20;
    if (q>m) q=m;
    if (q>20) q=20;
    xx=0.0;
    for (i=0; i<=n-1; i++)
        xx=xx+x[i]/(1.0*n);
    yy=0.0;
    for (i=0; i<=m-1; i++)
        yy=yy+y[i]/(1.0*m);
    d1=1.0*n; apx[0]=0.0;
    for (i=0; i<=n-1; i++)
        apx[0]=apx[0]+x[i]-xx;
    apx[0]=apx[0]/d1;
    for (j=0; j<=m-1; j++)
    { v[j]=0.0;
        for (i=0; i<=n-1; i++)
            v[j]=v[j]+z[i*m+j];
        v[j]=v[j]/d1;
    }
    if (p>1)
    { d2=0.0; apx[1]=0.0;
        for (i=0; i<=n-1; i++)
        { g=x[i]-xx-apx[0];
            d2=d2+g*g;
            apx[1]=apx[1]+(x[i]-xx)*g*g;
        }
        apx[1]=apx[1]/d2;
        bx[1]=d2/d1;
        for (j=0; j<=m-1; j++)
        { v[m+j]=0.0;
            for (i=0; i<=n-1; i++)
            { g=x[i]-xx-apx[0];
                v[m+j]=v[m+j]+z[i*m+j]*g;
            }
            v[m+j]=v[m+j]/d2;
        }
        d1=d2;
    }
    for (k=2; k<=p-1; k++)
    { d2=0.0; apx[k]=0.0;
        for (j=0; j<=m-1; j++) v[k*m+j]=0.0;
        for (i=0; i<=n-1; i++)
        { g1=1.0; g2=x[i]-xx-apx[0];
            for (j=2; j<=k; j++)
            { g=(x[i]-xx-apx[j-1])*g2-bx[j-1]*g1;
                g1=g2; g2=g;
            }
            d2=d2+g*g;
            apx[k]=apx[k]+(x[i]-xx)*g*g;
            for (j=0; j<=m-1; j++)
                v[k*m+j]=v[k*m+j]+z[i*m+j]*g;
        }
        for (j=0; j<=m-1; j++)
            v[k*m+j]=v[k*m+j]/d2;
        apx[k]=apx[k]/d2;
        bx[k]=d2/d1;
        d1=d2;
    }
    d1=m; apy[0]=0.0;
    for (i=0; i<=m-1; i++)
        apy[0]=apy[0]+y[i]-yy;
    apy[0]=apy[0]/d1;
    for (j=0; j<=p-1; j++)
    { u[j][0]=0.0;
        for (i=0; i<=m-1; i++)
            u[j][0]=u[j][0]+v[j*m+i];
        u[j][0]=u[j][0]/d1;
    }
    if (q>1)
    { d2=0.0; apy[1]=0.0;
        for (i=0; i<=m-1; i++)
        { g=y[i]-yy-apy[0];
            d2=d2+g*g;
            apy[1]=apy[1]+(y[i]-yy)*g*g;
        }
        apy[1]=apy[1]/d2;
        by[1]=d2/d1;
        for (j=0; j<=p-1; j++)
        { u[j][1]=0.0;
            for (i=0; i<=m-1; i++)
            { g=y[i]-yy-apy[0];
                u[j][1]=u[j][1]+v[j*m+i]*g;
            }
            u[j][1]=u[j][1]/d2;
        }
        d1=d2;
    }
    for (k=2; k<=q-1; k++)
    { d2=0.0; apy[k]=0.0;
        for (j=0; j<=p-1; j++) u[j][k]=0.0;
        for (i=0; i<=m-1; i++)
        { g1=1.0;
            g2=y[i]-yy-apy[0];
            for (j=2; j<=k; j++)
            { g=(y[i]-yy-apy[j-1])*g2-by[j-1]*g1;
                g1=g2; g2=g;
            }
            d2=d2+g*g;
            apy[k]=apy[k]+(y[i]-yy)*g*g;
            for (j=0; j<=p-1; j++)
                u[j][k]=u[j][k]+v[j*m+i]*g;
        }
        for (j=0; j<=p-1; j++)
            u[j][k]=u[j][k]/d2;
        apy[k]=apy[k]/d2;
        by[k]=d2/d1;
        d1=d2;
    }
    v[0]=1.0; v[m]=-apy[0]; v[m+1]=1.0;
    for (i=0; i<=p-1; i++)
        for (j=0; j<=q-1; j++)
            a[i*q+j]=0.0;
    for (i=2; i<=q-1; i++)
    { v[i*m+i]=v[(i-1)*m+(i-1)];
        v[i*m+i-1]=-apy[i-1]*v[(i-1)*m+i-1]+v[(i-1)*m+i-2];
        if (i>=3)
            for (k=i-2; k>=1; k--)
                v[i*m+k]=-apy[i-1]*v[(i-1)*m+k]+
                        v[(i-1)*m+k-1]-by[i-1]*v[(i-2)*m+k];
        v[i*m]=-apy[i-1]*v[(i-1)*m]-by[i-1]*v[(i-2)*m];
    }
    for (i=0; i<=p-1; i++)
    { if (i==0) { t[0]=1.0; t1[0]=1.0;}
        else
        { if (i==1)
            { t[0]=-apx[0]; t[1]=1.0;
                t2[0]=t[0]; t2[1]=t[1];
            }
            else
            { t[i]=t2[i-1];
                t[i-1]=-apx[i-1]*t2[i-1]+t2[i-2];
                if (i>=3)
                    for (k=i-2; k>=1; k--)
                        t[k]=-apx[i-1]*t2[k]+t2[k-1]
                                -bx[i-1]*t1[k];
                t[0]=-apx[i-1]*t2[0]-bx[i-1]*t1[0];
                t2[i]=t[i];
                for (k=i-1; k>=0; k--)
                { t1[k]=t2[k]; t2[k]=t[k];}
            }
        }
        for (j=0; j<=q-1; j++)
            for (k=i; k>=0; k--)
                for (l=j; l>=0; l--)
                    a[k*q+l]=a[k*q+l]+u[i][j]*t[k]*v[j*m+l];
    }
    dt[0]=0.0; dt[1]=0.0; dt[2]=0.0;
    for (i=0; i<=n-1; i++)
    { x1=x[i]-xx;
        for (j=0; j<=m-1; j++)
        { y1=y[j]-yy;
            x2=1.0; dd=0.0;
            for (k=0; k<=p-1; k++)
            { g=a[k*q+q-1];
                for (kk=q-2; kk>=0; kk--)
                    g=g*y1+a[k*q+kk];
                g=g*x2; dd=dd+g; x2=x2*x1;
            }
            dd=dd-z[i*m+j];
            if (fabs(dd)>dt[2]) dt[2]=fabs(dd);
            dt[0]=dt[0]+dd*dd;
            dt[1]=dt[1]+fabs(dd);
        }
    }
    free(v);
    return;
}
////////////////////////////////////////
// 实数冒泡排序 P492
void CEquation::mbbub(double* p,int n)
{

    int m,k,j,i;
    double d;
    k=0; m=n-1;
    while (k<m)
    { j=m-1; m=0;
        for (i=k; i<=j; i++)
            if (p[i]>p[i+1])
            { d=p[i]; p[i]=p[i+1]; p[i+1]=d; m=i;}
        j=k+1; k=0;
        for (i=m; i>=j; i--)
            if (p[i-1]>p[i])
            { d=p[i]; p[i]=p[i-1]; p[i-1]=d; k=i;}
    }
    return;

}
///////////////////////////////////////////////////
// 求实对称矩阵特征值及特征向量的雅可比法 P204
int CEquation::eejcb(double* a,int n,double *v,double eps,int jt)
{
    int i,j,p,q,u,w,t,s,l;
    double fm,cn,sn,omega,x,y,d;
    l=1;
    for (i=0; i<=n-1; i++)
    {
        v[i*n+i]=1.0;
        for (j=0; j<=n-1; j++)
        {
            if (i!=j) v[i*n+j]=0.0;
        }
    }
    while (1==1)
    {
        fm=0.0;
        for (i=0; i<=n-1; i++)
            for (j=0; j<=n-1; j++)
            {
                d=fabs(a[i*n+j]);
                if ((i!=j)&&(d>fm))
                {
                    fm=d; p=i; q=j;
                }
            }
        if (fm<eps)  return(1);
        if (l>jt)  return(-1);
        l=l+1;
        u=p*n+q; w=p*n+p; t=q*n+p; s=q*n+q;
        x=-a[u]; y=(a[s]-a[w])/2.0;
        omega=x/sqrt(x*x+y*y);
        if (y<0.0) omega=-omega;
        sn=1.0+sqrt(1.0-omega*omega);
        sn=omega/sqrt(2.0*sn);
        cn=sqrt(1.0-sn*sn);
        fm=a[w];
        a[w]=fm*cn*cn+a[s]*sn*sn+a[u]*omega;
        a[s]=fm*sn*sn+a[s]*cn*cn-a[u]*omega;
        a[u]=0.0; a[t]=0.0;
        for (j=0; j<=n-1; j++)
        {
            if ((j!=p)&&(j!=q))
            {
                u=p*n+j; w=q*n+j;
                fm=a[u];
                a[u]=fm*cn+a[w]*sn;
                a[w]=-fm*sn+a[w]*cn;
            }
        }
        for (i=0; i<=n-1; i++)
        {
            if ((i!=p)&&(i!=q))
            {
                u=i*n+p; w=i*n+q;
                fm=a[u];
                a[u]=fm*cn+a[w]*sn;
                a[w]=-fm*sn+a[w]*cn;
            }
        }
        for (i=0; i<=n-1; i++)
        {
            u=i*n+p; w=i*n+q;
            fm=v[u];
            v[u]=fm*cn+v[w]*sn;
            v[w]=-fm*sn+v[w]*cn;
        }
    }
    return(1);
}


/////////////////////////////////////////////////////////
//用FFT计算离散傅里叶(Fourier)变换 P418
//pr:当l=0时，存放n个采样输入的实部，返回时存放离散傅立叶变换的模
//   当l=1时，存放n个采样输入的实部，返回时存放逆傅立叶变换的模
//pi:当l=0时，存放n个采样输入的虚部，返回时存放离散傅立叶变换的幅角
//   当l=1时，存放n个采样输入的虚部，返回时存放逆傅立叶变换的幅角
//l:当=0时，要求本函数计算傅立叶变换
//  当=1时，要求本函数计算逆傅立叶变换
void CEquation::kbfft(double* pr,double *pi,int n,int k,double* fr,double* fi,int l,int il)
{
    int it,m,is,i,j,nv,l0;
    double p,q,s,vr,vi,poddr,poddi;
    for (it=0; it<=n-1; it++)
    {
        m=it; is=0;
        for (i=0; i<=k-1; i++)
        {
            j=m/2; is=2*is+(m-2*j); m=j;
        }
        fr[it]=pr[is]; fi[it]=pi[is];
    }
    pr[0]=1.0; pi[0]=0.0;
    p=6.283185306/(1.0*n);
    pr[1]=cos(p); pi[1]=-sin(p);
    if (l!=0)
    {
        pi[1]=-pi[1];
    }
    for (i=2; i<=n-1; i++)
    {
        p=pr[i-1]*pr[1]; q=pi[i-1]*pi[1];
        s=(pr[i-1]+pi[i-1])*(pr[1]+pi[1]);
        pr[i]=p-q; pi[i]=s-p-q;
    }
    for (it=0; it<=n-2; it=it+2)
    {
        vr=fr[it]; vi=fi[it];
        fr[it]=vr+fr[it+1]; fi[it]=vi+fi[it+1];
        fr[it+1]=vr-fr[it+1]; fi[it+1]=vi-fi[it+1];
    }
    m=n/2; nv=2;
    for (l0=k-2; l0>=0; l0--)
    {
        m=m/2; nv=2*nv;
        for (it=0; it<=(m-1)*nv; it=it+nv)
        {
            for (j=0; j<=(nv/2)-1; j++)
            {
                p=pr[m*j]*fr[it+j+nv/2];
                q=pi[m*j]*fi[it+j+nv/2];
                s=pr[m*j]+pi[m*j];
                s=s*(fr[it+j+nv/2]+fi[it+j+nv/2]);
                poddr=p-q; poddi=s-p-q;
                fr[it+j+nv/2]=fr[it+j]-poddr;
                fi[it+j+nv/2]=fi[it+j]-poddi;
                fr[it+j]=fr[it+j]+poddr;
                fi[it+j]=fi[it+j]+poddi;
            }
        }
    }
    if (l!=0)
    {
        for (i=0; i<=n-1; i++)
        {
            fr[i]=fr[i]/(1.0*n);
            fi[i]=fi[i]/(1.0*n);
        }
    }
    if (il!=0)
    {
        for (i=0; i<=n-1; i++)
        {
            pr[i]=sqrt(fr[i]*fr[i]+fi[i]*fi[i]);
            if (fabs(fr[i])<0.000001*fabs(fi[i]))
            {
                if ((fi[i]*fr[i])>0) pi[i]=90.0;
                else pi[i]=-90.0;
            }
            else
                pi[i]=atan(fi[i]/fr[i])*360.0/6.283185306;
        }
    }
    return;
}
//一元线性回归
void CEquation::regres(float* x,float* y,int n,double* para,double* dt)
{
    int i;
    double  a, b;
    double  q, s, p, umax, umin, u;

    double xx,yy,e,f;
    xx=0.0; yy=0.0;
    for (i=0; i<=n-1; i++)
    { xx=xx+x[i]/n; yy=yy+y[i]/n; }
    e=0.0; f=0.0;
    for (i=0; i<=n-1; i++)
    {
        q=x[i]-xx; e=e+q*q;
        f=f+q*(y[i]-yy);
    }
    a = f/e;  b = yy-a*xx;
    q=0.0; u=0.0; p=0.0;
    umax=0.0; umin=1.0e+30;
    for (i=0; i<=n-1; i++)
    {
        s=a*x[i]+b;
        q=q+(y[i]-s)*(y[i]-s);
        p=p+(s-yy)*(s-yy);
        e=fabs(y[i]-s);
        if (e>umax) umax=e;
        if (e<umin) umin=e;
        u=u+e/n;
    }
    s = sqrt(q/n);
    para[0]=b; para[1]=a;
    dt[0]=q;   //偏差平方和
    dt[1]=s;   //平均标准偏差
    dt[2]=p;   //回归平方和
    dt[3]=umax;//最大偏差
    dt[4]=umin;//最小偏差
    dt[5]=u;   //偏差平均值
}

//一元线性回归
void CEquation::regres(double* x,double* y,int n,double* para,double* dt)
{
    int i;
    double  a, b;
    double  q, s, p, umax, umin, u;

    double xx,yy,e,f;
    xx=0.0; yy=0.0;
    for (i=0; i<=n-1; i++)
    { xx=xx+x[i]/n; yy=yy+y[i]/n; }
    e=0.0; f=0.0;
    for (i=0; i<=n-1; i++)
    {
        q=x[i]-xx; e=e+q*q;
        f=f+q*(y[i]-yy);
    }
    a = f/e;  b = yy-a*xx;
    q=0.0; u=0.0; p=0.0;
    umax=0.0; umin=1.0e+30;
    for (i=0; i<=n-1; i++)
    {
        s=a*x[i]+b;
        q=q+(y[i]-s)*(y[i]-s);
        p=p+(s-yy)*(s-yy);
        e=fabs(y[i]-s);
        if (e>umax) umax=e;
        if (e<umin) umin=e;
        u=u+e/n;
    }
    s = sqrt(q/n);
    para[0]=b; para[1]=a;
    dt[0]=q;   //偏差平方和
    dt[1]=s;   //平均标准偏差
    dt[2]=p;   //回归平方和
    dt[3]=umax;//最大偏差
    dt[4]=umin;//最小偏差
    dt[5]=u;   //偏差平均值
}
}  // namespace tools
}  // namespace openmldb
