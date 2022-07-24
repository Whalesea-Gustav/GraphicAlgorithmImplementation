#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_Model_v2.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_FrameBuffer.h>

#include "ShadowMapPass.h"
#include "GBufferPass.h"
#include "MipmapPass.h"
#include "DirectPass.h"
#include "IndirectPass.h"

class SSRApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameSSR();

    unsigned int loadTexture(const char *path);

private:

    void updateCamera();

    Light getLight();

    std::vector<Model<VertexPosNormalTex>> m_models;

    std::shared_ptr<Modelv2<VertexPosNormalTexTanBitan>> m_pModelv2;
    unsigned AlbedoMap, NormalMap;

    std::vector<Shader> m_shaders;

    std::vector<Light> m_lights;

    m_Camera m_camera;

    std::shared_ptr<FrameBuffer> fbo_ptr;
    int SHADOW_WIDTH;
    int SHADOW_HEIGHT;

    float dt = 0.05;

    float lastX;
    float lastY;

    float lightRadience = 3.0f;

    ShadowMapPass shadowmap_pass;
    GBufferPass gbuffer_pass;
    MipmapComputePass mipmap_pass;
    DirectPass direct_pass;
    IndirectPass indirect_pass;
};

void SSRApp::initialize()
{
    //this->m_models.emplace_back("../../../asset/Cave/cave.obj");
    m_shaders.emplace_back("../../../asset/SSR/ScreenQuad_vs.glsl",
                           "../../../asset/SSR/ScreenQuad_fs.glsl");

    auto pModelv2 = std::make_shared<Modelv2<VertexPosNormalTexTanBitan>>("../../../asset/Cave/cave.obj", false, true);
    m_pModelv2 = pModelv2;
    this->m_lights.push_back(getLight());

    AlbedoMap = loadTexture("../../../asset/Cave/albedo.jpg");
    NormalMap = loadTexture("../../../asset/Cave/normal.jpg");

    //m_camera.setPosition({ 0, 2, 3 });
    //m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPosition({3.54, 0.58, 1.55});
    m_camera.setDirection(3.19, 0.17);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);


    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    shadowmap_pass.setModel(pModelv2);
    shadowmap_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    shadowmap_pass.InitPass();

    gbuffer_pass.setModel(pModelv2);
    gbuffer_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    gbuffer_pass.InitPass();

    mipmap_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    mipmap_pass.InitPass();

    direct_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    direct_pass.InitPass();

    indirect_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    indirect_pass.InitPass();
}

void SSRApp::frame()
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

    frameSSR();
}

void SSRApp::destroy()
{

}

void SSRApp::updateCamera() {
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

Light SSRApp::getLight()
{
    glm::vec3 light_pos = glm::vec3(1, 7, 1);
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

void SSRApp::frameSSR()
{
    //Todo:

    // 1. Generate ShadowMap from light Viewpoint

    auto& light = m_lights[0];

    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, m_camera.getNearZ(), m_camera.getFarZ());
    glm::mat4 lightView = glm::lookAt(light.position, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    glm::vec3 lightDir = glm::normalize(glm::vec3(0, 1, 0) - light.position);

    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();

    auto p_shadowmap_shader = shadowmap_pass.GetShader();

    p_shadowmap_shader->use();

    p_shadowmap_shader->setMat4("model", model);
    p_shadowmap_shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    shadowmap_pass.RenderPass();

    // 2. Generate Two Gbuffers

    auto p_gbuffer_shader = gbuffer_pass.GetShader();
    p_gbuffer_shader->use();
    p_gbuffer_shader->setMat4("model", model);
    p_gbuffer_shader->setMat4("view", view);
    p_gbuffer_shader->setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, AlbedoMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalMap);

    p_gbuffer_shader->setInt("AlbedoMap", 0);
    p_gbuffer_shader->setInt("NormalMap", 1);

    gbuffer_pass.RenderPass();

    // 3. Generate Mipmaps

    auto m_mipmap_shader = mipmap_pass.m_pMipmapShader;
    m_mipmap_shader->use();
    glBindImageTexture(0, gbuffer_pass.gOutput1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    mipmap_pass.RenderPass();

    // 4. Generate Direct Light Flux Texture


    auto m_direct_shader = direct_pass.GetShader();
    m_direct_shader->use();
    m_direct_shader->setVec3("LightDir", lightDir);
    m_direct_shader->setVec3("LightRadiance", glm::vec3(lightRadience));
    m_direct_shader->setMat4("LightProjView", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowmap_pass.depthTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gOutput0);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gOutput1);

    m_direct_shader->setInt("ShadowMap", 3);
    m_direct_shader->setInt("GBuffer0", 4);
    m_direct_shader->setInt("GBuffer1", 5);

    direct_pass.RenderPass();

    auto m_indirect_shader = indirect_pass.GetShader();
    m_indirect_shader->use();
    m_indirect_shader->setMat4("View", view);
    m_indirect_shader->setMat4("Proj", projection);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, direct_pass.directFluxTexture);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gOutput0);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gOutput1);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, mipmap_pass.m_mipmap);

    m_indirect_shader->setInt("Direct", 6);
    m_indirect_shader->setInt("GBuffer0", 7);
    m_indirect_shader->setInt("GBuffer1", 8);
    m_indirect_shader->setInt("ViewDepth", 9);

    indirect_pass.RenderPass();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shaders[0].use();
    glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gOutput0);
    //glBindTexture(GL_TEXTURE_2D, indirect_pass.indirectFluxTexture);
    glBindTexture(GL_TEXTURE_2D, direct_pass.directFluxTexture);
    m_shaders[0].setInt("u_Texture2D", 1);
    ExamplesVAO::renderQuad();

}

unsigned int SSRApp::loadTexture(const char *path)
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