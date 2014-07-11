set(HASHTABLE_SRCS hashtable.c hashtable_itr.c)
set(TDFS_SRCS unionfs.c stats.c opts.c debug.c findbranch.c readdir.c general.c unlink.c cow.c cow_utils.c string.c rmdir.c)

add_executable(tdfs ${TDFS_SRCS} ${HASHTABLE_SRCS})
target_link_libraries(tdfs fuse pthread m rt)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/tdfs DESTINATION bin)
