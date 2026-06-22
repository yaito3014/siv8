# Post-build: stage the exe + its vcpkg runtime DLLs next to engine/ + example/
# (the engine chdirs to the exe's dir at startup). file(COPY) skips up-to-date.
file(GLOB SIV3D_APP_DLLS "${SRC_DIR}/*.dll")
file(COPY ${EXE} ${SIV3D_APP_DLLS} DESTINATION "${DST_DIR}")
