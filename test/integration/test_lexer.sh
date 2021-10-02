TEST_PATH=test/lexer/integration
TMP_BUILD=_build

rm -rf ${TMP_BUILD}
mkdir ${TMP_BUILD}
cmake -S . -B ${TMP_BUILD}
cd ${TMP_BUILD}
make -j ${TMP_BUILD}/Makefile

# for file in ${TEST_PATH}/*.cool; do
  # echo $file
# done
