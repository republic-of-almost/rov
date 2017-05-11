#ifndef ROV_INCLUDED_7964B68E_F4A3_4F34_BD80_C488CCB98569
#define ROV_INCLUDED_7964B68E_F4A3_4F34_BD80_C488CCB98569


#include <stdint.h>
#include <math/math.hpp> //temp


// -------------------------------------------------------- [ Types and ID's] --


constexpr uint32_t rovPixel_R8        = 1;
constexpr uint32_t rovPixel_RG8       = 2;
constexpr uint32_t rovPixel_RGB8      = 3;
constexpr uint32_t rovPixel_RGBA8     = 4;

constexpr uint32_t rovClearFlag_Color = 1;
constexpr uint32_t rovClearFlag_Depth = 2;

constexpr uint32_t rovInputFormat_PNT = 1;

constexpr uint32_t rovShader_Fullbright = 1;
constexpr uint32_t rovShader_Lit        = 2;
constexpr uint32_t rovShader_DirLight   = 3;


// ------------------------------------------------------------- [ Lifetime ] --


void        rov_initialize();
void        rov_destroy();
void        rov_execute();


// ------------------------------------------------------------ [ Resources ] --


uint32_t    rov_createTexture(const uint8_t *data, size_t size, uint32_t format);
uint32_t    rov_createMesh(const float *pos, const float *normals, const float *tex_coords, size_t count);


// ----------------------------------------------------------- [ Renderpass ] --


void        rov_startRenderPass(const float view[16], const float proj[16], uint32_t clear_flags);


// ----------------------------------------------------- [ General Settings ] --


void        rov_setColor(float r, float g, float b, float a);
void        rov_setCamera(const float view[16], const float proj[16]);
void        rov_setTexture(uint32_t texture_id, uint32_t texture_slot);
void        rov_setMesh(uint32_t mesh_id);
void        rov_setShader(uint32_t shader_id);


// -------------------------------------------------------- [ Mesh Renderer ] --


void        rov_submitMeshTransform(const float world[16]);



#endif // inc guard


// ------------------------------------------------------------- [ ALL APIs ] --


#ifdef ROV_GL_IMPL
#ifndef ROV_DATA_INCLUDED
#define ROV_DATA_INCLUDED



#include <vector>
#include <string>
#include <stdio.h>


namespace
{
  struct rovMat4
  {
    float data[16];
  };
  
  struct Camera
  {
    rovMat4 view;
    rovMat4 proj;
  };
  
  std::vector<Camera> rov_cameras;

  constexpr uint32_t rov_max_textures = 3;

  float       curr_rov_clear_color[4]{0, 0, 0, 1};
  uint8_t     curr_rov_textures[rov_max_textures]{0};
  uint32_t    curr_rov_mesh = 0;
  uint8_t     curr_rov_mesh_shader = rovShader_Fullbright;
}


// -- //


namespace
{
  struct rovMaterial
  {
    uint64_t material;
    uint32_t draw_calls;
  };
  
  struct rovDrawCall
  {
    uint32_t mesh;
    rovMat4 world;
  };

  struct rovRenderPass
  {
    rovMat4 view;
    rovMat4 proj;
    
    uint32_t clear_flags;
    
    std::vector<rovMaterial> materials;
    std::vector<rovDrawCall> draw_calls;
  };
  
  std::vector<rovRenderPass> rov_render_passes;
  
  
  inline uint64_t
  rov_curr_material()
  {
    /*
      Convert color to uint8_t
    */
    uint8_t red   = (uint8_t)(curr_rov_clear_color[0] * 255);
    uint8_t green = (uint8_t)(curr_rov_clear_color[1] * 255);
    uint8_t blue  = (uint8_t)(curr_rov_clear_color[2] * 255);
    uint8_t alpha = (uint8_t)(curr_rov_clear_color[3] * 255);
    
    /*
      Pack color together
    */
    const uint32_t color = lib::bits::pack8888(red, green, blue, alpha);
    
    
    /*
      Pack in the other details
    */
    uint8_t shader_type = curr_rov_mesh_shader;
    uint8_t texture_01 = curr_rov_textures[0];
    uint8_t texture_02 = curr_rov_textures[1];
    uint8_t texture_03 = curr_rov_textures[2];
    
    const uint32_t details = lib::bits::pack8888(shader_type, texture_01, texture_02, texture_03);
    
    return lib::bits::pack3232(details, color);
  }
}


// ----------------------------------------------------------- [ Renderpass ] --


void
rov_startRenderPass(const float view[16], const float proj[16], uint32_t clear_flags)
{
  rov_render_passes.emplace_back();
  
  memcpy(rov_render_passes.back().view.data, view, sizeof(float) * 16);
  memcpy(rov_render_passes.back().proj.data, proj, sizeof(float) * 16);
  
  rov_render_passes.back().clear_flags = clear_flags;
  
  rovMaterial mat{};
  mat.material = rov_curr_material();
  
  rov_render_passes.back().materials.push_back(mat);
}


// ----------------------------------------------------- [ General Settings ] --


void
rov_setColor(float r, float g, float b, float a)
{
  curr_rov_clear_color[0] = r;
  curr_rov_clear_color[1] = g;
  curr_rov_clear_color[2] = b;
  curr_rov_clear_color[3] = a;
}


void
rov_setTexture(uint32_t texture_id, uint32_t texture_slot)
{
  if(texture_slot < rov_max_textures)
  {
    curr_rov_textures[texture_slot] = texture_id;
  }
}


void
rov_setMesh(uint32_t mesh_id)
{
  curr_rov_mesh = mesh_id;
}


void
rov_setShader(uint32_t shader_type)
{
  curr_rov_mesh_shader = shader_type;
}


// -------------------------------------------------------- [ Mesh Renderer ] --


void
rov_submitMeshTransform(const float world[16])
{
  rovDrawCall dc;
  dc.mesh = curr_rov_mesh;
  memcpy(dc.world.data, world, sizeof(float) * 16);
  
  rov_render_passes.back().materials.back().draw_calls += 1;
  rov_render_passes.back().draw_calls.emplace_back(dc);
}



#endif // impl guard
#endif // impl request


// -------------------------------------------------------------- [ OPEN GL ] --


#ifdef ROV_GL_IMPL
#ifndef ROV_IMPL_INCLUDED
#define ROV_IMPL_INCLUDED


// ------------------------------------------------------------ [ Resources ] --


uint32_t
rov_createTexture(const uint8_t *data, size_t size, uint32_t format)
{
}


namespace
{
  struct rov_gl_mesh
  {
    GLuint gl_id;
    size_t vertex_count;
  };
  

  std::vector<rov_gl_mesh> rov_meshes;
}



uint32_t
rov_createMesh(const float *positions, const float *normals, const float *tex_coords, size_t count)
{
  constexpr size_t stride = 3 + 3 + 2;

  std::vector<GLfloat> vertices;
  vertices.resize(count * stride);
  
  for(size_t i = 0; i < count; ++i)
  {
    const size_t index = stride * i;
  
    vertices[index + 0] = positions[(i * 3) + 0];
    vertices[index + 1] = positions[(i * 3) + 1];
    vertices[index + 2] = positions[(i * 3) + 2];

    vertices[index + 3] = normals[(i * 3) + 0];
    vertices[index + 4] = normals[(i * 3) + 1];
    vertices[index + 5] = normals[(i * 3) + 2];
    
    vertices[index + 6] = tex_coords[(i * 2) + 0];
    vertices[index + 7] = tex_coords[(i * 2) + 1];
  }
  
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
  
  const rov_gl_mesh rov_mesh{vbo, count};
  rov_meshes.emplace_back(rov_mesh);
  
  return (uint32_t)rov_meshes.size();
}


// ------------------------------------------------------------- [ Lifetime ] --


namespace
{
  struct rov_gl_shader
  {
    GLuint program;
    
    GLint vs_in_pos;
    GLint vs_in_norm;
    GLint vs_in_uv;
    
    GLint uni_wvp;
  };
  
  rov_gl_shader rov_mesh_shaders[3]; // Fullbright / Lit / DirLight
  
  inline
  void create_mesh_program(
    const char *vs_src,
    const char *gs_src,
    const char *fs_src,
    rov_gl_shader *out)
  {
    auto shd_compiled = [](const GLuint shd_id) -> bool
    {
      GLint is_compiled = 0;
      
      glGetShaderiv(shd_id, GL_COMPILE_STATUS, &is_compiled);
      if(is_compiled == GL_FALSE)
      {
        GLint max_length = 0;
        glGetShaderiv(shd_id, GL_INFO_LOG_LENGTH, &max_length);

        // The maxLength includes the NULL character
        std::vector<GLchar> error_log(max_length);
        
        glGetShaderInfoLog(shd_id, max_length, &max_length, &error_log[0]);

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(shd_id); // Don't leak the shader.
        
        printf("SHD ERR: %s\n", error_log.data());
        
        return false;
      }
      
      return true;
    };
    
    const GLuint vert_shd = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shd, 1, &vs_src, NULL);
    glCompileShader(vert_shd);

    if(!shd_compiled(vert_shd))
    {
      return;
    }
    
    GLuint geo_shd;
    if(gs_src)
    {
      geo_shd = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geo_shd, 1, &gs_src, NULL);
      glCompileShader(geo_shd);

      if(!shd_compiled(geo_shd))
      {
        return;
      }
    }
    
    const GLuint frag_shd = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shd, 1, &fs_src, NULL);
    glCompileShader(frag_shd);
    
    
    if(!shd_compiled(frag_shd))
    {
      return;
    }
    
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert_shd);

    if(gs_src)
    {
      glAttachShader(prog, vert_shd);
    }

    glAttachShader(prog, frag_shd);
    glLinkProgram(prog);
    
    GLint is_linked = 0;

    glGetProgramiv(prog, GL_LINK_STATUS, &is_linked);
    if(is_linked == GL_FALSE)
    {
      GLint max_length = 0;
      glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &max_length);

      // The maxLength includes the NULL character
      std::vector<GLchar> error_log(max_length);
      
      glGetProgramInfoLog(prog, max_length, &max_length, &error_log[0]);

      // Provide the infolog in whatever manor you deem best.
      // Exit with failure.
      
      printf("SHD ERR: %s\n", error_log.data());
    }
    
    
    glUseProgram(prog);
    
    out->vs_in_pos  = glGetAttribLocation(prog, "vs_in_position");
    out->vs_in_norm = glGetAttribLocation(prog, "vs_in_normal");
    out->vs_in_uv   = glGetAttribLocation(prog, "vs_in_texture_coords");
    
    out->uni_wvp = glGetUniformLocation(prog, "uni_wvp");
    
    out->program = prog;

    glBindVertexArray(0);
  }
}


namespace {

  GLuint vao;

}


void
rov_initialize()
{
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  /*
    Fullbright shader
  */
  {
    const std::string vs = R"GLSL(
      #version 330 core
    
      in vec3 vs_in_position;
      in vec3 vs_in_normal;
      in vec2 vs_in_texture_coords;
    
      uniform mat4 uni_wvp;
    
      out vec2 ps_in_texture_coords;
      out vec3 ps_in_color;
    
      void
      main()
      {
        gl_Position = uni_wvp * vec4(vs_in_position, 1.0);
        ps_in_texture_coords = vs_in_texture_coords;
        ps_in_color = vs_in_normal;
      }
    )GLSL";
    
    
    std::string fs = R"GLSL(
      #version 330 core
    
      in vec2 ps_in_texture_coords;
      in vec3 ps_in_color;
    
      uniform sampler2D uni_texture_0;
    
      uniform vec4 uni_color;
    
      out vec4 ps_out_final_color;
    
      void
      main()
      {
        ps_out_final_color = vec4(1,0,1,1);
      }
    )GLSL";
    
    create_mesh_program(vs.c_str(), nullptr, fs.c_str(), &rov_mesh_shaders[0]);
  }
  
  /*
    Lit Shader
  */
  {
    const std::string vs = R"GLSL(
      #version 330 core
    
      in vec3 vs_in_position;
      in vec3 vs_in_normal;
      in vec2 vs_in_texture_coords;
    
      uniform mat4 uni_wvp;
    
      out vec2 ps_in_texture_coords;
      out vec3 ps_in_color;
    
      void
      main()
      {
        gl_Position = uni_wvp * vec4(vs_in_position, 1.0);
        ps_in_texture_coords = vs_in_texture_coords;
        ps_in_color = vs_in_normal;
      }
    )GLSL";
    
    
    std::string fs = R"GLSL(
      #version 330 core
    
      in vec2 ps_in_texture_coords;
      in vec3 ps_in_color;
    
      uniform sampler2D uni_texture_0;
    
      uniform vec4 uni_color;
    
      out vec4 ps_out_final_color;
    
      void
      main()
      {
        ps_out_final_color = vec4(0,1,1,1);
      }
    )GLSL";
    
    create_mesh_program(vs.c_str(), nullptr, fs.c_str(), &rov_mesh_shaders[1]);
  }
  
  /*
    Dir Light shader
  */
  {
    const std::string vs = R"GLSL(
      #version 330 core
    
      in vec3 vs_in_position;
      in vec3 vs_in_normal;
      in vec2 vs_in_texture_coords;
    
      uniform mat4 uni_wvp;
    
      out vec2 ps_in_texture_coords;
      out vec3 ps_in_color;
    
      void
      main()
      {
        gl_Position = uni_wvp * vec4(vs_in_position, 1.0);
        ps_in_texture_coords = vs_in_texture_coords;
        ps_in_color = vs_in_normal;
      }
    )GLSL";
    
    
    std::string fs = R"GLSL(
      #version 330 core
    
      in vec2 ps_in_texture_coords;
      in vec3 ps_in_color;
    
      uniform sampler2D uni_texture_0;
    
      uniform vec4 uni_color;
    
      out vec4 ps_out_final_color;
    
      void
      main()
      {
        ps_out_final_color = vec4(0,0,1,1);
      }
    )GLSL";
    
    create_mesh_program(vs.c_str(), nullptr, fs.c_str(), &rov_mesh_shaders[2]);
  }
}


void
rov_destroy()
{
  
}


void
rov_execute()
{
  glBindVertexArray(vao);

  /*
    For each renderpass.
  */
  for(auto &rp : rov_render_passes)
  {
    GLbitfield clear_flags = 0;
    if(rp.clear_flags & rovClearFlag_Color) { clear_flags |= GL_COLOR_BUFFER_BIT; }
    if(rp.clear_flags & rovClearFlag_Depth) { clear_flags |= GL_DEPTH_BUFFER_BIT; }
  
    glClearColor(1, 1, 0, 1);
    glClear(clear_flags);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1280, 720);
    
    const math::mat4 proj = math::mat4_init_with_array(rp.proj.data);
    const math::mat4 view = math::mat4_init_with_array(rp.view.data);
    
    /*
      For each material in the pass.
    */
    for(auto &mat : rp.materials)
    {
      // Pull Material Info Out
//      const uint32_t color   = lib::bits::upper32(mat.material);
      const uint32_t details = lib::bits::lower32(mat.material);
      
      const uint8_t shader = lib::bits::first8(details);
//      const uint8_t texture_01 = lib::bits::second8(details);
//      const uint8_t texture_02 = lib::bits::third8(details);
//      const uint8_t texture_03 = lib::bits::forth8(details);
      
      size_t dc_index = 0;
      glUseProgram(rov_mesh_shaders[shader].program);
      
      size_t pointer = 0;
      
      if(rov_mesh_shaders[shader].vs_in_pos >= 0)
      {
        glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_pos);
        glVertexAttribPointer(rov_mesh_shaders[shader].vs_in_pos, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)pointer);
      }
      
      pointer += (sizeof(float) * 3);
      
      if(rov_mesh_shaders[shader].vs_in_norm >= 0)
      {
        glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_norm);
        glVertexAttribPointer(rov_mesh_shaders[shader].vs_in_norm, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)pointer);
      }
      
      pointer += (sizeof(float) * 3);
      
      if(rov_mesh_shaders[shader].vs_in_uv)
      {
        glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_uv);
        glVertexAttribPointer(rov_mesh_shaders[shader].vs_in_uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)pointer);
      }
      /*
        For each draw call in the material.
      */
      for(uint32_t i = 0; i < mat.draw_calls; ++i)
      {
        auto &dc = rp.draw_calls[dc_index++];
      
        auto vbo = rov_meshes[shader].gl_id;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        const math::mat4 world   = math::mat4_init_with_array(dc.world.data);
        const math::mat4 wvp_mat = math::mat4_multiply(world, view, proj);

        glUniformMatrix4fv(rov_mesh_shaders[shader].uni_wvp, 1, GL_FALSE, math::mat4_get_data(wvp_mat));
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
    }
  }
  
  glBindVertexArray(0);
  
  rov_render_passes.clear();
}


#endif // impl guard
#endif // impl request
