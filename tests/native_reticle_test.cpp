#include "../native/native_reticle.h"
#include "../native/automatic_feedback.h"
#include <cassert>
using namespace bel;
int main(){
    NativeFrame f;f.complete=true;
    for(auto [name,size]:{std::pair{"texture0",1048576},{"texture1",1048576},{"palette",16384},{"colorxlat",49152},{"luma",32768},{"gamma",256}})
        f.blobs.push_back({name,std::vector<uint8_t>(size)});
    auto resources=Model2Resources(f);
    NativePolygon p;p.count=3;p.window=4;p.focus={400,400};p.luma=252;
    p.matrix={0,.5,0,-.5,0,0,0,0,.5,3,-2,5};
    // Authored local triangle (.1,0),(.2,0),(.1,.2), rotated and translated.
    p.vertices={NativeVertex{1200,-780,5,0,0},NativeVertex{1200,-760,5,0,0},NativeVertex{1160,-780,5,0,0}};
    f.polygons.push_back(p);
    auto line=p;line.vertices[1].y=3200;f.polygons.push_back(line); // Long native ruler must be removed.
    auto a=native_reticle(f,resources);assert(a.size()==3);
    assert(std::abs(a[0].x-.1)<1e-5&&std::abs(a[0].y)<1e-5);
    assert(std::abs(a[2].x-.1)<1e-5&&std::abs(a[2].y-.2)<1e-5);
    for(auto& q:f.polygons){q.matrix[9]+=2;for(auto& v:q.vertices)v.x+=800;}
    auto b=native_reticle(f,resources);assert(a==b); // Native aim motion cannot move the world marker's local mesh.
    f.polygons[0].matrix.fill(0);assert(native_reticle(f,resources).empty());
    f.polygons.clear();assert(native_reticle(f,resources).empty());
    assert(automatic_recoil(.02)>0&&automatic_recoil(.1)==0);
    assert(automatic_recoil(-.1)==0&&automatic_recoil(NAN)==0);
}
