//
// Created by redkc on 16/01/2024.
//

#include "ECS/Entity.h"
#include "DirLight.h"
void DirLight::InnitShadow() {
    if (initializedShadow) {
        DeleteShadow();
    }
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm::mat4 lightProjection, lightView;
    float near_plane = 1.0f, far_plane = 2000.0f;
    lightProjection = glm::ortho(-750.0f, 750.0f, -750.0f, 750.0f, near_plane, far_plane);

    float scale_factor = 1000.0f;
// assume data.direction contains Euler angles (yaw, pitch, roll)
    glm::vec3 eulerAngles = data.direction;
    glm::vec3 direction = glm::vec3(
            cos(eulerAngles.x) * cos(eulerAngles.y), // dx
            sin(eulerAngles.y), // dy
            sin(eulerAngles.x) * cos(eulerAngles.y) // dz
    );
    glm::vec3 translatedPos = -scale_factor * direction; //adjust the sign and scale
    lightView = glm::lookAt(translatedPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    data.position = glm::vec4(translatedPos, 1.0f);
    data.lightSpaceMatrix = lightProjection * lightView;

    initializedShadow = true;
}

void DirLight::SetUpShadowBuffer(ShaderType shaderType,Shader* shadowMapShader,Shader* instanceShadowMapShader) {
    if (shaderType == Normal) {
        shadowMapShader->use();
        shadowMapShader->setMatrix4("lightSpaceMatrix", false, glm::value_ptr(data.lightSpaceMatrix));
    } else {
        instanceShadowMapShader->use();
        instanceShadowMapShader->setMatrix4("lightSpaceMatrix", false, glm::value_ptr(data.lightSpaceMatrix));
    }

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

DirLight::DirLight(DirLightData data) :
        data(data) {
    lightType = Directional;
}

void DirLight::showImGuiDetails(Camera *camera) {
    ImGui::PushID(uniqueID);
    if (ImGui::TreeNode("Directional light")) {
        ImGui::InputFloat4("Color", glm::value_ptr(data.color));
        EditLight(camera);
        // Display other light properties...
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void DirLight::EditLight(Camera *camera) {

    getEntity()->transform.ManipulateModelMatrix(camera);

// Extract the rotation as a quaternion
    glm::quat q = glm::toQuat(getEntity()->transform.getModelMatrix());

// Convert the quaternion to Euler angles
    glm::vec3 eulerAngles = glm::eulerAngles(q);
    data.direction = glm::vec4(eulerAngles, 1);
}




