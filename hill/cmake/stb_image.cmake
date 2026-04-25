add_library(stb_image STATIC "extern/stb_image/stb_image.c" "extern/stb_image/stb_image.h")
target_include_directories(stb_image PUBLIC "extern/stb_image")
