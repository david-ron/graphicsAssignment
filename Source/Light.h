//
//  Light.hpp
//  COMP371_Framework
//
//  Created by David Ronci on 2019-07-28.
//  Copyright © 2019 Concordia. All rights reserved.
//
#pragma once
#ifndef Light_hpp
#define Light_hpp

#include <stdio.h>

#endif /* Light_hpp */

#include <glm/glm.hpp>
#include <iostream>
#include "ParsingHelper.h"
#include <vector>
class Light
{
public:
    Light();
    glm::vec4 getPosition() const       { return lPosition;}
    glm::vec3 getColor() const          { return lColor;}
    glm::vec3 getAttenuation() const    { return lAttenuation;}
    
    void Load(ci_istringstream& iss);
    bool ParseLine(std::vector<ci_string> token);

    ~Light();
    
private:
    ci_string name; // The light name is mainly for debugging
    glm::vec4 lPosition;
    glm::vec3 lColor;
    glm::vec3 lAttenuation;
};
