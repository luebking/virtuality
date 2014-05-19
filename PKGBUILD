# Maintainer : Cazimi
pkgbase=virtuality-git
pkgname=('virtuality-qt4-git' 'virtuality-qt5-git')
pkgver=r25.9b6d7c7
pkgrel=1
pkgdesc='qt4 and qt5 widget style'
arch=('i686' 'x86_64')
url='https://github.com/luebking/virtuality'
license=('GPL2')
groups=('virtuality-git')
makedepends=('cmake' 'automoc4' 'qt4' 'qt5-svg' 'qt5-x11extras')
optdepends=('kdebase-workspace')
source=('git://github.com/luebking/virtuality.git')
md5sums=('SKIP')

pkgver() {
  cd virtuality
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}
build() {
  cd virtuality
  mkdir -p build4
  mkdir -p build5
  cd build4
  cmake .. -DCMAKE_BUILD_TYPE=Release  \
	       -DCMAKE_INSTALL_PREFIX=/usr \
		   -DWITH_QT5=OFF ..	 
  make

  cd ../build5
  cmake .. -DCMAKE_BUILD_TYPE=Release \
           -DCMAKE_INSTALL_PREFIX=/usr \
           -DWITH_QT5=ON ..
  make
}

package_virtuality-qt4-git() {
  depends=('qt4')
  cd virtuality/build4
  make DESTDIR="$pkgdir" install
  mv "${pkgdir}"/usr/bin/virtuality "${pkgdir}"/usr/bin/virtuality-qt4
}

package_virtuality-qt5-git() {
  depends=('qt5-svg' 'qt5-x11extras')
  cd virtuality/build5
  make DESTDIR="$pkgdir" install
}
