#include "../include/default.h"
#include "vk.h"

// macro to ensure identical structure of init functions
#define INIT_FNC(a) i32 vk_init_##a(struct vk_##a##_cfg*, struct vk_##a*);

INIT_FNC(inst);
INIT_FNC(dvc);