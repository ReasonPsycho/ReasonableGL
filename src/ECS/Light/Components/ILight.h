//
// Created by redkc on 16/01/2024.
//

#ifndef OPENGLGP_ILIGHT_H
#define OPENGLGP_ILIGHT_H

#include "Camera.h"
#include "ECS/Component.h"


enum LightType {
    Directional,
    Spot,
    Point,
};

enum ShaderType {
    Instance,
    Normal,
};

class ILight : public Component {
public:
    ILight();

    ~ILight();

    enum LightType lightType;


    virtual void showImGuiDetails(Camera *camera) = 0; // Pure virtual function
    virtual void EditLight(Camera *camera) = 0;

    virtual void SetUpShadowBuffer(ShaderType shaderType, Shader *shadowMapShader,
                                   Shader *instanceShadowMapShader) = 0; // Pure virtual function
    virtual void InnitShadow() = 0;

    void DeleteShadow();

    unsigned int depthMap{};
    
protected:
    int uniqueID;     // Instance variable to store the unique ID for each object

    //For shadows
    bool initializedShadow = false;
    unsigned int depthMapFBO{};
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    glm::mat4 shadowProj{};
};

#endif //OPENGLGP_ILIGHT_H
