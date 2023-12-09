

#include "../window_wrapper.hpp"
#include "../transforms/transform.hpp"
#include "../objs/creator.hpp"
#include "../common.hpp"

#include "glcore/context.hpp"
#include "glcore/vertexarray.hpp"
#include "glcore/framebuffer.hpp"
#include "glcore/rasterizer.hpp"
#include "glcore/textures.h"
#include "glcore/shader.h"

#include "utilities/files/files.hpp"



int main(int argc, char const *argv[])
{
    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    auto draw_mode = [argc_ = argc](){
        if(argc_ > 1){
            std::cout<<"Patch Mode"<<std::endl;
            return glcore::VertexArray::draw_mode::patches;
        }
        std::cout<<"Traingles Mode"<<std::endl;
        return glcore::VertexArray::draw_mode::triangles;
    }();

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();

        auto curve_points = std::vector<utils::vec3f> {
                                {-0.5, -0.5, 0 },
                                { 0.0,  0.0, 0 },
                                { 0.5, -0.5, 0 }
                            };
        
        
        auto curve =  std::make_shared<glcore::Buffer>();
        curve->write_data(curve_points);

        print_error();

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = curve;
        //vao.index = std::move(plane.indices);
        vao.set_draw_mode(draw_mode, 3);
        vao.bind();

        print_error();

        auto [width, height] = window.window_dim();

        auto _fovy = 45.0f;
        auto near_plane = 0.1f;
        auto far_plane  = 200.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, -3} };

        auto vert = utils::reader::read_binary("shaders/triangle.vert");
        auto frag = utils::reader::read_binary("shaders/triangle.frag");
        auto tesc = utils::reader::read_binary("shaders/triangle.tesc");
        auto tese = utils::reader::read_binary("shaders/triangle.tese");
        
        auto str1 = std::string{};
        str1.append(vert.begin(), vert.end());
        auto str2 = std::string{};
        str2.append(frag.begin(), frag.end());

        auto stages = glcore::shader::Stages{};
        stages.vertex = std::string{}.append(vert.begin(), vert.end());
        stages.fragment = std::string{}.append(frag.begin(), frag.end());
        if(draw_mode == glcore::VertexArray::draw_mode::patches)
        {
            stages.tess_control    = std::string{}.append(tesc.begin(), tesc.end());
            stages.tess_evaluation = std::string{}.append(tese.begin(), tese.end());
        }
        

        auto shader = glcore::Shader{stages};

        glcore::Rasterizer::get_instance().set_polygonMode(glcore::Rasterizer::polygonMode::line);

        print_error();
    
        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(45.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection *  model);
            
            vao.draw();
        });

    }
    return 0;
}
