#!/bin/bash
# This script can run the moss cmake installation targets to simplify the
# installation to just one command.
# It can be also used to run tests.

TARGET=
BUILD_DIR=build

case $1 in
    -h|--help)
        printf "Moss installation script.\nUsage: bash $0\n" 
        exit 256
        ;;
    release|debug|all|tests|docs|package)
        [ -z "${TARGET}" ] || {
            echo "Only one install command can be specified"
            exit 1
        }
        TARGET="$1"
        ;;
esac

# Use default target if not set
[ -z "${TARGET}" ] && TARGET="release"
echo "Running installation target: ${TARGET}"

# Regenerates passed in .ms notebook file if it has changed to its git version.
regenerate_docs() {
    local file="$1"
    local out_path="$2"
    local base_name
    base_name="${file%.ms}"

    if ! git diff --quiet HEAD -- "$file"; then
        echo "[CHANGED] $file has changed, regenerating..."
        moss -f md -O ${base_name}.md $file || exit 1
        echo "[GENERATED] ${base_name}.md."
    else
        echo "[SKIPPED] $file has not changed, skipping."
    fi
}

if [ "${TARGET}" = "release" ]; then
    # Default builds moss, moss library and installs it as a command.
    # This requires sudo privileges.
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target moss
    cmake --build $BUILD_DIR -j $(nproc) --target libms installation || exit 1
elif [ "${TARGET}" = "debug" ]; then
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target moss
    cmake --build $BUILD_DIR -j $(nproc) --target libms installation
elif [ "${TARGET}" = "tests" ]; then
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target moss testsmoss || exit 1
    cmake --build $BUILD_DIR -j $(nproc) --target libms installation || exit 1
    # Run tests
    echo "Running unit tests"
    sudo -u $SUDO_USER ./$BUILD_DIR/testsmoss || exit 1
    echo "Running moss tests"
    sudo -u $SUDO_USER moss tests/run-tests.ms -test-dir tests/ || exit 1
elif [ "${TARGET}" = "all" ]; then
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) || exit 1
    cmake --build $BUILD_DIR -j $(nproc) --target libms installation
elif [ "${TARGET}" == "docs" ]; then
    echo "Regenerating docs..."
    # Recursively loop over all .ms files in docs/
    find docs/language-reference -type f -name "*.ms" | while read -r f; do
        regenerate_docs "$f"
    done
    regenerate_docs docs/readme.ms
    mv docs/readme.md ./ 2>/dev/null
    echo "Done regenerating docs."
elif [ "${TARGET}" == "package" ]; then
    if ! [[ $2 = "linux" || $2 = "darwin" ]]; then
        echo "Expected platform as the second argument ([linux/darwin])."
        exit 1
    fi
    platform_name=$2
    if [ $2 = "darwin" ]; then
        platform_name="darwin-macos"
    fi

    echo "Creating general release build package..."
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target moss
    version=$(./$BUILD_DIR/moss --version | grep -oP '(?<=moss )\d+\.\d+\.\d+')

    echo "Compiling moss libraries..."
    output_name=moss-${version}-release-${platform_name}_x86
    release_dir=$BUILD_DIR/$output_name
    
    mkdir -p $release_dir
    # Copy all needed
    cp $BUILD_DIR/moss $release_dir
    cp LICENSE $release_dir
    ./$BUILD_DIR/moss -f md -o $release_dir/readme.md docs/releases/release_readme.ms $2
    cp docs/releases/release_install.sh $release_dir/install.sh
    cp stdlib/mossy.css $release_dir
    cp stdlib/waters.css.LICENSE $release_dir
    # Compile all stdlib files
    for file in stdlib/*.ms; do
        file_base=$(basename "$file" .ms)
        ./$BUILD_DIR/moss -W all -X2 --compile-only -O "$release_dir/$file_base.msb" "$file"
    done
    # Change permissions on release folder to not be sudo
    sudo chown -R "$SUDO_USER:$SUDO_USER" $release_dir
    echo "Creating archive..."
    sudo -u $SUDO_USER tar -czf "$output_name.tar.gz" -C "$release_dir" .
    echo "Created package: ${output_name}.tar.gz"
fi