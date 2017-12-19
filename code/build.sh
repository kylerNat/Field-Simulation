mkdir "../build"
pushd "../build"

rm "field_simulation.exe"
clang -g -o "field_simulation.exe" -D DEBUG ../code/"main.cpp"

./"field_simulation.exe"
popd