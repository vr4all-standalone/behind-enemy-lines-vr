// SPDX-License-Identifier: GPL-2.0-or-later
#include "gl_scene.h"
#include "model2_scene.h"
#include "model2_scanline.h"
#include "shaders/model2_shaders.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace bel {
static GLuint compile(GLenum type,const char* source) {
    std::string code=source;
#ifdef __ANDROID__
    code.replace(0,code.find('\n'),"#version 300 es\nprecision highp float;\nprecision highp int;\nprecision highp usampler2D;\nprecision highp sampler2D;");
#endif
    auto shader=glCreateShader(type);const char* text=code.c_str();glShaderSource(shader,1,&text,nullptr);glCompileShader(shader);
    GLint ok{};glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[4096]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);glDeleteShader(shader);throw std::runtime_error(log);}return shader;
}
static GLuint program(const char* vertex,const char* fragment) {
    auto vs=compile(GL_VERTEX_SHADER,vertex),fs=compile(GL_FRAGMENT_SHADER,fragment);auto p=glCreateProgram();
    glAttachShader(p,vs);glAttachShader(p,fs);glLinkProgram(p);glDeleteShader(vs);glDeleteShader(fs);
    GLint ok{};glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){char log[4096]{};glGetProgramInfoLog(p,sizeof(log),nullptr,log);glDeleteProgram(p);throw std::runtime_error(log);}return p;
}
struct GLScene::Impl {
    GLuint world{},panel{},vao{},vbo{},panel_vao{},panel_vbo{};
    GLuint resources[6]{},rasters[3]{},scanlines{};
    // Flat integer attributes preserve every primitive's native material and
    // source scanline index in one ordered triangle stream.
    struct WorldVertex {std::array<float,5> raster;std::array<int32_t,4> header,properties;std::array<float,3> camera,original;};
    std::vector<SceneTriangle> triangles;std::shared_ptr<const NativeFrame> frame;
    std::unique_ptr<Model2Resources> materials;
    std::shared_ptr<const NativeReticle> reticle;
    GLsizei vertex_count{};
    bool usable{},wider_view{},immersive{},minimal_hud{};
    NativeLens presentation_override;
    struct WorldUniforms {GLint vp,lens,scale,distance,immersive,hud_size;} w{};
    struct PanelUniforms {GLint vp,lens,distance,size,backdrop,inverse_vp,eye,hud_size;} p{};
    void raster(GLuint texture,const std::vector<uint32_t>& pixels,bool transparent) {
        std::vector<uint8_t> rgba(pixels.size()*4);
        for(size_t i=0;i<pixels.size();i++) {auto c=pixels[i];rgba[i*4]=c>>16;rgba[i*4+1]=c>>8;rgba[i*4+2]=c;rgba[i*4+3]=transparent&&c==0?0:255;}
        glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,frame->width,frame->height,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());
    }
    void draw_raster(unsigned index,const glm::mat4& vp,const NativeLens& lens,float distance,float hud_size=1.f) {
        const auto& pixels=index==0?frame->background:index==1?frame->foreground:frame->native;
        if(pixels.size()!=size_t(frame->width)*frame->height)return;
        glUseProgram(panel);glUniformMatrix4fv(p.vp,1,GL_FALSE,glm::value_ptr(vp));
        const bool backdrop=index==0&&minimal_hud&&wider_view;
        glUniform1i(p.backdrop,backdrop);
        glUniform1f(p.hud_size,index==1?hud_size:1.f);
        if(backdrop) {
            const auto inverse=glm::inverse(vp);auto origin=inverse*glm::vec4(0,0,1,0);
            const auto eye=glm::vec3(origin)/origin.w;
            glUniformMatrix4fv(p.inverse_vp,1,GL_FALSE,glm::value_ptr(inverse));glUniform3f(p.eye,eye.x,eye.y,eye.z);
        }
        glUniform4f(p.lens,lens.fx,lens.fy,lens.cx,lens.cy);glUniform1f(p.distance,distance);glUniform2f(p.size,frame->width,frame->height);
        glBindVertexArray(panel_vao);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,rasters[index]);glDrawArrays(GL_TRIANGLES,0,6);
    }
};
GLScene::GLScene():impl_(std::make_unique<Impl>()) {
    auto& i=*impl_;i.world=program(Model2Vertex,Model2Fragment);i.panel=program(Model2PanelVertex,Model2PanelFragment);
    i.w={glGetUniformLocation(i.world,"vp"),glGetUniformLocation(i.world,"lens"),glGetUniformLocation(i.world,"scale"),glGetUniformLocation(i.world,"distance"),glGetUniformLocation(i.world,"immersive"),glGetUniformLocation(i.world,"hud_size")};
    i.p={glGetUniformLocation(i.panel,"vp"),glGetUniformLocation(i.panel,"lens"),glGetUniformLocation(i.panel,"distance"),glGetUniformLocation(i.panel,"size"),glGetUniformLocation(i.panel,"backdrop"),glGetUniformLocation(i.panel,"inverse_vp"),glGetUniformLocation(i.panel,"eye"),glGetUniformLocation(i.panel,"hud_size")};
    glUseProgram(i.world);constexpr const char* names[]={"texture0","texture1","palette","colorxlat","lumaram","gamma"};
    for(int n=0;n<6;n++)glUniform1i(glGetUniformLocation(i.world,names[n]),n);
    glUniform1i(glGetUniformLocation(i.world,"source_scanlines"),6);
    glUseProgram(i.panel);glUniform1i(glGetUniformLocation(i.panel,"image"),0);
    glGenTextures(6,i.resources);glGenTextures(3,i.rasters);glGenTextures(1,&i.scanlines);
    glBindTexture(GL_TEXTURE_2D,i.scanlines);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    for(auto texture:i.resources){glBindTexture(GL_TEXTURE_2D,texture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);}
    for(auto texture:i.rasters){glBindTexture(GL_TEXTURE_2D,texture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}
    glGenVertexArrays(1,&i.vao);glBindVertexArray(i.vao);glGenBuffers(1,&i.vbo);glBindBuffer(GL_ARRAY_BUFFER,i.vbo);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Impl::WorldVertex),nullptr);
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Impl::WorldVertex),reinterpret_cast<void*>(3*sizeof(float)));
    glEnableVertexAttribArray(2);glVertexAttribIPointer(2,4,GL_INT,sizeof(Impl::WorldVertex),reinterpret_cast<void*>(offsetof(Impl::WorldVertex,header)));
    glEnableVertexAttribArray(3);glVertexAttribIPointer(3,4,GL_INT,sizeof(Impl::WorldVertex),reinterpret_cast<void*>(offsetof(Impl::WorldVertex,properties)));
    glEnableVertexAttribArray(4);glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,sizeof(Impl::WorldVertex),reinterpret_cast<void*>(offsetof(Impl::WorldVertex,camera)));
    glEnableVertexAttribArray(5);glVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,sizeof(Impl::WorldVertex),reinterpret_cast<void*>(offsetof(Impl::WorldVertex,original)));
    glGenVertexArrays(1,&i.panel_vao);glBindVertexArray(i.panel_vao);glGenBuffers(1,&i.panel_vbo);glBindBuffer(GL_ARRAY_BUFFER,i.panel_vbo);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
}
GLScene::~GLScene(){auto& i=*impl_;glDeleteProgram(i.world);glDeleteProgram(i.panel);glDeleteBuffers(1,&i.vbo);glDeleteBuffers(1,&i.panel_vbo);glDeleteVertexArrays(1,&i.vao);glDeleteVertexArrays(1,&i.panel_vao);glDeleteTextures(6,i.resources);glDeleteTextures(3,i.rasters);glDeleteTextures(1,&i.scanlines);}
void GLScene::set_immersive(bool enabled){if(impl_->immersive!=enabled){impl_->immersive=enabled;auto frame=impl_->frame;impl_->frame={};prepare(std::move(frame),impl_->wider_view);}}
void GLScene::set_gameplay_hud(bool minimal){if(impl_->minimal_hud!=minimal){impl_->minimal_hud=minimal;auto frame=impl_->frame;impl_->frame={};prepare(std::move(frame),impl_->wider_view);}}
void GLScene::set_presentation_lens(const NativeLens& lens){impl_->presentation_override=lens;}
std::shared_ptr<const NativeReticle> GLScene::reticle()const{return impl_->reticle;}
SceneAim GLScene::trace_aim(Ray ray,float scale,float distance) const {
    const auto& i=*impl_;if(!i.frame)return {};
    const auto lens=i.immersive?native_camera_lens(*i.frame):presentation_lens(*i.frame,&i.presentation_override);
    return trace_prepared_scene_aim(ray,*i.frame,i.triangles,i.usable?i.materials.get():nullptr,scale,distance,lens,i.immersive,i.minimal_hud);
}
void GLScene::prepare(std::shared_ptr<const NativeFrame> frame,bool wider_view) {
    auto& i=*impl_;if(i.frame==frame&&i.wider_view==wider_view)return;
    i.frame=std::move(frame);i.wider_view=wider_view;i.triangles.clear();i.materials.reset();i.reticle.reset();i.vertex_count=0;i.usable=false;if(!i.frame||!i.frame->complete)return;
    const auto& f=*i.frame;i.materials=std::make_unique<Model2Resources>(f);const auto& resource=*i.materials;
    const bool crop_hud=i.minimal_hud;
    i.triangles=f.rejected==0?scene_triangles(f,wider_view,i.immersive,crop_hud):std::vector<SceneTriangle>{};
    i.usable=resource.valid()&&!i.triangles.empty();
    if(i.usable&&crop_hud)i.reticle=std::make_shared<NativeReticle>(native_reticle(f,resource));
    glActiveTexture(GL_TEXTURE0);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    constexpr const char* names[]={"texture0","texture1","palette","colorxlat","luma","gamma"};
    if(i.usable)for(int n=0;n<6;n++)for(const auto& b:f.blobs)if(b.name==names[n]) {
        const int rows=int((b.bytes.size()+2047)/2048);std::vector<uint8_t> padded(size_t(rows)*2048);std::copy(b.bytes.begin(),b.bytes.end(),padded.begin());
        glBindTexture(GL_TEXTURE_2D,i.resources[n]);glTexImage2D(GL_TEXTURE_2D,0,GL_R8UI,2048,rows,0,GL_RED_INTEGER,GL_UNSIGNED_BYTE,padded.data());
    }
    if(f.background.size()==size_t(f.width)*f.height)i.raster(i.rasters[0],f.background,false);
    if(f.foreground.size()==size_t(f.width)*f.height)i.raster(i.rasters[1],crop_hud?essential_hud_raster(f.foreground,f.width,f.height):f.foreground,true);
    if(f.native.size()==size_t(f.width)*f.height)i.raster(i.rasters[2],f.native,false);
    if(i.usable) {
        // Two headers per source polygon, then compact records for its native
        // scanline spans and its original near-clipped ring. Integer offsets
        // are stored as separate x/y coordinates so float precision cannot
        // truncate an index. The 2048-square cap is portable to GLES 3.
        constexpr size_t limit=2048*2048;
        std::vector<std::array<float,4>> atlas(f.polygons.size()*2);
        auto location=[](size_t at){return std::array<float,2>{float(at%2048),float(at/2048)};};
        for(size_t n=0;n<f.polygons.size();n++) {
            if(crop_hud&&native_aim_grid(f.polygons[n]))continue;
            const auto raster=native_polygon_raster(f,f.polygons[n]);
            if(atlas.size()+raster.rows.size()*2+raster.wider_vertices.size()*2>limit)throw std::runtime_error("Model 2 scanline atlas exceeds portable texture bounds");
            auto at=location(atlas.size());atlas[n*2]={at[0],at[1],float(raster.first_y),float(raster.rows.size())};
            for(const auto& row:raster.rows) {
                atlas.push_back({row.start[0],row.start[1],row.start[2],float(row.start_x)});
                atlas.push_back({row.step[0],row.step[1],row.step[2],float(row.stop_x)});
            }
            at=location(atlas.size());atlas[n*2+1]={at[0],at[1],float(raster.wider_vertices.size()),source_front_polygon(f.polygons[n])?0.f:1.f};
            for(const auto& v:raster.wider_vertices) {
                atlas.push_back({v.x,v.y,v.q[0],v.q[1]});atlas.push_back({v.q[2],0,0,0});
            }
        }
        const size_t rows=(atlas.size()+2047)/2048;atlas.resize(rows*2048);
        glBindTexture(GL_TEXTURE_2D,i.scanlines);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,2048,GLsizei(rows),0,GL_RGBA,GL_FLOAT,atlas.data());
    }
    std::vector<Impl::WorldVertex> vertices;vertices.reserve(i.triangles.size()*3);
    for(auto it=i.triangles.rbegin();it!=i.triangles.rend();++it) {
        const auto& p=f.polygons[it->polygon];
        const std::array<int32_t,4> header={int32_t(p.header[0]),int32_t(p.header[1]),int32_t(p.header[2]),int32_t(p.header[3])};
        // Bit0 retains native panel projection; bit1 identifies BEL's known
        // status mesh, after its original source-region crop (not aim meshes).
        const std::array<int32_t,4> properties={int32_t(p.luma),int32_t(p.lod),int32_t(it->panel)|(crop_hud&&native_hud_geometry(p)?2:0),int32_t(it->polygon)};
        // Preserve reverse native occupancy order, including window overrides,
        // checker holes, alpha keys and interleaved world/panel primitives.
        for(const auto& v:it->vertices)vertices.push_back({{v.pixel_x,v.pixel_y,v.depth,v.u,v.v},header,properties,
            {float(v.camera.x),float(v.camera.y),float(v.camera.z)},
            {float(v.native_homogeneous.x),float(v.native_homogeneous.y),float(v.native_homogeneous.z)}});
    }
    i.vertex_count=GLsizei(vertices.size());
    glBindBuffer(GL_ARRAY_BUFFER,i.vbo);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Impl::WorldVertex),vertices.data(),GL_STREAM_DRAW);
    const float w=float(f.width),h=float(f.height);float quad[]={0,0,w,0,w,h,0,0,w,h,0,h};
    glBindBuffer(GL_ARRAY_BUFFER,i.panel_vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STREAM_DRAW);
}
void GLScene::draw(const glm::mat4& vp,float scale,float distance,float hud_size) {
    auto& i=*impl_;if(!i.frame||!i.frame->complete||scale<=0||distance<=0)return;
    const auto lens=i.immersive?native_camera_lens(*i.frame):presentation_lens(*i.frame,&i.presentation_override);if(!lens.valid())return;
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_ALWAYS);glDepthMask(GL_TRUE);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glDisable(GL_STENCIL_TEST);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    if(!i.usable){i.draw_raster(2,vp,lens,distance);glDepthFunc(GL_LESS);return;}
    // minimal_hud is the host's verified gameplay/non-Service gate. Small view
    // and inactive/native fallback paths keep100% without changing saved size.
    if(!i.minimal_hud||!std::isfinite(hud_size)||hud_size<.5f||hud_size>2.f)hud_size=1.f;
    // Native background is a backdrop rather than an opaque depth plane.
    if(i.immersive||i.minimal_hud)glDepthMask(GL_FALSE);
    i.draw_raster(0,vp,lens,distance);
    glDepthMask(GL_TRUE);
    glUseProgram(i.world);glUniformMatrix4fv(i.w.vp,1,GL_FALSE,glm::value_ptr(vp));glUniform4f(i.w.lens,lens.fx,lens.fy,lens.cx,lens.cy);
    glUniform1f(i.w.scale,scale);glUniform1f(i.w.distance,distance);glUniform1i(i.w.immersive,i.immersive);glUniform1f(i.w.hud_size,hud_size);glBindVertexArray(i.vao);
    for(int n=0;n<6;n++){glActiveTexture(GL_TEXTURE0+n);glBindTexture(GL_TEXTURE_2D,i.resources[n]);}
    glActiveTexture(GL_TEXTURE6);glBindTexture(GL_TEXTURE_2D,i.scanlines);
    glDrawArrays(GL_TRIANGLES,0,i.vertex_count);
    if(i.immersive)glDepthMask(GL_FALSE);
    i.draw_raster(1,vp,lens,distance,hud_size);glDepthMask(GL_TRUE);glActiveTexture(GL_TEXTURE0);glDepthFunc(GL_LESS);
}
}
