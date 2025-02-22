# This script can run the moss cmake installation targets to simplify the
# installation to just one command.
# It can be also used to run tests.

TARGET=
BUILD_DIR=build

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            printf "Moss installation script.\nUsage: bash $0\n" 
            exit 256
            ;;
        release|debug|all|tests)
            [ -z "${TARGET}" ] || {
                echo "Only one install command can be specified"
                exit 1
            }
            TARGET="$1"
            shift
            ;;
        *)
            echo "ERROR: Unknown argument: '$1'"
            exit 1
            #POSITIONAL_ARGS+=("$1")
            #shift
            ;;
    esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

# Use default target if not set
[ -z "${TARGET}" ] && TARGET="release"
echo "Running installation target: ${TARGET}"

if [ "${TARGET}" = "release" ]; then
    # Default builds moss, moss library and installs it as a command.
    # This requires sudo privileges.
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target moss
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) --target libms installation
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
    sudo -u $SUDO_USER bash tests/run-tests.sh -test-dir tests/ || exit 1
elif [ "${TARGET}" = "all" ]; then
    sudo -u $SUDO_USER cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug || exit 1
    sudo -u $SUDO_USER cmake --build $BUILD_DIR -j $(nproc) || exit 1
    cmake --build $BUILD_DIR -j $(nproc) --target libms installation
fi