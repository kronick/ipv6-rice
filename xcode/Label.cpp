//
//  Label.cpp
//  Rice
//
//  Created by Sam Kronick on 4/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "Label.h"
#include "Grain.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"

using namespace std;
using namespace ci;

Label::Label() {
    this->center = Vec2f(0,0);
    //this->contour = PolyLine();
    this->age = 0;
}
Label::Label(Vec2f basePoint, std::string label) {
    this->basePoint = Vec2f(basePoint);
    this->text = label;
    this->baseOffset = 60;
    
    float angle = Rand::randFloat(M_PI*2);
    this->center = Vec2f(baseOffset * cos(angle) * 0.5,
                         baseOffset * sin(angle) * 0.5) + basePoint;
    this->mass = DEFAULT_MASS;
    
    this->drift = Vec2f(-2,0);
    yOffset = Rand::randFloat(40, 60);
    if(Rand::randBool()) yOffset *= -1;
    
    boundingBox = Rectf(0,0, 440,24);
    clearSpaceBox = Rectf(0,0, 440,32);
    boundingBox.offsetCenterTo(center);
    clearSpaceBox.offsetCenterTo(center);
    age = 0;
    
    leadLength = 10;
    
    alpha = 1;
    textureInitiated = false;
    xShift = 150;
}

void Label::setText(string text) {
    this->text = text;
    textureInitiated = false;   // Force regeneration of label texture
}

void Label::update(float dT) {
    if(!textureInitiated) {
        gl::pushMatrices();
        Area oldViewport = gl::getViewport();
        float w = boundingBox.getWidth();
        float h = boundingBox.getHeight();
        try {
            gl::Fbo frameBuffer(w,h);
            frameBuffer.bindFramebuffer();
            CameraOrtho cam(0,w, 0,h, -1, 1);
            gl::setMatrices(cam);
            gl::setViewport(Area(0,0,w,h));        
            //gl::setMatricesWindow(w,h);
            labelFont = Font("Courier-Bold", 24.0f);
            gl::clear(ColorA(1,1,1,0));
            //gl::clear(ColorA(0,0,0));
            gl::color(1,1,1);
            gl::drawString(text, Vec2f(0,12), Color(1,1,1), labelFont); 
            frameBuffer.unbindFramebuffer();
            textTexture = frameBuffer.getTexture();
            textureInitiated = true;
        }
        catch (Exception e) { }
        
        gl::setViewport(oldViewport);
        gl::popMatrices();
    }
    age++;
    velocity *= VELOCITY_DAMP;
    
    float fMag = (center.distance(basePoint) - baseOffset) * CENTER_FORCE;
    Vec2f force = basePoint - center;
    force.normalize();
    force *= fMag;

    Vec2f yForce(0, yOffset - (center.y - basePoint.y));
    force += yForce * YOFFSET_FORCE;
    
    force += externalForce;
    
    velocity += force / mass;
    
    center += velocity * dT;
    center += drift;
    boundingBox.offsetCenterTo(center);
    clearSpaceBox.offsetCenterTo(center);
}

void Label::draw() {
    // Fade in
    float maxAlpha = (age-Grain::AGE_THRESHOLD)/(float)Grain::AGE_LIMIT;
    if(alpha > maxAlpha) alpha = maxAlpha;
    if(alpha > 1) alpha = 1;
    if(alpha < 0) alpha = 0;
    
    gl::color(1,1,1, alpha);
    //gl::drawStrokedRect(boundingBox);
    Vec2f a, b;
    if(true || center.x > basePoint.x) {
        a = Vec2f(boundingBox.getUpperLeft().x - 7 + xShift, center.y);
        b = a - Vec2f(leadLength,0);
    }
    else {
        a = Vec2f(boundingBox.getUpperRight().x, center.y);
        b = a + Vec2f(leadLength,0);        
    }
    glLineWidth(2);
    gl::drawLine(a*2, b*2);
    gl::drawLine(b*2, basePoint*2);
    gl::drawSolidCircle(basePoint*2,4);
    gl::drawSolidCircle(a*2,4);
    
    /*
    glLineWidth(1);
    gl::color(1,0,0,alpha);
    gl::drawStrokedRect(boundingBox);
    gl::color(1,1,1,alpha);
    gl::drawStrokedRect(clearSpaceBox);
    gl::drawLine(center, externalForce+center);
    gl::drawSolidCircle(externalForce+center, 2);
    gl::color(1,1,1,alpha);
    */
    /*
    if(labelFont != NULL)
        gl::drawString(text, boundingBox.getUpperLeft()*2, ColorA(1,1,1, alpha), labelFont);     
     */
    Vec2f textPosition = boundingBox.getUpperLeft()*2 + Vec2f(xShift*2,6);
    if(textureInitiated && textTexture != NULL) {
        try {
            gl::draw(textTexture, textPosition);
        }
        catch(...) { app::console() << "Trouble drawing that texture..." << endl; }
    }
}

void Label::setBasePoint(ci::Vec2f newBasePoint) {
    basePoint = newBasePoint;
}

bool Label::operator==(const Label &other) const {
    return (basePoint == other.basePoint);
}