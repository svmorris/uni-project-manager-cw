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

function check_dir()
{
    ls $1 > /dev/null 2> /dev/null
    check $? $2
}

# check if subdirectories exist
function check_sub_dirs()
{
    sum=0
    ls test_project/src &>  /dev/null
    sum=$(($sum+$?))
    ls test_project/bin &>  /dev/null
    sum=$(($sum+$?))
    ls test_project/lib &>  /dev/null
    sum=$(($sum+$?))
    ls test_project/conf &>  /dev/null
    sum=$(($sum+$?))
    ls test_project/tests &>  /dev/null
    sum=$(($sum+$?))

    check $sum $1
}


rm -rf test_project


echo -n "Creating project... "
init_default test_project  0

echo -n "Checking if project is created twice... "
init_default test_project 17

echo -n "Checking if project crashes on invalid characters..."
init_default "*(';.[]')" 22

echo -n "Checking if project dir was created... "
check_dir test_project 0

echo -n "Checking if subdirectories were created..."
check_sub_dirs 0

echo -n "Checking if project was added to git..."
check_dir test_project/.git 0


