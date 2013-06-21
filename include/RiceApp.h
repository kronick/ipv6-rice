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
#include "cinder/thread.h"
#include "ExtendedBlobDetector.h"
#include "Grain.h"
#include "Label.h"
#include "LabelManager.h"

class RiceApp : public ci::app::AppBasic {
public:
	void setup();
    void shutdown();
	void mouseDown(ci::app::MouseEvent event);
    void mouseDrag(ci::app::MouseEvent event);
	void keyDown(ci::app::KeyEvent event);
	void update();
	void draw();
    void drawLabel(Label &l);
    bool saveGrainImage(const Grain &g);
    
    void processCamera();
    
    std::thread cameraThread;
    bool runCamera;

    int ipCounter;
    std::string ipPrefix;
    std::string grainImageDirectory;
    std::string countFilePath;
    float downsample;
    bool newCameraFrame;
    float beltSpeed;
    float beltXOffset;
    ci::Capture capture;
    ci::gl::Texture cameraTexture;
    ci::gl::Texture oldCameraTexture;
    ci::Surface cameraSurface;
    cv::ExtendedBlobDetector blobDetector;
    cv::SimpleBlobDetector::Params blobParams;
    std::vector< std::vector<cv::Point> > blobContours;
    std::vector< ci::PolyLine<ci::Vec2f> > blobPolyLines;
    std::vector<cv::KeyPoint> blobCenters;
    
    std::vector<Grain> grains;
    
    LabelManager labelMan;
    ci::Font infoFont;
    ci::Font labelFont;
    
    bool drawThreshold;
    
    ci::gl::Texture dottedCircleTexture;
    
    int grainsFound;
};


#endif
