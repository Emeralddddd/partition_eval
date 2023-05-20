install(
    TARGETS eval_exe
    RUNTIME COMPONENT eval_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
