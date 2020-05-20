//
// Created by Constantin on 1/14/2019.
//

#include "ATextElement.h"


ATextElement::ATextElement():
        IUpdateable(TAG),IDrawable(TAG)
{

}

void ATextElement::setupPosition() {

}


void ATextElement::updateGL() {

}

void ATextElement::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {

}

std::array<OSDTextObj *, ATextElement::N_TEXT_OBJ>
ATextElement::createAllElements(BatchingManager &batchingManager) {
    return std::array<OSDTextObj *, N_TEXT_OBJ>();
}
