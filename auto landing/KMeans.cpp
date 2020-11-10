#include "KMeans.h"



#include <assert.h>
#include <string>
#include <time.h>  //for srand
#include <limits.h> //for INT_MIN INT_MAX

namespace autolanding_lmz {

	void KMeans::initClusterAssment()
	{
		tNode node(-1, -1);
		for (int i = 0; i < index_.size(); i++)
		{
			clusterAssment_.push_back(node);
		}
	}

	void KMeans::kmeans(std::vector<int> &cluster)
	{
		//for (int i = 0; i < slope_.size(); i++) {
			//std::cout << slope_[i] << " " << std::endl;
		//}

		initClusterAssment();
		bool clusterChanged = true;
		//the termination condition can also be the loops less than	some number such as 1000
		while (clusterChanged)
		{
			clusterChanged = false;
			//step one : find the nearest centroid of each point
			//std::cout << "find the nearest centroid of each point : " << std::endl;
			for (int i = 0; i < index_.size(); i++)
			{
				int minIndex = -1;
				double minDist = INT_MAX;
				for (int j = 0; j < k; j++)
				{
					double distJI = abs(centroids_[j]-slope_[i]);
					if (distJI < minDist)
					{
						minDist = distJI;
						minIndex = j;
					}
				}
				if (clusterAssment_[i].minIndex != minIndex)
				{
					clusterChanged = true;
					clusterAssment_[i].minIndex = minIndex;
					clusterAssment_[i].minDist = minDist;
				}
			}

			//step two : update the centroids
			//std::cout << "update the centroids:" << std::endl;
			for (int cent = 0; cent < k; cent++) {
				decltype(slope_.size()) cnt = 0;
				double slope = 0.0;
				for (decltype(slope_.size()) i = 0; i < slope_.size(); i++)
				{
					//the first center
					if (clusterAssment_[i].minIndex == cent)
					{
						++cnt;
						slope += slope_[i];
					}
				}
				if (cnt != 0)
					slope /= cnt;

				centroids_[cent] = slope;
			}

		}//while
		for (int i = 0; i < clusterAssment_.size(); i++) {
			cluster.push_back(clusterAssment_[i].minIndex);
			//std::cout << clusterAssment_[i].minIndex << " " << std::endl;
		}
	}

	void KMeans::init(int k, const std::vector<int> &index, const std::vector<double> &slope)
	{
		this->k = k;
		index_ = index;
		slope_ = slope;
		assert(index_.size() == slope_.size());
		initCent();
	}

	void KMeans::initCent()
	{
		double min_d = slope_[0], max_d = slope_[0];
		for (decltype(slope_.size()) i = 1; i < slope_.size(); i++) {
			if (slope_[i] < min_d)
				min_d = slope_[i];
			else if (slope_[i] > max_d)
				max_d = slope_[i];
			else
				continue;
		}
		srand(time(NULL));
		double rangeIdx = max_d - min_d;
		//init centroids 
		for (int i = 0; i < k; i++) {
			centroids_.push_back(min_d + rangeIdx * (rand() / (double)RAND_MAX));
		}
		//std::cout << min_d << " " << max_d << " " << centroids_[0] << " " << centroids_[1] << std::endl;
	}

}