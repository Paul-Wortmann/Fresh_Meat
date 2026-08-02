
#ifndef INCLUDES_HPP
#define INCLUDES_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cfloat>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

// Windows includes
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif // defined

// Linux includes
#if defined(__linux__)
    #include <dirent.h>
#endif // defined

#include "defines.hpp"
#include "../utils/file_utils.hpp"
#include "../utils/random_utils.hpp"
#include "../utils/xml_utils.hpp"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include <GLFW/glfw3.h>

#endif // INCLUDES_HPP
