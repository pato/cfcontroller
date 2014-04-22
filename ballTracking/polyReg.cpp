#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace std;

// Define the maximum amount of points
#define  DM  30

inline double pw(double x, int k) {
    if (x==0)  return 0;
    else return (exp(k*log(x)));
}

/**
 * perform polynomial regression
 * xcoords - vector of x coordinates (as doubles)
 * ycoords - vector of y coordinates (as doubles)
 * m - degree of desired polynomial
 */
vector<double> polyReg(vector<double> &xcoords, vector<double> &ycoords, int m){
    int    i, ij, j, k, n1, m1, m2;
    double c[DM][DM];
    double a[DM], b[DM], x[DM], xc[DM], y[DM], yx[DM];
    double p, xx, s, yc;

    int n = xcoords.size();

    for (int p=0;p<n;p++){
        x[p] = xcoords[p];
        y[p] = ycoords[p];
    }

    // Useful for loops
    n1=n+1; 
    m1=m+1; 
    m2=m+2;

    for (k=1; k<=m2; k++){
        xc[k]=0;
        for (i=1; i<=n1; i++){
            xc[k]+=pw(x[i],k);
        }
    }

    yc=0;

    for (i=1; i<=n1; i++){
        yc+=y[i];
    }

    for (k=1; k<=m; k++){
        yx[k]=0;
        for (i=1; i<=n1; i++){
            yx[k]+=y[i]*pw(x[i],k);
        }
    }

    for (i=1; i<=m1; i++){
        for (j=1; j<=m1; j++){
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

    for (i=m; i>0; i--){
        s=0;
        for (k=i+1; k<=m1; k++){
            s+=c[i][k]*a[k];
        }
        a[i]=(b[i]-s)/c[i][i];
    }

    vector<double> coefficients;

    for (i=1; i<=m1; i++){
        coefficients.push_back(round(a[i]*1000)/1000);
    }

    return coefficients;
}

int main(){
    vector<double> xcoords;
    vector<double> ycoords;

    for (double i=0;i<0;i++){
        xcoords.push_back(i);
        ycoords.push_back(1 + i*i);
    }

    vector<double> coefficients = polyReg(xcoords, ycoords, 2);

    for (int i=0;i<coefficients.size();i++){
        cout<< coefficients[i] << "x^" << (i) << " + ";
    }
    cout<< "0" << endl;
}
