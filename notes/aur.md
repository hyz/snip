
    gpg --recv-keys --keyserver hkp://pgp.mit.edu 1D1F0DC78F173680
    gpg --keyserver pgp.mit.edu --recv-keys 85F345DD59683006

    gpg2 --recv-key 93BDB53CD4EBC740

    makepkg --skippgpcheck

### https://wiki.archlinux.org/index.php/AUR_helpers

https://github.com/rmarquis/pacaur

rslsync-2.4.2-1-x86_64.pkg.tar.xz
kcptun-git-v20161111.r1.g754b4a7-1-x86_64.pkg.tar.xz


### PKGBUILD

    source=("$pkgname"::'git+file:///xhome/windows.desktop/vcpkg/.git'      ...)

