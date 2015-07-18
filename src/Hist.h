/**
 * Author: rodrigo
 * 2015
 */
#pragma once

#include <vector>
#include <ostream>

enum Dimension
{
	ANGLE, OTHER
};

struct Bins
{
	std::vector<double> bins;
	double step;
	Dimension dimension;

	Bins()
	{
		bins.clear();
		step = 0;
		dimension = OTHER;
	}
};

class Hist
{
public:
	Hist(const Dimension _dimension = OTHER);
	Hist(const std::vector<double> &_data);
	~Hist();

	void add(const double _element);
	void getBins(const double _binSize, const double _lowerBound, const double _upperBound, Bins &_bins) const;
	void getBins(const double _binSize, Bins &_bins) const;

private:
	std::vector<double> data;
	double minData;
	double maxData;
	Dimension dimension;
};

std::ostream& operator<<(std::ostream &_stream, const Bins &_bins);
void printBins(const Bins &_data);
