//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 15/7/15.
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//
#include "Light.h"
#include "Billboard.h"
#include "Renderer.h"
#include "World.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/common.hpp>


using namespace std;
using namespace glm;


// This is used for sorting the list of Billboards
bool CompareBillboardAlongZ::operator()(const Billboard* a, const Billboard* b)
{
    mat4 viewMatrix = World::GetInstance()->GetCurrentCamera()->GetViewMatrix();
    return ((viewMatrix*vec4(a->position, 1.0f)).z < (viewMatrix*vec4(b->position, 1.0f)).z);
}


BillboardList::BillboardList(unsigned int maxNumBillboards, int textureID)
: mTextureID(textureID), mMaxNumBillboards(maxNumBillboards)
{
    // Pre-allocate Vertex Buffer - 6 vertices by billboard (2 triangles)
    mVertexBuffer.resize(maxNumBillboards * 6);
    
    // Initialize vertices to white, with UVs taking all the texture
    for (int i=0; i<(int)mVertexBuffer.size(); ++i)
    {
        // White vertex
        mVertexBuffer[i].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Normal on Z axis
        mVertexBuffer[i].normal = vec3(0.0f, 0.0f, 1.0f);
        
        // Texture coordinates
        switch(i%6)
        {
            case 0: mVertexBuffer[i].textureCoordinate = vec2(0.0f, 1.0f); break;
            case 1: mVertexBuffer[i].textureCoordinate = vec2(0.0f, 0.0f); break;
            case 2: mVertexBuffer[i].textureCoordinate = vec2(1.0f, 1.0f); break;
            case 3: mVertexBuffer[i].textureCoordinate = vec2(1.0f, 1.0f); break;
            case 4: mVertexBuffer[i].textureCoordinate = vec2(0.0f, 0.0f); break;
            case 5: mVertexBuffer[i].textureCoordinate = vec2(1.0f, 0.0f); break;
        }
    }
    
    // Create a vertex array
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    // Upload Vertex Buffer to the GPU, keep a reference to it (mVertexBufferID)
    // Note the vertex buffer will change over time, we use GL_DYNAMIC_DRAW
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mVertexBuffer.size() * sizeof(BillboardVertex), &mVertexBuffer[0], GL_DYNAMIC_DRAW);
    Renderer::CheckForErrors();
    
    // 1st attribute buffer : vertex Positions
    glVertexAttribPointer(0,                // attribute. No particular reason for 0, but must match the layout in the shader.
                          3,                // size
                          GL_FLOAT,        // type
                          GL_FALSE,        // normalized?
                          sizeof(BillboardVertex), // stride
                          (void*)0        // array buffer offset
                          );
    glEnableVertexAttribArray(0);

    // 2nd attribute buffer : vertex normal
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(BillboardVertex),
                          (void*)sizeof(vec3)    // Normal is Offseted by vec3 (see class Vertex)
                          );
    glEnableVertexAttribArray(1);

    
    // 3rd attribute buffer : vertex color
    glVertexAttribPointer(2,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(BillboardVertex),
                          (void*) (2* sizeof(vec3)) // Color is Offseted by 2 vec3 (see class Vertex)
                          );
    glEnableVertexAttribArray(2);

    // 3rd attribute buffer : texture coordinates
    glVertexAttribPointer(3,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(BillboardVertex),
                          (void*) (2* sizeof(vec3) + sizeof(vec4)) // texture coordinates are Offseted by 2 vec3 (see class Vertex) and a vec4
                          );
    glEnableVertexAttribArray(3);


}

BillboardList::~BillboardList()
{
	// Free the GPU from the Vertex Buffer
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);

	mVertexBuffer.resize(0);
	mBillboardList.resize(0);
}

void BillboardList::AddBillboard(Billboard* b)
{
    mBillboardList.push_back(b);
    
    assert(mBillboardList.size() <= mVertexBuffer.size() / 6.0f);
}

void BillboardList::RemoveBillboard(Billboard* b)
{
    list<Billboard*>::iterator it = find(mBillboardList.begin(), mBillboardList.end(), b);
    assert((*it != nullptr));
    mBillboardList.remove(*it);
}

void BillboardList::Update(float dt)
{
    // Sort billboards according to their depth, the functor CompareBillboardAlongZ does the magic!
    CompareBillboardAlongZ comp;
    mBillboardList.sort(comp);
    
    // Set Data in Vertex Buffer
    unsigned long firstVertexIndex = 0;

    // Maybe the view matrix will be useful to align the billboards
    const Camera* cam = World::GetInstance()->GetCurrentCamera();
    mat4 viewMatrix = cam->GetViewMatrix();
    
    for (list<Billboard*>::iterator it = mBillboardList.begin(); it != mBillboardList.end(); ++it)
    {
        const Billboard* b = *it;
        
        // Colors
        mVertexBuffer[firstVertexIndex].color = mVertexBuffer[firstVertexIndex + 1].color = mVertexBuffer[firstVertexIndex +2].color = mVertexBuffer[firstVertexIndex + 3].color = mVertexBuffer[firstVertexIndex + 4].color = mVertexBuffer[firstVertexIndex + 5].color = b->color;

        
        // @TODO 5 - Align billboards with Camera plane
        //
        // For each billboard, update each vertex position and normals
        // Currently, positions are aligned with the X-Y plane, billboards must face the camera
        //
        // You must update the positions and normals for the 6 vertices below

        vec3 rightVector = {viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]};
        vec3 upViewVector = {viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]};
        vec3 lookAtVector = {viewMatrix[0][2],viewMatrix[1][2],viewMatrix[2][2]};
        vec3 rightRotatedNorm = 0.5f* b->size.x *normalize(glm::rotate(rightVector, radians(b->angle), lookAtVector));
        vec3 upRotatedNorm =  normalize(glm::rotate(upViewVector, radians(b->angle), lookAtVector)) * 0.5f*b->size.y;

        
        // Normals
        mVertexBuffer[firstVertexIndex].normal =
        mVertexBuffer[firstVertexIndex + 1].normal =
        mVertexBuffer[firstVertexIndex +2].normal =
        mVertexBuffer[firstVertexIndex + 3].normal =
        mVertexBuffer[firstVertexIndex + 4].normal =
        mVertexBuffer[firstVertexIndex + 5].normal = -normalize(lookAtVector) ;// wrong...?
        
        // First triangle
        // Top left
        mVertexBuffer[firstVertexIndex].position = b->position + upRotatedNorm - rightRotatedNorm;
       // Bottom Left
        mVertexBuffer[firstVertexIndex + 1].position = b->position - upRotatedNorm - rightRotatedNorm;
        // Top Right
        mVertexBuffer[firstVertexIndex + 2].position = b->position + upRotatedNorm + rightRotatedNorm;
        // Second Triangle
        // Top Right
        mVertexBuffer[firstVertexIndex + 3].position = mVertexBuffer[firstVertexIndex + 2].position;
        // Bottom Left
        mVertexBuffer[firstVertexIndex + 4].position = mVertexBuffer[firstVertexIndex + 1].position;
        // Bottom Right
        mVertexBuffer[firstVertexIndex + 5].position = b->position - upRotatedNorm + rightRotatedNorm;

        // do not touch this...
        firstVertexIndex += 6;
    }
    
    Renderer::CheckForErrors();
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6*sizeof(BillboardVertex)*mBillboardList.size(), (void*)&mVertexBuffer[0]);
}

void BillboardList::Draw()
{
    Renderer::CheckForErrors();

    
    // Set current shader to be the Textured Shader
    ShaderType oldShader = (ShaderType)Renderer::GetCurrentShader();
    
    Renderer::SetShader(SHADER_TEXTURED);
    glUseProgram(Renderer::GetShaderProgramID());

    Renderer::CheckForErrors();

    
    GLuint textureLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "mySamplerTexture");
    glActiveTexture(GL_TEXTURE0);
    
    Renderer::CheckForErrors();

    
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glUniform1i(textureLocation, 0);				// Set our Texture sampler to user Texture Unit 0

    
    Renderer::CheckForErrors();

    // This looks for the MVP Uniform variable in the Vertex Program
    GLuint VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
    GLuint VMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
    GLuint PMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
    GLuint worldLightLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
    GLuint lightColorLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
    GLuint lightAttenuationLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");
    // Send the view projection constants to the shader
    const Camera* currentCamera = World::GetInstance()->GetCurrentCamera();
    mat4 V = currentCamera->GetViewMatrix();
    mat4 P = currentCamera->GetProjectionMatrix();

    
    
    const vector<Light*> pointLights = World::GetInstance()->GetCurrentLights();
    const Light* light = pointLights[0];
    vec4 lightPosition = light->getPosition();
    vec3 lightColor = light->getColor();
    vec3 lightAttenuation = light->getAttenuation();
    mat4 VP = currentCamera->GetViewProjectionMatrix();
    glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);
    glUniform3f(lightColorLocation,lightColor.x ,lightColor.y, lightColor.z);
    glUniform3f(lightAttenuationLocation,lightAttenuation.x ,lightAttenuation.y, lightAttenuation.z);
    glUniform4f(worldLightLocation,lightPosition.x ,lightPosition.y, lightPosition.z, lightPosition.w);
    
    light = pointLights[1];
    vec4 lightPosition1 = light->getPosition();
    vec3 lightColor1 = light->getColor();
    vec3 lightAttenuation1 = light->getAttenuation();
    GLuint worldLightLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition1");
    GLuint lightColorLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor1");
    GLuint lightAttenuationLocation1 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation1");
    glUniform3f(lightColorLocation1,lightColor1.x ,lightColor1.y, lightColor1.z);
    glUniform3f(lightAttenuationLocation1,lightAttenuation1.x ,lightAttenuation1.y, lightAttenuation1.z);
    glUniform4f(worldLightLocation1,lightPosition1.x ,lightPosition1.y, lightPosition1.z, lightPosition1.w);
    
    light = pointLights[2];
    vec4 lightPosition2 = light->getPosition();
    vec3 lightColor2 = light->getColor();
    vec3 lightAttenuation2 = light->getAttenuation();
    GLuint worldLightLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition2");
    GLuint lightColorLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor2");
    GLuint lightAttenuationLocation2 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation2");
    glUniform3f(lightColorLocation2,lightColor2.x ,lightColor2.y, lightColor2.z);
    glUniform3f(lightAttenuationLocation2,lightAttenuation2.x ,lightAttenuation2.y, lightAttenuation2.z);
    glUniform4f(worldLightLocation2,lightPosition2.x ,lightPosition2.y, lightPosition2.z, lightPosition2.w);
    
    light = pointLights[3];
    vec4 lightPosition3 = light->getPosition();
    vec3 lightColor3 = light->getColor();
    vec3 lightAttenuation3 = light->getAttenuation();
    GLuint worldLightLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition3");
    GLuint lightColorLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor3");
    GLuint lightAttenuationLocation3 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation3");
    glUniform3f(lightColorLocation3 ,lightColor3.x ,lightColor3.y, lightColor3.z);
    glUniform3f(lightAttenuationLocation3, lightAttenuation3.x ,lightAttenuation3.y, lightAttenuation3.z);
    glUniform4f(worldLightLocation3,lightPosition3.x ,lightPosition3.y, lightPosition3.z, lightPosition3.w);

    light = pointLights[4];
    vec4 lightPosition4 = light->getPosition();
    vec3 lightColor4 = light->getColor();
    vec3 lightAttenuation4 = light->getAttenuation();
    GLuint worldLightLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition4");
    GLuint lightColorLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor4");
    GLuint lightAttenuationLocation4 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation4");
    glUniform3f(lightColorLocation4 ,lightColor4.x ,lightColor4.y, lightColor4.z);
    glUniform3f(lightAttenuationLocation4, lightAttenuation4.x ,lightAttenuation4.y, lightAttenuation4.z);
    glUniform4f(worldLightLocation4,lightPosition4.x ,lightPosition4.y, lightPosition4.z, lightPosition4.w);

    
    light = pointLights[5];
    vec4 lightPosition5 = light->getPosition();
    vec3 lightColor5 = light->getColor();
    vec3 lightAttenuation5 = light->getAttenuation();
    GLuint worldLightLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition5");
    GLuint lightColorLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor5");
    GLuint lightAttenuationLocation5 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation5");
    glUniform3f(lightColorLocation5 ,lightColor5.x ,lightColor5.y, lightColor5.z);
    glUniform3f(lightAttenuationLocation5, lightAttenuation5.x ,lightAttenuation5.y, lightAttenuation5.z);
    glUniform4f(worldLightLocation5,lightPosition5.x ,lightPosition5.y, lightPosition5.z, lightPosition5.w);
    
    light = pointLights[6];
    vec4 lightPosition6 = light->getPosition();
    vec3 lightColor6 = light->getColor();
    vec3 lightAttenuation6 = light->getAttenuation();
    GLuint worldLightLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition6");
    GLuint lightColorLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor6");
    GLuint lightAttenuationLocation6 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation6");
    glUniform3f(lightColorLocation6 ,lightColor6.x ,lightColor6.y, lightColor6.z);
    glUniform3f(lightAttenuationLocation6, lightAttenuation6.x ,lightAttenuation6.y, lightAttenuation6.z);
    glUniform4f(worldLightLocation6, lightPosition6.x ,lightPosition6.y, lightPosition6.z, lightPosition6.w);
    
    light = pointLights[7];
    vec4 lightPosition7 = light->getPosition();
    vec3 lightColor7 = light->getColor();
    vec3 lightAttenuation7 = light->getAttenuation();
    GLuint worldLightLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition7");
    GLuint lightColorLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor7");
    GLuint lightAttenuationLocation7 = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation7");
    glUniform3f(lightColorLocation7 ,lightColor7.x ,lightColor7.y, lightColor7.z);
    glUniform3f(lightAttenuationLocation7, lightAttenuation7.x ,lightAttenuation7.y, lightAttenuation7.z);
    glUniform4f(worldLightLocation7, lightPosition7.x ,lightPosition7.y, lightPosition7.z, lightPosition7.w);
    
    glUniformMatrix4fv(VMatrixLocation, 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(PMatrixLocation, 1, GL_FALSE, &P[0][0]);
    // Draw the Vertex Buffer
    // Note this draws a unit Cube
    // The Model View Projection transforms are computed in the Vertex Shader
    glBindVertexArray(mVAO);
    
    GLuint WorldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");

    // Billboard position are all relative to the origin
    mat4 worldMatrix(1.0f);
    glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
    
    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) mBillboardList.size()*6); // 6 vertices by billboard
    
    Renderer::CheckForErrors();
    
    Renderer::SetShader(oldShader);
}
