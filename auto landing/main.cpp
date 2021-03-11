//
// Created by limz on 2020/8/21.
//



#include <iostream>
#include "auto_landing.h"
#include <vector>

#include "fileUtils.h"

#include <omp.h>
#include <time.h>

using namespace std;

void test()
{
	int a = 0;
	for (int i = 0; i < 100000000; i++)
		a++;
}

int main(int argc, char** argv)
{
	autolanding_lmz::autoLanding node;
	//node.test_ipm();
	node.test_visual();
	//node.test();
	
	//test_openMP();

	return 0;
}
void test_openMP() {
	
	clock_t t1 = clock();

	int sum = 0;

	int a[10] = { 11,2,33,49,115,26,327,258,689,16 };
#pragma omp parallel for
	for (int i = 0; i < 10; i++) {
		//std::cout << omp_get_thread_num() <<" "<< i << std::endl;
		int temp = a[i];
		//#pragma omp critical
		if (temp > sum)
			sum = temp;
	}
	std::cout << "sum: " << sum << std::endl;
	//for (int i = 0; i < 8; i++)
		//test();
	clock_t t2 = clock();
	std::cout << "time: " << t2 - t1 << std::endl;
	//autolanding_lmz::fileUtils node;
	//node.dataCombine();
/*
section:
#pragma omp parallel sections
	{
	#pragma omp section
		{
			std::cout << "sum11111111111: " << omp_get_thread_num() << std::endl;
		}
	#pragma omp section
		{
			std::cout << "sum222222222222222: " << omp_get_thread_num() << std::endl;
		}
	#pragma omp section
		{
			std::cout << "sum3333333333333: " << omp_get_thread_num() << std::endl;
		}
	}
*/


}