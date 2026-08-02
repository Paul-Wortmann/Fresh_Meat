
#ifndef MODEL_OBJ_HPP
#define MODEL_OBJ_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include "model_define.hpp"

sModelData* gLoadOBJ(const std::string &_fileName);
void        gSaveOBJ(sModelData *&_model, const std::string &_fileName);
void        gFreeOBJ(sModelData *&_model);

#endif // MODEL_OBJ_HPP

