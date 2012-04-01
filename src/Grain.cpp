//
//  Grain.cpp
//  Rice
//
//  Created by Samuel Kronick on 1/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "Grain.h"

using namespace std;
using namespace ci;

Grain::Grain() {
    this->center = Vec2f(0,0);
    //this->contour = PolyLine();
    this->age = 0;
}
Grain::Grain(Vec2f center, ci::PolyLine<ci::Vec2f> contour) {
    this->center = Vec2f(center);
    this->contour = contour;
    this->age = 0;
}

bool Grain::isSameGrain(Grain other) {
    return (other.center.distance(center) < CENTER_THRESHOLD);
    //return abs(other.center.x - center.x) < CENTER_THRESHOLD;
}

float Grain::distanceTo(Grain other) {
    return other.center.distance(center);
}

void Grain::update() {
    age++;
}

void Grain::draw() {
    gl::draw(contour);
}