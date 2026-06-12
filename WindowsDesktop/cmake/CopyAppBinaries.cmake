# Post-build helper: stage the executable and its vcpkg-deployed runtime DLLs
# into the App/ folder. The engine chdirs to the executable's directory at
# startup (SetWorkingDirectory in Siv3DMainHelper.cpp), so the runnable copy
# must live next to engine/ and example/. file(COPY) skips up-to-date files.
file(GLOB SIV3D_APP_DLLS "${SRC_DIR}/*.dll")
file(COPY ${EXE} ${SIV3D_APP_DLLS} DESTINATION "${DST_DIR}")
