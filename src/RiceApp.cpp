#include "RiceApp.h"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;

void RiceApp::setup() {
    //this->setFullScreen(true);
    this->setWindowSize(1280, 720);
    try {
        capture = Capture(1280,720, Capture::getDevices()[0]);
        //capture = Capture(640,480, Capture::getDevices()[0]);
        //capture = Capture(1920,1080, Capture::getDevices()[0]);
        //capture = Capture(1280,1024);
        capture.start();
    }
    catch( ... ) {
        console() << "Couldn't open capture device." << endl;
    }
    
    blobParams.minThreshold = 160;
    blobParams.maxThreshold = 180;
    blobParams.thresholdStep = 10;
    blobParams.minArea = 50;
    blobParams.maxArea = 4000;
    blobParams.minConvexity = 0.1;
    blobParams.maxConvexity = 10;
    blobParams.minCircularity = 0.3;
    blobParams.maxCircularity = 1;
    blobParams.blobColor = 0;
    blobParams.filterByConvexity = false;
    blobParams.filterByArea = true;
    blobParams.filterByColor = true;
    blobParams.filterByInertia = false;
    blobParams.filterByCircularity = false;
    
    blobDetector = ExtendedBlobDetector(blobParams);
    blobDetector.create("SimpleBlob");
    
    infoFont = Font("Helvetica", 24.0f);
    
    drawThreshold = false;
    
    dottedCircleTexture = loadImage(loadResource("dotted-circle.png"));
    
    grainsFound = 0;
}

void RiceApp::mouseDrag( MouseEvent event ) {
    this->mouseDown(event);
}

void RiceApp::mouseDown( MouseEvent event ) {
    float thresh = this->getMousePos().y /
    (float)this->getWindowHeight() * 255;
    //blobParams.minThreshold = thresh-11;
    //blobParams.maxThreshold = thresh+10;
    blobParams.minThreshold = thresh - 5;
    blobParams.maxThreshold = thresh + 6;
    blobDetector = ExtendedBlobDetector(blobParams);
    blobDetector.create("SimpleBlob");
}

void RiceApp::keyDown(KeyEvent event) {
    if(event.getChar() == 'b') {
        drawThreshold = !drawThreshold;
    }
}

void RiceApp::update() {
	if(capture && capture.checkNewFrame() ) {
        camSurface = capture.getSurface();
		Mat input(toOcv(camSurface));
        Mat output;
        cv::resize(input, output, cv::Size(), 0.5, 0.5);
        //output = input;
        //cv::GaussianBlur(output, output, cv::Size(9,9), 20);
        blobDetector.detect(output, blobCenters, blobContours);
        blobPolyLines.clear();
        PolyLine<ci::Vec2f> _pl;
        
        vector<Grain> currentGrains;
        
        for(int i=0; i<blobContours.size(); i++) {
            _pl = PolyLine<ci::Vec2f>();
            for(int j=0; j<blobContours[i].size(); j++) {
                _pl.push_back(fromOcv(blobContours[i][j]));
            }
            blobPolyLines.push_back(_pl);
            
            Grain thisGrain(ci::Vec2f(blobCenters[i].pt.x, blobCenters[i].pt.y), _pl);
            
            bool alreadyExists = false;
            int bestMatchIndex = -1;
            float bestMatchDistance = MAXFLOAT;
            for(int k=0; k<grains.size(); k++) {
                if(grains[k].isSameGrain(thisGrain)) {
                    if(grains[k].distanceTo(thisGrain) < bestMatchDistance) {
                        bestMatchIndex = k;
                        bestMatchDistance = grains[k].distanceTo(thisGrain);
                    }
                    alreadyExists = true;
                }
            }
            
            if(!alreadyExists) {
                // Add
                grains.push_back(thisGrain);
                grainsFound++;
            }
            else {
                // Update
                grains[bestMatchIndex].center = thisGrain.center;
                grains[bestMatchIndex].contour = thisGrain.contour;
                grains[bestMatchIndex].age = 0;
            }
        }
        
        if(drawThreshold) {
            float thresh = this->getMousePos().y /
                            (float)this->getWindowHeight() * 255;
            //cv::cvtColor(input, output, CV_RGB2GRAY);
            
            Mat grayscaleImage(input.rows, input.cols, CV_8UC1 );
            Mat HSVImage;
            
            cvtColor(input, HSVImage, CV_RGB2HSV);
            int mix[] = {1,0};
            mixChannels(&HSVImage, 1, &grayscaleImage, 1, mix,1);
            
            cv::threshold(grayscaleImage, output, thresh, 255, THRESH_BINARY);
            //cv::erode(output, output, Mat());
            //cv::GaussianBlur(output, output, cv::Size(9,9), 20);
            //output = grayscaleImage;
        }
        
        if(!drawThreshold)
            texture = gl::Texture(camSurface);
            //texture = gl::Texture(fromOcv(input));
        else
            texture = gl::Texture(fromOcv(output));
        
        vector<Grain> survivingGrains;
        for(int i=0; i<grains.size(); i++) {
            grains[i].update();
            if(grains[i].age < 7)
                survivingGrains.push_back(grains[i]);
        }
        grains = survivingGrains;
	}	
}

void RiceApp::draw() {
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::enableAlphaBlending();
    if(texture) {
        glPushMatrix();
        float scaleFactor = min(getWindowWidth() / (float)capture.getWidth(),
                                getWindowHeight() / (float)capture.getHeight());
        gl::scale(scaleFactor, scaleFactor);
        gl::color(255,255,255);
        gl::draw(texture);
        
        glPushMatrix();
            gl::scale(2,2);
            glEnable(GL_LINE_SMOOTH);
            //glLineWidth(3);
            /*
            for(int i=0; i<blobPolyLines.size(); i++) {
                gl::draw(blobPolyLines[i]);
            }
             */
            for(int i=0; i<grains.size(); i++) {
                //grains[i].draw();
                //gl::color(0, 0, 1, 1 - grains[i].age/30.);
                
                //gl::drawStrokedCircle(grains[i].center, 20);

                int circleSize = 35;
                if(grains[i].center.x > circleSize) {
                    grains[i].draw();
                    gl::color(1, 1, 1);
                    glPushMatrix();
                        gl::translate(grains[i].center);
                        gl::rotate((getElapsedFrames()+i)*5);
                        gl::draw(dottedCircleTexture, Rectf(-circleSize, -circleSize,
                                                             circleSize,  circleSize));
                    glPopMatrix();
                }
            }
        glPopMatrix();
        
        glPopMatrix();
        
        ostringstream ss;
        ss << getAverageFps();
        string framerate(ss.str());
        
        gl::drawString( "Framerate: " + framerate, ci::Vec2f( 10.0f, 20.0f ), Color::white(), infoFont );
        
        ostringstream ss2;
        float thresh = this->getMousePos().y /
        (float)this->getWindowHeight() * 255;
        ss2 << thresh;
        string threshold(ss2.str());
        
        gl::drawString( "Threshold: " + threshold, ci::Vec2f( 10.0f, 40.0f ), Color::white(), infoFont );
        
        
        ostringstream ss3;
        ss3 << grainsFound;
        string grainsFoundString(ss3.str());
        gl::drawString( "Grains: " + grainsFoundString, ci::Vec2f( 10.0f,60.0f ), Color::white(), infoFont );
        
        ostringstream ss4;
        ss4 << grainsFound/(float)getElapsedSeconds();
        string grainRate(ss4.str());
        gl::drawString( "Grains/sec: " + grainRate, ci::Vec2f( 10.0f,80.0f ), Color::white(), infoFont );        
    }
}


CINDER_APP_BASIC( RiceApp, RendererGl )
