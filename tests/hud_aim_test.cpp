#include "../native/model2_scene.h"
#include "../native/model2_hud.h"
#include <cassert>
#include <cmath>
using namespace bel;
static NativePolygon quad(float depth,unsigned window,float focus=400) {
    NativePolygon p;p.count=4;p.window=window;p.focus={focus,focus};
    p.center={248,320};p.viewport={0,128,497,512};p.luma=252;
    p.vertices={NativeVertex{-148*depth,92*depth,depth,0,0},
        NativeVertex{148*depth,92*depth,depth,0,0},
        NativeVertex{148*depth,-92*depth,depth,0,0},
        NativeVertex{-148*depth,-92*depth,depth,0,0}};
    return p;
}
int main() {
    NativeFrame f;f.complete=true;f.width=496;f.height=384;f.yoffset=128;
    f.lens={496,384,400,400,248,192};
    for(auto [name,size]:{std::pair{"texture0",1048576}, {"texture1",1048576},
        {"palette",16384},{"colorxlat",49152},{"luma",32768},{"gamma",256}})
        f.blobs.push_back({name,std::vector<uint8_t>(size)});
    f.polygons={quad(1,4),quad(10,2),quad(20,2)};
    Model2Resources resources(f);assert(resources.valid());
    auto triangles=scene_triangles(f,true);
    Ray center{{0,0,0},{0,0,-1}};
    auto old=trace_prepared_scene_aim(center,f,triangles,&resources,1,1.8,f.lens);
    assert(old.target&&std::abs(old.target->z+1)<1e-6);
    auto hit=trace_prepared_scene_aim(center,f,triangles,&resources,1,1.8,f.lens,false,true);
    assert(hit.target&&std::abs(hit.target->z+10)<1e-6); // Nearest world, not overlay or later far world.
    auto miss=trace_prepared_scene_aim({{0,0,0},{1,0,-1}},f,triangles,&resources,1,1.8,f.lens,false,true);
    assert(miss.target&&std::sqrt(dot(*miss.target,*miss.target))>40); // No flat panel stop on a world miss.
    auto minimal=scene_triangles(f,true,false,true);
    for(auto& t:minimal)assert(f.polygons[t.polygon].window!=4);
    assert(minimal.size()==4);
    // Preserve the genuine health/fuel/ammo pixels; remove frame, score and titles.
    assert(essential_hud_pixel(30,180,496,384));
    assert(essential_hud_pixel(450,180,496,384));
    assert(essential_hud_pixel(250,10,496,384));
    assert(essential_hud_pixel(35,15,496,384)&&essential_hud_pixel(460,15,496,384)); // Native Missile availability.
    assert(essential_hud_pixel(250,350,496,384));
    assert(essential_hud_pixel(167,360,496,384));
    assert(essential_hud_pixel(326,360,496,384));
    assert(essential_hud_pixel(200,302,496,384)); // Warning glyphs above the ammo gauge.
    assert(essential_hud_pixel(290,320,496,384));
    auto regions=essential_hud_regions(496,384);
    for(size_t i=0;i<regions.size();i++)for(size_t j=i+1;j<regions.size();j++){
        auto a=regions[i],b=regions[j];
        assert(std::min(a.right,b.right)<=std::max(a.left,b.left)||std::min(a.bottom,b.bottom)<=std::max(a.top,b.top));
    } // Disjoint regions avoid submitting the same native HUD geometry twice.
    // Native ammo grows outwards from the red inner bars as capacity changes.
    // A label-only crop loses the yellow/green sections on both outer ends.
    for(int y=338;y<366;y++)for(int x=48;x<448;x++)assert(essential_hud_pixel(x,y,496,384));
    assert(!essential_hud_pixel(4,180,496,384));
    assert(!essential_hud_pixel(120,30,496,384));
    assert(!essential_hud_pixel(250,200,496,384));
    auto pixels=essential_hud_raster(std::vector<uint32_t>(496*384,0xff123456),496,384);
    assert(pixels[180*496+30]==0xff123456&&pixels[200*496+250]==0&&pixels[180*496+4]==0);
    f.polygons={quad(1,4,330)};
    f.polygons[0].vertices={NativeVertex{-248,192,1,0,0},NativeVertex{248,192,1,0,0},
        NativeVertex{248,-192,1,0,0},NativeVertex{-248,-192,1,0,0}};
    auto gauges=scene_triangles(f,true,false,true);
    assert(!gauges.empty());for(auto& t:gauges)assert(t.panel);
}
