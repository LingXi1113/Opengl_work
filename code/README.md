# New Scene 项目说明（交付版）

## 1. 项目完成了什么

本项目在 LearnOpenGL 基础框架上实现了一个 **综合场景（new_scene）**：

- 草地地形（terrain）+ 天空盒（skybox）
- 4 个静态角色模型（OBJ）缩放到原来的 1/4，并按 2×2 布局摆放，保证都可见
- HOUSE 目录下的房屋/围栏/道路/树等静态模型（OBJ）使用 **Blinn–Phong** 光照渲染
- CITY 目录下的城市组件（OBJ）自动扫描加载并摆放到草地上，避免与房屋区域重叠
  - 为避免一次性载入大量模型导致卡死，采用 **逐帧增量加载（lazy load）**
  - city 模型与间距按要求整体缩小到原来的 1/3
  - city 渲染使用 **PBR（Cook–Torrance / GGX）** 光照模型
- 一只带骨骼动画的海鸥（FBX）在空中水平往返飞行

## 2. 目录结构（交付约定）

- `code/`：源代码 + 工程文件 + 运行所需资源（模型/纹理/着色器）
- `release/`：可运行的可执行程序与运行脚本

## 3. 运行方式

### 3.1 直接运行 release

1. 进入 `release/`
2. 双击 `run_new_scene.bat`

该脚本会设置环境变量 `LOGL_ROOT_PATH` 指向 `../code`，保证运行时能找到 shader 和资源文件。

### 3.2 从源码构建（Windows / CMake）

在 `code/` 目录执行：

```powershell
cmake -S . -B build
cmake --build build --config Release
```

然后把生成的可执行程序复制到 `release/`，或直接从构建输出目录运行。

## 4. 关键实现说明

### 4.1 city 自动扫描与摆放

- 启动时递归扫描 `city/Models/OBJ format` 下的 `.obj`（跳过纹理目录）
- 为保持窗口响应：每帧加载少量模型（如 2 个/帧）
- 每个模型加载后计算 Y 方向包围盒，做统一高度缩放并贴地

### 4.2 city 使用 PBR 光照

- 新增 `src/new_scene/city_pbr.vs`、`src/new_scene/city_pbr.fs`
- 使用 Cook–Torrance BRDF（GGX 法线分布 + Smith 几何项 + Schlick Fresnel）
- 对 OBJ 的 `texture_diffuse1` 作为 albedo（无金属/粗糙度贴图时使用默认参数）

### 4.3 海鸥骨骼动画（FBX）

- 由于当前 Assimp 构建对某些 `.glb` 存在 “Unsupported binary glTF version” 问题，改用 FBX
- 通过 Animator 每帧更新骨骼矩阵，驱动 skinned shader
- 位置按时间 `sin(t)` 做水平往返移动

## 5. 遇到的问题与解决

- **问题 A：一次性加载大量 city OBJ 导致窗口假死/空白**
  - 解决：启动只收集文件列表，渲染循环中按帧增量加载，避免长时间阻塞

- **问题 B：glb 导入失败（Assimp 版本/编译特性差异）**
  - 解决：切换到 FBX 资源（Seagull.fbx）完成骨骼动画

- **问题 C：PBR 初期整体偏暗、颜色发红**
  - 解决：将点光源移动到 city 区域上方并调大环境光占比，使受光更合理


