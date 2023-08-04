vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH
  SOURCE_PATH
  REPO
  Donald-Rupin/mpsc_zib
  REF
  451328815a5cc3977750d61e1b7cd85ffd7108ae
  SHA512
  ad0e6469bdf84d8ac04e570b6344394979200527ab5172d887b67df6a67162d890ad3d7cabed3a5304bbd2375cc6ab45cb81fd0e538396ab35abedd29730287e
  HEAD_REF
  main)

# Copy header files
file(
        INSTALL ${SOURCE_PATH}/includes
        DESTINATION ${CURRENT_PACKAGES_DIR}
        FILES_MATCHING
        PATTERN "*.hpp")

file(RENAME ${CURRENT_PACKAGES_DIR}/includes ${CURRENT_PACKAGES_DIR}/include)
