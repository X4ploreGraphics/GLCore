

#ifndef GLCORE_DEV_CREATOR_HPP
#define GLCORE_DEV_CREATOR_HPP

#include "glcore/buffer.hpp"
#include "glcore/textures.h"
#include "../image_rect.hpp"

#include "checker_gen.hpp"
#include "utilities/memory/memory.hpp"
#include  <tuple>
#include <numeric>
#include <limits>

#include <generator/PlaneMesh.hpp>
#include <generator/BoxMesh.hpp>

namespace nitros
{
    struct IndexedBuffers
    {
        public:
        utils::Uptr<glcore::Buffer>  vertices;
        utils::Uptr<glcore::Buffer>  normals;
        utils::Uptr<glcore::Buffer>  indices;
        utils::Uptr<glcore::Buffer>  colors;
        utils::Uptr<glcore::Buffer>  tex_coords;

        std::vector<utils::vec3f>   vertices_data;
        std::vector<utils::vec3f>   normals_data;
        std::vector<utils::vec3Ui>  indices_data;
        std::vector<utils::vec3f>   colors_data;
        std::vector<utils::vec2f>   tex_coords_data;
    };

    class Creator
    {
        public:
        Creator(const Creator &) = delete;
        Creator(Creator &&) = delete;
        ~Creator() = default;

        static auto get_instance() -> Creator{
            return Creator{};
        }

        auto indexed_plane() const -> IndexedBuffers {
            auto vert = std::make_unique<glcore::Buffer>();
            auto indi = std::make_unique<glcore::Buffer>();
            auto cols = std::make_unique<glcore::Buffer>();
            auto norm = std::make_unique<glcore::Buffer>();

            const auto cube_pts = cube_points();
            {
                auto& pts = cube_pts;
                auto points = std::decay_t<decltype(pts)>{};
                points.insert(points.begin(), pts.begin(), pts.begin() + 4);
                vert->write_data(points);
            }
            const auto cube_color = cube_colors();
            {
                auto& pts = cube_color;
                auto points = std::decay_t<decltype(pts)>{};
                points.insert(points.begin(), pts.begin(), pts.begin() + 4);
                cols->write_data(points);
            }
            const auto cube_normal = cube_normals();
            {
                auto& pts = cube_normal;
                auto points = std::decay_t<decltype(pts)>{};
                points.insert(points.begin(), pts.begin(), pts.begin() + 4);
                norm->write_data(points);
            }
            const auto cube_ind = cube_index();
            {
                auto& pts = cube_ind;
                auto points = std::decay_t<decltype(pts)>{};
                points.insert(points.begin(), pts.begin(), pts.begin() + 2);
                indi->write_data(points);
            }

            return {
                std::move(vert),
                std::move(norm),
                std::move(indi),
                std::move(cols)
            };
        }

        template <typename Mesh>
        auto gen_mesh(Mesh &mesh) const
        {
            auto vertices = mesh.vertices();
            auto indices = mesh.triangles();

            std::vector<utils::vec3f>  position, normal;
            std::vector<utils::vec2f> tex_coord;
            std::vector<utils::vec3Ui> index;

            while (!vertices.done())
            {
                auto v = vertices.generate();
                auto pos = v.position;
                auto nor = v.normal;
                auto tex = v.texCoord;

                position.push_back(
                    {gsl::narrow_cast<float>(pos[0]),
                     gsl::narrow_cast<float>(pos[1]),
                     gsl::narrow_cast<float>(pos[2])
                    });

                normal.push_back(
                    {gsl::narrow_cast<float>(nor[0]),
                     gsl::narrow_cast<float>(nor[1]),
                     gsl::narrow_cast<float>(nor[2])
                    });

                tex_coord.push_back(
                    {gsl::narrow_cast<float>(tex[0]),
                     gsl::narrow_cast<float>(tex[1])
                    });
                vertices.next();
            }

            while (!indices.done())
            {
                auto ind = indices.generate().vertices;

                index.push_back(
                    {
                        gsl::narrow_cast<std::uint32_t>(ind[0]),
                        gsl::narrow_cast<std::uint32_t>(ind[1]),
                        gsl::narrow_cast<std::uint32_t>(ind[2])
                    }
                );

                indices.next();
            }

            auto index_buffer = IndexedBuffers{};

            index_buffer.vertices = std::make_unique<glcore::Buffer>();
            index_buffer.vertices->write_data(position);
            index_buffer.vertices_data = std::move(position);

            index_buffer.normals  = std::make_unique<glcore::Buffer>();
            index_buffer.normals->write_data(normal);
            index_buffer.normals_data = std::move(normal);

            index_buffer.indices  = std::make_unique<glcore::Buffer>();
            index_buffer.indices->write_data(index);
            index_buffer.indices_data = std::move(index);

            index_buffer.tex_coords = std::make_unique<glcore::Buffer>();
            index_buffer.tex_coords->write_data(tex_coord);
            index_buffer.tex_coords_data = std::move(tex_coord);

            return index_buffer;
        }

        auto indexed_plane(const std::array<double,2>  &dim, const utils::vec2i  &seg) const
        {
            auto plane = generator::PlaneMesh{{dim[0], dim[1]}, {seg[0], seg[1]}};
            return gen_mesh(plane);
        }

        auto indexed_cube2(const utils::vec3f &dim, const utils::vec3i  &segs) const -> IndexedBuffers 
        {
            auto cube = generator::BoxMesh{{dim[0], dim[1], dim[2]}, {segs[0], segs[1], segs[2]}};
            return gen_mesh(cube);
        }

        // Gives color
        auto indexed_cube() const -> IndexedBuffers {
            auto vert = std::make_unique<glcore::Buffer>();
            auto indi = std::make_unique<glcore::Buffer>();
            auto cols = std::make_unique<glcore::Buffer>();
            auto norm = std::make_unique<glcore::Buffer>();

            vert->write_data(cube_points());
            indi->write_data(cube_index());
            cols->write_data(cube_colors());
            norm->write_data(cube_normals());

            return {
                std::move(vert),
                std::move(norm),
                std::move(indi),
                std::move(cols)
            };
        }   

        private:

        auto cube_points(const utils::vec3f  &offset = {0.f, 0.f, 0.f}, const utils::vec3f  &dim = {0.5f, 0.5f, 0.5f}) const -> std::vector<utils::vec3f>{
            return {
                // front
             {-dim[0] + offset[0], -dim[1] + offset[1],  dim[2] + offset[2]},
             { dim[0] + offset[0], -dim[1] + offset[1],  dim[2] + offset[2]},
             { dim[0] + offset[0],  dim[1] + offset[1],  dim[2] + offset[2]},
             {-dim[0] + offset[0],  dim[1] + offset[1],  dim[2] + offset[2]},
             // back
             {-dim[0] + offset[0], -dim[1] + offset[1], -dim[2] + offset[2]},
             { dim[0] + offset[0], -dim[1] + offset[1], -dim[2] + offset[2]},
             { dim[0] + offset[0],  dim[1] + offset[1], -dim[2] + offset[2]},
             {-dim[0] + offset[0],  dim[1] + offset[1], -dim[2] + offset[2]}
            };
        }

        auto cube_index() const -> std::vector<utils::vec3Ui> {
            return {
                {0, 1, 2},
                {2, 3, 0},

                {4, 5, 6},
                {6, 7, 4}
            };
        }

        auto cube_colors() const -> std::vector<utils::vec3f> {
            return {
             {1.0, 0.0, 0.0},
             {0.0, 1.0, 0.0},
             {0.0, 0.0, 1.0},
             {1.0, 1.0, 1.0},

             {1.0, 0.0, 0.0},
             {0.0, 1.0, 0.0},
             {0.0, 0.0, 1.0},
             {1.0, 1.0, 1.0}
            };
        }

        auto cube_normals() const -> std::vector<utils::vec3f> {
            return {
             {0.0, 0.0, 1.0},
             {0.0, 0.0, 1.0},
             {0.0, 0.0, 1.0},
             {0.0, 0.0, 1.0},

             {0.0, 0.0,-1.0},
             {0.0, 0.0,-1.0},
             {0.0, 0.0,-1.0},
             {0.0, 0.0,-1.0},
            };
        }


        Creator() = default;
    };

}
#endif