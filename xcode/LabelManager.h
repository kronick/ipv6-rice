//
//  LabelManager.h
//  Rice
//
//  Created by Sam Kronick on 4/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Rice_LabelManager_h
#define Rice_LabelManager_h

#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "Label.h"

class LabelManager {
public:    
    std::vector<Label *> labels;
    
    
    LabelManager();
    //Label(ci::Vec2f basePoint, std::string label);
    
    void update(float dT);
    void draw();
    void clear();
    void add(Label *l);
    bool remove(const Label &l);
};

#endif
