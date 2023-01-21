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


function add_feature()
{
    cd tag_test
    ../../bin/pm add_feature "$1"  &> /dev/null
    check $? $2
    cd ..
}

function add_tag()
{
    cd tag_test/feature1
    ../../../bin/pm add_tag "$1"
    check $? $2
    cd ../..
}

function check_file()
{
    cat tag_test/feature1/.pm_tag &> /dev/null
    check $? $1
}

function check_contents()
{
    c=$(cat tag_test/feature1/.pm_tag)
    check $c $1
}

rm -rf tag_test


echo -n "Creating project... "
init_default tag_test 0


echo -n "Adding feature... "
add_feature feature1 0


echo -n "Adding tag to feature1... "
add_tag f1 0


echo -n "Checking if tag was created... "
check_file 0

echo -n "Checking if tag has right contents... "
check_contents f1




