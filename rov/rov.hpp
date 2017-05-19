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


void        rov_startRenderPass(const float view[16], const float proj[16], const uint32_t viewport[4], uint32_t clear_flags);


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
#include <utilities/utilities.hpp>


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
  uint32_t    curr_rov_mesh = curr_rov_mesh;
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
    
    uint32_t viewport[4];
    
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
rov_startRenderPass(const float view[16], const float proj[16], const uint32_t viewport[4], uint32_t clear_flags)
{
  rov_render_passes.emplace_back();
  
  memcpy(rov_render_passes.back().view.data, view, sizeof(float) * 16);
  memcpy(rov_render_passes.back().proj.data, proj, sizeof(float) * 16);
  
  memcpy(rov_render_passes.back().viewport, viewport, sizeof(rovRenderPass::viewport));
  
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
  
  size_t draw_calls = 0;
  
  rovMaterial new_mat{};
  new_mat.draw_calls = 1;
  new_mat.material = rov_curr_material();

  
  for(auto it = std::begin(rov_render_passes.back().materials); it != std::end(rov_render_passes.back().materials); ++it)
  {
    // Found a same material
    if(it->material == new_mat.material)
    {
      it->draw_calls += 1;
      rov_render_passes.back().draw_calls.insert(std::begin(rov_render_passes.back().draw_calls) + draw_calls, dc);
      break;
    }
  
    // Found insert point.
    if(it->material > new_mat.material)
    {
      rov_render_passes.back().materials.insert(it, new_mat);
      rov_render_passes.back().draw_calls.insert(std::begin(rov_render_passes.back().draw_calls) + draw_calls, dc);
      break;;
    }
    
    draw_calls += it->draw_calls;
  }
  
  // Insert into the end if we must
  if(rov_render_passes.back().draw_calls.size() == draw_calls)
  {
    rov_render_passes.back().materials.emplace_back(new_mat);
    rov_render_passes.back().draw_calls.emplace_back(dc);
  }
  
//  rov_render_passes.back().materials.back().draw_calls += 1;
//  rov_render_passes.back().draw_calls.emplace_back(dc);
}



#endif // impl guard
#endif // impl request


// -------------------------------------------------------------- [ OPEN GL ] --


#ifdef ROV_GL_IMPL
#ifndef ROV_IMPL_INCLUDED
#define ROV_IMPL_INCLUDED


namespace {

  GLuint vao;

}


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
    size_t vertex_stride;
    size_t vertex_count;
  };
  

  std::vector<rov_gl_mesh> rov_meshes;
}



uint32_t
rov_createMesh(const float *positions, const float *normals, const float *tex_coords, size_t count)
{
  glBindVertexArray(vao);

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
  
  const rov_gl_mesh rov_mesh{vbo, 8, count};
  rov_meshes.emplace_back(rov_mesh);
  
  glBindVertexArray(0);
  
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
    GLint uni_world;
    GLint uni_eye;
    GLint uni_color;
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
    
    out->uni_wvp   = glGetUniformLocation(prog, "uni_wvp");
    out->uni_world = glGetUniformLocation(prog, "uni_world");
    out->uni_eye   = glGetUniformLocation(prog, "uni_eye");
    out->uni_color = glGetUniformLocation(prog, "uni_color");
    
    out->program = prog;

    glBindVertexArray(0);
  }
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
        ps_out_final_color = uni_color;
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
      uniform mat4 uni_world;
    
      out vec2 ps_in_texture_coords;
      out vec3 ps_in_normal;
      out vec3 ps_in_fragpos;
    
      void
      main()
      {
        gl_Position = uni_wvp * vec4(vs_in_position, 1.0);
        ps_in_texture_coords = vs_in_texture_coords;
        ps_in_normal = normalize(mat3(transpose(inverse(uni_world))) * vs_in_normal);
        ps_in_fragpos = vec3(uni_world * vec4(vs_in_position, 1.0));
      }
    )GLSL";
    
    
    std::string fs = R"GLSL(
      #version 330 core
    
      in vec2 ps_in_texture_coords;
      in vec3 ps_in_normal;
      in vec3 ps_in_fragpos;
    
      uniform sampler2D uni_texture_0;
      uniform vec3 uni_eye;
      uniform vec4 uni_color;
    
      out vec4 ps_out_final_color;
    
      void
      main()
      {
        // set the specular term to black
        vec4 spec = vec4(0.0);
        vec3 view_dir = normalize(uni_eye - ps_in_fragpos);
        vec3 l_dir = normalize(vec3(-0.35,1.0,-1.25));
//        vec4 diffuse = texture(uni_map_01, texture_coord);
        vec4 diffuse = vec4(uni_color);//
        vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
        vec4 specular = vec4(0.1, 0.1, 0.1, 1);
        float shininess = 32;
        // normalize both input vectors
        vec3 n = normalize(ps_in_normal);
        vec3 e = normalize(view_dir);
        float intensity = max(dot(n,l_dir), 0.0);
        // if the vertex is lit compute the specular color
        if (intensity > 0.0)
        {
            // compute the half vector
            vec3 h = normalize(l_dir + e);
            // compute the specular term into spec
            float intSpec = max(dot(h,n), 0.0);
            spec = specular * pow(intSpec,shininess);
        }
        ps_out_final_color = max(intensity *  diffuse + spec, ambient);
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
  
  glUseProgram(0);
  glDisable(GL_BLEND);
//  glDisable(GL_CULL_FACE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  
  /*
    For each renderpass.
  */
  for(auto &rp : rov_render_passes)
  {
    GLbitfield cl_flags = 0;
    if(rp.clear_flags & rovClearFlag_Color) { cl_flags |= GL_COLOR_BUFFER_BIT; }
    if(rp.clear_flags & rovClearFlag_Depth) { cl_flags |= GL_DEPTH_BUFFER_BIT; }
    
    glClearColor(1, 1, 0, 1);
    glClear(cl_flags);
    glEnable(GL_DEPTH_TEST);
    glViewport(rp.viewport[0], rp.viewport[1], rp.viewport[2], rp.viewport[3]);
    
    const math::mat4 proj = math::mat4_init_with_array(rp.proj.data);
    const math::mat4 view = math::mat4_init_with_array(rp.view.data);
    
    /*
      For each material in the pass.
    */
    size_t dc_index = 0;
    
    for(auto &mat : rp.materials)
    {
      // Pull Material Info Out
      const uint32_t color   = lib::bits::upper32(mat.material);
      
      const float colorf[4] {
        math::to_float(lib::bits::first8(color)) / 255.f,
        math::to_float(lib::bits::second8(color)) / 255.f,
        math::to_float(lib::bits::third8(color)) / 255.f,
        math::to_float(lib::bits::forth8(color)) / 255.f,
      };
      
      const uint32_t details = lib::bits::lower32(mat.material);
      
      const uint8_t shader = lib::bits::first8(details);
//      const uint8_t texture_01 = lib::bits::second8(details);
//      const uint8_t texture_02 = lib::bits::third8(details);
//      const uint8_t texture_03 = lib::bits::forth8(details);
      
      
      glUseProgram(rov_mesh_shaders[shader].program);
  
      /*
        For each draw call in the material.
      */
      for(uint32_t i = 0; i < mat.draw_calls; ++i)
      {
        auto &dc = rp.draw_calls[dc_index++];
      
        const rov_gl_mesh vbo = rov_meshes[dc.mesh];
        glBindBuffer(GL_ARRAY_BUFFER, vbo.gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        // Vertex
        {
          size_t pointer = 0;
      
          if(rov_mesh_shaders[shader].vs_in_pos >= 0)
          {
            glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_pos);
            glVertexAttribPointer(
              rov_mesh_shaders[shader].vs_in_pos,
              3,
              GL_FLOAT,
              GL_FALSE,
              8 * sizeof(float),
              (void*)pointer
            );
          }
          
          pointer += (sizeof(float) * 3);
          
          if(rov_mesh_shaders[shader].vs_in_norm >= 0)
          {
            glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_norm);
            glVertexAttribPointer(
              rov_mesh_shaders[shader].vs_in_norm,
              3,
              GL_FLOAT,
              GL_FALSE,
              8 * sizeof(float),
              (void*)pointer
            );
          }
          
          pointer += (sizeof(float) * 3);
          
          if(rov_mesh_shaders[shader].vs_in_uv)
          {
            glEnableVertexAttribArray(rov_mesh_shaders[shader].vs_in_uv);
            glVertexAttribPointer(
              rov_mesh_shaders[shader].vs_in_uv,
              2,
              GL_FLOAT,
              GL_FALSE,
              8 * sizeof(float),
              (void*)pointer
            );
          }
        }
        
        const math::mat4 world   = math::mat4_init_with_array(dc.world.data);
        const math::mat4 wvp_mat = math::mat4_multiply(world, view, proj);
        const math::vec4 pos     = math::mat4_multiply(math::vec4_init(0,0,0,1), view);

        glUniformMatrix4fv(rov_mesh_shaders[shader].uni_wvp, 1, GL_FALSE, math::mat4_get_data(wvp_mat));
        glUniformMatrix4fv(rov_mesh_shaders[shader].uni_world, 1, GL_FALSE, math::mat4_get_data(world));
        glUniform3fv(rov_mesh_shaders[shader].uni_eye, 1, pos.data);
        glUniform4fv(rov_mesh_shaders[shader].uni_color, 1, colorf);
       
        glDrawArrays(GL_TRIANGLES, 0, vbo.vertex_count);
      }
    }
  }
  
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  
  glBindVertexArray(0);
  
  rov_render_passes.clear();
}


#endif // impl guard
#endif // impl request
