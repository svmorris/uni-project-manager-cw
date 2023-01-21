#!/bin/bash

function check()
{
    if [ "$1" == "$2" ]
    then
        echo -e "\n\t\t\t\t\t\t\t\t[✔] Success"
    else
        echo -e "\n\t\t\t\t\t\t\t\t[X] Failed!"
        exit -1
    fi
}

function init_default()
{
    ../bin/pm create_project $1 &> /dev/null
    check $? $2
}

function rename_feature()
{
    ../bin/pm rename_feature $1 $2 &> /dev/null
    check $? $3
}

rm -rf rename_test
rm -rf test_renamed

echo -n "Creating project... "
init_default rename_test  0

echo -n "Trying to rename directory..."
rename_feature rename_test test_renamed 0
