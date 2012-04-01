//
//  ExtendedBlobDetector.h
//  Rice
//
//  Created by Samuel Kronick on 1/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Rice_ExtendedBlobDetector_h
#define Rice_ExtendedBlobDetector_h

#include "CinderOpenCV.h"

namespace cv {
class ExtendedBlobDetector : public cv::SimpleBlobDetector {
public:    
    ExtendedBlobDetector(const cv::SimpleBlobDetector::Params &parameters = cv::SimpleBlobDetector::Params());

    void detect(const cv::Mat& image,
                std::vector<cv::KeyPoint>& keypoints,
                std::vector< std::vector<cv::Point> >& _contours);
    const void findBlobContours(const cv::Mat &image,
                          const cv::Mat &binaryImage,
                          std::vector<cv::SimpleBlobDetector::Center> &centers,
                          std::vector< std::vector<cv::Point> >& _contours);
};
}


#endif
