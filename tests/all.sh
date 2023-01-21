#!/bin/bash

function printnew()
{
    echo -e "\n\nRunning $1..."
    printf '=%.0s' $(seq 1 $(tput cols))
    echo -e "\n"
}

function check()
{
    if [ "$1" == "$2" ]
    then
        echo -en "[✔] Success"
    else
        echo -en "[X] Some tests failed!"
        exit -1
    fi
}

chmod +x *


printnew project_init
./project_init.sh
check $? 0

printnew add_feature
./add_feature.sh
check $? 0

printnew rename_feature
./rename_feature.sh
check $? 0

printnew add_tag
./add_tag.sh
check $? 0

printnew find_tag
./find_tag.sh
check $? 0

