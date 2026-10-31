#include "../native/gl_scene.h"
#include "../native/gl_context.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
using namespace bel;
int main() {
    GLContext context;NativeFrame f;f.width=496;f.height=384;f.yoffset=128;f.complete=true;
    f.lens={496,384,400,400,248,192};
    for(auto [name,size]:{std::pair{"texture0",1048576},{"texture1",1048576},
        {"palette",16384},{"colorxlat",49152},{"luma",32768},{"gamma",256}})
        f.blobs.push_back({name,std::vector<uint8_t>(size)});
    for(int y=0;y<384;y++)for(int x=0;x<496;x++)f.background.push_back(0xff000040|((x&255)<<16)|((y&255)<<8));
    f.foreground.resize(496*384);
    NativePolygon p;p.count=3;p.window=2;p.focus={400,400};p.center={248,320};p.viewport={0,128,497,512};
    p.vertices={NativeVertex{-80,-1000,10,0,0},NativeVertex{80,-1000,10,0,0},NativeVertex{0,-1200,10,0,0},NativeVertex{}};
    f.polygons={p};
    GLuint fb{},color{},depth{};glGenFramebuffers(1,&fb);glBindFramebuffer(GL_FRAMEBUFFER,fb);
    glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,96,96,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
    glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,96,96);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    GLScene scene;scene.set_gameplay_hud(true);
    auto frame=std::make_shared<NativeFrame>(f);
    auto render=[&](float eye,bool wider,float yaw=0) {
        scene.prepare(frame,wider);glViewport(0,0,96,96);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        auto vp=glm::perspective(glm::radians(90.f),1.f,.05f,100000.f)*
            glm::rotate(glm::mat4(1),glm::radians(yaw),glm::vec3(0,1,0))*glm::translate(glm::mat4(1),glm::vec3(-eye,0,0));
        scene.draw(vp,.6f,1.8f);std::vector<uint8_t> image(96*96*4);glReadPixels(0,0,96,96,GL_RGBA,GL_UNSIGNED_BYTE,image.data());return image;
    };
    auto left=render(-.25,true),right=render(.25,true);
    unsigned changed=0;for(int y=64;y<90;y++)for(int x=10;x<86;x++)for(int c=0;c<3;c++)changed+=left[(y*96+x)*4+c]!=right[(y*96+x)*4+c];
    assert(changed==0); // Sky direction, not a nearby moving screen.
    left=render(-.25,false);right=render(.25,false);changed=0;
    for(int y=64;y<90;y++)for(int x=10;x<86;x++)for(int c=0;c<3;c++)changed+=left[(y*96+x)*4+c]!=right[(y*96+x)*4+c];
    assert(changed>100); // Preserve the accepted original screen-mode behavior.
    auto side=render(0,true,75);assert(side[(80*96+48)*4+2]>=60); // Wider view has native sky color, not a black card edge.
    auto original=render(0,true);
    auto changed_native=std::make_shared<NativeFrame>(f);
    changed_native->native.assign(496*384,0xffff00ff);frame=changed_native;
    auto alternate=render(0,true);
    assert(original==alternate); // Healthy 3D presentation does not consume duplicate CPU-world pixels.
    // Wider view Off must restore the entire original foreground/HUD, even
    // during gameplay. Compare against the production unfiltered renderer.
    auto full_hud=std::make_shared<NativeFrame>(f);
    for(int y=280;y<300;y++)for(int x=350;x<400;x++)full_hud->foreground[y*496+x]=0xffff00ff;
    frame=full_hud;scene.set_gameplay_hud(false);auto reference=render(0,false);
    scene.set_gameplay_hud(true);auto screen_hud=render(0,false);
    assert(reference==screen_hud);
    assert(glGetError()==GL_NO_ERROR);
    glDeleteRenderbuffers(1,&depth);glDeleteTextures(1,&color);glDeleteFramebuffers(1,&fb);
}
