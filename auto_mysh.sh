make clean
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
make mysh >> log.txt
echo "If nothing prints out after this line, then you pass the tests. Otherwise check log.txt."
