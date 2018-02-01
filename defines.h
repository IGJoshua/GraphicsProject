#pragma once

#define internal_function static
#define global_variable static
#define persistent_variable static

#define SAFE_RELEASE(resource) if((resource)) { (resource)->Release(); (resource) = NULL; }
