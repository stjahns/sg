#pragma once

#include <memory>

#define BUFFER_OFFSET(offset) ((void *)(offset))

typedef uint16_t u16;
typedef uint32_t u32;

template <typename T> using shared_ptr = std::shared_ptr<T>;

#define V4_RED 		1.0f, 0.0f, 0.0f, 1.0f
#define V4_GREEN 	0.0f, 1.0f, 0.0f, 1.0f
#define V4_BLUE 	0.0f, 0.0f, 1.0f, 1.0f
#define V4_YELLOW 	1.0f, 1.0f, 0.0f, 1.0f
#define V4_WHITE 	1.0f, 1.0f, 1.0f, 1.0f

#define VEC3_ARGS(v) (v.x), (v.y), (v.z)
