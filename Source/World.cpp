//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "World.h"
#include "Renderer.h"
#include "ParsingHelper.h"

#include "StaticCamera.h"
#include "FirstPersonCamera.h"
#include "Light.h"
#include "CubeModel.h"
#include "SphereModel.h"
#include "Animation.h"
#include "Billboard.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"
#include "TextureLoader.h"

#include "ParticleDescriptor.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"


using namespace std;
using namespace glm;

World* World::instance;


World::World()
{
    instance = this;

	// Setup Camera
	mCamera.push_back(new FirstPersonCamera(vec3(3.0f, 5.0f, 20.0f)));
	mCamera.push_back(new StaticCamera(vec3(3.0f, 30.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	mCamera.push_back(new StaticCamera(vec3(0.5f,  0.5f, 5.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	mCurrentCamera = 0;

    
#if defined(PLATFORM_OSX)
//    int billboardTextureID = TextureLoader::LoadTexture("Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("Textures/Particle.png");
#else
//    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/Particle.png");
#endif
    assert(billboardTextureID != 0);

    mpBillboardList = new BillboardList(2048, billboardTextureID);

    
    // TODO - You can un-comment out these 2 temporary billboards and particle system
    // That can help you debug billboards, you can set the billboard texture to billboardTest.png
    /*    Billboard *b = new Billboard();
     b->size  = glm::vec2(2.0, 2.0);
     b->position = glm::vec3(0.0, 3.0, 0.0);
     b->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
     
     Billboard *b2 = new Billboard();
     b2->size  = glm::vec2(2.0, 2.0);
     b2->position = glm::vec3(0.0, 3.0, 1.0);
     b2->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

     mpBillboardList->AddBillboard(b);
     mpBillboardList->AddBillboard(b2);
     */    // TMP
}

World::~World()
{
	// Models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		delete *it;
	}

	mModel.clear();

	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		delete *it;
	}

	mAnimation.clear();

	for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
	{
		delete *it;
	}

	mAnimationKey.clear();

	// Camera
	for (vector<Camera*>::iterator it = mCamera.begin(); it < mCamera.end(); ++it)
	{
		delete *it;
	}
	mCamera.clear();
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it < mParticleSystemList.end(); ++it)
    {
        delete *it;
    }
    mParticleSystemList.clear();
    
    for (vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        delete *it;
    }
    mParticleDescriptorList.clear();

    
	delete mpBillboardList;
}

World* World::GetInstance()
{
    return instance;
}

void World::Update(float dt)
{
	// User Inputs
	// 0 1 2 to change the Camera
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_1 ) == GLFW_PRESS)
	{
		mCurrentCamera = 0;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_2 ) == GLFW_PRESS)
	{
		if (mCamera.size() > 1)
		{
			mCurrentCamera = 1;
		}
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_3 ) == GLFW_PRESS)
	{
		if (mCamera.size() > 2)
		{
			mCurrentCamera = 2;
		}
	}

	// Spacebar to change the shader
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_0 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_SOLID_COLOR);
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_9 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_BLUE);
	}

    // Update animation and keys
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        (*it)->Update(dt);
    }


	// Update current Camera
	mCamera[mCurrentCamera]->Update(dt);

	// Update models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		(*it)->Update(dt);
	}
    
    // Update billboards
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it != mParticleSystemList.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    mpBillboardList->Update(dt);

}

void World::Draw()
{
	Renderer::BeginFrame();
	
	// Set shader to use
	glUseProgram(Renderer::GetShaderProgramID());

	// This looks for the MVP Uniform variable in the Vertex Program
    GLuint VMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint PMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
    GLuint worldLightLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
    GLuint lightColorLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
    GLuint lightAttenuationLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");

    // to phase out TODO
	GLuint VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
    // Send the view projection constants to the shader
    mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();

    mat4 V = mCamera[mCurrentCamera]->GetViewMatrix();
    mat4 P = mCamera[mCurrentCamera]->GetProjectionMatrix();
    vector<Light*> lights = mLight;
    Light* l = lights[0];
    
    vec4 lightPosition = l->getPosition();
    vec3 lightColor = l->getColor();
    vec3 lightAttenuation = l->getAttenuation();

    glUniform3f(lightColorLocation,lightColor.x ,lightColor.y, lightColor.z);
    glUniform3f(lightAttenuationLocation,lightAttenuation.x ,lightAttenuation.y, lightAttenuation.z);
    glUniform4f(worldLightLocation,lightPosition.x ,lightPosition.y, lightPosition.z, lightPosition.w);
    
    
    
    l = lights[1];
    vec4 lightPosition1 = l->getPosition();
    vec3 lightColor1 = l->getColor();
    vec3 lightAttenuation1 = l->getAttenuation();
    GLuint worldLightLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition1");
    GLuint lightColorLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor1");
    GLuint lightAttenuationLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation1");
    glUniform3f(lightColorLocation1,lightColor1.x ,lightColor1.y, lightColor1.z);
    glUniform3f(lightAttenuationLocation1,lightAttenuation1.x ,lightAttenuation1.y, lightAttenuation1.z);
    glUniform4f(worldLightLocation1,lightPosition1.x ,lightPosition1.y, lightPosition1.z, lightPosition1.w);
    
    l = lights[2];
    vec4 lightPosition2 = l->getPosition();
    vec3 lightColor2 = l->getColor();
    vec3 lightAttenuation2 = l->getAttenuation();
    GLuint worldLightLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition2");
    GLuint lightColorLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor2");
    GLuint lightAttenuationLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation2");
    glUniform3f(lightColorLocation2,lightColor2.x ,lightColor2.y, lightColor2.z);
    glUniform3f(lightAttenuationLocation2,lightAttenuation2.x ,lightAttenuation2.y, lightAttenuation2.z);
    glUniform4f(worldLightLocation2,lightPosition2.x ,lightPosition2.y, lightPosition2.z, lightPosition2.w);
    
    l = lights[3];
    vec4 lightPosition3 = l->getPosition();
    vec3 lightColor3 = l->getColor();
    vec3 lightAttenuation3 = l->getAttenuation();
    GLuint worldLightLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition3");
    GLuint lightColorLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor3");
    GLuint lightAttenuationLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation3");
    glUniform3f(lightColorLocation3 ,lightColor3.x ,lightColor3.y, lightColor3.z);
    glUniform3f(lightAttenuationLocation3, lightAttenuation3.x ,lightAttenuation3.y, lightAttenuation3.z);
    glUniform4f(worldLightLocation3,lightPosition3.x ,lightPosition3.y, lightPosition3.z, lightPosition3.w);
    
    l = lights[4];
    vec4 lightPosition4 = l->getPosition();
    vec3 lightColor4 = l->getColor();
    vec3 lightAttenuation4 = l->getAttenuation();
    GLuint worldLightLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition4");
    GLuint lightColorLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor4");
    GLuint lightAttenuationLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation4");
    glUniform3f(lightColorLocation4 ,lightColor4.x ,lightColor4.y, lightColor4.z);
    glUniform3f(lightAttenuationLocation4, lightAttenuation4.x ,lightAttenuation4.y, lightAttenuation4.z);
    glUniform4f(worldLightLocation4,lightPosition4.x ,lightPosition4.y, lightPosition4.z, lightPosition4.w);
    
    
    l = lights[5];
    vec4 lightPosition5 = l->getPosition();
    vec3 lightColor5 = l->getColor();
    vec3 lightAttenuation5 = l->getAttenuation();
    GLuint worldLightLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition5");
    GLuint lightColorLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor5");
    GLuint lightAttenuationLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation5");
    glUniform3f(lightColorLocation5 ,lightColor5.x ,lightColor5.y, lightColor5.z);
    glUniform3f(lightAttenuationLocation5, lightAttenuation5.x ,lightAttenuation5.y, lightAttenuation5.z);
    glUniform4f(worldLightLocation5,lightPosition5.x ,lightPosition5.y, lightPosition5.z, lightPosition5.w);
    
    l = lights[6];
    vec4 lightPosition6 = l->getPosition();
    vec3 lightColor6 = l->getColor();
    vec3 lightAttenuation6 = l->getAttenuation();
    GLuint worldLightLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition6");
    GLuint lightColorLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor6");
    GLuint lightAttenuationLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation6");
    glUniform3f(lightColorLocation6 ,lightColor6.x ,lightColor6.y, lightColor6.z);
    glUniform3f(lightAttenuationLocation6, lightAttenuation6.x ,lightAttenuation6.y, lightAttenuation6.z);
    glUniform4f(worldLightLocation6, lightPosition6.x ,lightPosition6.y, lightPosition6.z, lightPosition6.w);
    
    l = lights[7];
    vec4 lightPosition7 = l->getPosition();
    vec3 lightColor7 = l->getColor();
    vec3 lightAttenuation7 = l->getAttenuation();
    GLuint worldLightLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition7");
    GLuint lightColorLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor7");
    GLuint lightAttenuationLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation7");
    glUniform3f(lightColorLocation7 ,lightColor7.x ,lightColor7.y, lightColor7.z);
    glUniform3f(lightAttenuationLocation7, lightAttenuation7.x ,lightAttenuation7.y, lightAttenuation7.z);
    glUniform4f(worldLightLocation7, lightPosition7.x ,lightPosition7.y, lightPosition7.z, lightPosition7.w);
    glUniformMatrix4fv(VMatrixLocation, 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(PMatrixLocation, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);
    

// TODO for 8 lights...
//    for (vector<Light*>::iterator it = mLight.begin(); it<mLight.end();++it)
//    {
//        (*it)->
//    }
	// Draw models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
        
		(*it)->Draw();
	}
    
    
	// Draw Path Lines
	
	// Set Shader for path lines
	unsigned int prevShader = Renderer::GetCurrentShader();
	Renderer::SetShader(SHADER_PATH_LINES);
	glUseProgram(Renderer::GetShaderProgramID());

	// Send the view projection constants to the shader
	VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");

	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
		glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

		(*it)->Draw();
	}

	for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
	{
		mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
		glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

		(*it)->Draw();
	}

    Renderer::CheckForErrors();
    
    // Draw Billboards
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mpBillboardList->Draw();
    glDisable(GL_BLEND);


	// Restore previous shader
	Renderer::SetShader((ShaderType) prevShader);

	Renderer::EndFrame();
}

void World::LoadScene(const char * scene_path)
{
	// Using case-insensitive strings and streams for easier parsing
	ci_ifstream input;
	input.open(scene_path, ios::in);

	// Invalid file
	if(input.fail() )
	{	 
		fprintf(stderr, "Error loading file: %s\n", scene_path);
		getchar();
		exit(-1);
	}

	ci_string item;
	while( std::getline( input, item, '[' ) )   
	{
        ci_istringstream iss( item );

		ci_string result;
		if( std::getline( iss, result, ']') )
		{
			if( result == "cube" )
			{
				// Box attributes
				CubeModel* cube = new CubeModel();
				cube->Load(iss);
				mModel.push_back(cube);
			}
            else if( result == "sphere" )
            {
                SphereModel* sphere = new SphereModel();
                sphere->Load(iss);
                mModel.push_back(sphere);
            }
			else if ( result == "animationkey" )
			{
				AnimationKey* key = new AnimationKey();
				key->Load(iss);
				mAnimationKey.push_back(key);
			}
			else if (result == "animation")
			{
				Animation* anim = new Animation();
				anim->Load(iss);
				mAnimation.push_back(anim);
			}
            else if (result == "particledescriptor")
            {
                ParticleDescriptor* psd = new ParticleDescriptor();
                psd->Load(iss);
                AddParticleDescriptor(psd);
            }
            else if(result == "light"){
                Light* light = new Light();
                light->Load(iss);
                mLight.push_back(light);
            }
			else if ( result.empty() == false && result[0] == '#')
			{
				// this is a comment line
			}
			else
			{
				fprintf(stderr, "Error loading scene file... !");
				getchar();
				exit(-1);
			}
	    }
	}
	input.close();
	// Set Animation vertex buffers
	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		// Draw model
		(*it)->CreateVertexBuffer();
	}
}

Animation* World::FindAnimation(ci_string animName)
{
    for(std::vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        if((*it)->GetName() == animName)
        {
            return *it;
        }
    }
    return nullptr;
}

AnimationKey* World::FindAnimationKey(ci_string keyName)
{
    for(std::vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        if((*it)->GetName() == keyName)
        {
            return *it;
        }
    }
    return nullptr;
}

const Camera* World::GetCurrentCamera() const
{
     return mCamera[mCurrentCamera];
}

void World::AddBillboard(Billboard* b)
{
    mpBillboardList->AddBillboard(b);
}

void World::RemoveBillboard(Billboard* b)
{
    mpBillboardList->RemoveBillboard(b);
}

void World::AddParticleSystem(ParticleSystem* particleSystem)
{
    mParticleSystemList.push_back(particleSystem);
}

void World::RemoveParticleSystem(ParticleSystem* particleSystem)
{
    vector<ParticleSystem*>::iterator it = std::find(mParticleSystemList.begin(), mParticleSystemList.end(), particleSystem);
    mParticleSystemList.erase(it);
}

void World::AddParticleDescriptor(ParticleDescriptor* particleDescriptor)
{
    mParticleDescriptorList.push_back(particleDescriptor);
}

ParticleDescriptor* World::FindParticleDescriptor(ci_string name)
{
    for(std::vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        if((*it)->GetName() == name)
        {
            return *it;
        }
    }
    return nullptr;
}
