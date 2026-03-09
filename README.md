# OpenGL 3D Rendering Project

这是一个基于著名的 [LearnOpenGL](https://learnopengl.com/) 教程框架并在其基础上衍生扩展的 3D 图形渲染工程。

## ✨ 主要功能与特性 (Features)

* **核心渲染架构**：包含基础的 OpenGL 渲染管线、模型加载 (Assimp) 以及摄像机漫游系统。
* **自定义场景渲染 (New Scene)**：扩展了自定义的大型场景漫游，包括城市 (city_pbr)、房屋 (house) 等复杂的室外场景。
* **骨骼蒙皮动画 (Skeletal Animation)**：结合 Assimp 解析和着色器 (skinned.vs / skinned.fs)，项目支持加载并播放带骨架的复杂 3D 模型动画（如带各种动作的游戏人物）。
* **程序化地形 (Procedural Terrain)**：支持通过 python 脚本生成并渲染广阔的地形环境 (	errain.obj)。
* **高级光照与环境**：实现了基于物理的渲染着色器 (PBR) 支持，以及完整的天空盒 (Skybox) 系统。

## 🛠️ 构建与运行指南 (Build & Run)

本项目使用 **CMake** 进行构建，并已预留了所有需要的外部依赖 (GLFW, GLAD, GLM, Assimp等静态库或头文件)。

### Windows 环境下编译

1. 确保你的电脑已安装 **CMake** 和现代的 C++ 编译器（如 Visual Studio 2022）。
2. 在项目根目录下打开终端，执行以下命令：
   \\\powershell
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   \\\
3. **运行**: 构建完成后，可执行文件通常位于 elease 目录或者 uild 目录下，直接运行即可（如双击 un_new_scene.bat 脚本快捷启动）。

---
*版权声明：本项目核心管线参考了 Joey de Vries 的 [LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL)，项目内部分模型和贴图归原作者所有。*
