#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class GuiData {
public:
    float alterSize;
    float treeChanceThresh;
    bool updateTerrain;
    float recentRenderTime;
    float avgRenderTime;
    float updateRate;
    float recentUpdateRate;
};

class UseImGui {
public:
  void Init(GLFWwindow* window, const char* glsl_version);
  virtual void Update(GuiData* gd);
  void Render();
  void Shutdown();
};
