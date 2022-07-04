
build_type="Debug Release"
standard="11 14 17 20"
colorful="AUTO ON OFF"
thread_safe="ON OFF"
auto_init="ON OFF"


SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

function gen_folder {
    echo "cpp$1-$2-color$3-thread$4-init$5"
}

function to-abs-path {
    local target="$1"

    if [ "$target" == "." ]; then
        echo "$(pwd)"
    elif [ "$target" == ".." ]; then
        echo "$(dirname "$(pwd)")"
    else
        echo "$(cd "$(dirname "$1")"; pwd)/$(basename "$1")"
    fi
}

dir=$(to-abs-path "$2")

for s in $standard; do
    for b in $build_type; do
        for c in $colorful; do
            for t in $thread_safe; do
                for a in $auto_init; do
                    folder=$(gen_folder $s $b $c $t $a)
                    echo "build folder: $dir/build/$folder"
                    if [[ $1 == "config" ]]; then
                        echo "source folder: $SCRIPTPATH/.."
                        cmake -B $dir/build/$folder -S $SCRIPTPATH/.. -DCMAKE_BUILD_TYPE=$b -DCMAKE_CXX_STANDARD=$s -DCOLORFUL_OUTPUT=$c -DENABLE_THREAD_SAFE=$t -DENABLE_AUTO_INIT=$a
                    elif [[ $1 == "build" ]]; then
                        cmake --build $dir/build/$folder --config $b -j $(nproc)
                    elif [[ $1 == "test" ]]; then
                        cd $dir/build/$folder && ctest --verbose -C $b 
                    fi
                done
            done
        done
    done
done