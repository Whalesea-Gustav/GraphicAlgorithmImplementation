#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_FrameBuffer.h>

#include "GBufferPass.h"
#include "SSAOCalculationPass.h"
#include "SSAOApplyPass.h"

class SSAOApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameSSAO();

private:

    void updateCamera();

    Light getLight();

    std::vector<Model<VertexPosNormalTex>> m_models;

    std::shared_ptr<Model<VertexPosNormalTex>> p_marry_model;

    std::vector<Shader> m_shaders;

    std::vector<Light> m_lights;

    m_Camera m_camera;

    std::shared_ptr<FrameBuffer> fbo_ptr;
    int SHADOW_WIDTH;
    int SHADOW_HEIGHT;

    float dt = 0.05;

    float lastX;
    float lastY;

    bool bUseSSAO = true;
    GBufferPass gbuffer_pass;
    SSAOCalculationPass ssao_calculation_pass;
    SSAOApplyPass ssao_apply_pass;
};

void SSAOApp::initialize()
{
    this->m_models.emplace_back("../../../asset/mary/mary.obj");

    p_marry_model = std::make_shared<Model<VertexPosNormalTex>>(m_models[0]);

    this->m_models.emplace_back("../../../asset/Geometry/ground.obj");

    this->m_shaders.emplace_back("../../../asset/ModelDraw/model_loading_vs.glsl", "../../../asset/ModelDraw/model_loading_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/PhongLighting/PhongLighting_vs.glsl", "../../../asset/PhongLighting/PhongLighting_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/DepthMap/LightPass_vs.glsl", "../../../asset/DepthMap/LightPass_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/SSAO/ModelDraw_vs.glsl", "../../../asset/SSAO/ModelDraw_fs.glsl");

    this->m_lights.push_back(getLight());


    m_camera.setPosition({ 0, 2, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    gbuffer_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    gbuffer_pass.setModel(p_marry_model);
    gbuffer_pass.initV();

    ssao_calculation_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    ssao_calculation_pass.initV();

    ssao_apply_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    ssao_apply_pass.initV();
}

void SSAOApp::frame()
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
        ImGui::Text("SSAO Effect Checkbox");
        ImGui::Checkbox("SSAO ON", &bUseSSAO);
    }

    //update camera
    updateCamera();

    frameSSAO();

    ImGui::End();
}

void SSAOApp::destroy()
{

}

void SSAOApp::updateCamera() {
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

Light SSAOApp::getLight()
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

void SSAOApp::frameSSAO()
{

    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();

    //Gbuffer pass
    auto p_gbuffer_shader = gbuffer_pass.getShader();
    p_gbuffer_shader->use();
    p_gbuffer_shader->setMat4("model", model);
    p_gbuffer_shader->setMat4("view", view);
    p_gbuffer_shader->setMat4("projection", projection);
    gbuffer_pass.updateV();

    //SSAO calculation pass
    auto p_ssao_calculation_shader = ssao_calculation_pass.getShader();
    p_ssao_calculation_shader->use();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gPositionDepth);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gNormal);
    p_ssao_calculation_shader->setInt("gPositionDepth", 3);
    p_ssao_calculation_shader->setInt("gNormal", 4);
    p_ssao_calculation_shader->setMat4("projection", projection);
    ssao_calculation_pass.updateV();


    //Apply SSAO results to GBuffers;
    auto p_apply_ssao_shader = ssao_apply_pass.getShader();
    p_apply_ssao_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gPositionDepth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_calculation_pass.m_Blur_Buffer);

    p_apply_ssao_shader->setInt("gPositionDepth", 0);
    p_apply_ssao_shader->setInt("gNormal", 1);
    p_apply_ssao_shader->setInt("gAlbedo", 2);
    p_apply_ssao_shader->setInt("ssao", 3);
    p_apply_ssao_shader->setVec3("light.Position", m_lights[0].position);
    p_apply_ssao_shader->setVec3("light.Color", m_lights[0].specular);
    p_apply_ssao_shader->setFloat("light.Linear", m_lights[0].linear);
    p_apply_ssao_shader->setFloat("light. Quadratic", m_lights[0].quadratic);
    p_apply_ssao_shader->setBool("bUseSSAO", bUseSSAO);

    ssao_apply_pass.updateV();
}

