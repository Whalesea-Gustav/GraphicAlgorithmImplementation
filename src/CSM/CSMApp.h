#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_FrameBuffer.h>
#include <m_common/m_ComputeShader.h>
#include <m_common/m_ExamplesVAO.h>
#include "RSMGBufferPass.h"
#include "GBufferPass.h"
#include "ShadingComputePass.h"

class CSMApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameDebug();

    void frameRSM();

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

    RSMGBufferPass rsm_gbuffer_pass; //light source pass
    GBufferPass gbuffer_pass;
    ShadingComputePass shadingcompute_pass;
};

void CSMApp::initialize()
{
    m_camera.setPosition({ 0, 2, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();


    //light
    this->m_lights.push_back(getLight());

    auto pModel = std::make_shared<Model<VertexPosNormalTex>>("../../../asset/Sponza/sponza.obj");
    int RSM_resolution = 1024;

    rsm_gbuffer_pass.setWH(RSM_resolution, RSM_resolution);
    rsm_gbuffer_pass.setModel(pModel);
    rsm_gbuffer_pass.initV();

    gbuffer_pass.setWH(RSM_resolution, RSM_resolution);
    gbuffer_pass.setModel(pModel);
    gbuffer_pass.initV();

    //shadingcompute_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    shadingcompute_pass.setWH(RSM_resolution, RSM_resolution);
    shadingcompute_pass.initV();

    //Quad Shader
    m_shaders.emplace_back("../../../asset/RSM/ScreenQuad_vs.glsl", "../../../asset/RSM/ScreenQuad_fs.glsl");

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

    //update camera
    updateCamera();

    //frameRSM();
    frameDebug();
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


void CSMApp::frameRSM()
{
    auto& light = m_lights[0];


    // Light Pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    fbo_ptr->Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, m_camera.getNearZ(), m_camera.getFarZ());
    glm::mat4 lightView = glm::lookAt(light.position, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    m_shaders[2].use();
    m_shaders[2].setMat4("lightSpaceMatrix", lightSpaceMatrix);
    m_shaders[2].setMat4("model", glm::identity<glm::mat4>());

    m_models[0].Draw(m_shaders[2]);
    m_models[1].Draw(m_shaders[2]);

    fbo_ptr->UnBind();

    // Compute Shader Pass : Generate SAT
    m_shaders[4].use();
    m_shaders[4].setInt("input_image", 0);
    m_shaders[4].setInt("output_image", 1);
    glBindImageTexture(0, fbo_ptr->GetTextureAttachment()[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, m_textures[0].id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(SHADOW_WIDTH, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(0, m_textures[0].id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, m_textures[1].id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(SHADOW_WIDTH, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Object Pass
    glViewport(0, 0, window_->get_framebuffer_width(), window_->get_framebuffer_height());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaders[3].use();
    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();

    m_shaders[3].setMat4("model", model);
    m_shaders[3].setMat4("view", view);
    m_shaders[3].setMat4("projection", projection);
    m_shaders[3].setMat4("lightSpaceMatrix", lightSpaceMatrix);

    //Light Info
    m_shaders[3].setVec3("light.position", light.position);
    m_shaders[3].setVec3("light.ambient", light.ambient);
    m_shaders[3].setVec3("light.diffuse", light.diffuse);
    m_shaders[3].setVec3("light.specular", light.specular);
    m_shaders[3].setFloat("light.constant", light.constant);
    m_shaders[3].setFloat("light.linear", light.linear);
    m_shaders[3].setFloat("light.quadratic", light.quadratic);
    m_shaders[3].setFloat("shininess", 32.0f);
    m_shaders[3].setVec3("viewPos", m_camera.getPosition());
    m_shaders[3].setBool("blinn", true);

    float light_size = 50.0f;
    m_shaders[3].setFloat("u_LightSize", light_size);
    m_shaders[3].setFloat("u_TextureSize", static_cast<float>(SHADOW_WIDTH));

    m_shaders[3].setInt("shadowMap", 1);

    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, m_textures[1].id);

    //PCF sampling radius
    m_shaders[3].setInt("sampleRadius", 10);

    m_models[0].Draw(m_shaders[3]);
    m_models[1].Draw(m_shaders[3]);
}

void CSMApp::frameDebug()
{
    //Todo:
    // 1. Add RSM Light GBuffer Pass
    // 2. Adjust Light Position, enable to see the inside of sponza
    // 3. Add Camera GBuffer Pass
    // 4. Add Shading Pass (Using Compute Shader)


    //RSM GBuffer Pass
    auto& light = m_lights[0];
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, m_camera.getNearZ(), m_camera.getFarZ());
    glm::mat4 lightView = glm::lookAt(light.position, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    auto p_rsm_gbuffer_shader = rsm_gbuffer_pass.getShader();
    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();
    p_rsm_gbuffer_shader->use();
    p_rsm_gbuffer_shader->setMat4("model", model);
    p_rsm_gbuffer_shader->setMat4("view", view);
    p_rsm_gbuffer_shader->setMat4("projection", projection);
    p_rsm_gbuffer_shader->setMat4("lightViewProjection", lightSpaceMatrix);
    rsm_gbuffer_pass.updateV();

    //Camera GBuffer Pass
    auto p_gbuffer_shader = gbuffer_pass.getShader();
    p_gbuffer_shader->use();
    p_gbuffer_shader->setMat4("model", model);
    p_gbuffer_shader->setMat4("view", view);
    p_gbuffer_shader->setMat4("projection", projection);
    gbuffer_pass.updateV();


    //Shading Compute Pass
    auto p_shading_cs = shadingcompute_pass.getShader();
    p_shading_cs->use();

    //6 texture
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gAlbedo);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gNormal);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gPosition);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gFlux);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gNormal);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gPosition);

    p_shading_cs->setInt("u_AlbedoTexture", 4);
    p_shading_cs->setInt("u_NormalTexture", 5);
    p_shading_cs->setInt("u_PositionTexture", 6);
    p_shading_cs->setInt("u_RSMFluxTexture", 7);
    p_shading_cs->setInt("u_RSMNormalTexture", 8);
    p_shading_cs->setInt("u_RSMPositionTexture", 9);


    p_shading_cs->setMat4("u_LightVPMatrixMulInverseCameraViewMatrix", lightSpaceMatrix * glm::inverse(view));

    p_shading_cs->setFloat("u_MaxSampleRadius", 25.0f);
    p_shading_cs->setInt("u_RSMSize", rsm_gbuffer_pass.m_height);
    p_shading_cs->setInt("u_VPLNum", shadingcompute_pass.m_VPLNum);
    glm::vec4 lightDir = glm::vec4(glm::vec3(0, 1, 0) - light.position, 0.0f);
    p_shading_cs->setVec3("u_LightDirInViewSpace", glm::normalize(glm::vec3(view * lightDir)));

    shadingcompute_pass.updateV();

    //Render Quad
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto& screen_shader = m_shaders[0];
    screen_shader.use();
    screen_shader.setInt("u_Texture2D", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadingcompute_pass.m_texture);
    ExamplesVAO::renderQuad();
}
