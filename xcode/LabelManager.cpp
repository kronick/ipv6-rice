//
//  LabelManager.cpp
//  Rice
//
//  Created by Sam Kronick on 4/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "LabelManager.h"
#include "Grain.h"
using namespace std;
using namespace ci;

LabelManager::LabelManager() {
    
}

void LabelManager::update(float dT) {
    for(int i=0; i<labels.size(); i++) {
        Vec2f overlapForce;
        Rectf a = labels[i]->clearSpaceBox;
        // Check for overlap
        for(int j=0; j<labels.size(); j++) {
            Rectf b = labels[j]->clearSpaceBox;
            if(j != i && labels[j]->age > Grain::AGE_THRESHOLD && a.intersects(b)) {               
                float mag = b.getClipBy(a).calcArea() * 0.001;
                mag *= labels[j]->age / (float)labels[i]->age;
                if(mag > 4) mag = 4;
                //app::console() << mag << endl;
                //float mag = b.getCenter().y - a.getCenter().y
                /*
                Vec2f dir = labels[i]->center - labels[j]->center;
                if(dir.lengthSquared() > 0) {
                    dir.normalize();
                    overlapForce += (dir * mag);
                }
                */
                overlapForce += Vec2f(0,(labels[i]->center.y > labels[j]->center.y) ? 1 : -1 * mag);
            }
        }
        if(overlapForce.x != overlapForce.x ||
           overlapForce.y != overlapForce.y)
            overlapForce = Vec2f();
        overlapForce.x = 0;
        labels[i]->externalForce = overlapForce;
        labels[i]->update(dT);
    }
}

void LabelManager::draw() {
    for(int i=0; i<labels.size(); i++) {
        labels[i]->draw();
    }    
}

void LabelManager::add(Label *l) {
    labels.push_back(l);
}

void LabelManager::clear() {
    labels.clear();
}

bool LabelManager::remove(const Label &l) {
    /*
    // find the element
    auto it = std::find(labels.begin(), labels.end(), l);
    if (it == labels.end()) return false;
    // here we swap the element to be removed with the last one
    *it = labels.back();
    // and then resize the vector by one
    labels.resize(labels.size()-1);
    return true;    
    */
    return false;
}