*There is nothing to see here _yet_ just thinking outloud pretty much*

oglcxx is built on top of GLEW. Instead of including the GLEW header, include `ogl/ogl.h`. Also instead of using `glewInit` use `ogl::init`.

The idea for oglcxx is to provide a c++ typesafe interface to opengl with a minimum and a maximum supported OpenGL version to allow for as little overhead as possible. If, for example OpenGL 4.5 is available and is within the range you allow, direct state access is used in the object interface otherwise there is overhead with binding and unbinding all the time.

On top of oglcxx the hope would be to make an oglcxx-optimizer which turns calls like this.
```c++
ogl::array_buffer vbo;
vbo.data( ... );
```

Which is essentially this
```c++
//ogl::array_buffer constructor
GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);

//vbo.data
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, ...);
```

Into this.
```c++
GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, ...);
```

This gets rid of the redundant calls to `glBindBuffer` because of the c++ implementation overhead. Of course this isn't an issue with direct state access but that is only available in OpenGL version 4.5. 

**WARNING**: do *not* call gl\* functions at all. Only use the functions and objects inside the ogl namespace. oglcxx keeps track of the raw objects and function calls so accessing OpenGL functions directly may cause undefined behaviour. GLEW (WGLEW and GLEWX as well) specific functions should also be avoided.

## Progress

### Buffers
 - map
 - clear
 - copy
 
0%

### Renderbuffers
0%

### Samplers
0%

### Textures
0%

### Framebuffers
0%

### Transform Feedbacks
0%

### Vertex Arrays
0%

### Programs
0%

### Shaders
0%

### Program Pipelines
0%

### Syncs
0%
