#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
using namespace std;

vector<int> ground_truth;
vector<int> result;

void read_input(const char *filename, vector<int> &buf)
{
    ifstream input(filename, ios::in);
    int data;
    while (true)
    {
        input >> data;
        if (input.fail())
            break;
        buf.push_back(data);
    }
}

int main()
{
    read_input("input.txt", ground_truth);
    read_input("output.txt", result);
    sort(ground_truth.begin(), ground_truth.end());
    if (ground_truth.size() != result.size())
    {
        cout << "[ERROR]: sorted data size(" << result.size() << ") doesn't match ground truth data size(" << ground_truth.size() << ")!\n";
        return 1;
    }
    if (ground_truth != result)
    {
        cout << "[ERROR]: sorted data is different from ground truth data!\n";
        return 2;
    }
    cout << "Sorted data(size: " << result.size() << ") is the same as ground truth data(size: " << ground_truth.size() << ")! Check passed!\n";
    return 0;
}