#pragma once
#include "demo.h"

#include <m_common/m_Vertex.h>
#include <m_common/m_Model.h>
#include <m_common/m_CameraV2.h>
#include <m_common/m_Light.h>


class ModelApplication : public agz::gl::Demo
{
public :
    using Demo::Demo;
protected:

    void initialize() override;

    void frame() override;

    void destroy() override;

private:

    void updateCamera();

    Light getLight();


    std::vector<Model<VertexPosNormalTex>> m_models;
    std::vector<Shader> m_shaders;
    m_Camera m_camera;

    float dt = 0.05;

    float lastX;
    float lastY;

};

void ModelApplication::initialize()
{
    this->m_models.emplace_back("../../../asset/mary/mary.obj");

    this->m_shaders.emplace_back("../../../asset/ModelDraw/model_loading_vs.glsl", "../../../asset/ModelDraw/model_loading_fs.glsl");


    m_camera.setPosition({ 0, 2, 3 });
    m_camera.setDirection(-3.14159f * 0.5f, 0);
    m_camera.setPerspective(60.0f, 0.1f, 100.0f);

    this->mouse_->show_cursor(true);
    window_->do_events();
}

void ModelApplication::frame()
{
    // window events

    if(window_->get_keyboard()->is_down(agz::event::keycode::keycode_constants::KEY_ESCAPE))
    {
        window_->set_close_flag(true);
    }

    if(keyboard_->is_down(agz::event::keycode::keycode_constants::KEY_LCTRL))
    {
        mouse_->show_cursor(!mouse_->is_cursor_visible());
    }

    //update camera
    updateCamera();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);


    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaders[0].use();
    auto model = glm::identity<glm::mat4>();
    auto view = m_camera.getView();
    auto projection = m_camera.getProj();

    m_shaders[0].setMat4("model", model);
    m_shaders[0].setMat4("view", view);
    m_shaders[0].setMat4("projection", projection);

    m_models[0].Draw(m_shaders[0]);


}
void ModelApplication::destroy()
{

}

void ModelApplication::updateCamera() {
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

Light ModelApplication::getLight()
{
    glm::vec3 light_pos = glm::vec3(10, 10, 10);
    glm::vec3 light_dest = glm::vec3(0, 1, 0);
    return Light{
            .position = light_pos,
            .fadeCosBegin = 1,
            .direction = glm::normalize(light_dest - light_pos),
            .fadeCosEnd = 0.97,
            .incidence = glm::vec3(80, 80, 80),
            .ambient = 0.01f
    };
}