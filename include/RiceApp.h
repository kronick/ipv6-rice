//
//  RiceApp.h
//  Rice
//
//  Created by Samuel Kronick on 1/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Rice_RiceApp_h
#define Rice_RiceApp_h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "ExtendedBlobDetector.h"
#include "Grain.h"

class RiceApp : public ci::app::AppBasic {
public:
	void setup();
	void mouseDown(ci::app::MouseEvent event);
    void mouseDrag(ci::app::MouseEvent event);
	void keyDown(ci::app::KeyEvent event);
	void update();
	void draw();
    
    ci::Capture capture;
    ci::gl::Texture texture;
    ci::Surface camSurface;
    cv::ExtendedBlobDetector blobDetector;
    cv::SimpleBlobDetector::Params blobParams;
    std::vector< std::vector<cv::Point> > blobContours;
    std::vector< ci::PolyLine<ci::Vec2f> > blobPolyLines;
    std::vector<cv::KeyPoint> blobCenters;
    
    std::vector<Grain> grains;
    
    ci::Font infoFont;
    
    bool drawThreshold;
    
    ci::gl::Texture dottedCircleTexture;
    
    int grainsFound;
};


#endif
