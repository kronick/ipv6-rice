//
//  ExtendedBlobDetector.cpp
//  Rice
//
//  Created by Samuel Kronick on 1/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ExtendedBlobDetector.h"

using namespace cv;
using namespace std;

ExtendedBlobDetector::ExtendedBlobDetector(const Params &parameters) : SimpleBlobDetector(parameters) {
}

void ExtendedBlobDetector::detect(const Mat& image,
                                            vector<KeyPoint>& keypoints,
                                            vector< vector<Point> >& _contours) {
	keypoints.clear();
    _contours.clear();
    Mat grayscaleImage(image.rows, image.cols, CV_8UC1 );
    Mat HSVImage;
    
	if (image.channels() == 3) {
		//cvtColor(image, grayscaleImage, CV_BGR2GRAY);
        cvtColor(image, HSVImage, CV_RGB2HSV);
        int mix[] = {1,0};
        mixChannels(&HSVImage, 1, &grayscaleImage, 1, mix,1);
    }
	else
		grayscaleImage = image;
    
	vector< vector<Center> > centers;
    vector< vector< vector<Point> > > contours;
    
	for (double thresh = params.minThreshold; thresh < params.maxThreshold; thresh += params.thresholdStep) {
		Mat binarizedImage;
		threshold(grayscaleImage, binarizedImage, thresh, 255, THRESH_BINARY);
        
        // Dilate/erode helps break up clumps
        cv::dilate(binarizedImage, binarizedImage, Mat(), cv::Point(-1,-1), 3);
		
        vector<Center> curCenters;
        vector< vector<Point> > curContours;
		findBlobContours(grayscaleImage, binarizedImage, curCenters, curContours);
		vector< vector<Center> > newCenters;
        vector< vector< vector<Point> > > newContours;
		for (size_t i = 0; i < curCenters.size(); i++) {
			bool isNew = true;
			for (size_t j = 0; j < centers.size(); j++) {
				double dist = norm(centers[j][ centers[j].size() / 2 ].location - curCenters[i].location);
				isNew = dist >= params.minDistBetweenBlobs && dist >= centers[j][ centers[j].size() / 2 ].radius && dist >= curCenters[i].radius;
				if (!isNew) {
					centers[j].push_back(curCenters[i]);
                    contours[j].push_back(curContours[i]);
                    
                    // Sort this point + contour according to radius
					size_t k = centers[j].size() - 1;
					while( k > 0 && centers[j][k].radius < centers[j][k-1].radius )
					{
						centers[j][k] = centers[j][k-1];
                        contours[j][k] = contours[j][k-1];
						k--;
					}
					centers[j][k] = curCenters[i];
                    contours[j][k] = curContours[i];
                    
					break;
				}
			}
			if (isNew) {
				newCenters.push_back(vector<Center> (1, curCenters[i]));
                newContours.push_back(vector< vector<Point> > (1, curContours[i]));
                
				//centers.push_back(vector<Center> (1, curCenters[i]));
			}
		}
		std::copy(newCenters.begin(), newCenters.end(), std::back_inserter(centers));
		std::copy(newContours.begin(), newContours.end(), std::back_inserter(contours));        
	}
    
	for (size_t i = 0; i < centers.size(); i++) {
		if (centers[i].size() < params.minRepeatability)
			continue;
		Point2d sumPoint(0, 0);
		double normalizer = 0;
		for (size_t j = 0; j < centers[i].size(); j++)
		{
			sumPoint += centers[i][j].confidence * centers[i][j].location;
			normalizer += centers[i][j].confidence;
		}
		sumPoint *= (1. / normalizer);
		KeyPoint kpt(sumPoint, (float)(centers[i][centers[i].size() / 2].radius));
		keypoints.push_back(kpt);
        
        _contours.push_back(contours[i][contours[i].size() / 2]);
	}    
}

const void ExtendedBlobDetector::findBlobContours(const Mat &image,
                            const Mat &binaryImage,
                            vector<Center> &centers,
                            vector< vector<Point> >& _contours){
	(void)image;
	centers.clear();
    _contours.clear();
    
	vector < vector<Point> > contours;
	Mat tmpBinaryImage = binaryImage.clone();
	findContours(tmpBinaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        
	for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
	{
		Center center;
		center.confidence = 1;
		Moments moms = moments(Mat(contours[contourIdx]));
		if (params.filterByArea)
		{
			double area = moms.m00;
			if (area < params.minArea || area >= params.maxArea)
				continue;
		}
        
		if (params.filterByCircularity)
		{
			double area = moms.m00;
			double perimeter = arcLength(Mat(contours[contourIdx]), true);
			double ratio = 4 * CV_PI * area / (perimeter * perimeter);
			if (ratio < params.minCircularity || ratio >= params.maxCircularity)
				continue;
		}
        
		if (params.filterByInertia)
		{
			double denominator = sqrt(pow(2 * moms.mu11, 2) + pow(moms.mu20 - moms.mu02, 2));
			const double eps = 1e-2;
			double ratio;
			if (denominator > eps)
			{
				double cosmin = (moms.mu20 - moms.mu02) / denominator;
				double sinmin = 2 * moms.mu11 / denominator;
				double cosmax = -cosmin;
				double sinmax = -sinmin;
                
				double imin = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmin - moms.mu11 * sinmin;
				double imax = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmax - moms.mu11 * sinmax;
				ratio = imin / imax;
			}
			else
			{
				ratio = 1;
			}
            
			if (ratio < params.minInertiaRatio || ratio >= params.maxInertiaRatio)
				continue;
            
			center.confidence = ratio * ratio;
		}
        
		if (params.filterByConvexity)
		{
			vector < Point > hull;
			convexHull(Mat(contours[contourIdx]), hull);
			double area = contourArea(Mat(contours[contourIdx]));
			double hullArea = contourArea(Mat(hull));
			double ratio = area / hullArea;
			if (ratio < params.minConvexity || ratio >= params.maxConvexity)
				continue;
		}
        
		center.location = Point2d(moms.m10 / moms.m00, moms.m01 / moms.m00);
        
		if (params.filterByColor)
		{
			if (binaryImage.at<uchar> (cvRound(center.location.y), cvRound(center.location.x)) != params.blobColor)
				continue;
		}
        
		//compute blob radius
		{
			vector<double> dists;
			for (size_t pointIdx = 0; pointIdx < contours[contourIdx].size(); pointIdx++)
			{
				Point2d pt = contours[contourIdx][pointIdx];
				dists.push_back(norm(center.location - pt));
			}
			std::sort(dists.begin(), dists.end());
			center.radius = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
		}
        
		centers.push_back(center);
        _contours.push_back(contours[contourIdx]);
	}
}
