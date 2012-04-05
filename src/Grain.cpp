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
    this->age = this->ageSinceUpdate = 0;
    this->saved = false;
}
Grain::Grain(Vec2f center, ci::PolyLine<ci::Vec2f> contour) {
    this->center = Vec2f(center);
    this->centerTarget = this->center;
    this->contour = contour;
    this->age = this->ageSinceUpdate = 0;
    
    this->label = Label(this->center, string("2001:DB8::026:100F"));
    this->drift = Vec2f(-3,0);
    this->saved = false;
}

void Grain::setLabel(string text) {
    label.setText(text);
}

bool Grain::isSameGrain(Grain other) {
    float dX = center.x - other.center.x;
    float dY = center.y - other.center.y;
    return (dX < CENTER_THRESH_X_MAX && dX > CENTER_THRESH_X_MIN && dY < CENTER_THRESH_Y_MAX && dY > CENTER_THRESH_Y_MIN);
    //return (other.center.distance(center) < CENTER_THRESHOLD && abs(center.y - other.center.y) < CENTER_THRESHOLD/4);
    //return abs(other.center.x - center.x) < CENTER_THRESHOLD;
}

float Grain::distanceTo(Grain other) {
    return other.center.distance(center);
}

void Grain::moveTo(Vec2f newCenter) {
    velocity = newCenter - centerTarget;
    centerTarget = newCenter;
}

void Grain::update() {
    //velocity *= VELOCITY_DAMP;
    //centerTarget += velocity * 0.01;
    
    centerTarget += drift;
    center += (centerTarget - center) * CENTER_K;
        
    label.setBasePoint(center);
    float newAlpha = 1.2-1.2*(ageSinceUpdate/(float)AGE_LIMIT);
    if (newAlpha > 1) newAlpha = 1;
    label.alpha = newAlpha;
    //label.update(1);
    age++; ageSinceUpdate++;
}

void Grain::draw() {
    //gl::draw(contour);
    //label.draw();
}