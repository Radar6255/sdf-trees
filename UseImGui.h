#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class UseImGui {
public:
  void Init(GLFWwindow* window, const char* glsl_version);
  virtual void Update();
  void Render();
  void Shutdown();
};
