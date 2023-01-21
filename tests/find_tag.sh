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
    ../bin/pm create_project $1 > /dev/null
    check $? $2
}

function add_feature()
{
    cd find_tag_test
    ../../bin/pm add_feature "$1"  > /dev/null
    check $? $2
    cd ..
}
function add_tag()
{
    cd find_tag_test/
    ../../bin/pm add_tag "$1" &> /dev/null
    check $? $2
    cd ..
}


function add_tag_feature()
{
    cd find_tag_test/feature1
    ../../../bin/pm add_tag "$1" > /dev/null
    check $? $2
    cd ../..
}

function find_tag()
{
    cd find_tag_test/
    ../../bin/pm find_tag "$1" &> /dev/null
    check $? $2
    cd ..
}

rm -rf find_tag_test

echo -n "Creating project... "
init_default find_tag_test 0

echo -n "Adding feature... "
add_feature feature1 0

echo -n "Adding tag to project directory..."
add_tag test 0

echo -n "Adding tag to feature1... "
add_tag_feature r_test 0

echo -n "Checking if tag in working directory can be found..."
find_tag test 0

echo -n "Checking if recursive find tag works..."
find_tag r_test 0

echo -n "Checking error codes for tag not found..."
find_tag fake 2
