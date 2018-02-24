
    :help cedit

    :set makeprg=/opt/bin/g++\ -std=c++11\ -I/opt/boost\ %\ ~/lib/libboost_system.a\ ~/lib/libboost_serialization.a

    :g/:10014..devtok/ .w >> user-10014.txt
    :g/"cmd":\(99\|211\|111\|112\),/ .w >>10014-99-211.txt
    :g/^\s*LOG.*\\n/s/\\n//

    $ vim +"echo &runtimepath" +q
    $ proxychains vim +PlugInstall +q

### http://vim.wikia.com/wiki/Power_of_g

### http://stackoverflow.com/questions/4789811/how-do-i-repeat-any-command-in-vim-like-c-x-z-in-emacs

    @:
    q:k
    :help repeating

    :normal A^I^I^[J
    :normal \t\t\Ctrl-v<Esc>

    :.!xargs rm -f

### ubuntu

    apt-get install vim-scripts

### http://askubuntu.com/questions/216818/how-do-i-use-plugins-in-the-vim-scripts-package

    vam ## vim-addon-manager
    vam install taglist

https://github.com/Valloric/YouCompleteMe

### https://github.com/VundleVim/Vundle.vim

    git clone https://github.com/VundleVim/Vundle.vim.git ~/.vim/bundle/Vundle.vim

### https://github.com/Valloric/YouCompleteMe/wiki/Building-Vim-from-source

    sudo checkinstall
    dpkg -r vim
    ...
    cd ~/.vim/bundle/YouCompleteMe ; ./install.py --clang-completer --system-libclang

https://aur.archlinux.org/packages/vim-youcompleteme-core-git/
http://howiefh.github.io/2015/05/22/vim-install-youcompleteme-plugin/?utm_source=tuicool&utm_medium=referral
http://www.cnblogs.com/linux-sir/p/4676647.html

### http://stackoverflow.com/questions/71323/how-to-replace-a-character-by-a-newline-in-vim?rq=1

-1
    echo bar > test
    vim test '+s/b/\n/' '+s/a/\r/' +wq
    vim a.txt '+set fileformat=unix' +wq

-2
    :set magic
    Control-v ...

### 

    set makeprg=g++\ -g\ -pthread\ -std=c++0x\ -I/BOOST_ROOT\ -I.\ %\ -lrt


### YCM/ycm
### https://github.com/Valloric/YouCompleteMe/issues/538

    ./install.sh --clang-completer --system-libclang

### golang

    https://github.com/fatih/vim-go  ~/.vim/bundle/vim-go
    mkdir -p ~/.vim/autoload ~/.vim/bundle && curl -LSso ~/.vim/autoload/pathogen.vim https://tpo.pe/pathogen.vim
    https://github.com/nsf/gocode

### http://coderoncode.com/tools/2017/04/16/vim-the-perfect-ide.html
