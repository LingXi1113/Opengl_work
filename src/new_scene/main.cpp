#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// sunset state
bool isSunset = false;
float sunsetFactor = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    auto fileExists = [](const std::string& p) -> bool {
        std::ifstream f(p.c_str(), std::ios::binary);
        return f.good();
    };

    namespace fs = std::filesystem;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: New Scene", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    const std::string terrainVS = FileSystem::getPath("src/new_scene/shader.vs");
    const std::string terrainFS = FileSystem::getPath("src/new_scene/shader.fs");
    const std::string skyboxVS = FileSystem::getPath("src/new_scene/skybox.vs");
    const std::string skyboxFS = FileSystem::getPath("src/new_scene/skybox.fs");
    const std::string characterVS = FileSystem::getPath("src/new_scene/cat.vs");
    const std::string characterFS = FileSystem::getPath("src/new_scene/cat.fs");
    const std::string houseVS = FileSystem::getPath("src/new_scene/house.vs");
    const std::string houseFS = FileSystem::getPath("src/new_scene/house.fs");
    const std::string cityPbrVS = FileSystem::getPath("src/new_scene/city_pbr.vs");
    const std::string cityPbrFS = FileSystem::getPath("src/new_scene/city_pbr.fs");
    const std::string skinnedVS = FileSystem::getPath("src/new_scene/skinned.vs");
    const std::string skinnedFS = FileSystem::getPath("src/new_scene/skinned.fs");

    std::cout << "=== new_scene runtime paths ===" << std::endl;
    std::cout << "terrainVS:   " << terrainVS << std::endl;
    std::cout << "terrainFS:   " << terrainFS << std::endl;
    std::cout << "skyboxVS:    " << skyboxVS << std::endl;
    std::cout << "skyboxFS:    " << skyboxFS << std::endl;
    std::cout << "characterVS: " << characterVS << std::endl;
    std::cout << "characterFS: " << characterFS << std::endl;
    std::cout << "houseVS:     " << houseVS << std::endl;
    std::cout << "houseFS:     " << houseFS << std::endl;
    std::cout << "cityPbrVS:   " << cityPbrVS << std::endl;
    std::cout << "cityPbrFS:   " << cityPbrFS << std::endl;
    std::cout << "skinnedVS:   " << skinnedVS << std::endl;
    std::cout << "skinnedFS:   " << skinnedFS << std::endl;

    Shader ourShader(terrainVS.c_str(), terrainFS.c_str());
    Shader skyboxShader(skyboxVS.c_str(), skyboxFS.c_str());
    Shader catShader(characterVS.c_str(), characterFS.c_str());
    Shader houseShader(houseVS.c_str(), houseFS.c_str());
    Shader cityPbrShader(cityPbrVS.c_str(), cityPbrFS.c_str());
    Shader skinnedShader(skinnedVS.c_str(), skinnedFS.c_str());

    // load models
    // -----------
    const std::string terrainModelPath = FileSystem::getPath("resources/objects/lawn/terrain.obj");
    const std::string characterAPath = FileSystem::getPath("resources/objects/character/character-a.obj");
    const std::string characterBPath = FileSystem::getPath("resources/objects/character/character-b.obj");
    const std::string characterCPath = FileSystem::getPath("resources/objects/character/character-c.obj");
    const std::string characterDPath = FileSystem::getPath("resources/objects/character/character-d.obj");
    const std::string cityObjDirPath = FileSystem::getPath("city/Models/OBJ format");
    const std::string birdsFbxPath = FileSystem::getPath("BIRDS/source/Seagull.fbx");
    const std::string seagullGlbPath = FileSystem::getPath("animated_seagull__3d_model_free_download.glb");
    const std::string seagullGltfPath = FileSystem::getPath("animated_seagull__3d_model_free_download.gltf");
    const std::string seagullFbxPath = FileSystem::getPath("animated_seagull__3d_model_free_download.fbx");
    const std::string fallbackAnimPath = FileSystem::getPath("Capoeira.fbx");

    // NOTE: current Assimp build in this project fails to load this specific .glb with:
    //   "GLTF: Unsupported binary glTF version"
    // So we do NOT auto-select .glb. Prefer FBX/GLTF(JSON). Otherwise fall back to Capoeira.
    std::string animatedModelPath = fallbackAnimPath;
    if (fileExists(birdsFbxPath))
        animatedModelPath = birdsFbxPath;
    else if (fileExists(seagullFbxPath))
        animatedModelPath = seagullFbxPath;
    else if (fileExists(seagullGltfPath))
        animatedModelPath = seagullGltfPath;
    const std::string houseDirPath = FileSystem::getPath("HOUSE/Models/OBJ format");

    std::cout << "terrainModel:   " << terrainModelPath << std::endl;
    std::cout << "characterA:     " << characterAPath << std::endl;
    std::cout << "characterB:     " << characterBPath << std::endl;
    std::cout << "characterC:     " << characterCPath << std::endl;
    std::cout << "characterD:     " << characterDPath << std::endl;
    std::cout << "cityObjDir:     " << cityObjDirPath << std::endl;
    std::cout << "animatedModel:  " << animatedModelPath << std::endl;
    std::cout << "animatedModel exists: " << (fileExists(animatedModelPath) ? "yes" : "NO") << std::endl;
    std::cout << "houseDir:       " << houseDirPath << std::endl;

    Model ourModel(terrainModelPath);
    Model characterA(characterAPath);
    Model characterB(characterBPath);
    Model characterC(characterCPath);
    Model characterD(characterDPath);

    // Load all city OBJ models and place them on the grass without overlapping the existing scene.
    // NOTE: City has many models; loading them all up-front can freeze the window for a long time.
    // We gather paths here and then load a few per frame after the window is up.
    std::vector<std::string> cityObjFiles;
    std::vector<std::unique_ptr<Model>> cityModels;
    std::vector<glm::mat4> cityModelMatrices;
    size_t cityLoadIndex = 0;
    const float cityGroundY = -1.0f;      // terrain is translated down by -1
    const float cityTargetHeight = 6.0f;  // consistent city asset height
    const float cityGlobalScale = 1.0f / 3.0f; // city 建筑整体缩小 1/3
    const float cityStartX = 12.0f;       // keep far from houses (roughly x/z in [-4..4])
    const float cityStartZ = -12.0f;
    const float citySpacing = 8.0f * cityGlobalScale;
    const int cityColumns = 6;
    if (fs::exists(cityObjDirPath) && fs::is_directory(cityObjDirPath))
    {
        for (const auto& entry : fs::recursive_directory_iterator(cityObjDirPath))
        {
            if (!entry.is_regular_file())
                continue;
            const auto ext = entry.path().extension().string();
            if (ext != ".obj")
                continue;
            if (entry.path().string().find("Textures") != std::string::npos)
                continue;
            
            // Skip "detail" objects (awnings, parasols) because the auto-scaling logic 
            // (based on height) makes them huge if they are flat, causing visual artifacts.
            if (entry.path().filename().string().find("detail") != std::string::npos)
                continue;

            cityObjFiles.push_back(entry.path().string());
        }
        std::sort(cityObjFiles.begin(), cityObjFiles.end());
        std::cout << "city: discovered obj files=" << cityObjFiles.size() << std::endl;
    }

    SkinnedModel animatedModel(animatedModelPath);
    Animation animatedAnimation(animatedModelPath, &animatedModel);
    Animator animator(&animatedAnimation);

    // Compute simple Y bounds from mesh vertices (model space). Used to auto-scale and place the model on the lawn.
    float animatedMinY = 0.0f;
    float animatedMaxY = 1.0f;
    bool animatedYBoundsValid = false;
    if (!animatedModel.meshes.empty())
    {
        float minY = std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        for (const auto& mesh : animatedModel.meshes)
        {
            for (const auto& v : mesh.vertices)
            {
                minY = std::min(minY, v.Position.y);
                maxY = std::max(maxY, v.Position.y);
            }
        }
        if (std::isfinite(minY) && std::isfinite(maxY) && (maxY - minY) > 1e-5f)
        {
            animatedMinY = minY;
            animatedMaxY = maxY;
            animatedYBoundsValid = true;
        }
    }

    std::cout << "animated: duration=" << animatedAnimation.GetDuration()
              << " ticksPerSecond=" << animatedAnimation.GetTicksPerSecond() << std::endl;
    std::cout << "animated: boneCount=" << animatedModel.GetBoneCount()
              << " boneMapSize=" << animatedModel.GetBoneInfoMap().size() << std::endl;
    std::cout << "animated: meshes=" << animatedModel.meshes.size();
    if (!animatedModel.meshes.empty())
        std::cout << " firstMeshVertices=" << animatedModel.meshes[0].vertices.size();
    std::cout << std::endl;

    if (animatedYBoundsValid)
    {
        std::cout << "animated: yBounds(min,max)=(" << animatedMinY << "," << animatedMaxY << ") height="
                  << (animatedMaxY - animatedMinY) << std::endl;
    }

    // Load Sword Model (Skeletal Animation)
    // Use X Bot.fbx for the mesh, and the specific animation file for the motion.
    const std::string swordModelPath = FileSystem::getPath("sword/X Bot.fbx");
    const std::string swordAnimPath = FileSystem::getPath("sword/sword and shield idle.fbx");
    
    SkinnedModel swordModel(swordModelPath);
    
    // HACK: Clear any textures loaded by Assimp for the sword model.
    // This ensures that Mesh::Draw() won't bind its own (likely empty/failed) textures,
    // allowing our manual binding of xBotTexture to persist on GL_TEXTURE2.
    for (auto& mesh : swordModel.meshes) {
        mesh.textures.clear();
    }

    Animation swordAnimation(swordAnimPath, &swordModel);
    Animator swordAnimator(&swordAnimation);

    // Compute bounds for sword model to auto-scale it
    float swordMinY = 0.0f;
    float swordMaxY = 1.0f;
    bool swordYBoundsValid = false;
    if (!swordModel.meshes.empty())
    {
        float minY = std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        for (const auto& mesh : swordModel.meshes)
        {
            for (const auto& v : mesh.vertices)
            {
                minY = std::min(minY, v.Position.y);
                maxY = std::max(maxY, v.Position.y);
            }
        }
        if (std::isfinite(minY) && std::isfinite(maxY) && (maxY - minY) > 1e-5f)
        {
            swordMinY = minY;
            swordMaxY = maxY;
            swordYBoundsValid = true;
        }
    }
    std::cout << "sword: loaded from " << swordModelPath << std::endl;
    std::cout << "sword: meshes=" << swordModel.meshes.size() << std::endl;
    if (!swordModel.meshes.empty()) {
        std::cout << "sword: v0=" << swordModel.meshes[0].vertices.size() << " indices=" << swordModel.meshes[0].indices.size() << std::endl;
    }
    std::cout << "sword: yBounds=(" << swordMinY << ", " << swordMaxY << ")" << std::endl;

    // Debug: Print bone names to find the right hand
    const auto& boneMap = swordModel.GetBoneInfoMap();
    for (const auto& pair : boneMap) {
        if (pair.first.find("Hand") != std::string::npos) {
            std::cout << "Bone found: " << pair.first << " ID: " << pair.second.id << std::endl;
        }
    }
    
    // Debug: Check if sword mesh has UVs
    if (!swordModel.meshes.empty()) {
        bool hasUV = false;
        // Check first vertex of first mesh
        if (!swordModel.meshes[0].vertices.empty()) {
            // In our Vertex struct, TexCoords is always present, but might be (0,0)
            // We can't easily check "hasUV" flag from here as it was lost during Assimp->Mesh conversion
            // But we can check if they are all zero.
            const auto& v = swordModel.meshes[0].vertices[0];
            std::cout << "Sword Mesh 0 Vertex 0 UV: " << v.TexCoords.x << ", " << v.TexCoords.y << std::endl;
        }
    }

    // Load the actual Sword mesh (static model)
    // We'll attach this to the character's hand.
    const std::string weaponPath = FileSystem::getPath("sword/sheath sword 1.fbx");
    Model weaponModel(weaponPath);

    // Load a texture for the X Bot character manually
    // Since X Bot doesn't have embedded textures, we'll use one of the character textures as a test skin.
    unsigned int xBotTexture;
    glGenTextures(1, &xBotTexture);
    glBindTexture(GL_TEXTURE_2D, xBotTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int xBotW, xBotH, xBotC;
    // Using texture-a.png (the blue character texture)
    const std::string xBotTexPath = FileSystem::getPath("resources/objects/character/Textures/texture-a.png");
    unsigned char* xBotData = stbi_load(xBotTexPath.c_str(), &xBotW, &xBotH, &xBotC, 0);
    if (xBotData)
    {
        GLenum format = (xBotC == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, xBotW, xBotH, 0, format, GL_UNSIGNED_BYTE, xBotData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load X Bot texture: " << xBotTexPath << std::endl;
    }
    stbi_image_free(xBotData);

    // Optional: load all house objects (OBJ) from resources/objects/house
    std::vector<std::unique_ptr<Model>> houseModels;
    std::vector<glm::mat4> houseModelMatrices;
    try
    {
        if (fs::exists(houseDirPath) && fs::is_directory(houseDirPath))
        {
            std::vector<std::string> houseObjPaths;
            for (const auto& entry : fs::directory_iterator(houseDirPath))
            {
                if (!entry.is_regular_file())
                    continue;
                auto p = entry.path();
                if (p.has_extension() && p.extension() == ".obj")
                    houseObjPaths.push_back(p.generic_string());
            }

            std::sort(houseObjPaths.begin(), houseObjPaths.end());
            std::cout << "house: found " << houseObjPaths.size() << " .obj files" << std::endl;
            for (const auto& p : houseObjPaths)
                std::cout << "  house obj: " << p << std::endl;

            // Place them on the lawn in a simple grid
            const int columns = 5;
            const float spacing = 2.0f;
            const float startX = -4.0f;
            const float startZ = -4.0f;
            const float houseScale = 1.0f;
            for (size_t i = 0; i < houseObjPaths.size(); ++i)
            {
                houseModels.push_back(std::make_unique<Model>(houseObjPaths[i]));
                int col = static_cast<int>(i % columns);
                int row = static_cast<int>(i / columns);
                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, glm::vec3(startX + col * spacing, -1.0f, startZ + row * spacing));
                m = glm::scale(m, glm::vec3(houseScale));
                houseModelMatrices.push_back(m);
            }
        }
        else
        {
            std::cout << "house: directory not found (skipping). Put .obj files under: " << houseDirPath << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "house: failed to enumerate/load: " << e.what() << std::endl;
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // texture coords
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // skybox VAO
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // load and create a texture 
    // -------------------------
    unsigned int texture1;
    // texture 1
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    
    // Try to load texture from relative path
    const std::string grassPath = FileSystem::getPath("resources/textures/grass.png");
    std::cout << "grassTexture:  " << grassPath << std::endl;
    unsigned char *data = stbi_load(grassPath.c_str(), &width, &height, &nrChannels, 0);
    
    if (data)
    {
        GLenum format = GL_RGBA;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 2)
            format = GL_RG;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Load character texture manually as fallback - REMOVED
    /*
    unsigned int charTexture;
    glGenTextures(1, &charTexture);
    glBindTexture(GL_TEXTURE_2D, charTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    const char* charPath = "../../../resources/objects/character/Textures/texture-a.png";
    data = stbi_load(charPath, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load character texture" << std::endl;
    }
    stbi_image_free(data);
    */

    ourShader.use();
    ourShader.setInt("texture1", 0);
    
    catShader.use();
    // catShader.setInt("texture_manual", 15); // Use texture unit 15 to avoid conflict with Model class
    catShader.setInt("texture_diffuse1", 1); // character uses texture unit 1 (see Draw offset below)

    skinnedShader.use();
    skinnedShader.setInt("texture_diffuse1", 2); // skinned FBX uses texture unit 2

    cityPbrShader.use();
    cityPbrShader.setInt("texture_diffuse1", 9); // city uses texture unit 9

    // Fallback texture so the character never samples the grass texture by accident.
    // If Assimp loads a real diffuse texture, it will override this binding.
    unsigned int whiteTexture;
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // update animation
        animator.UpdateAnimation(deltaTime);
        swordAnimator.UpdateAnimation(deltaTime);

        // update sunset factor
        if (isSunset)
            sunsetFactor += deltaTime * 0.5f; // 2 seconds to full sunset
        else
            sunsetFactor -= deltaTime * 0.5f;
        
        sunsetFactor = std::clamp(sunsetFactor, 0.0f, 1.0f);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Blue-ish sky color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // render boxes
        // glBindVertexArray(VAO);
        // glm::mat4 model = glm::mat4(1.0f);
        // ourShader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        // render the loaded model
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down
        model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", model2);
        ourModel.Draw(ourShader, 0);

        // render characters
        catShader.use();
        catShader.setMat4("projection", projection);
        catShader.setMat4("view", view);

        // Bind fallback on unit 1. If a character has textures, Model::Draw will bind real ones on unit 1+
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);

        // Put the 4 characters in front of a house and scale them down.
        // User request: make them 1/4 of the current size.
        const float charScale = 0.35f * 0.25f;

        glm::mat4 charModelMatrix = glm::mat4(1.0f);
        charModelMatrix = glm::translate(charModelMatrix, glm::vec3(-4.2f, -1.0f, -2.2f));
        charModelMatrix = glm::scale(charModelMatrix, glm::vec3(charScale));
        catShader.setMat4("model", charModelMatrix);
        characterA.Draw(catShader, 1);

        charModelMatrix = glm::mat4(1.0f);
        charModelMatrix = glm::translate(charModelMatrix, glm::vec3(-2.8f, -1.0f, -2.2f));
        charModelMatrix = glm::scale(charModelMatrix, glm::vec3(charScale));
        catShader.setMat4("model", charModelMatrix);
        characterB.Draw(catShader, 1);

        charModelMatrix = glm::mat4(1.0f);
        charModelMatrix = glm::translate(charModelMatrix, glm::vec3(-4.2f, -1.0f, -3.6f));
        charModelMatrix = glm::scale(charModelMatrix, glm::vec3(charScale));
        catShader.setMat4("model", charModelMatrix);
        characterC.Draw(catShader, 1);

        charModelMatrix = glm::mat4(1.0f);
        charModelMatrix = glm::translate(charModelMatrix, glm::vec3(-2.8f, -1.0f, -3.6f));
        charModelMatrix = glm::scale(charModelMatrix, glm::vec3(charScale));
        catShader.setMat4("model", charModelMatrix);
        characterD.Draw(catShader, 1);

        // render house objects (if any) using Blinn-Phong lighting (texture unit offset 3)
        if (!houseModels.empty())
        {
            houseShader.use();
            houseShader.setMat4("projection", projection);
            houseShader.setMat4("view", view);
            houseShader.setInt("texture_diffuse1", 3);

            // Simple point light
            houseShader.setVec3("lightPos", glm::vec3(2.0f, 6.0f, 2.0f));
            houseShader.setVec3("viewPos", camera.Position);
            houseShader.setVec3("lightColor", glm::vec3(1.0f));
            houseShader.setFloat("ambientStrength", 0.15f);
            houseShader.setFloat("specularStrength", 0.6f);
            houseShader.setFloat("shininess", 64.0f);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);

            for (size_t i = 0; i < houseModels.size(); ++i)
            {
                houseShader.setMat4("model", houseModelMatrices[i]);
                houseModels[i]->Draw(houseShader, 3);
            }
        }

        // render city models on the grass (static) using the same Blinn-Phong shader
        // Lazy-load a couple per frame to keep the window responsive.
        if (cityLoadIndex < cityObjFiles.size())
        {
            const int modelsPerFrame = 2;
            for (int n = 0; n < modelsPerFrame && cityLoadIndex < cityObjFiles.size(); ++n)
            {
                const std::string& path = cityObjFiles[cityLoadIndex];
                auto m = std::make_unique<Model>(path);

                float minY = std::numeric_limits<float>::infinity();
                float maxY = -std::numeric_limits<float>::infinity();
                for (const auto& mesh : m->meshes)
                {
                    for (const auto& v : mesh.vertices)
                    {
                        minY = std::min(minY, v.Position.y);
                        maxY = std::max(maxY, v.Position.y);
                    }
                }

                float scale = 1.0f;
                if (std::isfinite(minY) && std::isfinite(maxY) && (maxY - minY) > 1e-5f)
                    scale = cityTargetHeight / (maxY - minY);
                scale *= cityGlobalScale;
                scale = std::max(0.001f, std::min(50.0f, scale));
                const float y = (std::isfinite(minY) ? (cityGroundY - minY * scale) : cityGroundY);

                const int idx = static_cast<int>(cityLoadIndex);
                const int col = idx % cityColumns;
                const int row = idx / cityColumns;
                const float x = cityStartX + static_cast<float>(col) * citySpacing;
                const float z = cityStartZ + static_cast<float>(row) * citySpacing;

                glm::mat4 mat = glm::mat4(1.0f);
                mat = glm::translate(mat, glm::vec3(x, y, z));
                mat = glm::scale(mat, glm::vec3(scale));

                cityModels.push_back(std::move(m));
                cityModelMatrices.push_back(mat);
                ++cityLoadIndex;
            }
        }

        if (!cityModels.empty())
        {
            cityPbrShader.use();
            cityPbrShader.setMat4("projection", projection);
            cityPbrShader.setMat4("view", view);

            // Simple point light positioned over the city area (so it isn't too far away)
            cityPbrShader.setVec3("lightPos", glm::vec3(cityStartX + 8.0f, 10.0f, cityStartZ + 8.0f));
            cityPbrShader.setVec3("lightColor", glm::vec3(50.0f));
            cityPbrShader.setVec3("viewPos", camera.Position);

            // Default material params for OBJ assets (no metal/roughness maps)
            cityPbrShader.setFloat("metallic", 0.0f);
            cityPbrShader.setFloat("roughness", 0.65f);
            cityPbrShader.setFloat("ao", 1.0f);

            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);

            for (size_t i = 0; i < cityModels.size(); ++i)
            {
                cityPbrShader.setMat4("model", cityModelMatrices[i]);
                cityModels[i]->Draw(cityPbrShader, 9);
            }
        }

        // render animated (skinned) model on the lawn
        skinnedShader.use();
        skinnedShader.setMat4("projection", projection);
        skinnedShader.setMat4("view", view);

        // Bind fallback on unit 2. If the model has textures, SkinnedModel::Draw will bind real ones on unit 2+
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < static_cast<int>(transforms.size()); ++i)
        {
            skinnedShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        // Auto-scale and place the bird in the sky.
        // We align the model's lowest point to a chosen sky height.
        const float skyMinY = 2.5f;
        const float targetModelHeight = 0.8f; // tweak if you want the bird bigger/smaller
        float animScale = 1.0f;
        if (animatedYBoundsValid)
            animScale = targetModelHeight / (animatedMaxY - animatedMinY);

        const float animY = skyMinY - animatedMinY * animScale;

        // Horizontal movement (left-right) in the sky
        const float t = static_cast<float>(glfwGetTime());
        const float flyAmplitude = 4.0f; // meters
        const float flySpeed = 0.6f;     // cycles per second-ish
        const float animX = std::sin(t * flySpeed) * flyAmplitude;

        glm::mat4 animModelMatrix = glm::mat4(1.0f);
        animModelMatrix = glm::translate(animModelMatrix, glm::vec3(animX, animY, -6.0f));
        animModelMatrix = glm::scale(animModelMatrix, glm::vec3(animScale));
        skinnedShader.setMat4("model", animModelMatrix);
        animatedModel.Draw(skinnedShader, 2);

        // Render Sword Character
        // ----------------------
        skinnedShader.use();
        // Re-bind transforms for the sword character
        auto swordTransforms = swordAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < static_cast<int>(swordTransforms.size()); ++i)
            skinnedShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", swordTransforms[i]);

        // Calculate scale to make it roughly 1.8m tall (standard human height)
        float swordScale = 1.0f;
        const float targetSwordHeight = 1.8f;
        if (swordYBoundsValid)
            swordScale = targetSwordHeight / (swordMaxY - swordMinY);
        
        // Place it to the right of the house grid.
        // House grid occupies roughly X=[-4, 4].
        // We place the sword character at X=6.0, Z=0.0 to avoid overlap.
        glm::mat4 swordMatrix = glm::mat4(1.0f);
        float swordY = -1.0f; // on the ground
        if (swordYBoundsValid) swordY -= swordMinY * swordScale; // align bottom to ground

        swordMatrix = glm::translate(swordMatrix, glm::vec3(6.0f, swordY, 0.0f));
        swordMatrix = glm::scale(swordMatrix, glm::vec3(swordScale));
        // Rotate it to face somewhat towards the camera (or just 45 degrees)
        swordMatrix = glm::rotate(swordMatrix, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        skinnedShader.setMat4("model", swordMatrix);

        // Bind the manual texture to unit 2 (which skinnedShader uses)
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, xBotTexture);

        swordModel.Draw(skinnedShader, 2);

        // Render the Sword (Weapon) attached to the Right Hand
        // ---------------------------------------------------
        // 1. Find the bone transform for the Right Hand
        // Note: Mixamo usually uses "mixamorig:RightHand"
        std::string handBoneName = "mixamorig:RightHand";
        auto boneInfoMap = swordModel.GetBoneInfoMap();
        if (boneInfoMap.find(handBoneName) == boneInfoMap.end()) {
            // Fallback if prefix is different or missing
            handBoneName = "RightHand"; 
        }

        // We need the current transform of the hand bone in Model Space.
        // The Animator gives us FinalBoneMatrices, which are (InverseBindPose * CurrentTransform).
        // We need just CurrentTransform. 
        // However, Animator doesn't expose the raw node transforms easily without modifying it.
        // BUT, we can cheat: The FinalBoneMatrix = GlobalTransform * OffsetMatrix.
        // So GlobalTransform = FinalBoneMatrix * inverse(OffsetMatrix).
        
        if (boneInfoMap.find(handBoneName) != boneInfoMap.end())
        {
            int boneID = boneInfoMap[handBoneName].id;
            glm::mat4 finalBoneMatrix = swordTransforms[boneID];
            glm::mat4 offsetMatrix = boneInfoMap[handBoneName].offset;
            glm::mat4 globalBoneTransform = finalBoneMatrix * glm::inverse(offsetMatrix);

            // Now we have the hand's position/rotation in Model Space.
            // Construct the weapon's model matrix:
            // WeaponWorld = CharacterWorld * HandModelSpace * LocalAdjustments
            
            glm::mat4 weaponMatrix = swordMatrix * globalBoneTransform;
            
            // Adjust the sword relative to the hand (trial and error for rotation/position)
            // Usually swords need to be rotated to fit the grip.
            weaponMatrix = glm::rotate(weaponMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Flip if needed
            weaponMatrix = glm::rotate(weaponMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            weaponMatrix = glm::scale(weaponMatrix, glm::vec3(10.0f)); // Scale up if the sword is tiny (common in FBX imports)

            // Render the weapon using the static shader (houseShader or ourShader)
            // We use houseShader for lighting.
            houseShader.use();
            houseShader.setMat4("projection", projection);
            houseShader.setMat4("view", view);
            houseShader.setMat4("model", weaponMatrix);
            houseShader.setInt("texture_diffuse1", 3); // reuse slot 3
            
            // Bind a texture for the sword (or white)
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            
            weaponModel.Draw(houseShader, 3);
        }

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        skyboxShader.setFloat("sunsetFactor", sunsetFactor);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W/S controls Up/Down (along camera Up vector)
    // A/D controls Left/Right
    float velocity = camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position += camera.Up * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Position -= camera.Up * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
        
    // Optional: Q/E for Forward/Backward
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        isSunset = !isSunset;
    }
}
