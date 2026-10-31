// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "native_reticle.h"
#include "shared/ui_compat.h"
#include <glm/glm.hpp>
namespace bel {
class ReticleRenderer {
    GLuint program_{},vao_{},buffer_{};NativeReticle uploaded_;
public:
    ~ReticleRenderer(){if(program_)glDeleteProgram(program_);if(vao_)glDeleteVertexArrays(1,&vao_);if(buffer_)glDeleteBuffers(1,&buffer_);}
    // 0 preserves supplied colors (Custom); 1/2 select native P1 green/P2 red.
    bool draw(const NativeReticle& source,const glm::mat4& mvp,int native_player=0){
        if(source.empty())return false;
        if(!program_){
            OpenGlSource vs,fs;
            vs.addSource("in vec2 in_pos;in vec4 in_color;uniform mat4 mvp;out vec4 color;void main(){gl_Position=mvp*vec4(in_pos,0,1);color=in_color;}");
            fs.addSource("in vec4 color;uniform int native_player;out vec4 result;void main(){result=color;if(native_player>0){float hi=max(color.r,color.g),lo=min(color.r,color.g);result=native_player==1?vec4(lo,hi,color.b,color.a):vec4(hi,lo,color.b,color.a);}}");
            program_=gl_CompileAndLink(vs.generate().c_str(),fs.generate().c_str());if(!program_)return false;
            glGenVertexArrays(1,&vao_);glGenBuffers(1,&buffer_);
        }
        glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,buffer_);
        if(uploaded_!=source){
            uploaded_=source;std::vector<float> vertices;vertices.reserve(source.size()*6);
            for(const auto& v:source)for(float f:{v.x,v.y,float((v.color>>16)&255)/255,float((v.color>>8)&255)/255,float(v.color&255)/255,1.f})vertices.push_back(f);
            glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STREAM_DRAW);
            glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);
            glEnableVertexAttribArray(3);glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));
        }
        glUseProgram(program_);glUniformMatrix4fv(glGetUniformLocation(program_,"mvp"),1,GL_FALSE,&mvp[0][0]);
        glUniform1i(glGetUniformLocation(program_,"native_player"),native_player);
        glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(GL_FALSE);glDisable(GL_CULL_FACE);glDisable(GL_BLEND);
        glDrawArrays(GL_TRIANGLES,0,GLsizei(source.size()));glDepthMask(GL_TRUE);glDepthFunc(GL_LESS);return true;
    }
};
}
