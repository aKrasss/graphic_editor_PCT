#!/bin/bash
set -e

FORCE=0
if [[ "$1" == "--force" ]]; then
    FORCE=1
    echo "Принудительная перезагрузка всех ресурсов."
fi

ICON_URLS=(
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/eraser.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/figure.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/fill.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/garbage.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/pen.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/pipette.png?raw=true"
    "https://github.com/aKrasss/graphic_editor_PCT/blob/feature/app-integration/src/Textures/selection.png?raw=true"
)

FONT_URL="https://github.com/liberationfonts/liberation-fonts/files/7261485/LiberationSans-Regular.ttf"
FALLBACK_FONT_URL="https://github.com/matomo-org/travis-scripts/raw/master/fonts/Arial.ttf"

BUILD_DIR="build"
EXECUTABLE="GraphicEditor"

if ! command -v cmake &> /dev/null; then
    echo "CMake не найден. Подгружаю"
    if [[ -f /etc/debian_version ]]; then
        sudo apt update && sudo apt install -y cmake
    elif [[ -f /etc/redhat-release ]]; then
        sudo yum install -y cmake
    else
        echo "Неизвестный дистрибутив, установите CMake вручную"
        exit 1
    fi
fi

if [[ ! -f "$BUILD_DIR/$EXECUTABLE" ]] || [[ $FORCE -eq 1 ]]; then
    echo "Сборка проекта..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
    make -j$(nproc)
    cd ..
fi

download_file() {
    local URL=$1
    local DEST=$2
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "$DEST" "$URL" && echo "  -> загружено: $DEST" || { echo "  -> ошибка: $URL"; return 1; }
    elif command -v curl &> /dev/null; then
        curl -# -o "$DEST" "$URL" && echo "  -> загружено: $DEST" || { echo "  -> ошибка: $URL"; return 1; }
    else
        echo "Ошибка. Загрузите $DEST вручную"
        return 1
    fi
    return 0
}

ensure_directory() {
    mkdir -p "$1"
}

ensure_directory "$BUILD_DIR/Textures"

for URL in "${ICON_URLS[@]}"; do
    RAW_NAME=$(basename "$URL")
    CLEAN_NAME="${RAW_NAME%%\?*}"
    DEST_FILE="$BUILD_DIR/Textures/$CLEAN_NAME"
    NEED_DOWNLOAD=0
    if [[ $FORCE -eq 1 ]]; then
        NEED_DOWNLOAD=1
    elif [[ ! -f "$DEST_FILE" ]]; then
        NEED_DOWNLOAD=1
    elif [[ ! -s "$DEST_FILE" ]]; then
        NEED_DOWNLOAD=1
    fi

    if [[ $NEED_DOWNLOAD -eq 1 ]]; then
        download_file "$URL" "$DEST_FILE"
    fi
done

FONT_FILENAME="arialmt.ttf"
FONT_DEST="$BUILD_DIR/$FONT_FILENAME"
FONT_DEST_FONTS="$BUILD_DIR/fonts/$FONT_FILENAME"
ensure_directory "$BUILD_DIR/fonts"

NEED_FONT=0
if [[ $FORCE -eq 1 ]]; then
    NEED_FONT=1
elif [[ ! -f "$FONT_DEST" ]] && [[ ! -f "$FONT_DEST_FONTS" ]]; then
    NEED_FONT=1
elif [[ -f "$FONT_DEST" ]] && [[ ! -s "$FONT_DEST" ]]; then
    NEED_FONT=1
elif [[ -f "$FONT_DEST_FONTS" ]] && [[ ! -s "$FONT_DEST_FONTS" ]]; then
    NEED_FONT=1
fi

if [[ $NEED_FONT -eq 1 ]]; then
    if [[ -f "$FONT_DEST" ]]; then
        cp "$FONT_DEST" "$FONT_DEST_FONTS"
        cp "$FONT_DEST" "$BUILD_DIR/../" 2>/dev/null || true
    fi
fi

cd "$BUILD_DIR"
echo "Запуск $EXECUTABLE ..."
if [[ -f "./$EXECUTABLE" ]]; then
    "./$EXECUTABLE"
else
    echo "Ошибка. Файл не найден в $BUILD_DIR"
    exit 1
fi