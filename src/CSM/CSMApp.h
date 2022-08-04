#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_FrameBuffer.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_ExamplesVAO.h>

#include "DepthMapsPass.h"
#include "MainRenderPass.h"

//Todo:
// 1. Generate ShadowMaps for Cascade Frustrum


class CSMApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameDebug();

    void frameCSM();

private:

    void updateCamera();

    Light getLight();

    std::vector<Model<VertexPosNormalTex>> m_models;

    std::vector<Shader> m_shaders;

    std::vector<Light> m_lights;

    std::vector<Texture> m_textures;

    m_Camera m_camera;

    std::shared_ptr<FrameBuffer> fbo_ptr;
    int SHADOW_WIDTH;
    int SHADOW_HEIGHT;

    float dt = 0.05;

    float lastX;
    float lastY;

    DirectionalLight dirLight;

    DepthMapsPass depthmap_pass;
    MainRenderPass main_render_pass;

    unsigned int loadTexture(const char *path);

    unsigned int woodTexture;
};

void CSMApp::initialize()
{
    m_camera.setPosition({ 0, 0, 0 });
    m_camera.setDirection(0.0f, 3.14159f * 0.5f);
    m_camera.setPerspective(60.0f, 0.1f, 500.0f);
    m_camera.recalculateMatrics();

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    int CSM_resolution = 10;

    depthmap_pass.setWH(CSM_resolution, CSM_resolution);
    depthmap_pass.setCameraZ(m_camera.getFarZ(), m_camera.getNearZ());
    depthmap_pass.cameraZoom = m_camera.getFovDegree();
    depthmap_pass.cameraView = m_camera.getView();
    depthmap_pass.aspectRatio = float(window_->get_framebuffer_width()) / float(window_->get_framebuffer_height());
    depthmap_pass.lightDir =  glm::normalize(glm::vec3(20.0f, 50, 20.0f));
    depthmap_pass.InitPass();

    main_render_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    main_render_pass.InitPass();

    woodTexture = loadTexture("../../../asset/CSM/wood.png");
}

void CSMApp::frame()
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

    bool gui = ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if(gui)
    {
        ImGui::Text("PCF Checkbox");
        ImGui::Checkbox("PCF ON", &main_render_pass.bUsePCF);
    }

    //update camera
    updateCamera();

    //frameCSM();
    frameDebug();

    ImGui::End();
}

void CSMApp::destroy()
{

}

void CSMApp::updateCamera() {
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

Light CSMApp::getLight()
{
    glm::vec3 light_pos = glm::vec3(2, 7, 2);
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


void CSMApp::frameCSM()
{
}

void CSMApp::frameDebug()
{
    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();

    auto depthmap_pass_shader = depthmap_pass.GetShader();
    depthmap_pass_shader->use();

    depthmap_pass.RenderPass();

    auto main_render_pass_shader = main_render_pass.GetShader();
    main_render_pass_shader->use();
    main_render_pass_shader->setMat4("model", model);
    main_render_pass_shader->setMat4("view", view);
    main_render_pass_shader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthmap_pass.lightDepthMaps);

    main_render_pass_shader->setInt("diffuseTexture", 0);
    main_render_pass_shader->setInt("shadowMap", 1);

    main_render_pass_shader->setVec3("lightDir", depthmap_pass.lightDir);
    main_render_pass_shader->setVec3("viewPos", m_camera.getPosition());
    main_render_pass_shader->setFloat("farPlane", depthmap_pass.zFar);
    main_render_pass_shader->setInt("cascadeCount", depthmap_pass.shadowCascadeLevels.size());
    for (int i = 0; i < depthmap_pass.shadowCascadeLevels.size(); ++i)
    {
        main_render_pass_shader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", depthmap_pass.shadowCascadeLevels[i]);
    }


    main_render_pass.RenderPass();
}

unsigned int CSMApp::loadTexture(const char *path)
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
