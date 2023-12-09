

//#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glcore/shader.h"
#include "glcore/textures.h"
#include "glcore/context.hpp"

#include "../common.hpp"
#include "../objs/creator.hpp"
#include "../window_wrapper.hpp"
#include "../transforms/transform.hpp"

#include "image/image.hpp"
#include "utilities/files/files.hpp"

#include "camera.hpp"

#include "glcore/rasterizer.hpp"
#include "glcore/framebuffer.hpp"
#include "glcore/vertexarray.hpp"
#include "glcore/buffer.hpp"
#include "glcore/commands.hpp"
#include "glcore/common_processing.hpp"

#include <iostream>

auto init_cube() -> nitros::utils::Uptr<nitros::glcore::VertexArray>{
    using namespace nitros;
    auto cube = nitros::Creator::get_instance().indexed_cube2({1, 1, 1}, {2, 2, 2});
    auto vao = std::make_unique<glcore::VertexArray>();
    vao->buffers[0] = std::move(cube.vertices);
    vao->buffers[1] = std::move(cube.normals);
    vao->buffers[2] = std::move(cube.tex_coords);
    vao->index = std::move(cube.indices);
    vao->bind();
    return vao;
}

auto init_plane() -> nitros::utils::Uptr<nitros::glcore::VertexArray> {
    using namespace nitros;
    auto plane = nitros::Creator::get_instance().indexed_plane({1, 1}, {1, 1});
    auto vao = std::make_unique<glcore::VertexArray>();
    vao->buffers[0] = std::move(plane.vertices);
    vao->buffers[1] = std::move(plane.normals);
    vao->buffers[2] = std::move(plane.tex_coords);
    vao->index = std::move(plane.indices);
    vao->bind();
    return vao;
}

auto apply_material(const nitros::Material  &material, nitros::glcore::Shader  &shader)
{
    using namespace nitros;

    shader.set_uniform("material.ambient", material.ambient);
    shader.set_uniform("material.diffuse", material.diffuse);

    shader.set_uniform("material.specular", material.specular);
    shader.set_uniform("material.shineness", material.shineness);
}

auto apply_light(const nitros::PointLight  &light, nitros::glcore::Shader  &shader)
{
    shader.set_uniform("lights.constant", light.constant);
    shader.set_uniform("lights.linear", light.linear);
    shader.set_uniform("lights.quadratic", light.quadratic);

    shader.set_uniform("lights.diffuse", light.diffuse);
    shader.set_uniform("lights.specular", light.specular);
    shader.set_uniform("lights.ambient", light.ambient);
    
    shader.set_uniform("lights.position", light.position);
}

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow *window);

void renderScene(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &cube);
void renderScene(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &cube, nitros::glcore::VertexArray  &plane);
void renderPlane(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &plane);
//void renderCube();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
bool shadows = true;
bool shadowsKeyPressed = false;

bool shift_pressed = false;

bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    auto window = Window{SCR_WIDTH, SCR_HEIGHT};;
    auto&& camera = window.camera;
    camera.Position = {0.0f, 0.0f, 3.0f};

    using namespace nitros;

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {

    // configure global opengl state
    // -----------------------------
    glcore::Rasterizer::get_instance().enable( glcore::Rasterizer::capability::depth_test );
    glcore::Rasterizer::get_instance().enable( glcore::Rasterizer::capability::cull_face );

    // build and compile shaders
    // -------------------------

    auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };
        
    
    auto shader = glcore::Shader{read_file("shaders/shadows/test/point_shadows.vs"), read_file("shaders/shadows/test/point_shadows.fs")};

    auto stages = glcore::shader::Stages{};
    stages.vertex   = read_file("shaders/shadows/test/point_shadows_depth.vs");
    stages.fragment = read_file("shaders/shadows/test/point_shadows_depth.fs");
    stages.geometry = read_file("shaders/shadows/test/point_shadows_depth.gs");
    auto simpleDepthShader = glcore::Shader{stages};

    // load textures
    // -------------
    auto wood_image = utils::image::read_image("resources/textures/wood.png");
    auto wood_t = glcore::ColorTexture{wood_image.meta_data(),glcore::texture::target::texture_2D, true};
    {
        auto params = std::make_unique<glcore::texture::Parameters>();
        {
            using f_min = glcore::texture::Parameters::filter_min_params;
            using f_max = glcore::texture::Parameters::filter_max_params;
            using p = glcore::texture::Parameters;
            using p_min = glcore::texture::Parameters::min_filter;
            using p_mag = glcore::texture::Parameters::mag_filter;

            params->add(
                p_min{f_min::nearest_mipmap_linear},
                p_mag{f_max::linear},
                p::wrap_s{p::wrap_params::mirrored_repeat},
                p::wrap_r{p::wrap_params::mirrored_repeat},
                p::wrap_t{p::wrap_params::mirrored_repeat}
            );
        }
        wood_t.desired_texture_parameters(std::move(params));
    }
    wood_t.texture(wood_image);
    auto& woodTexture = wood_t;

    auto cube_model  = init_cube();
    auto plane_model = init_plane();

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    // create depth cubemap texture

    auto depthCubemap = glcore::DepthTexture{utils::ImageMetaData{{SHADOW_WIDTH, SHADOW_HEIGHT}, utils::pixel::GREY32f::value}, glcore::texture::target::cube_map, false};
    {
        auto cube_params = std::make_unique<glcore::texture::Parameters>();
        {
            using f_min = glcore::texture::Parameters::filter_min_params;
            using f_max = glcore::texture::Parameters::filter_max_params;
            using p = glcore::texture::Parameters;
            using p_min = glcore::texture::Parameters::min_filter;
            using p_mag = glcore::texture::Parameters::mag_filter;
            cube_params->add(
                p_min{f_min::linear},
                p_mag{f_max::linear},
                p::wrap_s{p::wrap_params::clamp_to_edge},
                p::wrap_r{p::wrap_params::clamp_to_edge},
                p::wrap_t{p::wrap_params::clamp_to_edge}
            );
        }
        depthCubemap.desired_texture_parameters(std::move(cube_params));
    }

    auto color_cubeMap = glcore::ColorTexture{utils::ImageMetaData{{SHADOW_WIDTH, SHADOW_HEIGHT}, utils::pixel::RGBA8::value}, glcore::texture::target::cube_map, false};

    auto&& frameBuffer_default = glcore::FrameBuffer::get_default();
    auto frameBuffer_depth = glcore::FrameBuffer{[&col = color_cubeMap, &depth = depthCubemap](){
        auto f_attachment = std::make_unique<glcore::framebuffer::Attachment>();
        f_attachment->depth_view = std::make_unique<glcore::framebuffer::Attachment::DepthView>( depth.image_view(0) ) ;
        f_attachment->color_views.push_back(col.image_view(0));
        return f_attachment;
    }()};

    auto material = Material{};
    material.ambient = {0.2, 0, 0, 1.0f};
    material.diffuse = {1, 0, 0, 1.0f};
    material.shineness = 64;
    material.specular = {0.8, 0.8, 0.8, 1.0f};

    auto material2{material};
    material2.diffuse = {0, 1, 0, 1};

    // lighting info
    // -------------
    //glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
    //glm::vec3 lightPos(0.0f, 3.0f, 0.0f);

    auto light = PointLight{};
    light.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    light.specular = {1, 1, 1, 1};
    light.ambient = {0.2, 0.2, 0.2, 0.2};

    light.constant = 0.04;
    light.linear = 0.25;
    light.quadratic = 0.002;
    light.position  = {0.0f, 3.0f, 0.0f};


    // shader configuration
    // --------------------
    //shader.use();
    //shader.set_uniform("diffuseTexture", utils::vec1i{0});
    //shader.set_uniform("depthMap", utils::vec1i{1});
    //shader.set_uniform("material.ambient", material.ambient);
    //shader.set_uniform("material.diffuse", material.diffuse);
    //shader.set_uniform("material.diffuseTexture", utils::vec1i{0});

    //shader.set_uniform("material.specular", material.specular);
    //shader.set_uniform("material.shineness", material.shineness);

    
    auto first_pass = glcore::ViewDim{};
    first_pass.dimension = {SHADOW_WIDTH, SHADOW_HEIGHT};
    first_pass.left_bottom = {0, 0};

    auto screen_view_dim = glcore::ViewDim{};
    screen_view_dim.dimension = {SCR_WIDTH, SCR_HEIGHT};
    screen_view_dim.left_bottom = {0, 0};

    // render loop
    // -----------
    window.run([&]()
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // move light position over time
        //lightPos.z = sin(glfwGetTime() * 0.5) * 3.0;
        light.position[2] = sin(glfwGetTime() * 0.5) * 10.0;

        
        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane  = 25.0f;
        auto shadowProj = perspective_projection_matrix(glm::radians(90.0f), static_cast<float>(SHADOW_WIDTH), static_cast<float>(SHADOW_HEIGHT), near_plane, far_plane);
        
        auto lightPos = glm::vec3{light.position[0], light.position[1], light.position[2]};
        auto shadowTransforms = std::vector<glm::mat4>
        {
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        // 1. render scene to depth cubemap
        // --------------------------------
        glcore::ViewPort::dimension(first_pass);

        frameBuffer_depth.bind();
        frameBuffer_depth.clear({glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth});
        
        
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i){
            simpleDepthShader.set_uniform_matrix4fv("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        simpleDepthShader.set_uniform("far_plane", far_plane);
        simpleDepthShader.set_uniform("lightPos", utils::vec3f{lightPos[0], lightPos[1], lightPos[2]});
        //renderScene(simpleDepthShader, *cube_model);
        renderScene(simpleDepthShader, *cube_model, *plane_model);
        
        

        frameBuffer_default.bind();
        frameBuffer_default.clear({glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth});
        frameBuffer_default.clear_color(utils::vec4f{0.2f, 0.2f, 0.2f, 1.0f});
        
        // 2. render scene as normal 
        // -------------------------
        glcore::ViewPort::dimension(screen_view_dim);
        shader.use();
        auto projection = perspective_projection_matrix(camera.Zoom, static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f );
        glm::mat4 view = camera.GetViewMatrix();
        shader.set_uniform_matrix4fv("projection", projection);
        shader.set_uniform_matrix4fv("view", view);
        
        // set lighting uniforms
        shader.set_uniform("lightPos", light.position);
        shader.set_uniform("viewPos", to_vec3f(camera.Position));
        shader.set_uniform("shadows", static_cast<std::int32_t>(shadows)); // enable/disable shadows by pressing 'SPACE'
        shader.set_uniform("far_plane", far_plane);

        //shader.set_uniform("diffuseTexture", utils::vec1i{0});
        shader.set_uniform("depthMap", utils::vec1i{1});
        
        apply_light(light, shader);
        apply_material(material, shader);
        //shader.set_uniform("material.diffuseTexture", utils::vec1i{0});
        
        woodTexture.active_bind(0);
        depthCubemap.active_bind(1);
        
        //renderScene(shader, *cube_model);
        renderScene(shader, *cube_model, *plane_model);

    });

    std::cout<<woodTexture.get_id()<<std::endl;

    write_cubemap(*depthCubemap.image_view(), "pt/DIm2");
    write_cubemap(*color_cubeMap.image_view(), "pt/Im2");

    }

    return 0;
}

void renderPlane(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &plane)
{
    auto&& rasterizer = nitros::glcore::Rasterizer::get_instance();

    rasterizer.disable(nitros::glcore::Rasterizer::capability::cull_face);  // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.

    //rasterizer.enable(nitros::glcore::Rasterizer::capability::cull_face);
    // cubes
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0));
    //model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(5.0f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    plane.draw();
}

void renderScene(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &cube, nitros::glcore::VertexArray  &plane)
{
    auto&& rasterizer = nitros::glcore::Rasterizer::get_instance();

    auto model = nitros::model_matrix([](){
        auto srt = nitros::SRT{};
        srt.scale = {0.5, 0.5, 0.5};
        srt.translate = {0, 1, 0};
        return srt;
    }());


    shader.set_uniform_matrix4fv("model", model);
    cube.draw();

    auto model2 = nitros::model_matrix([](){
        auto srt = nitros::SRT{};
        srt.scale = {50, 50, 50};
        srt.rotate = {glm::radians(-90.f), 0, 0};
        srt.translate = {0, -5, 0};
        return srt;
    }());

    shader.set_uniform("material.diffuse", nitros::utils::vec4f{0, 1, 0, 1});
    shader.set_uniform_matrix4fv("model", model2);
    plane.draw();
}


// renders the 3D scene
// --------------------
void renderScene(nitros::glcore::Shader &shader, nitros::glcore::VertexArray  &cube)
{
    auto&& rasterizer = nitros::glcore::Rasterizer::get_instance();

    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));     //Inv Model not used

    rasterizer.disable(nitros::glcore::Rasterizer::capability::cull_face);  // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.

    shader.set_uniform("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    cube.draw();
    shader.set_uniform("reverse_normals", 0); // and of course disable it

    //rasterizer.enable(nitros::glcore::Rasterizer::capability::cull_face);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    cube.draw();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    cube.draw();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    cube.draw();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    cube.draw();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.set_uniform_matrix4fv("model", model);
    shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
    cube.draw();
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
/*void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        shift_pressed = true;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed)
    {
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        shadowsKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        shift_pressed = false;
}*/

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
/*void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if(shift_pressed)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}*/

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//    camera.ProcessMouseScroll(yoffset);
//}