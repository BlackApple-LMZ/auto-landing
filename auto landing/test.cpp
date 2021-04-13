#include"test.h"

using namespace std;
void delay_temp()
{
	int a = 0;
	for (int i = 0; i < 100000000; i++)
		a++;
}

void test_openMP() {

	clock_t t1 = clock();

	int sum = 0;

	/*
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
	
	*/
	
	//for (int i = 0; i < 8; i++)
		//test();
	
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

	
	//------------------------------schedule test-------------------------------
	//没用openmp是704
	//用openmp是285  用openmp schedule(static)是284 用openmp schedule(dynamic)是255
	//动态调度效率更高一些，尤其针对循环内每次迭代的计算量不相等 
	int(*ga)[10000] = new int[10000][10000];

	int *a = new int[100];
	//int a[100][100] = { 0 };
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < 10000; i=i+1) {
		for (int j = i; j < 10000; j++)
			ga[i][j] = ((i % 7)*(j % 13) % 23);
		//cout << omp_get_dynamic() << endl;
	}
	
	delete[]ga;
	
	
	


	clock_t t2 = clock();
	std::cout << "time: " << t2 - t1 << std::endl;
	return ;

}