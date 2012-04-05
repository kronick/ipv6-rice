//
//  Grain.h
//  Rice
//
//  Created by Samuel Kronick on 1/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Rice_Grain_h
#define Rice_Grain_h

#include "cinder/app/AppBasic.h"
#include "Label.h"
#include "LabelManager.h"

class Grain {
public:    
    static const int AGE_LIMIT = 20;
    static const int AGE_THRESHOLD = 10;
    static const float CENTER_THRESHOLD = 60;
    static const float CENTER_THRESH_X_MAX = 50;
    static const float CENTER_THRESH_X_MIN = -20;
    static const float CENTER_THRESH_Y_MAX = 7;
    static const float CENTER_THRESH_Y_MIN = -7;
    ci::Vec2f center;
    ci::Vec2f centerTarget;
    ci::Vec2f velocity;
    ci::Vec2f drift;
    static const float VELOCITY_DAMP = 0.95;
    static const float CENTER_K = 0.7;
    ci::PolyLine<ci::Vec2f> contour;
    float length;
    float angle;
    int age;
    int ageSinceUpdate;
    Label label;
    LabelManager labelMan;
    bool saved;
    
    Grain();
    Grain(ci::Vec2f center, ci::PolyLine<ci::Vec2f>contour);
    void setLabel(std::string text);
    bool isSameGrain(Grain other);
    float distanceTo(Grain other);
    void update();
    void draw();    
    
    void moveTo(ci::Vec2f newCenter);
};

#endif
