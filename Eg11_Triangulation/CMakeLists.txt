project(Eg11_Triangulation)

# build the target
aux_source_directory(./ DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})
target_link_libraries(${PROJECT_NAME} ${THIRD_LIBS})
