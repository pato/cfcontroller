//-------------------- nonlinear regression  -------------------------------
//  approximation of a discreet real function F(x) by least squares  by L.J.   

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
using namespace std;

#define  DM  30

int    i, ij, j, k, n, n1, m, m1, m2;
double c[DM][DM];
double a[DM], b[DM], x[DM], xc[DM], y[DM], yx[DM];
double p, xx, s, yc;


inline double pw(double x, int k) 
{
    if (x==0)  return 0;
    else return (exp(k*log(x)));
}


int main()  
{

    //......... Input ..............................
    cout<<"Number of points    : ";
    cin>>n;

    n--;
    cout<<"Degree of polynomial: ";
    cin>>m;

    n1=n+1; 
    m1=m+1; 
    m2=m+2;
    cout<<"Function to approximate:"<<endl;
    
    
    for (i=1; i<=n1; i++) 
    {   
        cout<<"x("<<i<<"), y("<<i<<") = ";
        cin>>x[i];
        cin>>y[i];
    }
    //...............................................


    for (k=1; k<=m2; k++) 
    {
            xc[k]=0;
            for (i=1; i<=n1; i++)  xc[k]+=pw(x[i],k);
    }

    yc=0;

    for (i=1; i<=n1; i++)  yc+=y[i];

    for (k=1; k<=m; k++) 
    {
            yx[k]=0;
            for (i=1; i<=n1; i++)  yx[k]+=y[i]*pw(x[i],k);
    }

    for (i=1; i<=m1; i++)
        for (j=1; j<=m1; j++) 
        {
                ij=i+j-2;
                if (i==1 && j==1) c[1][1]=n1;
                else c[i][j]=xc[ij];
            }

    b[1]=yc; 

    for (i=2; i<=m1; i++)  b[i]=yx[i-1];

    for (k=1; k<=m; k++)
            for (i=k+1; i<=m1; i++) 
        {
                b[i]-=c[i][k]/c[k][k]*b[k];

                for (j=k+1; j<=m1; j++)
                    c[i][j]-=c[i][k]/c[k][k]*c[k][j];
        }

    a[m1]=b[m1]/c[m1][m1];

    for (i=m; i>0; i--)  
    {
        s=0;
            for (k=i+1; k<=m1; k++)  s+=c[i][k]*a[k];
            a[i]=(b[i]-s)/c[i][i];
    }
    
    cout<<"\n Polynomial approximation of degree "<<m<<" ("<<n+1<<" points)\n";

    cout<<" Coefficients of polynomial:\n";

    for (i=1; i<=m1; i++)  cout<<"  a("<<i-1<<") = "<<a[i]<<endl;

    cout<<"\n Approximated function:\n";
    cout<<"        x           y\n";
    for (i=1; i<=n1; i++) 
    {
            xx=x[i]; p=0;
            for (k=1; k<=m1; k++)  p=p*xx+a[m1+1-k];
            printf(" %11.6f %11.6f\n", xx, p);
        //cout<<xx<<" "<<p;
    }
    cout<<"\n\n";

    return 0;

}
