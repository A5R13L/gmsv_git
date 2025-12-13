if [ -n "$1" ]; then
    PRESET="$1"
else
    ARCH=$(uname -m)

    if [ "$ARCH" = "x86_64" ]; then
        PRESET="x86_64"
    else
        PRESET="x86"
    fi
fi

if [ -n "$2" ]; then
    TAG_VERSION="$2"
else
    TAG_VERSION="unknown"
fi

if [ "$PRESET" = "x86_64" ]; then
    CONFIG="releasewithsymbols_x86_64"
    CXXFLAGS="-std=c++17 -m64"
    BUILD_PATH="projects/x64/linux/gmake2"
    LIB_PATH="source/thirdparty/libgit2/libs/x64"
    ZLIB_BUILD="build64"
    OPENSSL_DIST="dist64"
    ARCH_FLAG="-m64"
else
    CONFIG="releasewithsymbols_x86"
    CXXFLAGS="-std=c++17 -m32"
    BUILD_PATH="projects/x32/linux/gmake2"
    LIB_PATH="source/thirdparty/libgit2/libs/x32"
    ZLIB_BUILD="build32"
    OPENSSL_DIST="dist32"
    ARCH_FLAG="-m32"
fi

apt-get update
dpkg --add-architecture i386
apt-get update

apt-get install -y \
  build-essential \
  ninja-build \
  zip \
  unzip \
  tar \
  pkg-config \
  wget \
  gcc-multilib \
  g++-multilib \
  cmake \
  make \
  curl \
  git

wget https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz
tar xf premake-5.0.0-beta2-linux.tar.gz
mv premake5 /usr/local/bin/

wget https://zlib.net/zlib-1.3.1.tar.gz
tar xf zlib-1.3.1.tar.gz
cd zlib-1.3.1
mkdir $ZLIB_BUILD && cd $ZLIB_BUILD
CFLAGS="$ARCH_FLAG -fPIC" ../configure --static
make -j$(nproc)
cd ../..

wget https://www.openssl.org/source/openssl-1.1.1w.tar.gz
tar xf openssl-1.1.1w.tar.gz
cd openssl-1.1.1w

if [ "$PRESET" = "x86_64" ]; then
    ./Configure linux-generic64 no-shared no-tests no-threads --prefix=$PWD/$OPENSSL_DIST CFLAGS="$ARCH_FLAG -fPIC" LDFLAGS="$ARCH_FLAG -fPIC"
else
    ./Configure linux-generic32 no-shared no-tests no-threads --prefix=$PWD/$OPENSSL_DIST CFLAGS="$ARCH_FLAG -fPIC" LDFLAGS="$ARCH_FLAG -fPIC"
fi

make -j$(nproc)
make install_sw
cd ..

mkdir -p $LIB_PATH
cp openssl-1.1.1w/$OPENSSL_DIST/lib/lib*.a $LIB_PATH
cp zlib-1.3.1/$ZLIB_BUILD/libz.a $LIB_PATH

git clone --branch v1.8.0 https://github.com/libgit2/libgit2.git libgit2_src
cd libgit2_src
mkdir $ZLIB_BUILD && cd $ZLIB_BUILD

cmake .. -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF -DBUILD_CLAR=OFF -DBUILD_CLI=OFF -DCMAKE_C_FLAGS="$ARCH_FLAG -fPIC" -DCMAKE_CXX_FLAGS="$ARCH_FLAG -fPIC" -DZLIB_LIBRARY=$PWD/../../zlib-1.3.1/$ZLIB_BUILD/libz.a -DZLIB_INCLUDE_DIR=$PWD/../../zlib-1.3.1 -DOPENSSL_SSL_LIBRARY=$PWD/../../openssl-1.1.1w/$OPENSSL_DIST/lib/libssl.a -DOPENSSL_CRYPTO_LIBRARY=$PWD/../../openssl-1.1.1w/$OPENSSL_DIST/lib/libcrypto.a -DOPENSSL_INCLUDE_DIR=$PWD/../../openssl-1.1.1w/include

make -j$(nproc)
cd ../..

cp libgit2_src/$ZLIB_BUILD/libgit2.a $LIB_PATH

premake5 gmake2 --tag_version=$TAG_VERSION
cd $BUILD_PATH
make config=$CONFIG CXX="g++-10 $ARCH_FLAG" CXXFLAGS="$CXXFLAGS"