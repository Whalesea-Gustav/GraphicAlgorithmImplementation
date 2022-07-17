#pragma once

#include <agz-utils/graphics_api/gl/window.h>
#include <iostream>

using namespace agz::gl;

class Demo : public agz::misc::uncopyable_t
{
private:
    window_desc_t windowDesc_;
protected:

    std::unique_ptr<window_t> window_;

    keyboard_t *keyboard_;
    mouse_t    *mouse_;

    virtual void initialize() { }

    virtual void frame() { }

    virtual void destroy() { }

public:

    explicit Demo(window_desc_t windowDesc)
            : windowDesc_(std::move(windowDesc)),
              keyboard_(nullptr), mouse_(nullptr)
    {

    }

    virtual ~Demo() = default;

    void run()
    {
        try
        {
            window_ = std::make_unique<window_t>(windowDesc_);

            keyboard_ = window_->get_keyboard();
            mouse_ = window_->get_mouse();

            initialize();

            while(!window_->get_close_flag())
            {
                window_->do_events();
                window_->new_imgui_frame();

                frame();

                //window_->useDefaultRTVAndDSV();
                window_->use_default_viewport();

                window_->render_imgui();
                window_->swap_buffers();
            }

            destroy();
        }
        catch(const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

};
