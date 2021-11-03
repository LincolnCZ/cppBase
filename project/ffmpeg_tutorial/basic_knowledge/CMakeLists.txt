aux_source_directory(. BASIC_SRC)
#set(BASIC_SRC
#        simplest_mediadata_main.cpp
#        simplest_mediadata_raw.cpp
#        simplest_mediadata_h264.cpp
#        simplest_mediadata_aac.cpp
#        simplest_mediadata_flv.cpp
#        )

add_executable(basic_test_d ${BASIC_SRC})