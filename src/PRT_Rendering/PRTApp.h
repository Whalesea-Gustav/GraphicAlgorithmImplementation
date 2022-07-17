#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
//#include <m_common/m_Model.h>
#include <m_common/m_Model_v2.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_PhysicalLight.h>
#include <m_common/m_FrameBuffer.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_Texture.h>
#include <m_common/m_ExamplesVAO.h>

class PRTApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frame_Debug();

    void frameIBL();

    void frame_IBL_diffuse();

    void frame_IBL_specular_prefilter();

    void frame_IBL_specular_prefilter_precompute();

    void frame_IBL_Cerberus();

    void frame_PRT_diffuse_specular();

private:

    void updateCamera();

    unsigned int loadTexture(char const * path);

    Light getLight();

    void Load_SHProjected_Coeffs();

    std::vector<Model<VertexPosNormalTex>> m_models;
    // for pbr rendering
    std::vector<Modelv2<VertexPosNormalTexTanBitan>> m_models_v2;

    std::vector<Shader> m_shaders;

    std::vector<Light> m_lights;
    std::vector<PhysicalLight> m_pbrLights;

    std::vector<Texture> m_textures;
    std::vector<Texture2D> m_texture2Ds;
    // for pbr rendering
    std::vector<Texture> m_pbr_textures;

    m_Camera m_camera;

    std::shared_ptr<FrameBuffer> fbo_ptr;
    int SHADOW_WIDTH;
    int SHADOW_HEIGHT;

    float dt = 0.05;

    float lastX;
    float lastY;

    static int nRows;
    static int nColumns;
    static float spacing;

    std::vector<glm::vec3> m_SHProjected_Coeffs;
};

int PRTApp::nRows = 7;
int PRTApp::nColumns = 7;
float PRTApp::spacing = 2.5f;

void PRTApp::initialize()
{
    //camera and mouse settings
    m_camera.setPosition({ 0, 0, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    // load SH Projected Coefffs
    Load_SHProjected_Coeffs();


    // load shaders
    this->m_shaders.emplace_back("../../../asset/IBL/hdr2cubemap_vs.glsl", "../../../asset/IBL/hdr2cubemap_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/IBL/irradiancePrecompute_vs.glsl", "../../../asset/IBL/irradiancePrecompute_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/IBL/pbr_diffuse_vs.glsl", "../../../asset/IBL/pbr_diffuse_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/IBL/background_vs.glsl", "../../../asset/IBL/background_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/IBL/prefilter_vs.glsl", "../../../asset/IBL/prefilter_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/PRT/PRT_precompute_vs.glsl", "../../../asset/PRT/PRT_precompute_fs.glsl");
   //id: [6]
    this->m_shaders.emplace_back("../../../asset/IBL/pbr_diffuse_specular_vs.glsl", "../../../asset/IBL/pbr_diffuse_specular_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/IBL/cerberus_pbr_vs.glsl", "../../../asset/IBL/cerberus_pbr_fs.glsl");
    //id: [8]
    this->m_shaders.emplace_back("../../../asset/PRT/PRT_Sphere_vs.glsl", "../../../asset/PRT/PRT_Sphere_fs.glsl");


    stbi_set_flip_vertically_on_load(true);
    int hdr_width, hdr_height, hdr_nrComponents;
    float *data = stbi_loadf("../../../asset/IBL/hdr/newport_loft.hdr", &hdr_width, &hdr_height, &hdr_nrComponents, 0);
    //float *data = stbi_loadf("../../../asset/IBL/hdr/Theatre-Side_2k.hdr", &hdr_width, &hdr_height, &hdr_nrComponents, 0);
    //float *data = stbi_loadf("../../../asset/IBL/hdr/Milkyway_small.hdr", &hdr_width, &hdr_height, &hdr_nrComponents, 0);

    Texture2D hdrTexture;
    hdrTexture.Internal_Format = GL_RGB16F;
    hdrTexture.Image_Format = GL_RGB;
    hdrTexture.DataType = GL_FLOAT;
    hdrTexture.Wrap_S = GL_CLAMP_TO_EDGE;
    hdrTexture.Wrap_T = GL_CLAMP_TO_EDGE;
    hdrTexture.Generate(hdr_width, hdr_height, data);

    Texture hdrInfo;
    hdrInfo.id = hdrTexture.ID;
    hdrInfo.type = "Tex2D";
    hdrInfo.path = "../../../asset/IBL/hdr/newport_loft.hdr";
    m_textures.push_back(hdrInfo);

    //convert hdr to cubemap
    int length = 2048;

    fbo_ptr = FrameBuffer::Create();
    unsigned int captureRBO;
    glGenRenderbuffers(1, &captureRBO);

    fbo_ptr->Bind();
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, length, length);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    fbo_ptr->UnBind();

    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    Texture envCubemapInfo;
    envCubemapInfo.id = envCubemap;
    envCubemapInfo.type = "cubemap";
    envCubemapInfo.path = "";
    m_textures.push_back(envCubemapInfo);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     length, length, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
            {
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
            };

    auto& convertShader = m_shaders[0];

    convertShader.use();
    convertShader.setInt("equirectangularMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture.ID);
    convertShader.setMat4("projection", captureProjection);

    //configure the viewport to the capture dimensions.
    glViewport(0, 0, length, length);
    fbo_ptr->Bind();
    for (unsigned int i = 0; i < 6; ++i)
    {
        convertShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ExamplesVAO::renderCube(); // renders a 1x1 cube
    }
    fbo_ptr->UnBind();


    //precompute irradiance of every Normal
    int precompute_length = 32;
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    Texture irradianceMapInfo;
    irradianceMapInfo.id = irradianceMap;
    irradianceMapInfo.type = "cubemap";
    irradianceMapInfo.path = "";
    m_textures.push_back(irradianceMapInfo);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     GL_RGB16F,
                     precompute_length, precompute_length,0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //resize fbo for irradiance precomputation
    fbo_ptr->Bind();
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, precompute_length, precompute_length);
    fbo_ptr->UnBind();

    auto& irradiance_precompute_shader = m_shaders[1];
    irradiance_precompute_shader.use();
    irradiance_precompute_shader.setInt("environmentMap", 0);
    irradiance_precompute_shader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    fbo_ptr->Bind();
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradiance_precompute_shader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ExamplesVAO::renderCube();
    }
    fbo_ptr->UnBind();

    //prefilter the environment map for specular IBL
    int prefilter_length = 128;
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    Texture prefilterInfo;
    prefilterInfo.id = prefilterMap;
    prefilterInfo.type = "cubemap";
    prefilterInfo.path = "";
    m_textures.push_back(prefilterInfo);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilter_length, prefilter_length, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // quasi-monte-carlo integration
    auto& prefilterShader = m_shaders[4];
    prefilterShader.use();
    prefilterShader.setInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    prefilterShader.setMat4("projection", captureProjection);

    fbo_ptr->Bind();
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth  = static_cast<unsigned int>(prefilter_length * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(prefilter_length * std::pow(0.5, mip));
        // depth buffer
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ExamplesVAO::renderCube();
        }
    }
    fbo_ptr->UnBind();


    //precompute BRDF LUT
    //in PRT, this BRDF LUT is different from IBL because of different split methods
    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);
    Texture brdfInfo;
    brdfInfo.id = brdfLUTTexture;
    brdfInfo.type = "brdfLUT";
    brdfInfo.path = "";
    m_textures.push_back(brdfInfo);
    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto& brdfShader = m_shaders[5];

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    fbo_ptr->Bind();
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ExamplesVAO::renderQuad();
    fbo_ptr->UnBind();

    //reset normal width and height
    glViewport(0, 0, window_->get_framebuffer_width(), window_->get_framebuffer_height());

    //light settings
    glm::vec3 lightPositions[] = {
            glm::vec3(-10.0f,  10.0f, 10.0f),
            glm::vec3( 10.0f,  10.0f, 10.0f),
            glm::vec3(-10.0f, -10.0f, 10.0f),
            glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f)
    };
    for (int i = 0; i < 4; ++i)
    {
        m_pbrLights.emplace_back(lightPositions[i], lightColors[i]);
    }


    glm::vec3 albedo(0.8f, 0.8f, 0.8f);

    // Shader Default Settings
    updateCamera();
    glm::mat4 proj = m_camera.getProj();
    //pbr and background shader settings
    auto& pbrShader = m_shaders[2];
    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setVec3("albedo", albedo);
    pbrShader.setFloat("ao", 1.0f);
    pbrShader.setMat4("projection", proj);

    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);
    backgroundShader.setMat4("projection", proj);

    auto& iblShader = m_shaders[6];
    iblShader.use();
    iblShader.setInt("irradianceMap", 0);
    iblShader.setInt("prefilterMap", 1);
    iblShader.setInt("brdfLUT", 2);
    iblShader.setVec3("albedo", albedo);
    iblShader.setFloat("ao", 1.0f);
    iblShader.setMat4("projection", proj);

    auto& cerberusShader = m_shaders[7];
    cerberusShader.use();
    cerberusShader.setInt("texture_diffuse1", 0);
    cerberusShader.setInt("irradianceMap", 1);
    cerberusShader.setInt("prefilterMap", 2);
    cerberusShader.setInt("brdfLUT", 3);
    cerberusShader.setInt("normalMap", 4);
    cerberusShader.setInt("metallicMap", 5);
    cerberusShader.setInt("roughnessMap", 6);
    cerberusShader.setFloat("ao", 1.0f);
    cerberusShader.setMat4("projection", proj);

    auto& prt_sphere_Shader = m_shaders[8];
    prt_sphere_Shader.use();
    prt_sphere_Shader.setMat4("projection", proj);
    prt_sphere_Shader.setVec3("albedo", albedo);
    prt_sphere_Shader.setFloat("ao", 1.0f);
    for (int i = 0; i < m_SHProjected_Coeffs.size(); ++i)
    {
        prt_sphere_Shader.setVec3("u_PRT_SH_Coef[" + std::to_string(i) + "]", m_SHProjected_Coeffs[i]);
    }

    prt_sphere_Shader.setInt("u_PRT_BRDFLut", 0);


    //pbr Cerberus  Shading
    //this->m_models.emplace_back("../../../asset/mary/mary.obj");
    //this->m_models.emplace_back("../../../asset/Geometry/ground.obj");
    this->m_models_v2.push_back(Modelv2<VertexPosNormalTexTanBitan>("../../../asset/Cerberus_obj/Cerberus_LP.obj", false, true));
    // this->m_models_v2.emplace_back("../../../asset/Cerberus_obj/Cerberus_LP.obj", false, true);

    //Metallic Map
    unsigned int cerberusMetallicMap = this->loadTexture("../../../asset/Cerberus_obj/Cerberus_M.tga");
    Texture cerberusMetallicInfo;
    cerberusMetallicInfo.id = cerberusMetallicMap;
    cerberusMetallicInfo.type = "metallic texture";
    cerberusMetallicInfo.path = "../../../asset/Cerberus_obj/Cerberus_M.tga";
    m_pbr_textures.push_back(cerberusMetallicInfo);
    //Roughness Map
    unsigned int cerberusRoughnessMap = this->loadTexture("../../../asset/Cerberus_obj/Cerberus_R.tga");
    Texture cerberusRoughnessInfo;
    cerberusRoughnessInfo.id = cerberusRoughnessMap;
    cerberusRoughnessInfo.type = "roughness texture";
    cerberusRoughnessInfo.path = "../../../asset/Cerberus_obj/Cerberus_R.tga";
    m_pbr_textures.push_back(cerberusRoughnessInfo);
    //Normal Map
    unsigned int cerberusNormalMap = this->loadTexture("../../../asset/Cerberus_obj/Cerberus_N.tga");
    Texture cerberusNormalInfo;
    cerberusNormalInfo.id = cerberusNormalMap;
    cerberusNormalInfo.type = "normal map";
    cerberusNormalInfo.path = "../../../asset/Cerberus_obj/Cerberus_N.tga";
    m_pbr_textures.push_back(cerberusNormalInfo);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void PRTApp::frame()
{
    // window events

    if(window_->get_keyboard()->is_down(agz::event::keycode::keycode_constants::KEY_ESCAPE))
    {
        window_->set_close_flag(true);
    }

    if(keyboard_->is_down(agz::event::keycode::keycode_constants::KEY_LCTRL))
    {
        mouse_->show_cursor(!mouse_->is_cursor_visible());
        mouse_->set_cursor_lock(!mouse_->is_cursor_locked(), mouse_->get_cursor_lock_x(), mouse_->get_cursor_lock_y());
    }

    //update camera
    updateCamera();

    //frame_IBL_diffuse();
    //frame_IBL_specular_prefilter();
    //frame_Debug();
    //frameIBL();
    //frame_IBL_Cerberus();
    frame_PRT_diffuse_specular();
}

void PRTApp::destroy()
{

}

void PRTApp::updateCamera() {
    m_camera.setWOverH(window_->get_framebuffer_w_over_h());
    if(!mouse_->is_cursor_visible())
    {
        m_camera.update({
                                .front      = keyboard_->is_pressed('W'),
                                .left       = keyboard_->is_pressed('A'),
                                .right      = keyboard_->is_pressed('D'),
                                .back       = keyboard_->is_pressed('S'),
                                .up         = keyboard_->is_pressed(agz::event::keycode::keycode_constants::KEY_SPACE),
                                .down       = keyboard_->is_pressed(agz::event::keycode::keycode_constants::KEY_LSHIFT),
                                .cursorRelX = -static_cast<float>(mouse_->get_relative_cursor_x()),
                                .cursorRelY = static_cast<float>(mouse_->get_relative_cursor_y())
                        });
    }
    m_camera.recalculateMatrics();
}

Light PRTApp::getLight()
{
    glm::vec3 light_pos = glm::vec3(10, 10, 10);
    glm::vec3 light_ambient(0.05, 0.05, 0.05);
    glm::vec3 light_diffuse(1.0, 1.0, 1.0);
    glm::vec3 light_specular(1.0, 1.0, 1.0);

    float light_constant = 1.0f;
    float light_linear = 0.0007f;
    float light_quadratic = 0.0002f;

    return Light{
            .position = light_pos,
            .constant = light_constant,
            .ambient = light_ambient,
            .linear = light_linear,
            .diffuse = light_diffuse,
            .quadratic = light_quadratic,
            .specular = light_specular,
            .pending = 0
    };
}

void PRTApp::frameIBL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getView();
    glm::vec3 cameraPos = m_camera.getPosition();

    auto& iblShader = m_shaders[6];
    iblShader.use();

    iblShader.setMat4("view", view);
    iblShader.setVec3("camPos", cameraPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_textures[4].id);

    glm::mat4 model = glm::identity<glm::mat4>();

    for (int row = 0; row < PRTApp::nRows; ++row)
    {
        iblShader.setFloat("metallic", (float) row / (float) PRTApp::nRows);
        for (int col = 0; col < PRTApp::nColumns; col++)
        {
            iblShader.setFloat("roughness", glm::clamp((float)col / (float)PRTApp::nColumns, 0.05f, 1.0f));

            glm::vec3 translation((col - (PRTApp::nColumns / 2)) * PRTApp::spacing,
                                  (row - (PRTApp::nRows / 2)) * PRTApp::spacing,
                                  -2.0f);
            model = glm::identity<glm::mat4>();
            model = glm::translate(model, translation);

            iblShader.setMat4("model", model);
            ExamplesVAO::renderSphere();
        }
    }

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        iblShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        iblShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newLightPos);
        model = glm::scale(model, glm::vec3(0.5f));
        iblShader.setMat4("model", model);
        ExamplesVAO::renderSphere();
    }

    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setMat4("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[1].id);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id); // display prefilter map
    ExamplesVAO::renderCube();
}

void PRTApp::frame_IBL_diffuse()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getView();
    glm::vec3 cameraPos = m_camera.getPosition();

    auto& pbrShader = m_shaders[2];
    pbrShader.use();

    pbrShader.setMat4("view", view);
    pbrShader.setVec3("camPos", cameraPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);


    glm::mat4 model = glm::identity<glm::mat4>();

    for (int row = 0; row < PRTApp::nRows; ++row)
    {
        pbrShader.setFloat("metallic", (float) row / (float) PRTApp::nRows);
        for (int col = 0; col < PRTApp::nColumns; col++)
        {
            pbrShader.setFloat("roughness", glm::clamp((float)col / (float)PRTApp::nColumns, 0.05f, 1.0f));

            glm::vec3 translation((col - (PRTApp::nColumns / 2)) * PRTApp::spacing,
                                  (row - (PRTApp::nRows / 2)) * PRTApp::spacing,
                                  -2.0f);
            model = glm::identity<glm::mat4>();
            model = glm::translate(model, translation);

            pbrShader.setMat4("model", model);
            ExamplesVAO::renderSphere();
        }
    }

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newLightPos);
        model = glm::scale(model, glm::vec3(0.5f));
        pbrShader.setMat4("model", model);
        ExamplesVAO::renderSphere();
    }

    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setMat4("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[1].id);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
    ExamplesVAO::renderCube();
}

void PRTApp::frame_IBL_specular_prefilter()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getView();
    glm::vec3 cameraPos = m_camera.getPosition();

    auto& pbrShader = m_shaders[2];
    pbrShader.use();

    pbrShader.setMat4("view", view);
    pbrShader.setVec3("camPos", cameraPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);


    glm::mat4 model = glm::identity<glm::mat4>();

    for (int row = 0; row < PRTApp::nRows; ++row)
    {
        pbrShader.setFloat("metallic", (float) row / (float) PRTApp::nRows);
        for (int col = 0; col < PRTApp::nColumns; col++)
        {
            pbrShader.setFloat("roughness", glm::clamp((float)col / (float)PRTApp::nColumns, 0.05f, 1.0f));

            glm::vec3 translation((col - (PRTApp::nColumns / 2)) * PRTApp::spacing,
                                  (row - (PRTApp::nRows / 2)) * PRTApp::spacing,
                                  -2.0f);
            model = glm::identity<glm::mat4>();
            model = glm::translate(model, translation);

            pbrShader.setMat4("model", model);
            ExamplesVAO::renderSphere();
        }
    }

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newLightPos);
        model = glm::scale(model, glm::vec3(0.5f));
        pbrShader.setMat4("model", model);
        ExamplesVAO::renderSphere();
    }

    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setMat4("view", view);
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[1].id);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id); // display irradiance map
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id); // display prefilter map
    ExamplesVAO::renderCube();
}

void PRTApp::frame_IBL_specular_prefilter_precompute()
{

}

void PRTApp::frame_IBL_Cerberus()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getView();
    glm::vec3 cameraPos = m_camera.getPosition();

    auto& iblShader = m_shaders[6];
    iblShader.use();

    iblShader.setMat4("view", view);
    iblShader.setVec3("camPos", cameraPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_textures[4].id);

    glm::mat4 model = glm::identity<glm::mat4>();

    for (int row = 0; row < PRTApp::nRows; ++row)
    {
        iblShader.setFloat("metallic", (float) row / (float) PRTApp::nRows);
        for (int col = 0; col < PRTApp::nColumns; col++)
        {
            iblShader.setFloat("roughness", glm::clamp((float)col / (float)PRTApp::nColumns, 0.05f, 1.0f));

            glm::vec3 translation((col - (PRTApp::nColumns / 2)) * PRTApp::spacing,
                                  (row - (PRTApp::nRows / 2)) * PRTApp::spacing,
                                  -2.0f);
            model = glm::identity<glm::mat4>();
            model = glm::translate(model, translation);

            iblShader.setMat4("model", model);
            ExamplesVAO::renderSphere();
        }
    }

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        iblShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        iblShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newLightPos);
        model = glm::scale(model, glm::vec3(0.5f));
        iblShader.setMat4("model", model);
        ExamplesVAO::renderSphere();
    }

    model = glm::identity<glm::mat4>();
    model = glm::scale(model, glm::vec3(0.025, 0.025, 0.025));


    //Cerberus Rendering
    auto& cerberusShader = m_shaders[7];
    cerberusShader.use();

    cerberusShader.setMat4("view", view);
    cerberusShader.setMat4("model", model);
    cerberusShader.setVec3("camPos", cameraPos);

    //IBL textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_textures[4].id);

    //PBR textures
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[2].id);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[0].id);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[1].id);

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        cerberusShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        cerberusShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);
    }

    m_models_v2[0].Draw(cerberusShader);


    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setMat4("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[1].id);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id); // display prefilter map
    ExamplesVAO::renderCube();
}

void PRTApp::frame_Debug()
{
}

void PRTApp::frame_PRT_diffuse_specular()
{
    //Cannot recover high order information from SH Projection
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getView();
    glm::vec3 cameraPos = m_camera.getPosition();

    auto& prt_sphere_shader = m_shaders[8];
    prt_sphere_shader.use();

    prt_sphere_shader.setMat4("view", view);
    prt_sphere_shader.setVec3("camPos", cameraPos);

    //brdf lut
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[4].id);

    glm::mat4 model = glm::identity<glm::mat4>();

    for (int row = 0; row < PRTApp::nRows; ++row)
    {
        prt_sphere_shader.setFloat("metallic", (float) row / (float) PRTApp::nRows);
        for (int col = 0; col < PRTApp::nColumns; col++)
        {
            prt_sphere_shader.setFloat("roughness", glm::clamp((float)col / (float)PRTApp::nColumns, 0.05f, 1.0f));

            glm::vec3 translation((col - (PRTApp::nColumns / 2)) * PRTApp::spacing,
                                  (row - (PRTApp::nRows / 2)) * PRTApp::spacing,
                                  -2.0f);
            model = glm::identity<glm::mat4>();
            model = glm::translate(model, translation);

            prt_sphere_shader.setMat4("model", model);
            ExamplesVAO::renderSphere();
        }
    }

    //render light source
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        prt_sphere_shader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        prt_sphere_shader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newLightPos);
        model = glm::scale(model, glm::vec3(0.5f));
        prt_sphere_shader.setMat4("model", model);
        ExamplesVAO::renderSphere();
    }

    model = glm::identity<glm::mat4>();
    model = glm::scale(model, glm::vec3(0.025, 0.025, 0.025));


    //Cerberus Rendering
    auto& cerberusShader = m_shaders[7];
    cerberusShader.use();

    cerberusShader.setMat4("view", view);
    cerberusShader.setMat4("model", model);
    cerberusShader.setVec3("camPos", cameraPos);

    //IBL textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_textures[4].id);

    //PBR textures
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[2].id);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[0].id);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_pbr_textures[1].id);

    //add light info to cerberus shader
    for (unsigned int i = 0; i < m_pbrLights.size(); ++i)
    {
        glm::vec3 newLightPos = m_pbrLights[i].position;
        cerberusShader.setVec3("lightPositions[" + std::to_string(i) + "]", newLightPos);
        cerberusShader.setVec3("lightColors[" + std::to_string(i) + "]", m_pbrLights[i].radiance);
    }

    //m_models_v2[0].Draw(cerberusShader);


    auto& backgroundShader = m_shaders[3];
    backgroundShader.use();
    backgroundShader.setMat4("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[1].id);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[2].id); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_textures[3].id); // display prefilter map
    ExamplesVAO::renderCube();
}

void PRTApp::Load_SHProjected_Coeffs()
{
    m_SHProjected_Coeffs.reserve(16);
    std::string path;
    path = "../../../asset/PRT/Loft/SHCoefficients.txt";
    std::ifstream ifs(path);
    if (!ifs)
        throw std::runtime_error("open " + path + " failed");
    int i = 0;
    float r, g, b;
    while (ifs >> r >> g >> b)
    {
        m_SHProjected_Coeffs.emplace_back(r, g, b);
        i++;
    }
}

unsigned int PRTApp::loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}