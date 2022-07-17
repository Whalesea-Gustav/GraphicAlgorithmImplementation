#pragma once
#include <m_common/m_demo.h>

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>
#include <m_common/m_FrameBuffer.h>
#include <m_common/m_ComputeShader.h>

class VSMApp : public Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

    void frameVSM();

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

};

void VSMApp::initialize()
{
    this->m_models.emplace_back("../../../asset/mary/mary.obj");
    this->m_models.emplace_back("../../../asset/Geometry/ground.obj");

    this->m_shaders.emplace_back("../../../asset/ModelDraw/model_loading_vs.glsl", "../../../asset/ModelDraw/model_loading_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/PhongLighting/PhongLighting_vs.glsl", "../../../asset/PhongLighting/PhongLighting_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/VSM/VSMLightPass_vs.glsl", "../../../asset/VSM/VSMLightPass_fs.glsl");
    this->m_shaders.emplace_back("../../../asset/VSM/VSM_vs.glsl", "../../../asset/VSM/VSM_fs.glsl");
    this->m_shaders.push_back(ComputeShader("../../../asset/ComputeShader/SAT_1D.glsl"));
    this->m_lights.push_back(getLight());


    m_camera.setPosition({ 0, 2, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->set_cursor_lock(true, this->window_->get_framebuffer_width() / 2.0, this->window_->get_framebuffer_height() / 2.0);
    this->mouse_->show_cursor(false);
    window_->do_events();

    fbo_ptr = FrameBuffer::Create();
    SHADOW_WIDTH = 2048;
    SHADOW_HEIGHT = 2048;
    fbo_ptr->GenerateTexture2DAttachment(SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG32F, GL_RG, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    fbo_ptr->GenerateTexture2DAttachment(SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);

    //texture for compute shader calculation
    m_textures.resize(2);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    for (int i = 0; i < 2; ++i)
    {   m_textures[i].type = "COMPUTE_SHADER_BUFFER";
        glGenTextures(1, &(m_textures[i].id));
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT,
                     0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

}

void VSMApp::frame()
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

    frameVSM();
}

void VSMApp::destroy()
{

}

void VSMApp::updateCamera() {
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

Light VSMApp::getLight()
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

void VSMApp::frameVSM()
{
    //Todo:
    // 1. Generate SAT

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

