#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#define MAX_NUM 1000000
#define LARGE 1000000

int data[MAX_NUM];

int main()
{
    int d;
    ofstream output("input.txt", ios::out);
    FILE *fp = fopen("input.dat", "wb");
    random_device rd;
    mt19937_64 eng(rd());
    uniform_int_distribution<int> distr(0, LARGE);
    for (int n = 0; n < MAX_NUM; n++)
    {
        d = distr(eng);
        // printf("%d ", d);
        fwrite(&d, sizeof(int), 1, fp);
        data[n] = d;
        output << d << " ";
    }
    fclose(fp);
    return 0;
}