#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace std;

#define  DM  30

inline double pw(double x, int k) {
    if (x==0)  return 0;
    else return (exp(k*log(x)));
}

vector<double> polyReg(double x[DM], double y[DM], int n){
    int    i, ij, j, k, m;
    double c[DM][DM];
    double a[DM], b[DM], xc[DM], yx[DM];
    double p, xx, s, yc;

    int n1=n+1; 
    int m1=m+1; 
    int m2=m+2;

    for (k=1; k<=m2; k++) {
            xc[k]=0;
            for (i=1; i<=n1; i++){
                xc[k]+=pw(x[i],k);
            }
    }

    yc=0;

    for (i=1; i<=n1; i++){
        yc+=y[i];
    }

    for (k=1; k<=m; k++) {
        yx[k]=0;
        for (i=1; i<=n1; i++){
            yx[k]+=y[i]*pw(x[i],k);
        }
    }

    for (i=1; i<=m1; i++){
        for (j=1; j<=m1; j++) {
            ij=i+j-2;
            if (i==1 && j==1){
                c[1][1]=n1;
            }else{
                c[i][j]=xc[ij];
            }
        }
    }

    b[1]=yc; 

    for (i=2; i<=m1; i++){
        b[i]=yx[i-1];
    }

    for (k=1; k<=m; k++){
        for (i=k+1; i<=m1; i++){
            b[i]-=c[i][k]/c[k][k]*b[k];
            for (j=k+1; j<=m1; j++){
                c[i][j]-=c[i][k]/c[k][k]*c[k][j];
            }
        }
    }

    a[m1]=b[m1]/c[m1][m1];

    for (i=m; i>0; i--)  {
        s=0;
        for (k=i+1; k<=m1; k++){
            s+=c[i][k]*a[k];
        }
        a[i]=(b[i]-s)/c[i][i];
    }

    vector<double> coefficients;

    for (i=1; i<=m1; i++){
        coefficients.push_back(a[i]);
    }
}

int main(){

    double x[] = {0, 1, 2, 3};
    double y[] = {0, 1, 4, 9};
    vector<double> res = polyReg(x, y, 2);
    
    cout<<"Coefficients";
    cout<<res[0];
    cout<<res[1];
}
