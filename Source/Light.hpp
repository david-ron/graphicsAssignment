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
    glm::vec4 getPosition() const       { return lightPosition; }
    glm::vec3 getColor() const          { return lightColor; }
    glm::vec3 getAttenuation() const    { return lightAttenuation; }
    
    void Load(ci_istringstream& iss);
    bool ParseLine(std::vector<ci_string> token);
    
    void setPostion(glm::vec4 position);
    void setColor(glm::vec3 color);
    void setAttentuation(glm::vec3 attenuation);
    ~Light();
    
private:
    
    ci_string name; // The light name is mainly for debugging
    glm::vec4 lightPosition;
    glm::vec3 lightColor;
    glm::vec3 lightAttenuation;
    


};
