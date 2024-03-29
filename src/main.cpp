
#pragma region Includes


#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <ImGuizmo.h>
#include "Camera.h"
#include "modelLoading/Model.h"
#include <cstdio>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#define STB_IMAGE_IMPLEMENTATION


//#include <glad/glad.h>  // Initialize with gladLoadGL()


//Instancing
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

#include <iostream>
#include "spdlog/sinks/basic_file_sink.h"

#include "Systems/RenderSystem/PBR/PBRSystem.h"
#include "Systems/RenderSystem/PostProcessing/BloomSystem/BloomSystem.h"
#include "ECS/Light/LightSystem.h"
#include "ECS/Render/RenderSystem.h"
#include "Systems/EntitySystem/Scene.h"
#include "ECS/Render/Components/Render.h"

#ifndef ENTITY_H
#define ENTITY_H

#include <list> //std::list
#include <memory> //std::unique_ptr

#endif

Scene scene;
string modelPath = "res/models/asteroid/Asteroid.fbx";
Model model = Model(&modelPath);

shared_ptr<spdlog::logger> file_logger;
#pragma endregion Includes

#pragma region constants

#pragma endregion constants

#pragma region Function definitions

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool init();

void init_systems();

void load_enteties();

void init_imgui();

void init_camera();

void before_frame();

void input();

void update();

void render();

void render_scene();

void render_scene_to_depth();


void imgui_begin();

void imgui_render();

void imgui_end();

void end_frame();

void cleanup();

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

#pragma endregion Function definitions

#pragma region Orginal set up

constexpr int32_t WINDOW_WIDTH = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

GLFWwindow *window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const char *glsl_version = "#version 460";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 6;

//Not my things but I could probably change them


ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#pragma endregion Orginal set up

#pragma region My set up

//Camera set up
int display_w, display_h;
Camera camera(glm::vec3(200.0f, 40.0f, 0.0f));
float lastX = 0;
float lastY = 0;

LightSystem lightSystem(&camera);
PBRSystem pbrSystem(&camera);
RenderSystem renderSystem;
BloomSystem bloomSystem;


bool captureMouse = false;
bool captureMouseButtonPressed = false;

// timing
double deltaTime = 0;
double lastFrame = 0;
int timeStep = 1;
bool timeStepKeyPressed = false;

#pragma endregion My set up


int main(int, char **) {

#pragma region Init

    if (!init()) {
        spdlog::error("Failed to initialize project!");
        return EXIT_FAILURE;
    }
    spdlog::info("Initialized project.");
    file_logger->info("Initialized project.");

    init_systems();
    spdlog::info("Initialized textures and vertices.");
    file_logger->info("Initialized textures and vertices.");

    load_enteties();
    spdlog::info("Initialized entities.");
    file_logger->info("Initialized entities.");

    init_imgui();
    spdlog::info("Initialized ImGui.");
    file_logger->info("Initialized ImGui.");

    init_camera();
    spdlog::info("Initialized camera and viewport.");
    file_logger->info("Initialized camera and viewport.");

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glDepthFunc(GL_LEQUAL);

#pragma endregion Init

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        //Setting up things for the rest of functionalities (ex. delta time)
        file_logger->info("Before frame");
        before_frame();

        // Process I/O operations here
        file_logger->info("Input");
        input();

        // Update game objects' state here
        file_logger->info("Update");
        update();

        // OpenGL rendering code here
        file_logger->info("Render");
        render();

        // Draw ImGui
        file_logger->info("Imgui");
        imgui_begin();
        imgui_render(); // edit this function to add your own ImGui controls
        imgui_end(); // this call effectively renders ImGui


        // End frame and swap buffers (double buffering)
        file_logger->info("End frame");
        end_frame();
    }

    file_logger->info("Cleanup");
    cleanup();
    // Cleanup
    return 0;
}


#pragma region Functions

void cleanup() {
    //Orginal clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

bool init() {

    // Get current date and time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

// Convert time to local time
    std::tm *local_time = std::localtime(&now_time);

// Create a filename with date
    std::ostringstream filename;
    filename << "res/logs/logs_"
             << (local_time->tm_year + 1900) // Year starts from 1900
             << "-" << (local_time->tm_mon + 1) // Month starts from 0
             << "-" << local_time->tm_mday  // Day of month
             << "-" << local_time->tm_hour  // hour
             << "-" << local_time->tm_min  // minute
             << ".txt";
    try {

        file_logger = spdlog::basic_logger_mt("file_logger", filename.str());
        file_logger->flush_on(spdlog::level::info);
        file_logger->info("Init log file");
    }
    catch (const spdlog::spdlog_ex &ex) {
        spdlog::error("Log initialization failed.");
    }

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::error("Failed to initalize GLFW!");
        return false;
    }

    // GL 4.6 + GLSL 460
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui GLFW+OpenGL4 example", NULL, NULL);
    if (window == NULL) {
        spdlog::error("Failed to create GLFW Window!");
        return false;
    }


    glfwMakeContextCurrent(window);
    bool err = !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if (err) {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }

    stbi_set_flip_vertically_on_load(true);
    return true;
}


void init_systems() {
    lightSystem.Init();
    pbrSystem.Init();
    bloomSystem.Init(camera.saved_display_w, camera.saved_display_h);
    scene.systemManager.addSystem(&lightSystem);
    scene.systemManager.addSystem(&renderSystem);
}

void load_enteties() {
    model.loadModel();
    Entity *gameObject = scene.addGameObject();
    gameObject->transform.setLocalPosition({-0, 0, 0});
    const float scale = 10;
    gameObject->transform.setLocalScale({scale, scale, scale});
    gameObject->addComponent(new Render(&model));
    for (unsigned int i = 0; i < 2; ++i) {
        gameObject = scene.addGameObject(gameObject);
        gameObject->addComponent(new Render(&model));
        gameObject->transform.setLocalScale({scale, scale, scale});
        gameObject->transform.setLocalPosition({5, 0, 0});
        gameObject->transform.setLocalScale({0.2f, 0.2f, 0.2f});
    }
    gameObject = scene.addGameObject();
    gameObject->addComponent(new DirLight(DirLightData(glm::vec4(1), glm::vec4(1), glm::vec4(1), glm::mat4x4(1))));
    gameObject = scene.addGameObject();
    gameObject->addComponent(new PointLight(PointLightData(glm::vec4(1), 1.0f, 1.0f, 1.0f, 1.0f, glm::vec4(1))));
    gameObject = scene.addGameObject();
    gameObject->addComponent(new SpotLight(SpotLightData(glm::vec4(1), glm::vec4(1), 1.0f, 1.0f, 1.0f)));
    lightSystem.PushToSSBO();
}

void init_imgui() {
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
}

void before_frame() {
    // Setting up delta time
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
};


void input() {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetScrollCallback(window, scroll_callback);

    processInput(window);
}

void update() {
    scene.updateScene();
    lightSystem.Update(deltaTime);
    
}

void render() {
    render_scene_to_depth();

    lightSystem.PushDepthMapsToShader(&pbrSystem.pbrShader);
    lightSystem.PushDepthMapsToShader(&pbrSystem.pbrInstanceShader);

    glViewport(0, 0, camera.saved_display_w, camera.saved_display_h); // Needed after light generation

    bloomSystem.BindBuffer();
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    file_logger->info("Cleared.");

    pbrSystem.PrebindPBR(&camera);
    pbrSystem.RenderBackground();
    file_logger->info("Set up PBR.");

    pbrSystem.pbrShader.use();

    render_scene();

    file_logger->info("Rendered AsteroidsSystem.");

    bloomSystem.BlurBuffer();
    bloomSystem.Render();

}


void render_scene() {
    renderSystem.DrawScene(&pbrSystem.pbrShader);
    file_logger->info("Rendered Entities.");
}


void render_scene_to_depth() {
    // for (auto &light: lightSystem.lights) {
    //    light->SetUpShadowBuffer(Normal);
    //    glClear(GL_DEPTH_BUFFER_BIT);
    //    scene.drawScene(*light->shadowMapShader,*light->instanceShadowMapShader);
    //}
}

void imgui_begin() {
    // Start the Dear ImGui frame
    if (!captureMouse) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDrawCursor = true;
    } else {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDrawCursor = false;
    };

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void imgui_render() {
    ImGui::Begin("Debug menu");
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f", 1.0f / deltaTime);
    ImGui::Text(buffer);

    lightSystem.showLightTree();
    ImGui::End();


    bloomSystem.showImguiOptions();

}

void imgui_end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(UPWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWNWARD, deltaTime);


    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        if (!captureMouseButtonPressed) {
            captureMouse = !captureMouse;
        }
        captureMouseButtonPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE) {
        captureMouseButtonPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!timeStepKeyPressed) {
            if (timeStep == 0) {
                timeStep = 1;
            } else {
                timeStep = 0;
            }
        }
        timeStepKeyPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        timeStepKeyPressed = false;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    display_h = height;
    display_w = width;
    camera.UpdateCamera(width, height);
    bloomSystem.SetUpBuffers(width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (captureMouse) {
        if (timeStep != 0) {
            camera.ProcessMouseMovement(xoffset, yoffset, true, deltaTime);
        } else {
            camera.ProcessMouseMovement(xoffset, yoffset, true, 0.01f);
        }
    }


    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(xpos, ypos);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset), deltaTime);
}

void end_frame() {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void init_camera() {
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    lastX = display_w / 2.0f;
    lastY = display_h / 2.0f;
}

#pragma endregion Functions
