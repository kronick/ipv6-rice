#include "RiceApp.h"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;

void RiceApp::setup() {
    this->setFullScreen(true);
    //this->setWindowSize(1280, 1024);
    this->setFrameRate(60);
    try {
        //capture = Capture(960,720, Capture::getDevices()[0]);
        capture = Capture(1280,720, Capture::getDevices()[0]); downsample = 2;
        //capture = Capture(640,480, Capture::getDevices()[0]); downsample = 1;
        //capture = Capture(1920,1080, Capture::getDevices()[0]);
        //capture = Capture(1280,1024);
        capture.start();
    }
    catch( ... ) {
        console() << "Couldn't open capture device." << endl;
    }
    
    blobParams.minThreshold = 102;
    blobParams.maxThreshold = 104;
    blobParams.thresholdStep = 1;
    blobParams.minArea = 5;
    blobParams.maxArea = 1000;
    blobParams.minConvexity = 0.001;
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
    
    infoFont = Font("Courier", 24.0f);
    labelFont = Font("Courier-Bold", 12);
    
    drawThreshold = false;
    
    dottedCircleTexture = loadImage(loadResource("dotted-circle.png"));
    
    grainsFound = 0;
    beltSpeed = -5;
    beltXOffset = 0;
    
    newCameraFrame = false;
    runCamera = true;
    cameraThread = thread(&RiceApp::processCamera, this);
    
    ipPrefix = string("2607:f720:100:100::");
    ipCounter = 0;
    
    grainImageDirectory = string("/Library/WebServer/Documents/grain-images/");
    ///countFilePath = grainImageDirectory + "../counter.txt";
    
    ifstream countStream("/Library/WebServer/Documents/count.txt");
    string line;
    if(countStream.is_open()) {
        if(countStream.good()) {
            getline(countStream, line);
            ipCounter = atoi(line.c_str());
            cout << "Counting begins at: " << ipCounter;
        }
    }
    
    //cameraThread.join();    
}

void RiceApp::shutdown() {
    runCamera = false;
    cameraThread.join();
}

void RiceApp::mouseDrag( MouseEvent event ) {
    this->mouseDown(event);
}

void RiceApp::mouseDown( MouseEvent event ) {
    float thresh = this->getMousePos().y /
    (float)this->getWindowHeight() * 255;
    //blobParams.minThreshold = thresh-11;
    //blobParams.maxThreshold = thresh+10;
    blobParams.minThreshold = thresh - 1;
    blobParams.maxThreshold = thresh + 1;
    blobDetector = ExtendedBlobDetector(blobParams);
    blobDetector.create("SimpleBlob");
}

void RiceApp::keyDown(KeyEvent event) {
    if(event.getChar() == 'b') {
        drawThreshold = !drawThreshold;
    }
    if(event.getChar() == 'c') {
        runCamera = false;
    }    
}

void RiceApp::update() {
    // Transfer surface to OpenGL texture (all OpenGL calls must be in main thread)
    if(newCameraFrame) {
        oldCameraTexture = cameraTexture;
        cameraTexture = gl::Texture(cameraSurface);
        newCameraFrame = false;
        beltXOffset = 0;
        
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
                if(true || blobCenters[i].pt.x > cameraTexture.getWidth() * .75) {
                    // Add
                    grains.push_back(thisGrain);
                }
            }
            else {
                // Update
                grains[bestMatchIndex].moveTo(thisGrain.center);
                grains[bestMatchIndex].contour = thisGrain.contour;
                grains[bestMatchIndex].ageSinceUpdate = 0;
            }
        }        
    }
    
    beltXOffset += beltSpeed;

    labelMan.clear();
    vector<Grain> survivingGrains;
    float avgXSpeed;
    for(int i=0; i<grains.size(); i++) {
        if(grains[i].ageSinceUpdate < Grain::AGE_LIMIT) {
            survivingGrains.push_back(grains[i]);
            avgXSpeed += grains[i].velocity.x;
        }
    }
    grains = survivingGrains; 
    for(int i=0; i<grains.size(); i++) {
        grains[i].update();
        labelMan.add(&grains[i].label);

        // Assign the grain an ip address if it's old enough
        if(grains[i].age == Grain::AGE_THRESHOLD) {
            // Update label text
            ostringstream ipAddress;
            if(ipCounter < (1 << 16)) {
                ipAddress << ipPrefix << hex << ipCounter;
            }
            else {
                ipAddress << hex << ipPrefix << (ipCounter / (1<<16)) << ":" << ipCounter % (1<<16);
            }
            
            grains[i].setLabel(ipAddress.str());
            grainsFound++;
            ipCounter++;
        }    
        
        // If the grain has survived past the halfway mark, save its image
        if(grains[i].center.x * downsample < cameraSurface.getWidth() / 2 && grains[i].age > Grain::AGE_THRESHOLD && !grains[i].saved) {
            grains[i].saved = saveGrainImage(grains[i]);
            //saveGrainImage(grains[i]);
        }
    }    
    if(grains.size() > 0) {
        avgXSpeed /= grains.size();
        beltSpeed = avgXSpeed;
    }
    else beltSpeed = 0;
    beltSpeed = -5.5;
    //beltSpeed = 0;
    
    if(getElapsedFrames() % 60 == 0) {
        ofstream countStream("/Library/WebServer/Documents/count.txt");
        countStream << ipCounter;
        countStream.close();
    }
    
    labelMan.update(1);
}

void RiceApp::draw() {
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::enableAlphaBlending();
    if(cameraTexture) {
        glPushMatrix();
        float scaleFactor = min(getWindowWidth() / (float)capture.getWidth(),
                                getWindowHeight() / (float)capture.getHeight());
        
        gl::translate(0, (getWindowHeight() - capture.getHeight() * scaleFactor)/2);
        gl::scale(scaleFactor, scaleFactor);
        gl::color(1,1,1);

        gl::draw(cameraTexture);
        
        glPushMatrix();
        gl::translate(beltXOffset, 0);
        if(oldCameraTexture) {
            gl::color(1,1,1,0.2);
            gl::draw(oldCameraTexture);
        }
        
        glPopMatrix();        
        
        glPushMatrix();
            gl::scale(downsample,downsample);
        
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
                        gl::rotate((getElapsedFrames()+i)*0.5);
                        
                        //gl::draw(dottedCircleTexture, Rectf(-circleSize, -circleSize,
                        //                                     circleSize,  circleSize));
                        
                    glPopMatrix();
                }
            }
        
        glPopMatrix();
        
        labelMan.draw();
        
        glPopMatrix();
        
        ostringstream ss;
        ss << getAverageFps();
        string framerate(ss.str());
        
        //gl::drawString( "Framerate: " + framerate, ci::Vec2f( 10.0f, 20.0f ), Color::white(), infoFont );
        
        //\\]gl::drawString("http://newuntitledpage.com/rice", ci::Vec2f( 10.0f, 20.0f ), Color::white(), infoFont );
        
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

bool RiceApp::saveGrainImage(const Grain &g) {
    try {
        Area grainRegion(-50,-50, 50,50); grainRegion.offset(g.center * downsample);
        if(grainRegion.getX1() < 0)
            grainRegion.offset(ci::Vec2f(-grainRegion.getX1(),0));
        if(grainRegion.getY1() < 0)
            grainRegion.offset(ci::Vec2f(0,-grainRegion.getY1()));
        if(grainRegion.getX2() > cameraSurface.getWidth())
            grainRegion.offset(ci::Vec2f(cameraSurface.getWidth()-grainRegion.getX2(),0));
        if(grainRegion.getY2() > cameraSurface.getHeight())
            grainRegion.offset(ci::Vec2f(0,cameraSurface.getHeight()-grainRegion.getY2()));
        Surface grainImage = cameraSurface.clone(grainRegion);
        
        ostringstream pathStream;
        pathStream << grainImageDirectory << g.label.text << ".png";
        string path = pathStream.str();
        replace(path.begin(), path.end(), ':', '-');
        
        writeImage(path, grainImage, ImageTarget::Options(), "png");
        console() << "Wrote image to: " << pathStream.str() << endl;
        return true;
    }
    catch(ci::Exception e) {
        console() << "Error writing image." <<endl;
        return false;
    }
}

void RiceApp::processCamera() {
    while(runCamera) {
        //usleep(1000000);
        if(capture && capture.checkNewFrame()) {

            cameraSurface = capture.getSurface();
            if(cameraSurface == NULL) continue;
            Mat input(toOcv(cameraSurface));
            Mat output;
            if(downsample != 1)
                cv::resize(input, output, cv::Size(), 1.0/downsample, 1.0/downsample);        
            else
                output = input;
            //cv::GaussianBlur(output, output, cv::Size(9,9), 20);
            blobDetector.detect(output, blobCenters, blobContours);
            
            if(drawThreshold) {
                //float thresh = this->getMousePos().y /
                //                (float)this->getWindowHeight() * 255;
                float thresh = blobParams.minThreshold;
                //cv::cvtColor(input, output, CV_RGB2GRAY);
                
                Mat grayscaleImage(output.rows, output.cols, CV_8UC1 );
                Mat HSVImage;
                
                cvtColor(output, HSVImage, CV_RGB2HSV);
                int mix[] = {1,0};
                mixChannels(&HSVImage, 1, &grayscaleImage, 1, mix,1);
                
                cv::threshold(grayscaleImage, grayscaleImage, thresh, 255, THRESH_BINARY);
                cv::dilate(grayscaleImage, grayscaleImage, Mat(), cv::Point(-1,-1), 3);
                if(downsample != 1)
                    cv::resize(grayscaleImage, output, cv::Size(), downsample, downsample);   
                else 
                    output = grayscaleImage;
                //cv::dilate(output, output, Mat(), cv::Point(-1,-1), 7);
                //cv::GaussianBlur(output, output, cv::Size(9,9), 20);
                //output = grayscaleImage;
                
                cameraSurface = fromOcv(output);
            }
            
            newCameraFrame = true;
        }	    
    }
}

CINDER_APP_BASIC( RiceApp, RendererGl )
