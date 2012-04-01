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

class Grain {
public:    
    static const float CENTER_THRESHOLD = 60;
    ci::Vec2f center;
    ci::PolyLine<ci::Vec2f> contour;
    float length;
    float angle;
    int age;
    std::string label;
    
    Grain();
    Grain(ci::Vec2f center, ci::PolyLine<ci::Vec2f>contour);
    bool isSameGrain(Grain other);
    float distanceTo(Grain other);
    void update();
    void draw();
};

#endif
