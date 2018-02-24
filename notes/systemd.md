
### cat /lib/systemd/system/pyxls-grep.service

    [Unit]
    Description=pyxls-grep
    Documentation=
    After=network.target

    [Service]
    Type=simple
    #LimitNOFILE=32768
    Environment="LANG=en_US.UTF-8"
    User=wood
    WorkingDirectory=/home/wood/www
    ExecStart=/home/wood/bin/pyxls-grep

    [Install]
    WantedBy=multi-user.target

### cat bin/pyxls-grep

    #!/bin/bash

    env > /tmp/1.log
    exec python3 /home/wood/bin/pyxls-grep.py # 2>/tmp/1.log

### https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux

New-Style Daemons
https://www.freedesktop.org/software/systemd/man/daemon.html#New-Style%20Daemons

https://github.com/jirihnidek/daemon

### https://www.freedesktop.org/wiki/Software/systemd/FrequentlyAskedQuestions/

### https://superuser.com/questions/278396/systemd-does-not-run-etc-rc-local

/etc/rc.local