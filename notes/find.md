### find

    find jni -type f ! -path "*/Crypto/*" \( -name "*.h" -o -name "*.cpp" \)

    find * \( -name .repo -o -name .git -o -name out \) -prune -o -type f \( -name '*.h' \) -print >> h.files
    find * \( -name .repo -o -name .git -o -name out \) -prune -o -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \) -print >> cxx.files
    find * \( -path "*/protobuf" -o -path "build" \) -prune -o -print

### http://stackoverflow.com/questions/4210042/exclude-directory-from-find-command

    find -name "*.js" -not -path "./directory/*"

### http://stackoverflow.com/questions/762348/how-can-i-exclude-all-permission-denied-messages-from-find?noredirect=1&lq=1

    find . ! -readable -prune -o -print

    sudo find out/ ! -type l -user 1000 -exec chown build '{}' \;
    sudo find mt6580 -type l -user 1000 -exec chown -h build: '{}' \;

    sudo find . ! -user 1007 -exec chown -h build: '{}' \;

### find src -type f -perm \/111
