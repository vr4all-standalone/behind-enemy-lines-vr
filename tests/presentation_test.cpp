#include "../native/gl_context.h"
#include "../native/presentation.h"
#include "../native/launch_policy.h"
#include "../native/settings_io.h"
#include "../native/shared/glb_mesh.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
using namespace bel;
static void check(bool ok,const char* why){if(!ok){std::cerr<<"FAIL: "<<why<<'\n';std::exit(1);}}
int main(int argc,char** argv){
    if(argc!=2)return 2;_putenv_s("BEL_ASSETS",argv[1]);
    for(const char* name:{"models/controllers/left.glb","models/controllers/right.glb"}) {
        std::ifstream in(std::filesystem::path(argv[1])/name,std::ios::binary);
        std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
        vr::xr::GlbMesh mesh;std::string error;
        check(!bytes.empty()&&vr::xr::loadGlb(bytes.data(),bytes.size(),mesh,error,8,1024),"bundled controller GLBs load through the production loader");
        check(mesh.positions.size()>300&&!mesh.images.empty(),"controller GLBs contain their original geometry and textures");
    }
    GLContext context;Presentation ui;Settings settings;SharedRuntime runtime;PresentationInput input;
    ui.update(input,settings,runtime,0);input.toggle=true;ui.update(input,settings,runtime,.1);
    check(ui.open(),"menu opens while the arcade engine is loading");
    GLuint image{},fbo{},depth{};glGenTextures(1,&image);glBindTexture(GL_TEXTURE_2D,image);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,image,0);
    glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,256,256);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
    check(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"presentation probe framebuffer is complete");
    glViewport(0,0,256,256);auto vp=glm::perspective(glm::radians(70.f),1.f,.05f,100.f);
    auto render=[&](bool splash){
        glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);ui.draw(vp,settings,GameState{},false,.2,splash);
        std::vector<uint8_t> pixels(256*256*4);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());return pixels;
    };
    auto brand=render(true),menu=render(false);size_t changed=0;
    for(size_t i=0;i<brand.size();i+=4)if(brand[i]!=menu[i]||brand[i+1]!=menu[i+1]||brand[i+2]!=menu[i+2])++changed;
    check(changed>1000,"after the timed splash, an open loading menu must draw over the brand image");
    {
        // Native credits arrive during mode 11, before Start is permitted.
        // Preserve the already-drawn native image instead of covering it again.
        Presentation opening;NativeFrame credits;credits.complete=true;credits.width=496;credits.height=384;
        credits.native.assign(size_t(credits.width)*credits.height,0xffffffff);
        GameState boot;boot.valid=true;boot.mode=11;
        LaunchPolicy launch;launch.observe(&credits);
        glClearColor(.1f,.2f,.3f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        std::vector<uint8_t> before(256*256*4),after(before.size());
        glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,before.data());
        opening.draw(vp,settings,boot,native_frame_presentable(&credits),.2,launch.active());
        glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,after.data());
        check(before==after,"opening credits remain visible before the game becomes Start-ready");
    }
    {
        Presentation instructions;PresentationInput hands;Settings help;GameState game;game.valid=game.ready=true;
        for(int h=0;h<2;h++){hands.hands[h].tracked=true;hands.poses[h]=glm::translate(glm::mat4(1),glm::vec3(h==0?-.17f:.17f,0,-.6f));}
        instructions.update(hands,help,runtime,.3);
        glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        instructions.draw(vp,help,game,true,.3);
        std::vector<uint8_t> pixels(256*256*4);std::vector<float> depths(256*256);
        glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());glReadPixels(0,0,256,256,GL_DEPTH_COMPONENT,GL_FLOAT,depths.data());
        size_t lit[2]{},labels[2]{};
        for(int y=0;y<256;y++)for(int x=0;x<256;x++) {
            size_t i=y*256+x;int h=x/128;
            if(depths[i]<.999f&&(pixels[i*4]||pixels[i*4+1]||pixels[i*4+2]))lit[h]++;
            if(depths[i]>.999f&&pixels[i*4]>90&&pixels[i*4+1]>90&&pixels[i*4+2]>90)labels[h]++;
        }
        check(lit[0]>100&&lit[1]>100,"startup shows both textured tracked controllers");
        check(labels[0]>20&&labels[1]>20,"button-anchored instructional labels are visible beside both controllers");
        runtime.state.valid=true;runtime.state.service=true;
        instructions.update(hands,help,runtime,.4);check(instructions.service()&&!instructions.service_guns(),"native Service acknowledgement keeps list navigation separate from gun aiming");
        runtime.state.gun_calibration=true;runtime.state.service_busy=1;
        instructions.update(hands,help,runtime,.5);check(instructions.service_guns(),"actual busy native gun action enables gun-calibration input mode");
        instructions.toggle_service_guns();check(!instructions.service_guns(),"gun-calibration aiming can return to native Test/Service controls");
        runtime.state={};runtime.state.valid=true;runtime.state.ready=true;
        instructions.update(hands,help,runtime,.6);check(!instructions.service()&&!instructions.service_guns(),"only native Service exit acknowledgement returns to game controls");
    }
    {
        Presentation guns;PresentationInput hands;hands.focused=true;
        GameState game;game.valid=true;game.ready=true;game.playing=true;game.mode=2;for(auto& player:game.players){player.state=1;player.health=4;}
        CabinetInput cabinet;cabinet.hand={0,1};cabinet.cabinet.coins=1;guns.feedback(cabinet,0);
        for(int h=0;h<2;h++){hands.hands[h].tracked=true;hands.poses[h]=glm::translate(glm::mat4(1),glm::vec3(h==0?-.16f:.16f,0,-.65f));}
        guns.update(hands,settings,runtime,.3);settings.laser=false;
        glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        guns.draw(vp,settings,game,true,1,false);
        std::vector<uint8_t> before(256*256*4);std::vector<float> depths(256*256);
        glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,before.data());glReadPixels(0,0,256,256,GL_DEPTH_COMPONENT,GL_FLOAT,depths.data());
        std::array<std::array<int,2>,2> picks{};
        for(int h=0;h<2;h++) {
            bool found=false;
            for(int y=125;y>=70&&!found;y--)for(int x=h*128+12;x<(h+1)*128-12&&!found;x++){
                bool solid=true;for(int dy=-2;dy<=2;dy++)for(int dx=-2;dx<=2;dx++)if(depths[(y+dy)*256+x+dx]>=.999f)solid=false;
                if(!solid)continue;picks[h]={x,y};found=true;
                auto point=glm::inverse(vp)*glm::vec4((x+.5f)/128.f-1,(y+.5f)/128.f-1,depths[y*256+x]*2-1,1);
                auto behind=glm::vec3(point)/point.w*2.f;hands.aim_targets[h]=Vec3{behind.x,behind.y,behind.z};
            }
            check(found,"laser occlusion fixture has a solid interior gun pixel in each hand");
        }
        guns.update(hands,settings,runtime,1);settings.laser=true;
        glDepthMask(GL_TRUE);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);guns.draw(vp,settings,game,true,1,false);
        std::vector<uint8_t> after(before.size());glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,after.data());
        std::vector<float> after_depth(depths.size());glReadPixels(0,0,256,256,GL_DEPTH_COMPONENT,GL_FLOAT,after_depth.data());
        check(depths==after_depth,"translucent laser effects cannot write depth over either gun or the scene");
        for(auto xy:picks){size_t at=(xy[1]*256+xy[0])*4;check(before[at]==after[at]&&before[at+1]==after[at+1]&&before[at+2]==after[at+2],"laser endpoint behind either gun cannot draw through its solid body");}
    }
    {
        Presentation guns;PresentationInput hands;Settings effects;effects.laser=false;
        GameState game;game.valid=true;game.ready=true;game.playing=true;game.mode=2;for(auto& player:game.players){player.state=1;player.health=4;}
        CabinetInput cabinet;cabinet.hand={0,1};cabinet.cabinet.coins=1;guns.feedback(cabinet,0);
        for(int h=0;h<2;h++){
            hands.hands[h].tracked=true;
            hands.poses[h]=glm::translate(glm::mat4(1),glm::vec3(h==0?-.16f:.16f,0,-.65f))*glm::rotate(glm::mat4(1),glm::radians(70.f),glm::vec3(0,1,0));
        }
        guns.update(hands,effects,runtime,1);cabinet.accepted_shots={1,1};guns.feedback(cabinet,1);
        struct Frame {std::vector<uint8_t> rgb;std::vector<float> depth;};
        unsigned capture=0;
        auto frame=[&](const glm::mat4& view,double now){
            glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            guns.draw(view,effects,game,true,now,false);Frame f{std::vector<uint8_t>(256*256*4),std::vector<float>(256*256)};
            glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,f.rgb.data());glReadPixels(0,0,256,256,GL_DEPTH_COMPONENT,GL_FLOAT,f.depth.data());
            if(const char* directory=std::getenv("BEL_EFFECT_PROBE")){
                std::ofstream out(std::filesystem::path(directory)/(std::to_string(capture++)+".ppm"),std::ios::binary);out<<"P6\n256 256\n255\n";
                for(int y=255;y>=0;y--)for(int x=0;x<256;x++)out.write(reinterpret_cast<const char*>(&f.rgb[(y*256+x)*4]),3);
            }
            return f;
        };
        bool flash_visible=true;
        for(float x:{-.032f,.032f}){
            auto f=frame(vp*glm::lookAt(glm::vec3(x,.08f,.04f),glm::vec3(0,0,-.65f),glm::vec3(0,1,0)),1.045);
            size_t light=0;for(size_t i=0;i<f.depth.size();i++)if(f.depth[i]>.9999f&&f.rgb[i*4]>30&&f.rgb[i*4]>f.rgb[i*4+1]*1.1)++light;
            flash_visible=flash_visible&&light>20;
        }
        // Move both guns away: smoke must remain in the room at the previous shot.
        for(auto& pose:hands.poses)pose[3].x+=.6f;
        guns.update(hands,effects,runtime,1.2);
        auto smoke=frame(vp,1.2),again=frame(vp,1.2);
        check(smoke.rgb==again.rgb&&smoke.depth==again.depth,"stereo/repeated draws do not emit extra smoke or change depth");
        size_t puffs=0;
        for(int y=60;y<180;y++)for(int x=12;x<170;x++){
            size_t i=y*256+x;auto r=smoke.rgb[i*4],g=smoke.rgb[i*4+1],b=smoke.rgb[i*4+2];
            if(smoke.depth[i]>.9999f&&r>2&&std::abs(int(r)-g)<4&&std::abs(int(g)-b)<4)++puffs;
        }
        check(puffs>20,"soft smoke remains at the shot location after the guns move away");
        check(flash_visible,"shaped flash is visible outside angled barrels in both eyes after 35ms");
        auto expired=frame(vp,2.0);size_t lingering=0;
        for(size_t i=0;i<expired.depth.size();i++)if(expired.depth[i]>.9999f&&(expired.rgb[i*4]||expired.rgb[i*4+1]||expired.rgb[i*4+2]))++lingering;
        check(lingering==0,"smoke expires instead of leaving persistent room-space artifacts");
        for(int h=0;h<2;h++)hands.poses[h][3].x=h==0?-.16f:.16f;
        guns.update(hands,effects,runtime,3);guns.feedback(cabinet,3);frame(vp,3.01);
        for(auto& hand:hands.hands)hand.tracked=false;
        guns.update(hands,effects,runtime,3.02);guns.feedback(cabinet,3.02);auto lost=frame(vp,3.03);size_t stale=0;
        for(size_t i=0;i<lost.rgb.size();i+=4)if(lost.rgb[i]||lost.rgb[i+1]||lost.rgb[i+2])++stale;
        check(stale==0,"losing both tracked hands clears stale room-space gun effects");
        auto no_effects=[&](const Frame& f){
            for(size_t i=0;i<f.depth.size();i++)if(f.depth[i]>.9999f&&(f.rgb[i*4]||f.rgb[i*4+1]||f.rgb[i*4+2]))return false;
            return true;
        };
        for(auto& hand:hands.hands)hand.tracked=true;
        guns.update(hands,effects,runtime,3.04);
        check(no_effects(frame(vp,3.04)),"tracking recovery cannot replay the old flash or re-emit its smoke");
        guns.update(hands,effects,runtime,4);guns.feedback(cabinet,4);frame(vp,4.01);
        hands.focused=false;guns.update(hands,effects,runtime,4.02);guns.feedback(cabinet,4.02);frame(vp,4.03);
        hands.focused=true;guns.update(hands,effects,runtime,4.04);
        check(no_effects(frame(vp,4.04)),"focus recovery cannot replay stale accepted-shot effects");
        hands.hands[0].tracked=false;guns.update(hands,effects,runtime,5);cabinet.accepted_shots={1,0};guns.feedback(cabinet,5);frame(vp,5.01);
        hands.hands[0].tracked=true;guns.update(hands,effects,runtime,5.02);
        check(no_effects(frame(vp,5.02)),"a late shot for one untracked owning hand cannot replay on recovery");
        runtime.state=game;runtime.state.service=true;runtime.state.ready=runtime.state.playing=false;
        guns.update(hands,effects,runtime,6);auto service_before=frame(vp,6.045);
        guns.feedback(cabinet,6);auto service_after=frame(vp,6.045);
        check(service_before.rgb==service_after.rgb,"a late accepted-shot event cannot recoil or flash after native Service acknowledgement");
    }
    {
        Presentation sights;PresentationInput hands;Settings options;options.laser=false;
        GameState game;game.valid=game.ready=game.playing=true;game.players[0].state=1;game.players[0].health=4;runtime.state=game;
        CabinetInput cabinet;cabinet.hand={0,-1};cabinet.cabinet.coins=1;sights.feedback(cabinet,0);
        hands.hands[0].tracked=true;hands.poses[0]=glm::translate(glm::mat4(1),glm::vec3(-1,0,-.6));hands.aim_targets[0]=Vec3{0,0,-1};
        auto native=std::make_shared<NativeReticle>(NativeReticle{{-.8f,-.8f,0xff00ff00},{.8f,-.8f,0xff00ff00},{0,.8f,0xff00ff00}});
        sights.set_native_reticle(native);sights.update(hands,options,runtime,10);
        auto centroid=[&](float eye){
            glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            sights.draw(vp*glm::translate(glm::mat4(1),glm::vec3(-eye,0,0)),options,game,true,10);
            std::vector<uint8_t> pixels(256*256*4);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            double x=0;int count=0;for(int y=0;y<256;y++)for(int i=0;i<256;i++){auto at=(y*256+i)*4;if(pixels[at+1]>240&&pixels[at]<10&&pixels[at+2]<10){x+=i;count++;}}
            check(count>30,"live native reticle mesh renders at the actual 3D aim target");return x/count;
        };
        double left=centroid(-.032),right=centroid(.032);check(left-right>10&&left-right<14,"reticle has binocular disparity at the real one-metre hit depth");
        hands.poses[0][3].x=1;sights.update(hands,options,runtime,10.1);
        check(std::abs(centroid(-.032)-left)<.1,"moving the gun cannot drag a world reticle away from its unchanged hit");
        auto green_count=[&](float depth,float minimum,float shrink){
            options.reticle_min_size=minimum;options.reticle_shrink_distance=shrink;hands.aim_targets[0]=Vec3{0,0,-depth};sights.update(hands,options,runtime,10.2);
            glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sights.draw(vp,options,game,true,10.2);
            std::vector<uint8_t> image(256*256*4);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,image.data());int count=0;
            for(size_t i=0;i<image.size();i+=4)if(image[i+1]>240&&image[i]<10&&image[i+2]<10)count++;return count;
        };
        int distant=green_count(20,.25f,1.7f),farther=green_count(40,.25f,1.7f);
        check(distant>0&&std::abs(distant-farther)<=2,"minimum perceived size prevents the native reticle vanishing at distance");
        check(green_count(20,.5f,1.7f)>distant*2,"minimum size setting changes the actual rendered footprint");
        check(green_count(20,.25f,20)>distant*2,"larger shrink distance keeps the rendered sight larger");
        hands.aim_targets[0]=Vec3{0,0,-1};options.reticle_min_size=.25f;options.reticle_shrink_distance=1.7f;
        sights.set_native_reticle({});
        auto marker_pixels=[&](ReticleMode mode){
            options.reticle_mode=mode;sights.update(hands,options,runtime,11);
            glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            sights.draw(vp,options,game,true,11);std::vector<uint8_t> image(256*256*4);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,image.data());return image;
        };
        auto custom=marker_pixels(ReticleMode::Custom);size_t red=0;
        options.hud_size=2;check(marker_pixels(ReticleMode::Custom)==custom,"HUD size cannot enlarge the separate Custom world reticle");
        options.hud_size=.5f;check(marker_pixels(ReticleMode::Custom)==custom,"HUD size cannot shrink the separate Custom world reticle");options.hud_size=1;
        for(int y=80;y<176;y++)for(int x=80;x<176;x++){auto at=(y*256+x)*4;if(custom[at]>240&&custom[at+1]<40&&custom[at+2]<40)red++;}
        check(red>60,"custom P1 hollow X renders independently of laser and native artwork availability");
        for(int y=125;y<131;y++)for(int x=125;x<131;x++)check(custom[(y*256+x)*4]==0,"custom reticle center stays empty without an endpoint dot");
        auto none=marker_pixels(ReticleMode::None);size_t lit=0;
        for(int y=80;y<176;y++)for(int x=80;x<176;x++){auto at=(y*256+x)*4;if(none[at]||none[at+1]||none[at+2])lit++;}
        check(lit==0,"None draws no gameplay marker or auxiliary dot");
        // Native reticle color belongs to the logical player, across both hand
        // assignments; source artwork is shared but P2 must not inherit P1 green.
        options.reticle_mode=ReticleMode::Ingame;options.reticle_min_size=1;
        for(uint32_t tint:{0xff00ff00u,0xffff0000u}){
        auto source=std::make_shared<NativeReticle>(*native);for(auto& v:*source)v.color=tint;sights.set_native_reticle(source);
        for(int seat=0;seat<3;seat++)for(int hand=0;hand<2;hand++){
            const int player=seat%2; // Return to P1 after P2 to catch shader-state leakage.
            for(auto& p:game.players){p.state=0;p.health=0;}game.players[player].state=1;game.players[player].health=4;runtime.state=game;
            hands.hands={};hands.hands[hand].tracked=true;hands.poses[hand]=glm::translate(glm::mat4(1),glm::vec3(1,0,-.6));hands.aim_targets[hand]=Vec3{0,0,-1.7};
            cabinet.hand={-1,-1};cabinet.hand[player]=hand;sights.feedback(cabinet,12);sights.update(hands,options,runtime,12);
            glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sights.draw(vp,options,game,true,12);
            std::vector<uint8_t> image(256*256*4);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,image.data());int green=0,red=0,min_x=256,max_x=-1;
            for(int y=90;y<166;y++)for(int x=90;x<166;x++){auto at=(y*256+x)*4;bool g=image[at+1]>240&&image[at]<10&&image[at+2]<10;bool r=image[at]>240&&image[at+1]<10&&image[at+2]<10;green+=g;red+=r;if(g||r){min_x=std::min(min_x,x);max_x=std::max(max_x,x);}}
            check(player==0?green>30&&red==0:red>30&&green==0,"native reticle is P1 green / P2 red for either physical hand");
            check(max_x-min_x+1>=21&&max_x-min_x+1<=24,"100-percent native reticle has the requested 30-percent larger base footprint");
        }
        }
    }
    {
        // Drive the actual rows and save/reset paths, including the shifted
        // actions after HUD size. Stick movement alone must not confirm Reset.
        Presentation options_menu;Settings chosen;SharedRuntime own;PresentationInput controls;double now=20;
        auto step=[&](glm::vec2 stick){controls.stick=stick;now+=.5;options_menu.update(controls,chosen,own,now);};
        step({0,0});controls.toggle=true;step({0,0});controls.toggle=false;step({0,0});
        for(int row=0;row<14;row++){step({0,-1});step({0,0});}
        step({1,0});step({0,0});check(std::abs(chosen.hud_size-1.25f)<.0001f,"HUD size row changes only its saved percentage in5% steps");
        for(int i=0;i<50;i++){step({1,0});step({0,0});}check(chosen.hud_size==2,"HUD size clamps at200%");
        for(int i=0;i<50;i++){step({-1,0});step({0,0});}check(chosen.hud_size==.5f,"HUD size clamps at50%");
        chosen.hud_size=1.75f;chosen.wider_view=false;
        for(int row=14;row<20;row++){step({0,-1});step({0,0});}
        step({1,0});step({0,0});check(chosen.hud_size==1.75f&&!chosen.wider_view,"horizontal navigation cannot confirm Reset to defaults");
        controls.hands[0].start=true;step({0,0});controls.hands[0].start=false;step({0,0});
        check(chosen.hud_size==1.2f&&chosen.wider_view&&chosen.reticle_min_size==.35f&&chosen.reticle_max_size==1.45f&&chosen.reticle_shrink_distance==10.f&&options_menu.open(),"Reset restores captured BEL reticle and HUD defaults while keeping menu open");
        if(const char* path=std::getenv("BEL_SETTINGS")){std::ifstream saved(path);check(saved.good()&&read_settings(saved).hud_size==1.2f,"Reset HUD default survives production settings reload");}
    }
    check(glGetError()==GL_NO_ERROR,"presentation GL operations complete without errors");
    glDeleteRenderbuffers(1,&depth);
    glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&image);
    std::cout<<"Loading menu remains visible after the shared timed splash\n";
}
