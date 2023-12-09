

#ifndef SHADER_HEADER_HPP
#define SHADER_HEADER_HPP

#include <string>

#if !defined(__EMSCRIPTEN__) 
auto static const version = std::string{
"#version 450 core\n"
};

auto static const vert_header = version;
auto static const frag_header = version;
auto static const geom_header = version;

#else
auto static const version = std::string{
"#version 300 es\n"
};

auto static const vert_header = version + "precision highp float;\n";
auto static const frag_header = version + "precision mediump float;\nprecision lowp sampler2DShadow;\n";
auto static const geom_header = version + "precision highp float;\n";
#endif

#endif