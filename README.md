# GLCore
GLCore is an OOPs based wrapper library for OpenGL. It provides strong types over C based OpenGL and has C++ object wrappers for OpenGL.

# Resources are Managed with RAII
All openGL object resources are managed, this avoiding resource leaks

# Strong Types
Unlike other wrappers of OpenGL. GLCore uses strong types in its API.  
This restricts one from binding wrong resources into each other. And ensures, that if the program compiles, the OpenGL integration too is correct with the exception of shaders.  

Helps programmers to concentrate on logic, rather than ensuring if the Objects are connected correct.
