//
//  Light.cpp
//  COMP371_Framework
//
//  Created by David Ronci on 2019-07-28.
//  Copyright © 2019 Concordia. All rights reserved.
//

#include "Light.h"
Light::Light():lPosition(0.0f,0.0f,0.0f,0.0f), lColor(0.0f,0.0f,0.0f), lAttenuation(0.0f,0.0f,0.0f){}

bool Light::ParseLine(std::vector<ci_string> token)
{
    if (token.empty())
    {
        return true;
    }
    if (token[0].empty() == false && token[0][0] == '#')
    {
        return true;
    }
    else if (token[0] == "name")
    {
        assert(token.size() > 2);
        assert(token[1] == "=");
        
        name = token[2];
    }
    else if (token[0] == "position")
    {
        assert(token.size() > 4);
        assert(token[1] == "=");
        
        lPosition.x = static_cast<float>(atof(token[2].c_str()));
        lPosition.y = static_cast<float>(atof(token[3].c_str()));
        lPosition.z = static_cast<float>(atof(token[4].c_str()));
        lPosition.w = static_cast<float>(atof(token[5].c_str()));
    
    }
    else if (token[0] == "lightattenuation")
    {
        assert(token.size() > 4);
        assert(token[1] == "=");
        
        lAttenuation.x = static_cast<float>(atof(token[2].c_str()));
        lAttenuation.y = static_cast<float>(atof(token[3].c_str()));
        lAttenuation.z = static_cast<float>(atof(token[4].c_str()));
    }
    else if (token[0] == "lightcolor")
    {
        assert(token.size() > 4);
        assert(token[1] == "=");
        
        lColor.x = static_cast<float>(atof(token[2].c_str()));
        lColor.y = static_cast<float>(atof(token[3].c_str()));
        lColor.z = static_cast<float>(atof(token[4].c_str()));
    }
    else
    {
        fprintf(stderr, "Error loading scene file... token:  %s!", token[0].c_str());
        getchar();
        exit(-1);
    }
    
    return true;
}

void Light::Load(ci_istringstream& iss)
{
    ci_string line;
    
    // Parse model line by line
    while(std::getline(iss, line))
    {
        // Splitting line into tokens
        ci_istringstream strstr(line);
        std::istream_iterator<ci_string, char, ci_char_traits> it(strstr);
        std::istream_iterator<ci_string, char, ci_char_traits> end;
        std::vector<ci_string> token(it, end);
        
        if (ParseLine(token) == false)
        {
            fprintf(stderr, "Error loading scene file... token:  %s!", token[0].c_str());
            getchar();
            exit(-1);
        }
    }
}
Light::~Light()
{
}
