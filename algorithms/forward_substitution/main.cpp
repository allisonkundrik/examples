/* 
 * File:   main.cpp
 * Author: piter cf16 eu
 *
 * Created on March 7, 2014, 12:01 AM
 */

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>

void solve(int n, float a[4][4], float b[], float x[]){
  int i,j;
  float s;

  for(i = 0; i < n; i++) {
        s = 0;
        for( j = 0; j < i; j++) {
            float f1 = a[ i][ j];
            float f2 = x[j];
            s = s + a[ i][ j] * x[ j];
        }
        float f3 = a[i][i];
        x[ i] = ( b[ i] - s) / a[ i][ i];
        float f4 = x[i];
        int ip = 6;
   }
}

/*
 * 
 */
int main(int argc, char** argv) {

    int n = 4;
    float a[4][4] = { 3, 0, 0, 0, -1, 1, 0, 0, 3, -2, -1, 0, 1, -2, 6, 2};
    float b[4] = { 5, 6, 4, 2};
    float res[4];
    solve( n, a, b, res);
    std::copy( res, res + 4, std::ostream_iterator<float>( std::cout, ","));
    return 0;
}

