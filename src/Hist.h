/**
 * Author: rodrigo
 * 2015
 */
#pragma once

#include <vector>
#include <ostream>

using namespace std;

enum Dimension
{
	ANGLE, OTHER
};

struct Bins
{
	vector<double> bins;
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
	Hist(const vector<double> &_data);
	~Hist();

	void add(const double _element);
	void getBins(const int _binsNumber, const double _lowerBound, const double _upperBound, Bins &_bins) const;
	void getBins(const int _binsNumber, Bins &_bins) const;

private:
	vector<double> data;
	double minData;
	double maxData;
	Dimension dimension;
};

ostream& operator<<(ostream &_stream, const Bins &_bins);
void printBins(const Bins &_data);