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
#include "DirectLightPass.h"
#include "LightInjectPass.h"
#include "GeometryInjectPass.h"
#include "PropagationPass.h"
#include "IndirectLightPass.h"
#include "ScreenQuadPass.h"

class LPVApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameDebug();

    void frameLPV();

    std::vector<glm::vec3> findAABB(Model<VertexPosNormalTex>& model)
    {
        float min_x = 99999, min_y = 99999, min_z = 99999;
        float max_x = -99999, max_y = -99999, max_z = -99999;

        for (auto mesh : model.meshes)
        {
            for (auto v : mesh.vertices)
            {
                min_x = std::min(min_x, v.Position.x);
                min_y = std::min(min_y, v.Position.y);
                min_z = std::min(min_z, v.Position.z);
                max_x = std::max(max_x, v.Position.x);
                max_y = std::max(max_y, v.Position.y);
                max_z = std::max(max_z, v.Position.z);
            }
        }

        return { glm::vec3(min_x, min_y, min_z), glm::vec3(max_x, max_y, max_z) };
    }


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

    int screen_width;
    int screen_height;
    int rsm_resolution = 128;

    float dt = 0.05;

    float lastX;
    float lastY;

    DirectionalLight dirLight;

    RSMGBufferPass rsm_gbuffer_pass; //light source pass
    GBufferPass gbuffer_pass;
    ShadingComputePass shadingcompute_pass;
    DirectLightPass directlight_pass;
    LightInjectPass lightinject_pass;
    GeometryInjectPass geometryinject_pass;
    PropagationPass propatation_pass;
    IndirectLightPass indirectlight_pass;
    ScreenQuadPass screenquad_pass;
};

void LPVApp::initialize()
{
    m_camera.setPosition({ 0, 2, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    screen_width = this->window_->get_framebuffer_width();
    screen_height = this->window_->get_framebuffer_height();

    //light
    this->m_lights.push_back(getLight());

    auto pModel = std::make_shared<Model<VertexPosNormalTex>>("../../../asset/Sponza/sponza.obj");

    rsm_gbuffer_pass.setWH(rsm_resolution, rsm_resolution);
    rsm_gbuffer_pass.setModel(pModel);
    rsm_gbuffer_pass.initV();

    gbuffer_pass.setWH(screen_width, screen_height);
    gbuffer_pass.setModel(pModel);
    gbuffer_pass.initV();

    //shadingcompute_pass.setWH(window_->get_framebuffer_width(), window_->get_framebuffer_height());
    shadingcompute_pass.setWH(screen_width, screen_height);
    shadingcompute_pass.initV();

    directlight_pass.setWH(screen_width, screen_height);
    directlight_pass.initV();

    //Light Inject Pass Initialization
    std::vector<glm::vec3> aabb = findAABB(*pModel);
    glm::vec3 dimension_float = aabb[1] - aabb[0];
    //m_lights[0].position = 0.5f * (aabb[0] + aabb[1]);


    lightinject_pass.setWH(rsm_resolution, rsm_resolution);
    lightinject_pass.min_AABB = aabb[0];
    lightinject_pass.m_Dimensions = glm::ivec3(static_cast<int>(dimension_float.x),
                                               static_cast<int>(dimension_float.y),
                                               static_cast<int>(dimension_float.z));
    lightinject_pass.initV();

    geometryinject_pass.setWH(rsm_resolution, rsm_resolution);
    geometryinject_pass.min_AABB = aabb[0];
    geometryinject_pass.m_Dimensions = glm::ivec3(static_cast<int>(dimension_float.x),
                                               static_cast<int>(dimension_float.y),
                                               static_cast<int>(dimension_float.z));
    geometryinject_pass.initV();

    propatation_pass.setWH(rsm_resolution, rsm_resolution);
    propatation_pass.min_AABB = aabb[0];
    propatation_pass.max_AABB = aabb[1];
    propatation_pass.m_Dimensions = glm::ivec3(static_cast<int>(dimension_float.x),
                                                  static_cast<int>(dimension_float.y),
                                                  static_cast<int>(dimension_float.z));
    propatation_pass.initV();

    indirectlight_pass.setWH(screen_width, screen_height);
    indirectlight_pass.min_AABB = aabb[0];
    indirectlight_pass.initV();

    screenquad_pass.setWH(screen_width, screen_height);
    screenquad_pass.initV();

    //Quad Shader
    m_shaders.emplace_back("../../../asset/RSM/ScreenQuad_vs.glsl", "../../../asset/RSM/ScreenQuad_fs.glsl");
}

void LPVApp::frame()
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

    //frameLPV();
    frameDebug();
}

void LPVApp::destroy()
{

}

void LPVApp::updateCamera() {
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

Light LPVApp::getLight()
{
    //candidate light information

    //light pos = 0.5 * (min_AABB + max_AABB;)
    //light dir = glm::vec3(-0.8165, -0.57735, 0.00) //maybe -glm::vec3(~)
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

void LPVApp::frameLPV()
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

void LPVApp::frameDebug()
{

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
    auto p_directlight_cs = directlight_pass.getShader();
    p_directlight_cs->use();

    //6 texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gAlbedo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gPosition);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gLightDepthTexture);

    p_directlight_cs->setInt("u_AlbedoTexture", 0);
    p_directlight_cs->setInt("u_NormalTexture", 1);
    p_directlight_cs->setInt("u_PositionTexture", 2);
    p_directlight_cs->setInt("u_LightDepthTexture", 3);


    p_directlight_cs->setMat4("u_LightVPMatrix", lightSpaceMatrix);
    glm::vec4 lightDir = glm::vec4(glm::vec3(0, 1, 0) - light.position, 0.0f);
    p_directlight_cs->setVec3("u_wLightDir", glm::normalize(glm::vec3(-lightDir)));
    p_directlight_cs->setFloat("u_LightIntensity", 1.0f);

    directlight_pass.updateV();

    //light inject pass

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gFlux);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gPosition);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, rsm_gbuffer_pass.gNormal);

    auto p_lightinject_shader = lightinject_pass.getShader();
    p_lightinject_shader->use();
    p_lightinject_shader->setInt("u_RSMRadiantFluxTexture", 7);
    p_lightinject_shader->setInt("u_RSMPositionTexture", 8);

    lightinject_pass.updateV();


    //Geometry Inject Pass
    auto p_geometryinject_shader = geometryinject_pass.getShader();
    p_geometryinject_shader->use();
    p_geometryinject_shader->setInt("u_RSMPositionTexture", 8);
    p_geometryinject_shader->setInt("u_RSMNormalTexture", 9);
    p_geometryinject_shader->setMat4("u_LightViewMat", lightView);

    geometryinject_pass.updateV();

    //Propagation Pass
    auto p_propagation_shader = propatation_pass.getShader();
    p_propagation_shader->use();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, lightinject_pass.m_GridR);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, lightinject_pass.m_GridG);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_3D, lightinject_pass.m_GridB);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_3D, geometryinject_pass.m_GridGV);
    p_propagation_shader->setInt("LPVGridR", 4);
    p_propagation_shader->setInt("LPVGridG", 5);
    p_propagation_shader->setInt("LPVGridB", 6);
    p_propagation_shader->setInt("GeometryVolume", 7);

    propatation_pass.updateV();

    // Calculate indirect light
    auto p_indirectlight_shader = indirectlight_pass.getShader();
    p_indirectlight_shader->use();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, propatation_pass.m_AccumulateGridR.TextureID);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, propatation_pass.m_AccumulateGridG.TextureID);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_3D, propatation_pass.m_AccumulateGridB.TextureID);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gNormal);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, gbuffer_pass.gPosition);

    p_indirectlight_shader->setInt("u_RAccumulatorLPV", 4);
    p_indirectlight_shader->setInt("u_GAccumulatorLPV", 5);
    p_indirectlight_shader->setInt("u_BAccumulatorLPV", 6);
    p_indirectlight_shader->setInt("u_NormalTexture", 7);
    p_indirectlight_shader->setInt("u_PositionTexture", 8);
    p_indirectlight_shader->setMat4("u_InverseCameraViewMatrix", glm::inverse(view));
    indirectlight_pass.updateV();

    //Render Quad
    auto p_screenquad_shader = screenquad_pass.getShader();
    p_screenquad_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, directlight_pass.m_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, indirectlight_pass.indirectLight_TextureConfig.TextureID);

    p_screenquad_shader->setInt("u_DirectTexture", 0);
    p_screenquad_shader->setInt("u_IndirectTexture", 1);

    glViewport(0, 0, screen_width, screen_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenquad_pass.updateV();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Todo:
// 1. 调整RSM大小
