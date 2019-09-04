//
// Created by Constantin on 1/14/2019.
//

#ifndef FPV_VR_ATEXTELEMENT_H
#define FPV_VR_ATEXTELEMENT_H


#include <General/IDrawable.hpp>
#include <General/IPositionable.hpp>
#include <General/IUpdateable.hpp>
#include <General/ITextHeight.h>
#include <OSD/ElementBatching/OSDTextObj.hpp>


class ATextElement : public IDrawable,public IPositionable,public IUpdateable, public ITextHeight{
public:
    ATextElement();
public:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    virtual void updateSubElement(unsigned long id,OSDTextObj* obj)const=0;
    const int N_TEXT_OBJ=25;
    std::array<OSDTextObj*,N_TEXT_OBJ> createAllElements(BatchingManager &batchingManager);
};


#endif //FPV_VR_ATEXTELEMENT_H
