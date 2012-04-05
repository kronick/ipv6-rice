//
//  Label.h
//  Rice
//
//  Created by Sam Kronick on 4/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Rice_Label_h
#define Rice_Label_h

#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"


class Label {
public:    
    ci::Rectf boundingBox;
    ci::Rectf clearSpaceBox;
    ci::Vec2f center;
    ci::Vec2f velocity;
    ci::Vec2f drift;
    ci::Vec2f basePoint;
    ci::Vec2f externalForce;
    float mass;
    float baseOffset;
    float yOffset;
    float xShift;
    int age;
    float alpha;
    static const float VELOCITY_DAMP = 0.6;
    static const float DEFAULT_MASS = 10;
    static const float CENTER_FORCE = 0.3;
    static const float YOFFSET_FORCE = 0.5;
    
    ci::PolyLine<ci::Vec2f> contour;
    float length;
    float angle;
    std::string text;
    float leadLength;
    ci::Font labelFont;
    ci::gl::Texture textTexture;
    bool textureInitiated;
    
    Label();
    Label(ci::Vec2f basePoint, std::string label);
    void setText(std::string text);
    //float distanceTo(Label other);
    void update(float dT);
    void draw();
    void setBasePoint(ci::Vec2f newBasePoint);
    bool operator==(const Label &other) const;
};

#endif
