if(MSVC)
    include(CMakeMacroAddFlags)
    # systemintern functions for faster code; Optimize whole program
	add_flags(CMAKE_CXX_FLAGS_RELEASE /Oi /GL)
	# Strip unused symbols and us COMDAT folding
	add_flags(CMAKE_EXE_LINKER_FLAGS_RELEASE /OPT:REF /OPT:ICF)
	add_flags(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL /OPT:REF /OPT:ICF)
	add_flags(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO /OPT:REF /OPT:ICF) 
else()
    include(CheckAndAddFlag)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
        include(ArmConfig)
        set(RTTR_TARGET_BOARD "OFF" CACHE STRING "Set to your target device to enable extra optimzation flags. Available: ${RTTR_AVAILABLE_ARM_CFGS}")
        set_property(CACHE RTTR_TARGET_BOARD PROPERTY STRINGS ${RTTR_AVAILABLE_ARM_CFGS})
        if(RTTR_TARGET_BOARD)
            get_arm_tune(arm_tune_default "${RTTR_TARGET_BOARD}")
            set(RTTR_OPTIMZATION_TUNE ${arm_tune_default} CACHE STRING "" FORCE)
            if(NOT RTTR_TARGET_BOARD STREQUAL "Generic")
                get_arm_flags(arm_flags "${RTTR_TARGET_BOARD}")
                CheckAndAddFlags(${arm_flags})
            endif()
        endif()
    else()
        set_if_unset(RTTR_OPTIMZATION_VECTOR_EXT_DEFAULT SSE)
        set(RTTR_OPTIMZATION_VECTOR_EXT ${RTTR_OPTIMZATION_VECTOR_EXT_DEFAULT} CACHE STRING "Vector extension to use")
        # Set allowed options
        set_property(CACHE RTTR_OPTIMZATION_VECTOR_EXT PROPERTY STRINGS MMX SSE SSE2)
        if(RTTR_OPTIMZATION_VECTOR_EXT MATCHES "SSE|MMX")
            CheckAndAddFlag(-mmmx)
        endif()
        if(RTTR_OPTIMZATION_VECTOR_EXT MATCHES "SSE")
            CheckAndAddFlags(-msse -mfpmath=sse)
        endif()
        if(RTTR_OPTIMZATION_VECTOR_EXT MATCHES "SSE2")
            CheckAndAddFlags(-msse2)
        endif()
    endif()

    set(RTTR_OPTIMZATION_TUNE "${RTTR_OPTIMZATION_TUNE_DEFAULT}" CACHE STRING "Target architecture to tune for (e.g. core2, prescott)")
    
    CheckAndAddFlags(-O2 -ffast-math -fomit-frame-pointer -mtune=generic)
    
    if(RTTR_OPTIMZATION_TUNE)
        # Adding multiple mtune flags just uses the last one, so adding mtune=generic above is ok
        CheckAndAddFlags(-mtune=${RTTR_OPTIMZATION_TUNE})
    endif()
    
    if(APPLE)
        # ppc only?
        IF(CMAKE_OSX_ARCHITECTURES MATCHES "^ppc[^;]*$")
            CheckAndAddFlags(-faltivec -maltivec) # Clang and GCC version
        ENDIF()
    endif()
endif()
