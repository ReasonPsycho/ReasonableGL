//
// Created by redkc on 16/01/2024.
//

#include "PointLight.h"

void PointLight::InnitShadow() {
    if (initializedShadow) {
        DeleteShadow();
    }
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowProj = glm::perspective(glm::radians(90.0f), (float) SHADOW_WIDTH / (float) SHADOW_HEIGHT,
                                  1.0f, 25.0f); //TODO add based pn calculation
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(-1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(0.0f, 1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(0.0f, -1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(0.0f, 0.0f, 1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(
            shadowProj *
            glm::lookAt(glm::vec3(data.position), glm::vec3(data.position) + glm::vec3(0.0f, 0.0f, -1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)));


    initializedShadow = true;
}

void PointLight::GenerateShadow(void (*funcPtr)()) {
    // 1. render scene to depth cubemap
    // --------------------------------
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    shadowMapShader->use();
    for (unsigned int i = 0; i < 6; ++i)
        shadowMapShader->setMatrix4("shadowMatrices[" + std::to_string(i) + "]", false,
                                    glm::value_ptr(shadowTransforms[i]));
    shadowMapShader->setFloat("far_plane", 25.0);
    shadowMapShader->setVec3("lightPos", data.position.x, data.position.y, data.position.z);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
    funcPtr();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


