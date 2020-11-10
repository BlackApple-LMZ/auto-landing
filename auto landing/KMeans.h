//
// Created by limz on 2020/11/6.
// kmeans聚类将直线分为左右两部分
//

#pragma once
#include <iostream>
#include <vector>

namespace autolanding_lmz {
	class KMeans
	{
	private:
		std::vector<double> slope_;
		std::vector<int> index_;
		int k{ 2 };
		std::vector<double> centroids_;

		typedef struct Node
		{
			int minIndex; //the index of each node
			double minDist;
			Node(int idx, double dist) :minIndex(idx), minDist(dist) {}
		}tNode;
		std::vector<tNode>  clusterAssment_;

		void initClusterAssment();
		void initCent();

	public:
		KMeans() {};
		~KMeans() {};
		void init(int k, const std::vector<int> &index, const std::vector<double> &slope);
		void kmeans(std::vector<int> &cluster);
	};
}

