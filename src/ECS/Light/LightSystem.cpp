//
// Created by redkc on 09/01/2024.
//


#include "LightSystem.h"

void LightSystem::PushToSSBO() {


    GenerateShadowBuffers();

    std::vector<DirLightData> dirLightDataArray;
    for (const DirLight *light: dirLights) {
        dirLightDataArray.push_back(light->data);
    }

    std::vector<SpotLightData> spotLightDataArray;
    for (const SpotLight *light: spotLights) {
        spotLightDataArray.push_back(light->data);
    }

    std::vector<PointLightData> pointLightDataArray;
    for (const PointLight *light: pointLights) {
        pointLightDataArray.push_back(light->data);
    }


    GLuint currentId;
    glGenBuffers(1, &currentId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, dirLightDataArray.size() * sizeof(DirLightData), dirLightDataArray.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, dirLightBufferId, currentId);

    glGenBuffers(1, &currentId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pointLightDataArray.size() * sizeof(PointLightData),
                 pointLightDataArray.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pointLightBufferId, currentId);

    glGenBuffers(1, &currentId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spotLightDataArray.size() * sizeof(SpotLightData), spotLightDataArray.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, spotLightBufferId, currentId);


    dirLightDataArray.clear();
    pointLightDataArray.clear();
    spotLightDataArray.clear();
}


void LightSystem::showLightTree() {
    if (ImGui::TreeNode("Lights")) {
        for (auto &light: lights) {
            light->showImGuiDetails(camera);
        }
        if (ImGui::Button("Push light data to SSBO")) {
            PushToSSBO();
        }
        ImGui::TreePop();
    }
}

LightSystem::~LightSystem() {
    dirLights.clear();
    pointLights.clear();
    spotLights.clear();
}

LightSystem::LightSystem(Camera *camera) : camera(camera) {

}

void LightSystem::GenerateShadowBuffers() {
    for (auto &light: lights) {
        light->InnitShadow();
    }
}

void LightSystem::Init() {
    planeDepthShader.init();
    cubeDepthShader.init();

    instancePlaneDepthShader.init();
    instanceCubeDepthShader.init();
    PushToSSBO();
}


int TEXTURE_UNITS_OFFSET = 8;
int POINT_SHADOW_OFFSET = TEXTURE_UNITS_OFFSET + 5;

void LightSystem::PushDepthMapsToShader(Shader *shader) { //TODO this should be done throught ILight
    int planeShadowIndex = 0, cubeShadowIndex = 0;
    for (auto &light: lights) {
        if (light->lightType == Point) {
            std::string number = std::to_string(cubeShadowIndex);
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNITS_OFFSET +
                            cubeShadowIndex); // TEXTURE_UNITS_OFFSET is the number of non-shadow map textures you have
            glBindTexture(GL_TEXTURE_CUBE_MAP, light->depthMap);
            glUniform1i(glGetUniformLocation(shader->ID, ("cubeShadowMaps[" + number + "]").c_str()),
                        TEXTURE_UNITS_OFFSET + cubeShadowIndex);
            cubeShadowIndex++;
        } else {
            std::string number = std::to_string(planeShadowIndex);
            glActiveTexture(GL_TEXTURE0 + POINT_SHADOW_OFFSET +
                            planeShadowIndex); // TEXTURE_UNITS_OFFSET is the number of non-shadow map textures you have
            glBindTexture(GL_TEXTURE_2D, light->depthMap);
            glUniform1i(glGetUniformLocation(shader->ID, ("planeShadowMaps[" + number + "]").c_str()),
                        POINT_SHADOW_OFFSET + planeShadowIndex);
            planeShadowIndex++;
        }

    }
}

void LightSystem::Update(double deltaTime) {
    int offset = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dirLightBufferId);
    for (auto &light: dirLights) {
        if (light->getIsDirty()) {  // Only push it if it's dirty
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(light->data), &light->data);
        }
        offset += sizeof(light->data);
    }

    offset = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightBufferId);
    for (auto &light: pointLights) {

        if (light->getIsDirty()) {  // Only push it if it's dirty
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(light->data), &light->data);
        }
        offset += sizeof(light->data);
    }

    offset = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightBufferId);
    for (auto &light: spotLights) {
        if (light->getIsDirty()) {  // Only push it if it's dirty
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(light->data), &light->data);
        }
        offset += sizeof(light->data);
    }
}

void LightSystem::addComponent(void *component) {
    ILight *light = static_cast<ILight *>(component);

    switch (light->lightType) {
        case Point:
            pointLights.push_back(reinterpret_cast<PointLight *>(&light));
            break;
        case Directional:
            dirLights.push_back(reinterpret_cast<DirLight *>(&light));
            break;
        case Spot:
            spotLights.push_back(reinterpret_cast<SpotLight *>(&light));
            break;
    }
    lights.push_back(light);
}
