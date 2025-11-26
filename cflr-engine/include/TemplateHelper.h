//
// Created by enochii on 2025/7/22.
//

#ifndef POCR_SVF_TEMPLATEHELPER_H
#define POCR_SVF_TEMPLATEHELPER_H

#define INSTANTIATE_TEMPLATE(ClassTemplate) \
template class ClassTemplate<128>;\
template class ClassTemplate<256>;\
template class ClassTemplate<512>;\
template class ClassTemplate<1024>;\
template class ClassTemplate<2048>;\
template class ClassTemplate<4096>;\
template class ClassTemplate<8192>;

#define CHOOSE_CFL_ALGO(ClassTemplate, bv_size, cfg, filename) \
do {                                    \
    if (bv_size == 128) ClassTemplate<128>(cfg, filename).analyze(); \
    else if (bv_size == 256) ClassTemplate<256>(cfg, filename).analyze(); \
    else if (bv_size == 512) ClassTemplate<512>(cfg, filename).analyze(); \
    else if (bv_size == 1024) ClassTemplate<1024>(cfg, filename).analyze(); \
    else if (bv_size == 2048) ClassTemplate<2048>(cfg, filename).analyze(); \
    else if (bv_size == 4096) ClassTemplate<4096>(cfg, filename).analyze(); \
    else if (bv_size == 8192) ClassTemplate<8192>(cfg, filename).analyze(); \
} while (false);

#define CHOOSE_CFL_ALGO_(ClassTemplate, bv_size, cfg, filename, extra) \
do {                                    \
    if (bv_size == 128) ClassTemplate<128>(cfg, filename, extra).analyze(); \
    else if (bv_size == 256) ClassTemplate<256>(cfg, filename, extra).analyze(); \
    else if (bv_size == 512) ClassTemplate<512>(cfg, filename, extra).analyze(); \
    else if (bv_size == 1024) ClassTemplate<1024>(cfg, filename, extra).analyze(); \
    else if (bv_size == 2048) ClassTemplate<2048>(cfg, filename, extra).analyze(); \
    else if (bv_size == 4096) ClassTemplate<4096>(cfg, filename, extra).analyze(); \
    else if (bv_size == 8192) ClassTemplate<8192>(cfg, filename, extra).analyze(); \
} while (false);

#endif //POCR_SVF_TEMPLATEHELPER_H
