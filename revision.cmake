execute_process(COMMAND /bin/sh -c "git log --oneline | head -1 | cut -d ' ' -f1" OUTPUT_VARIABLE GIT_REVISION)
message (STATUS "Builing revision: ${GIT_REVISION}")
file(WRITE revision.h "#define GIT_REVISION ${GIT_REVISION}\n")